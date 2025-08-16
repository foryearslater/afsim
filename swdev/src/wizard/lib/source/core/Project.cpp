// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "Project.hpp"

#include <fstream>
#include <limits>

#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTextBrowser>

#include "ActionManager.hpp"
#include "AttributeSet.hpp"
#include "BackupPrefObject.hpp"
#include "ChangeHistory.hpp"
#include "Editor.hpp"
#include "EditorManager.hpp"
#include "Environment.hpp"
#include "ExportFiles.hpp"
#include "ParseResults.hpp"
#include "ParseWorker.hpp"
#include "ProjectComponent.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyMerge.hpp"
#include "ProxyWatcher.hpp"
#include "RevisionStore.hpp"
#include "RunEnvManager.hpp"
#include "RunManager.hpp"
#include "Signals.hpp"
#include "StartupFilesDialog.hpp"
#include "TextSource.hpp"
#include "TextSourceCache.hpp"
#include "TextSourceView.hpp"
#include "Undo.hpp"
#include "UtAlgorithm.hpp"
#include "UtCalendar.hpp"
#include "UtPath.hpp"
#include "UtQt.hpp"
#include "UtSleep.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfExe.hpp"
#include "WsfExeManager.hpp"
#include "WsfExecution.hpp"
#include "WsfPM_ObjectMap.hpp"
#include "WsfPProxyVisitor.hpp"
#include "WsfParseIndex.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseRule.hpp"
#include "WsfParser.hpp"
#include "XML_Context.hpp"

namespace wizard
{
struct AddDirToProject
{
   UtPath   mDir;
   Project* mProjectPtr;
   void     operator()() { mProjectPtr->AddProjectDirectory(mDir); }
};
} // namespace wizard

// Setting PROJECT_DEBUG_MODE to 1 will disable threading
#define PROJECT_DEBUG_MODE 0
UtCallbackListN<void(wizard::Project*)> wizard::Project::ProjectClosing;
UtCallbackListN<void(wizard::Project*)> wizard::Project::RegisterComponents;
wizard::Project*                        gProjectPtr = nullptr;

wizard::Project::Project(wizard::ProjectWorkspace* aWorkspacePtr)
   : mWorkspacePtr(aWorkspacePtr)
   , mQuitting(false)
   , mRevisionStore(new RevisionStore())
   , mParseWorker(nullptr)
   , mParsingAllComplete(false)
{
   assert(gProjectPtr == nullptr);
   gProjectPtr = this;

   mProxyAttributesRoot = ut::make_unique<AttributeSet>("proxy-settings");
   mCmdLineArgs         = GetDefaultCommandLineArgs();

   SetWorkingDirectory(mProjectDirectory);
   mProjectDirectories.push_back(mProjectDirectory);
   InitParseResults();

   // mCachePtr = new TextSourceCache(this, mWorkspacePtr->GetCacheWorkspace());
   mSourceProviderPtr = new CacheSourceProvider(&GetSourceCache());

   mParseWorker = new ParseWorker(this);

   mUtCallbacks += WsfExe::ExeUpdated.Connect(&Project::ExeUpdated, this);

   RegisterComponents(this);

   mWorkspacePtr->ProjectAdded(this);
   mTerrainMonitor = ut::make_unique<TerrainMonitor>();
}

wizard::Project* wizard::Project::Instance()
{
   return gProjectPtr;
}

QString wizard::Project::Name() const
{
   std::string name = mProjectFilePath.GetFileName(false);
   if (name.empty())
   {
      return "Project";
   }
   return name.c_str();
}

wizard::Project::~Project()
{
   wizSignals->handleProjectClose();

   wizSignals->ProxyInvalidate();

   // RemoveAllAbstractItems(true);
   delete mParseResultsPrivatePtr;
   delete mParseResultsTinyPtr;
   delete mParseWorker;
   delete mSourceProviderPtr;
   // delete mCachePtr;
   gProjectPtr = nullptr;
   delete mRevisionStore;
   UtReferenceTracked::FreeReference();
}

//! Called when a project item has been reloaded.
//! This method should return 'true' when the source needs to be parsed
bool wizard::Project::SourceReloaded(TextSource* aSourcePtr)
{
   ParseResultsTiny* results = TinyParseResults();
   if (results)
   {
      return results->IncludesFile(aSourcePtr);
   }
   return false;
}

//! Waits until the parsing thread is complete.  This should only be called from the main thread.
// public
void wizard::Project::WaitForParsing()
{
   mWorkspacePtr->WaitForParsing();
}

void wizard::Project::GotoRange(const UtTextDocumentRange& aRange)
{
   TextSource* sourcePtr = static_cast<TextSource*>(aRange.mSource);
   if (sourcePtr)
   {
      TextSourceView* view = sourcePtr->GetView(this);
      if (view)
      {
         // view->mEditorPtr->SetSelection(aRange);
         view->mEditorPtr->GoToRange(aRange);
         mWorkspacePtr->GetEditorManager()->SetCurrent(view->mEditorPtr);
         mWorkspacePtr->GetEditorManager()->RecordCurrentLocation();
         view->mEditorPtr->activateWindow();
      }
   }
}

//! Open the file and make it active.
//! @param aSourcePtr         The source to open / activate.
//! @param aRecordLocation    If true, records the location for forward/backward navigation.
wizard::Editor* wizard::Project::GotoFile(UtTextDocument* aSourcePtr, bool aRecordLocation)
{
   return mWorkspacePtr->GotoFile(aSourcePtr, aRecordLocation);
}

wizard::Editor* wizard::Project::GotoFile(const std::string& aFilePath, bool aRecordLocation)
{
   return mWorkspacePtr->GotoFile(aFilePath, aRecordLocation);
}

bool wizard::Project::CloseProject(bool aForce)
{
   if (!RequestClose(aForce))
   {
      return false;
   }
   emit Closed();
   if (GetParseResults() && GetParseResults()->Proxy() && !mWorkspacePtr->IsProcessing())
   {
      mRevisionStore->Backup();
      mRevisionStore->Close();
   }
   ProjectClosing(this);
   mWorkspacePtr->RemoveProject(this);
   return true;
}

WsfPProxy* wizard::Project::Proxy() const
{
   return mParseResultsPtr ? mParseResultsPtr->Proxy() : nullptr;
}

bool wizard::Project::RequestClose(bool aForce)
{
   if (mQuitting)
   {
      return true;
   }
   std::list<std::string>            modList;
   bool                              allowClose        = true;
   int                               modificationCount = 0;
   const TextSourceCache::SourceMap& sources           = GetSourceCache().GetSources();
   for (auto&& source : sources)
   {
      if (source.second->IsModified())
      {
         ++modificationCount;
         modList.push_back(source.first);
      }
   }
   if (modificationCount > 0 && !aForce)
   {
      std::stringstream ss;
      ss << "Are you sure you want to close the project? The modifications to the following " << modificationCount
         << " files will be lost:<ul>";
      std::list<std::string>::iterator it;
      int                              t = 0;
      for (it = modList.begin(); it != modList.end(); ++it)
      {
         if (t < 10)
         {
            ss << "<li><b>" << *it << "</b></li>";
         }
         ++t;
      }
      if (t > 10)
      {
         ss << "<li><b>...</b></li>";
      }
      ss << "</ul>";
      QMessageBox warn(QMessageBox::Warning, "Abandon Changes?", ss.str().c_str(), QMessageBox::Ok | QMessageBox::Cancel);
      warn.setTextFormat(Qt::RichText);
      const int bn = warn.exec();
      allowClose   = (bn == QMessageBox::Ok);
      if (allowClose)
      {
         // set flag to avoid asking more than once
         mQuitting = true;
      }
   }
   if (allowClose && !aForce)
   {
      if (mProjectFilePath.GetNormalizedPath().empty())
      {
         if (allowClose)
         {
            // set flag to avoid asking more than once
            mQuitting = true;
         }
      }
      else
      {
         // Determine if the scenario has changed by diffing the output XML
         if (mLoadedXML != GetXmlText())
         {
            int choice =
               QMessageBox::warning(wkfEnv.GetMainWindow(),
                                    "Save Project?",
                                    QString("Project <b>%1</b> has unsaved changes. Would you like to save?").arg(Name()),
                                    "Save",
                                    "Discard",
                                    "Cancel",
                                    0,
                                    2);
            if (choice == 0)
            {
               SaveProject();
            }
            else if (choice == 2)
            {
               mQuitting = allowClose = false;
            }
         }
      }
   }
   return allowClose;
}

void wizard::Project::SaveAll(bool aSaveProject)
{
   mWorkspacePtr->WaitForAbortParsing();

   // Save files if any have been modified
   const TextSourceCache::SourceMap& sources = GetSourceCache().GetSources();
   for (auto&& source : sources)
   {
      if (source.second->IsModified())
      {
         source.second->SaveWithNotifications();
      }
   }
   if (aSaveProject)
   {
      // If a project file has been opened
      // A file could be opened in the editor without a project file loaded
      // For example, The file could have been just dropped on the Wizard
      if (!mProjectFilePath.GetNormalizedPath().empty())
      {
         SaveProject();
      }
      else
      {
         mRevisionStore->RequestBackup();
      }
   }
}

void wizard::Project::TriggerReparse()
{
   InvalidateScenario(false);
}

void wizard::Project::InvalidateScenario(bool aInvalidateProxy)
{
   if (aInvalidateProxy)
   {
      wizSignals->ProxyInvalidate();
   }
   TakeParseResults();
   mWorkspacePtr->TouchParse();
}

void wizard::Project::FullUpdate()
{
   CleanUpFileList();
   emit ParseUpdatedFull();
}

void wizard::Project::DeferUpdate(bool aFullyReparsed)
{
   ParseUpdatedDeferred(aFullyReparsed);
   wizSignals->ParseUpdatedDeferred(this, aFullyReparsed);
}

void wizard::Project::CleanUpFileList()
{
   QSet<TextSource*> unusedSources;

   const TextSourceCache::SourceMap& allSources = GetSourceCache().GetSources();
   for (auto&& source : allSources)
   {
      // Unset the included flag on every file
      source.second->SetFlags((source.second->GetFlags() & ~TextSource::cINCLUDED));
   }

   ParseResults&                                            results  = *GetParseResults();
   const std::map<UtTextDocument*, WsfParseSourceInclude*>& includes = results.mFirstIncludes;
   for (auto&& include : includes)
   {
      TextSource* sourcePtr = GetSourceCache().FindSource(include.first);
      if (sourcePtr)
      {
         sourcePtr->SetFlags((sourcePtr->GetFlags() | TextSource::cINCLUDED));
      }
   }

   for (auto&& source : allSources)
   {
      // Unset the included flag on every file
      if (!source.second->IsCritical())
      {
         if (source.second->GetViews().empty())
         {
            unusedSources.insert(source.second);
         }
      }
   }

   emit wizSignals->SourceCleanupEligibility(unusedSources);
   for (auto&& source : unusedSources)
   {
      if ((!source->IsCritical()) && (source->IsLoaded()))
      {
         GetSourceCache().DeleteSource(source);
      }
   }
   //    const int cLARGE_FILE_SIZE = 500000;
   //    // Unload all non-critical files larger than cLARGE_FILE_SIZE
   //    for (TextSourceCache::SourceMap::const_iterator i = allSources.begin(); i != allSources.end(); ++i)
   //    {
   //       if (! i->second->IsCritical() && i->second->IsLoaded())
   //       {
   //          if (i->second->GetText().Size() > cLARGE_FILE_SIZE)
   //          {
   //             i->second->Unload();
   //          }
   //       }
   //    }
}

void wizard::Project::FindIncludes(TextSource* aSourcePtr)
{
   std::vector<UtTextDocumentRange> includes;
   ParseResults*                    resultsPtr = WaitForParseResults();
   if (resultsPtr)
   {
      resultsPtr->FindIncludes(aSourcePtr, includes);
      if (!includes.empty())
      {
         QMenu menu;
         for (auto&& include : includes)
         {
            menu.addAction(GoToLocationAction(include));
         }
         menu.setTitle("Include Locations");
         menu.exec(QCursor::pos());
      }
   }
}

struct GoToLocationFunctor
{
   wizard::Project*    mProjectPtr;
   UtTextDocumentRange mLocation;
   void                operator()() { mProjectPtr->GotoRange(mLocation); }
};

QAction* wizard::Project::GoToLocationAction(const UtTextDocumentRange& aRange)
{
   TextSource* srcPtr = GetSourceCache().FindSource(aRange.mSource);
   if (srcPtr)
   {
      size_t              line      = aRange.mSource->PositionToLineNumber(aRange.GetBegin());
      std::string         path      = mProjectDirectory.GetRelativePath(srcPtr->GetFilePath()).GetSystemPath();
      QString             txt       = QString::fromStdString(path) + " : " + QString::number(line);
      QAction*            newAction = new QAction(txt, nullptr);
      GoToLocationFunctor fn;
      fn.mLocation   = aRange;
      fn.mProjectPtr = this;
      QObject::connect(newAction, &QAction::triggered, fn);
      return newAction;
   }
   return nullptr;
}

void wizard::Project::ToXML(XML_Context& aContext) const
{
   UtQtXmlWriter& writer = aContext.GetWriter();
   writer.WriteStartElement("wsf-ide-project");
   {
      UtPath filePath = aContext.GetFileName();
      filePath.Up();
      UtPath relWorkingDir = filePath.GetRelativePath(mProjectDirectory);
      writer.WriteAttribute("project-directory", relWorkingDir.GetNormalizedPath());
      for (size_t i = 1; i < mProjectDirectories.size(); ++i)
      {
         writer.WriteStartElement("additional-directory");
         {
            writer.WriteAttribute("path", mProjectDirectory.GetRelativePath(mProjectDirectories[i]).GetSystemPath());
         }
         writer.WriteEndElement();
      }

      writer.WriteStartElement("wsf-ide-scenario");
      {
         writer.WriteAttribute("command-line-args", mCmdLineArgs.toStdString());
         UtPath workingDirRelPath = ProjectDirectory().GetRelativePath(mWorkingDirectory);
         writer.WriteAttribute("working-directory", workingDirRelPath.GetNormalizedPath());

         for (auto&& startupFile : mStartupFiles)
         {
            writer.WriteStartElement("file-item");
            {
               writer.WriteAttribute("file-path", mProjectDirectory.GetRelativePath(startupFile).GetNormalizedPath());
               writer.WriteAttribute("file-type", "file-main-source");
            }
            writer.WriteEndElement();
         }

         WsfExe* exePtr = GetExe();
         if (exePtr)
         {
            exePtr->ExeTypeToXML(aContext);
         }
      }
      writer.WriteEndElement();

      writer.WriteStartElement("proxy-settings");
      mProxyAttributesRoot->WriteXML("root", writer);
      writer.WriteEndElement();

      if (!mMapItemsXML.empty())
      {
         // writer.WriteStartElement("map-items");
         //{
         UtQtXmlReader reader;
         reader.SetInput(mMapItemsXML);
         reader.NextElementToken();
         writer.WriteElementFromReader(reader);
         //}
         // writer.WriteEndElement();
      }
      for (auto&& component : mComponents)
      {
         component.second->WriteXML(writer);
      }
   }
   writer.WriteEndElement();
}

bool wizard::Project::FromXML_StartupFile(XML_Context& aContext)
{
   if (aContext.GetReader().GetName() == "file-item")
   {
      if (aContext.GetReader().HasAttribute("file-type") && aContext.GetReader().HasAttribute("file-path"))
      {
         UtQtXml::UtQtXmlAttributeList attributes = aContext.GetReader().GetAttributes();
         bool                          isMain     = false;
         std::string                   path;
         for (auto&& attr : attributes)
         {
            if (attr.mName == "file-type" && attr.mValue == "file-main-source")
            {
               isMain = true;
            }
            else if (attr.mName == "file-path")
            {
               path = attr.mValue;
            }
         }
         if (isMain)
         {
            mStartupFiles.push_back(mProjectDirectory + path);
         }
      }
      return true;
   }
   else
   {
      return false;
   }
}

bool wizard::Project::FromXML(XML_Context& aContext)
{
   UtQtXmlReader&       xmlReader = aContext.GetReader();
   UtQtXmlElementReader rootElement(xmlReader);
   if (rootElement.GetName() != "wsf-ide-project-file")
   {
      return false;
   }
   for (;;)
   {
      UtQtXmlElementReader topElement(rootElement);
      if (!topElement)
      {
         break;
      }
      if (topElement.GetName() == "wsf-ide-project")
      {
         std::string projDir;
         if (topElement.HasAttribute("project-directory"))
         {
            UtQtXml::UtQtXmlAttributeList attributes = topElement.GetAttributes();
            for (auto&& attr : attributes)
            {
               if (attr.mName == "project-directory")
               {
                  projDir = attr.mValue;
               }
            }
            UtPath root = aContext.GetFileName();
            root.Up();
            SetProjectLocation(root + projDir, aContext.GetFileName());
            for (;;)
            {
               UtQtXmlElementReader element(topElement);
               if (!element)
               {
                  break;
               }
               if (FromXML_StartupFile(aContext))
               {
               }
               else if (element.GetName() == "wsf-ide-scenario")
               {
                  if (element.HasAttribute("command-line-args"))
                  {
                     mCmdLineArgs = element.Attribute("command-line-args").c_str();
                  }
                  else
                  {
                     mCmdLineArgs = Project::GetDefaultCommandLineArgs();
                  }

                  std::string workingDir = element.Attribute("working-directory");
                  if (!workingDir.empty())
                  {
                     SetWorkingDirectory(mProjectDirectory + workingDir);
                  }

                  for (;;)
                  {
                     UtQtXmlElementReader scenarioElement(element);
                     if (!scenarioElement)
                     {
                        break;
                     }
                     if (scenarioElement.GetName() == "wsf-exe-info")
                     {
                        SetExe(wizExeMgr.FindFromXML(aContext), true);
                     }
                     else if (FromXML_StartupFile(aContext))
                     {
                     }
                  }
               }
               else if (element.GetName() == "additional-directory")
               {
                  if (element.HasAttribute("path"))
                  {
                     std::string pathStr = element.Attribute("path").c_str();
                     mProjectDirectories.push_back(mProjectDirectory + UtPath(pathStr));
                  }
               }
               else if (element.GetName() == "proxy-settings")
               {
                  auto newAttrSet = ut::make_unique<AttributeSet>("proxy-settings");
                  if (newAttrSet->ReadSingleXML(element))
                  {
                     mProxyAttributesRoot = std::move(newAttrSet);
                  }
               }
               else if (element.GetName() == "project-item")
               {
                  UtQtXmlWriter stringWriter;
                  stringWriter.WriteElementFromReader(xmlReader);
                  element.SetAtEnd();
                  mMapItemsXML = stringWriter.ToString();
               }
               else
               {
                  bool processed = false;
                  for (ComponentMap::iterator i = mComponents.begin(); i != mComponents.end() && !processed; ++i)
                  {
                     processed = i->second->ReadXML(element);
                  }
               }
            }

            mProjectFilePath = aContext.GetFileName();
            QFile xmlFile(mProjectFilePath.GetSystemPath().c_str());
            xmlFile.open(QFile::ReadOnly);
            QByteArray fileData = xmlFile.readAll();
            mLoadedXML.assign(fileData.begin(), fileData.end());

            emit wizSignals->ProjectDirectoriesChanged(mProjectDirectories);

            std::vector<UtPath> mainFiles = GetStartupFiles();
            if (!mainFiles.empty())
            {
               GotoFile(mainFiles[0].GetSystemPath(), true);
            }

            emit wizSignals->ProjectStartupFilesChanged(mStartupFiles);

            return true;
         }
      }
   }
   return false;
}

bool wizard::Project::SaveAs()
{
   const QString& fileName = QFileDialog::getSaveFileName(QApplication::activeWindow(),
                                                          "Save Project",
                                                          QString::fromStdString(mProjectDirectory.GetSystemPath()),
                                                          "Wizard Project (*.afproj)");
   if (!fileName.isEmpty())
   {
      UtPath path(fileName.toStdString());
      if (!path.GetNormalizedPath().empty())
      {
         // Enforce the .afproj extension
         if (path.GetFileExtension() != ".afproj")
         {
            UtPath newPath = path;
            newPath.Up();
            newPath += path.GetFileName(false) + ".afproj";
            path = newPath;
         }
         return SetProjectFilePath(path) && SaveProject();
      }
   }
   return false;
}

bool wizard::Project::SaveProject()
{
   if (mProjectFilePath.GetNormalizedPath().empty())
   {
      return SaveAs();
   }
   else
   {
      SaveAll(false);
      std::ofstream outfile(mProjectFilePath.GetSystemPath(), std::ios::binary);
      mLoadedXML = GetXmlText();
      outfile << mLoadedXML;
      return true;
   }
   mRevisionStore->RequestBackup();
   return false;
}

void wizard::Project::OpenAction()
{
   QStringList selectedFiles =
      QFileDialog::getOpenFileNames(nullptr, "Open file(s)", mProjectDirectory.GetSystemPath().c_str(), "*.*");
   for (int i = 0; i < selectedFiles.size(); ++i)
   {
      UtPath      filePath(selectedFiles[i].toStdString());
      std::string fileSysPath = filePath.GetSystemPath();
      GotoFile(fileSysPath, true);
      //       bool canAdd = true;
      //       for (int i = 0; i < GetAbstractItemCount(); ++i)
      //       {
      //          ViFileItem* itemPtr = ViWorkspaceItem::Cast<ViFileItem>((ViWorkspaceItem*)GetAbstractItem(i));
      //          if (itemPtr)
      //          {
      //             if (itemPtr->GetFilePath() == fileSysPath)
      //             {
      //                canAdd = false;
      //             }
      //          }
      //       }
      //       if (canAdd)
      //       {
      //          ViFileItem* itemPtr = new ViFileItem(filePath.GetSystemPath());
      //          AddAbstractItem(itemPtr);
      //          itemPtr->SetCustomFile(this);
      //       }
      //    }
   }
}

void wizard::Project::BuildUpContextMenu(QMenu& aMenu)
{
   if (mWorkspacePtr->GetActionManager())
   {
      mWorkspacePtr->GetActionManager()->BuildUpProjectContextMenu(aMenu);
   }
   // Scan include paths. If any paths are not yet added to project, offer to add them
   ParseResultsTiny* results = TinyParseResults();
   if (results)
   {
      bool addSep = true;
      for (auto&& it : results->mIncludePaths)
      {
         UtPath path = WorkingDirectory() + it.Get();
         if (!IsDirectoryInProject(path))
         {
            if (addSep)
            {
               aMenu.addSeparator();
               addSep = false;
            }
            QString actionStr =
               QString("Add Directory \"%1\" to Project").arg(QString::fromStdString(path.GetSystemPath()));
            wizard::AddDirToProject addDirFunc;
            addDirFunc.mDir        = path;
            addDirFunc.mProjectPtr = this;
            QObject::connect(aMenu.addAction(actionStr), &QAction::triggered, addDirFunc);
         }
      }
   }
}

void wizard::Project::UpdateProxyEpoch(const wizard::ProxyChange& aProxyChange)
{
   UpdateProxyEpoch();
}

void wizard::Project::UpdateProxyEpoch()
{
   WsfPProxyNode root = GetParseResults()->ProxyRootNode();
   if (root.IsValid())
   {
      WsfPProxyNode sim = root + "scenario" + "simulationInput";
      if (sim.IsValid())
      {
         if (!sim.GetValue().GetAttr("startEpoch").IsUnset())
         {
            WsfProxy::Double* dPtr = sim.GetValue().GetAttr("startEpoch");
            if (dPtr)
            {
               mEpoch.SetEpoch(dPtr->GetValue());
            }
         }
         if (sim.GetValue().GetAttr("startTimeNow").GetBoolValue())
         {
            mEpoch.SetCurrentDateAndTime();
         }
         if ((!sim.GetValue().GetAttr("startMonth").IsUnset()) && (!sim.GetValue().GetAttr("startDay").IsUnset()) &&
             (!sim.GetValue().GetAttr("startYear").IsUnset()))
         {
            std::string mo    = sim.GetValue().GetAttr("startMonth").ValueToString();
            int         month = UtCalendar::GetMonth(mo);
            int         day   = 1;
            sim.GetValue().GetAttr("startDay").GetIntegerValue(day);
            int year = 2003;
            sim.GetValue().GetAttr("startYear").GetIntegerValue(year);
            mEpoch.SetDate(year, month, day);
         }
         if (!sim.GetValue().GetAttr("startTime").IsUnset())
         {
            std::string word = sim.GetValue().GetAttr("startTime").ValueToString();
            int         hour;
            int         minute;
            double      second;
            if (UtCalendar::GetTime(word, hour, minute, second))
            {
               mEpoch.SetTime(hour, minute, second);
            }
         }
      }
   }
   emit wizSignals->EpochUpdated(mEpoch);
}

void wizard::Project::ExportProjectAction()
{
   ExportFiles exportFiles(mProjectDirectory, wkfEnv.GetMainWindow());
   exportFiles.ExportProject(WaitForParseResults()->mFirstIncludes);
}


long long wizard::Project::GetMinimumSequenceNumber()
{
   long long         minSequenceNumber = std::numeric_limits<long long>::max();
   ParseResultsTiny* prt               = TinyParseResults();
   if (prt && minSequenceNumber > prt->mSequenceNumber)
   {
      minSequenceNumber = prt->mSequenceNumber;
   }
   if (minSequenceNumber == std::numeric_limits<long long>::max())
   {
      minSequenceNumber = 0;
   }
   return minSequenceNumber;
}

const UtPath& wizard::Project::WorkingDirectory() const
{
   return mWorkingDirectory;
}

void wizard::Project::P_CompleteGrammarParsing() noexcept
{
   mLastGrammarParseTime   = mParseTimer.GetClock();
   mGrammarParsingComplete = true;
}

void wizard::Project::P_GrammarParsingStarted() noexcept
{
   mParseTimer.ResetClock();
   mGrammarParsingComplete = false;
   mParsingAllComplete     = false;
}

void wizard::Project::P_CompleteInputParsing() noexcept
{
   mLastInputParseTime   = mParseTimer.GetClock();
   mInputParsingComplete = true;
}

void wizard::Project::P_NotifyParseUpdated() noexcept
{
   ParseUpdated(true);
   if (!OneTimeParseComplete.IsEmpty() && ParseIsUpToDate())
   {
      OneTimeParseComplete();
      OneTimeParseComplete.DisconnectAll();
   }
}

void wizard::Project::P_InputParsingStarted() noexcept
{
   mParseTimer.ResetClock();
   mInputParsingComplete = false;
   mParsingAllComplete   = false;
}

void wizard::Project::P_CompleteProxyDeserialize() noexcept
{
   mLastProxyDeserializeTime = mParseTimer.GetClock();
   mProxyDeserializeComplete = true;
}

void wizard::Project::P_ProxyDeserializeStarted() noexcept
{
   mParseTimer.ResetClock();
   mProxyDeserializeComplete = false;
   mParsingAllComplete       = false;
}

void wizard::Project::P_CompleteProxyMergeGUI() noexcept
{
   mLastProxyMergeGUI_Time = mParseTimer.GetClock();
   mProxyMergeGUI_Complete = true;
}

void wizard::Project::P_ProxyMergeGUI_Started() noexcept
{
   mParseTimer.ResetClock();
   mProxyMergeGUI_Complete = false;
   mParsingAllComplete     = false;
}

void wizard::Project::P_ParsingAborted() noexcept
{
   mParsingAllComplete = true;
}

void wizard::Project::P_CompleteAllParsing() noexcept
{
   mParsingAllComplete = true;
}

wizard::Project::ParseCompleteCallback* wizard::Project::ExecuteWhenParseComplete(ParseCompleteCallback* aCallbackPtr)
{
   if (ParseIsUpToDate())
   {
      (*aCallbackPtr)();
   }
   else
   {
      OneTimeParseComplete.Connect(aCallbackPtr);
   }
   return aCallbackPtr;
}

void wizard::Project::InitParseResults()
{
   mParseResultsPrivatePtr = new ParseResults(this);
   mCurrentTinyResultsPtr = mParseResultsTinyPtr = new ParseResultsTiny(this);
   mParseResultsPtr                              = nullptr;
}

void wizard::Project::SetStartupFiles(const std::vector<UtPath>& aMainFiles)
{
   if (mStartupFiles != aMainFiles)
   {
      mStartupFiles = aMainFiles;
      InvalidateScenario();
      emit wizSignals->ProjectStartupFilesChanged(mStartupFiles);
   }
}

const std::vector<UtPath>& wizard::Project::GetStartupFiles()
{
   return mStartupFiles;
}

void wizard::Project::ShowSettings()
{
   wizEnv.ProjectSettingsOpen(*this);
}

void wizard::Project::SetWorkingDirectory(const UtPath& aPath)
{
   if (aPath.Stat() == UtPath::cDIRECTORY)
   {
      mWorkingDirectory = aPath;
      mRevisionStore->SetProjectWorkingDir(aPath);
      InvalidateScenario();
   }
}

wizard::WsfExe* wizard::Project::GetExe() const
{
   WsfExe* exe = wizExeMgr.FindByPath(mExePath);

   return exe ? exe : wizExeMgr.GetNoExe();
}

// aEmit is set when the change indicates an actual preference (not for default state)
void wizard::Project::SetExe(WsfExe* aExePtr, bool aEmit)
{
   if (!aExePtr)
   {
      mExePath.clear();
      aExePtr = wizExeMgr.GetNoExe();
   }
   else
   {
      mExePath = aExePtr->GetPath().GetSystemPath();
   }
   if (aEmit)
   {
      wizSignals->WsfExeChanged(aExePtr);
   }
   InvalidateScenario(true);
}

void wizard::Project::SetCommandLineArgs(const QString& aArgs)
{
   mCmdLineArgs = aArgs;
}

QString wizard::Project::GetDefaultCommandLineArgs()
{
   return "$(SCENARIO_FILES)";
}

void wizard::Project::ExeUpdated(WsfExe* aExePtr)
{
   // If an .exe has changed, wipe out existing parse results
   if (GetExe() == aExePtr)
   {
      InvalidateScenario();
      delete mParseResultsPrivatePtr;
      delete mParseResultsTinyPtr;
      InitParseResults();
   }
}

wizard::ParseResults* wizard::Project::WaitForParseResults() const
{
   if (mParseResultsPtr)
   {
      return mParseResultsPtr;
   }
   // Process application events until the parse results are ready.
   // The results should be computed as long as the scenario is in a workspace.
   // Future: we may need to add a time-out.
   if (GetWorkspace())
   {
      for (; !mParseResultsPtr;)
      {
         QApplication::processEvents();
         UtSleep::Sleep(.005);
      }
   }
   return mParseResultsPtr;
}

void wizard::Project::WaitForAllParsing()
{
   WaitForParseResults();
   if (mParsingAllComplete)
   {
      return;
   }
   while (true)
   {
      // Process application events until the parse results are ready.
      // The results should be computed as long as the scenario is in a workspace.
      // Future: we may need to add a time-out.
      if (GetWorkspace())
      {
         for (; !mParsingAllComplete;)
         {
            QApplication::processEvents();
            UtSleep::Sleep(.005);
         }
      }

      if (mParsingAllComplete)
      {
         break;
      }
   }
}

wizard::ParseResults* wizard::Project::TakeParseResults()
{
   if (mParseResultsPtr != nullptr)
   {
      emit wizSignals->ParseResultsInvalidate(this);
      mParseResultsTinyPtr->TakeResults(*mParseResultsPtr);
      mParseResultsPtr       = nullptr;
      mCurrentTinyResultsPtr = mParseResultsTinyPtr;
   }
   return mParseResultsPrivatePtr;
}

bool wizard::Project::IncludesFile(TextSource* aSourcePtr)
{
   return mCurrentTinyResultsPtr->IncludesFile(aSourcePtr);
}

bool wizard::Project::IncludesStartupFile(TextSource* aSourcePtr)
{
   const UtPath& p = aSourcePtr->GetFilePath();
   return (std::find(mStartupFiles.begin(), mStartupFiles.end(), p) != mStartupFiles.end());
}

void wizard::Project::GiveParseResults()
{
   mParseResultsPtr = mParseResultsPrivatePtr;
   if (mCurrentTinyResultsPtr == mParseResultsTinyPtr)
   {
      // If we have proxy data in our tiny parse results, move those over to the new parse results.
      // This is the old proxy data, and will be replaced if/when the proxy is completed.
      mParseResultsTinyPtr->SwapProxy(*mParseResultsPtr, true);
   }
   mCurrentTinyResultsPtr = mParseResultsPtr;
}

namespace
{
void proxyModifiedCallback(WsfPProxy*                     aProxyPtr,
                           WsfPProxy::ProxyModifiedReason aProxyModifiedReason,
                           const WsfPProxyPath&           aPath,
                           const std::string*             aRenameName)
{
   aProxyPtr->mHasModifications = true;

   switch (aProxyModifiedReason)
   {
   case WsfPProxy::cPM_CHILD_ADDED:
   {
      WsfPProxyNode addedNode(aProxyPtr, aPath);

      wizSignals->editProxy();
      aProxyPtr->PropagateNodeAddition(addedNode);
      wizard::ProxyWatcher::Instance()->HandleProxyAdded(addedNode);
      break;
   }

   case WsfPProxy::cPM_CHILD_REMOVED:
   {
      WsfPProxyNode removedNode(aProxyPtr, aPath);

      wizSignals->editProxy();
      aProxyPtr->PropagateNodeDeletion(removedNode);
      wizard::ProxyWatcher::Instance()->HandleProxyRemoved(removedNode);
      break;
   }

   case WsfPProxy::cPM_CHILD_MOVED:
   {
      WsfPProxyPath newPath = aPath;
      WsfPProxyKey& key     = newPath.Back();

      if (key.IsString())
      {
         key = *aRenameName;
      }

      WsfPProxyNode oldNode(aProxyPtr, aPath);
      WsfPProxyNode newNode(aProxyPtr, newPath);

      wizSignals->editProxy();
      wizard::Project::Instance()->HandleProxyNodeMoved(oldNode, newNode);
      wizard::ProxyWatcher::Instance()->HandleProxyMoved(oldNode, newNode);
      break;
   }

   case WsfPProxy::cPM_VALUE_CHANGED:
   {
      WsfPProxyNode modifiedNode(aProxyPtr, aPath);

      wizSignals->editProxy();
      aProxyPtr->PropagateNodeChange(modifiedNode);
      wizard::ProxyWatcher::Instance()->HandleProxyModified(modifiedNode);
      break;
   }

   default:
      // We shouldn't reach here.
      assert(false);
      break;
   }
}

void proxyBeforeModifiedCallback(WsfPProxy*                     aProxyPtr,
                                 WsfPProxy::ProxyModifiedReason aProxyModifiedReason,
                                 const WsfPProxyPath&           aPath,
                                 const std::string*             aRenameName)
{
   wizard::Undo::Instance()->BeforeProxyChange(aProxyPtr, aProxyModifiedReason, aPath, aRenameName);
}
} // namespace

void wizard::Project::ProxyReady(std::unique_ptr<WsfPProxy> aProxy, std::unique_ptr<ProxyHash> aProxyHashPtr)
{
   Undo::Instance()->ClearProxyChanges();

   aProxy->SetProxyModifiedCallback(&proxyModifiedCallback);
   aProxy->SetBeforeProxyModifiedCallback(&proxyBeforeModifiedCallback);

   // we have a new proxy value.  If a previous one exists that is compatible, do a merge operation
   // to determine what has changed
   if (mParseResultsPrivatePtr->GetProxyHash() &&
       mParseResultsPrivatePtr->ProxyRegistry()->mApplicationPointer == aProxy->GetRegistry()->mApplicationPointer)
   {
      // assert(mParseResultsPrivatePtr->GetProxyHash() != aProxyHashPtr.get());

      // Take ownership temporarily of old proxy data for merge
      ParseResultsProxyData oldProxy;
      oldProxy.GiveProxy(std::move(aProxy), std::move(aProxyHashPtr));
      oldProxy.SwapProxy(*mParseResultsPrivatePtr, false); // false because we want to keep the old proxy pointer

      // If there has been a change to the proxy via the GUI, we need to recompute the hash now.
      // if no change was made, we can just use the hash created earlier
      ProxyHash* oldHashPtr = nullptr;
      if (oldProxy.Proxy()->mHasModifications)
      {
         oldHashPtr = new ProxyHash(oldProxy.ProxyRoot());
      }
      else
      {
         oldHashPtr = oldProxy.GetProxyHash();
      }

      ProxyMerge merger(oldHashPtr,
                        mParseResultsPrivatePtr->GetProxyHash(),
                        oldProxy.Proxy(),
                        mParseResultsPrivatePtr->Proxy());

      P_ProxyMergeGUI_Started();
      emit GetWorkspace()->ProxyUpdated(&merger);
      P_CompleteProxyMergeGUI();

      if (oldHashPtr != oldProxy.GetProxyHash())
      {
         delete oldHashPtr;
      }
   }
   else
   {
      if (mParseResultsPrivatePtr->GetProxyHash())
      {
         // our existing proxy is incompatible with the new one.
      }
      // No old proxy data to merge
      mParseResultsPrivatePtr->GiveProxy(std::move(aProxy), std::move(aProxyHashPtr));
      emit GetWorkspace()->ProxyUpdated(nullptr);
   }

   if (!mEpochListenerSetup)
   {
      std::vector<std::string> list      = {"scenario", "simulationInput"};
      WsfPProxyPath            proxyPath = WsfPProxyPath::FromStringList(GetParseResults()->ProxyRoot(), list);
      mUtCallbacks.Add(wizard::WatchProxy(proxyPath).ChildUpdate.Connect(&Project::UpdateProxyEpoch, this));
      mEpochListenerSetup = true;
      UpdateProxyEpoch();
   }
}

void wizard::Project::ShowStartupFileDialog()
{
   StartupFilesDialog dlg(wkfEnv.GetMainWindow(), GetStartupFiles(), ProjectDirectory());
   dlg.exec();
   SetStartupFiles(dlg.startupFiles());
}

void wizard::Project::RemoveProjectDirectory(const UtPath& aPath)
{
   if (mProjectDirectory == aPath)
   {
      return;
   }
   std::vector<UtPath>::iterator i = std::find(mProjectDirectories.begin(), mProjectDirectories.end(), aPath);
   if (i != mProjectDirectories.end())
   {
      mProjectDirectories.erase(i);
      emit wizSignals->ProjectDirectoriesChanged(mProjectDirectories);
   }
}

bool wizard::Project::AddProjectDirectory(const UtPath& aPath)
{
   // Don't add directories that are subsets
   for (auto&& dir : mProjectDirectories)
   {
      if (dir.IsAncestorOf(aPath) || dir == aPath)
      {
         return false;
      }
   }
   // Would have to change project directory...
   if (aPath.IsAncestorOf(mProjectDirectory))
   {
      return false;
   }

   // Remove existing directories that are subsets
   for (size_t i = 1; i < mProjectDirectories.size();)
   {
      if (aPath.IsAncestorOf(mProjectDirectories[i]))
      {
         mProjectDirectories.erase(mProjectDirectories.begin() + i);
      }
      else
      {
         ++i;
      }
   }
   mProjectDirectories.push_back(aPath);

   emit wizSignals->ProjectDirectoriesChanged(mProjectDirectories);
   return true;
}

void wizard::Project::AddProjectDirectoryBrowse()
{
   QString dirPath = QFileDialog::getExistingDirectory(wkfEnv.GetMainWindow(),
                                                       "Add Directory to Project",
                                                       QString::fromStdString(mProjectDirectory.GetSystemPath()));
   if (!dirPath.isEmpty())
   {
      UtPath dirPathPath = dirPath.toStdString();
      if (!AddProjectDirectory(dirPathPath))
      {
         if (dirPathPath.IsAncestorOf(mProjectDirectory))
         {
            QMessageBox::warning(wkfEnv.GetMainWindow(),
                                 "Add Directory Failure",
                                 "Cannot add a directory which contains the project directory.");
         }
         else
         {
            QMessageBox::warning(wkfEnv.GetMainWindow(), "Add Directory Failure", "Directory is already in the project.");
         }
      }
   }
}

void wizard::Project::ShowRevisionManager()
{
   ChangeHistory* changeHistory = new ChangeHistory(*mRevisionStore, WorkingDirectory(), wkfEnv.GetMainWindow());
   changeHistory->show();
}

bool wizard::Project::IsDirectoryInProject(const UtPath& aPath) const
{
   for (auto&& dir : mProjectDirectories)
   {
      if (dir == aPath || dir.IsAncestorOf(aPath))
      {
         return true;
      }
   }
   return false;
}

wizard::TextSourceCache& wizard::Project::GetSourceCache()
{
   return *mWorkspacePtr->GetSourceCache();
}

bool wizard::Project::SetProjectLocation(const UtPath& aProjDirPath, const UtPath& aProjFilePath)
{
   if (aProjDirPath.Stat() != UtPath::cDIRECTORY)
   {
      return false;
   }

   QString    oldName         = Name();
   const bool projDirChanged  = (mProjectDirectory != aProjDirPath);
   const bool projFileChanged = (mProjectFilePath != aProjFilePath);

   if (projDirChanged)
   {
      mProjectDirectory = aProjDirPath;
      // Update project directory list.  First is always project directory.  Any other entries can't be sub-directories
      // or parent directories
      mProjectDirectories[0] = aProjDirPath;
      for (size_t i = 1; i < mProjectDirectories.size();)
      {
         if (UtStringUtil::StartsWith(mProjectDirectory.GetSystemPath(), mProjectDirectories[i].GetSystemPath()) ||
             UtStringUtil::StartsWith(mProjectDirectories[i].GetSystemPath(), mProjectDirectory.GetSystemPath()))
         {
            mProjectDirectories.erase(mProjectDirectories.begin() + i);
         }
         else
         {
            ++i;
         }
      }
      emit wizSignals->ProjectDirectoriesChanged(mProjectDirectories);
      emit wizSignals->ProjectDirectoryChanged(mProjectDirectory);
      SetWorkingDirectory(mProjectDirectory);
      wkfEnv.GetMainWindow()->setWindowTitle(ConstructTitle());
      for (auto&& comp : mComponents)
      {
         comp.second->ProjectDirectoryChanged();
      }
   }

   mProjectFilePath = aProjFilePath;

   if (projDirChanged || projFileChanged)
   {
      mRevisionStore->Backup();
      mRevisionStore->Close();
      mRevisionStore->SetProject(mProjectDirectory, mProjectFilePath);
      if (wkfEnv.GetPreferenceObject<wizard::BackupPrefObject>()->Enabled())
      {
         mRevisionStore->Open();
      }
      mRevisionStore->RequestBackup();
      mRevisionStore->EnableAutoBackup(5); // Auto-backup every 5 min

      if (Name() != oldName)
      {
         wkfEnv.GetMainWindow()->setWindowTitle(ConstructTitle());
      }
   }

   return true;
}

QString wizard::Project::ConstructTitle()
{
   if (this == (void*)0x4)
   {
      return "NULL";
   } // avoid access of project in SaveSettings() call --  yes this is dumb

   QString title = QString("%1 (%2)").arg(Name(), QString::fromStdString(ProjectDirectory().GetSystemPath()));
   return title;
}

bool wizard::Project::SetProjectDirectory(const UtPath& aPath)
{
   return SetProjectLocation(aPath, mProjectFilePath);
}

bool wizard::Project::SetProjectFilePath(const UtPath& aPath)
{
   return SetProjectLocation(mProjectDirectory, aPath);
}

std::string wizard::Project::GetXmlText() const
{
   UtQtXmlWriter writer;
   WriteStartDocument(writer);
   XML_Context context(&writer);
   context.SetFileName(mProjectFilePath);
   ToXML(context);
   WriteEndDocument(writer);
   return writer.ToString();
}

wizard::AttributeSet* wizard::Project::FindOptions(const WsfPProxyNode& aNode)
{
   AttributeSet* n    = mProxyAttributesRoot.get();
   WsfPProxyPath path = aNode.GetStablePath();
   for (size_t i = 0; n && i < path.size(); ++i)
   {
      if (path[i].IsString())
      {
         n = n->FindChild(path[i].GetMapKey());
      }
      else
      {
         std::stringstream ss;
         ss << path[i].GetIndex();
         n = n->FindChild(ss.str());
      }
   }
   return n;
}

wizard::AttributeSet* wizard::Project::FindOptions(const WsfPProxyPath& aPath)
{
   if (GetParseResults() && GetParseResults()->Proxy())
   {
      return FindOptions(WsfPProxyNode(GetParseResults()->Proxy(), aPath));
   }
   return nullptr;
}

wizard::AttributeSet* wizard::Project::GetOptions(const WsfPProxyNode& aNode)
{
   AttributeSet* n    = mProxyAttributesRoot.get();
   WsfPProxyPath path = aNode.GetStablePath();
   for (size_t i = 0; n && i < path.size(); ++i)
   {
      if (path[i].IsString())
      {
         n = &n->Child(path[i].GetMapKey());
      }
      else
      {
         std::stringstream ss;
         ss << path[i].GetIndex();
         n = &n->Child(ss.str());
      }
   }
   return n;
}

wizard::RevisionStore& wizard::Project::GetRevisionStore()
{
   return *mRevisionStore;
}

void wizard::Project::WriteStartDocument(UtQtXmlWriter& aWriter) const
{
   aWriter.WriteStartDocument("1.0");
   aWriter.WriteStartElement("wsf-ide-project-file");
}

void wizard::Project::WriteEndDocument(UtQtXmlWriter& aWriter) const
{
   aWriter.WriteEndElement();
   aWriter.WriteEndDocument();
}

bool wizard::Project::ProjectFilePath(UtPath& aFilePath) const
{
   aFilePath = mProjectFilePath;
   return !aFilePath.GetNormalizedPath().empty();
}

void wizard::Project::HandleProxyNodeMoved(const WsfPProxyNode& aSrc, const WsfPProxyNode& aDest)
{
   // if a proxy node is moved, say, from a rename operation,
   // update any attributes associated with that node
   AttributeSet* srcOptions = FindOptions(aSrc);
   if (srcOptions)
   {
      srcOptions->Rename(aDest.GetPath().Back().ToString());
   }
}

void wizard::Project::RedoCommand()
{
   if (wizSignals->InVisualMode())
   {
      Undo::Instance()->UndoStack()->redo();
   }
}

void wizard::Project::UndoCommand()
{
   if (wizSignals->InVisualMode())
   {
      Undo::Instance()->UndoStack()->undo();
   }
}

bool wizard::Project::OpenFileInTextEditor(const std::string& aFilePath)
{
   TextSource* sourcePtr = GetWorkspace()->FindSource(aFilePath, true, false);
   if (sourcePtr && !Util::IsNonTextFile(aFilePath.c_str()))
   {
      return (GotoFile(sourcePtr, true) != nullptr);
   }
   return false;
}

struct OpenInTextEditor
{
   const std::string mFilePath;
   explicit OpenInTextEditor(const std::string& aFilePath)
      : mFilePath(aFilePath)
   {
   }
   void operator()()
   {
      if (!wizard::Project::Instance())
      {
         return;
      }
      wizard::Project::Instance()->OpenFileInTextEditor(mFilePath);
   }
};

bool wizard::Project::OpenFile(const std::string& aFilePath)
{
   // If the file is suitable for the text editor, always open that.
   // otherwise, an external tool could grab every text file as a lay-down file
   TextSource* sourcePtr    = GetWorkspace()->FindSource(aFilePath, true, false);
   bool        isBinaryFile = Util::IsNonTextFile(aFilePath.c_str());

   // don't consider any other options if it's a .txt file
   if (sourcePtr && !isBinaryFile && UtStringUtil::EndsWith(aFilePath, ".txt"))
   {
      OpenFileInTextEditor(aFilePath);
      return true;
   }

   QMenu menu;
   emit  wizSignals->ActionsForFileRequested(QString::fromStdString(aFilePath), &menu);

   if (sourcePtr && !isBinaryFile)
   {
      QObject::connect(menu.addAction("Open in Text Editor"), &QAction::triggered, OpenInTextEditor(aFilePath));
   }
   if (menu.actions().count() == 1)
   {
      menu.actions().front()->trigger();
      return true;
   }
   else if (!menu.isEmpty())
   {
      return (menu.exec(QCursor::pos()) != nullptr);
   }

   QMessageBox::information(QApplication::activeWindow(),
                            "Unknown file type",
                            "This file is non-ascii and no tools are configured for this file type. Cannot open file.");
   return false;
}

wizard::ProjectComponent* wizard::Project::FindComponent(const std::string& aName)
{
   auto i = mComponents.find(aName);
   if (i != mComponents.end())
   {
      return i->second.get();
   }
   return nullptr;
}

void wizard::Project::AddComponent(const std::string& aName, std::unique_ptr<ProjectComponent> aComponent)
{
   assert(mComponents.find(aName) == mComponents.end());
   mComponents[aName] = std::move(aComponent);
}

void wizard::Project::Reparse()
{
   mWorkspacePtr->ReparseAll();
}
