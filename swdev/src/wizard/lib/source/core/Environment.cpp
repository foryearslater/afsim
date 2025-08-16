// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "Environment.hpp"

#include <fstream>

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QUrlQuery>

#include "ComponentPropertyManager.hpp"
#include "DeveloperActions.hpp"
#include "DocumentationInterpretter.hpp"
#include "Editor.hpp"
#include "EditorManager.hpp"
#include "Project.hpp"
#include "ProjectSettingsDlg.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "RunManager.hpp"
#include "Signals.hpp"
#include "StartupDialog.hpp"
#include "TextSource.hpp"
#include "TextSourceView.hpp"
#include "ToolPrefObject.hpp"
#include "UiResources.hpp"
#include "UtPath.hpp"
#include "UtQtMemory.hpp"
#include "UtQtXmlReader.hpp"
#include "Util.hpp"
#include "VtkInterface.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfExe.hpp"
#include "WsfStandardApplication.hpp"
#include "XML_Context.hpp"
#include "wsf_extensions.hpp"

wizard::Environment* wizard::Environment::mInstancePtr = nullptr;

wizard::Environment::Environment()
   : QObject()
   , mWorkspacePtr(ut::make_unique<ProjectWorkspace>())
   , mVtkInterfacePtr(ut::make_unique<VtkInterface>())
   , mWsfApplicationPtr(nullptr)
{
   wkfEnv.SetFrameUpdateEnabled(true);
   mInstancePtr = this;
   UiResources::Create();
   ComponentPropertyManager::CreateInstance();
   ProxyWatcher::Create();

   wkfEnv.GetMainWindow()->installEventFilter(new ShutdownFilter(this));
   ut::qt::make_qt_ptr<DeveloperActions>(wkfEnv.GetMainWindow());
   qApp->installEventFilter(new ActivationFilter(this));

   connect(wizSignals, &Signals::ActionsForFileRequested, this, &Environment::ActionsForFileRequested);
}

wizard::Environment::~Environment()
{
   ProxyWatcher::Destroy();
   UiResources::Destroy();
   ComponentPropertyManager::DestroyInstance();
   mWorkspacePtr = nullptr;
   mInstancePtr  = nullptr;

   if (mWsfApplicationPtr)
   {
      emit WsfStandardApplicationPendingDestruction();
      delete mWsfApplicationPtr;
   }
}

wizard::Environment& wizard::Environment::GetInstance()
{
   assert(mInstancePtr != nullptr);
   return *mInstancePtr;
}

void wizard::Environment::Create()
{
   assert(mInstancePtr == nullptr);
   if (mInstancePtr == nullptr)
   {
      new Environment();
   }
}

void wizard::Environment::StartUp() {}

void wizard::Environment::Shutdown()
{
   if (mInstancePtr != nullptr)
   {
      delete mInstancePtr;
      mInstancePtr = nullptr;
   }
}

void wizard::Environment::UpdateMenus()
{
   QMenu* file = wkfEnv.GetMainWindow()->GetMenuByName("File");
   if (nullptr != file)
   {
      PopulateRecents();

      QAction* open = new QAction("Open Project...", file);
      connect(open, &QAction::triggered, this, &Environment::FileOpen);
      file->insertAction(*file->actions().begin(), open);
      file->addSeparator();
   }
}

void wizard::Environment::PopulateRecents()
{
   QMenu* file = wkfEnv.GetMainWindow()->GetMenuByName("File");

   if (file)
   {
      QMenu* recent = file->findChild<QMenu*>("Open Recent");
      if (!recent)
      {
         recent = new QMenu("Open Recent", file);
         recent->setObjectName("Open Recent");
         file->insertMenu(*file->actions().begin(), recent);
      }
      else
      {
         recent->clear();
      }

      QList<QStringList> rlist = wizRunMgr.GetRecentScenarios();
      for (auto&& it : rlist)
      {
         QAction* recentAct = new QAction(it.first(), recent);
         recentAct->setData(it);
         connect(recentAct, &QAction::triggered, this, &Environment::FileOpen);
         recent->addAction(recentAct);
      }
      if (rlist.isEmpty())
      {
         auto* action = recent->addAction("(no recent files)");
         action->setEnabled(false);
      }
   }
}

bool wizard::Environment::LoadScenario(const QStringList& aFiles)
{
   Project* projectPtr = nullptr;
   wkfEnv.GetMainWindow()->setCursor(Qt::WaitCursor);
   if (aFiles.size() == 1)
   {
      QFileInfo fi(*aFiles.begin());
      if (fi.suffix() == "afproj")
      {
         std::string path = aFiles.begin()->toStdString();
         // set the working directory to the project's directory
         if (wizRunMgr.SetWorkingDirectoryToProject(path))
         {
            UtQtXmlReader reader;
            if (reader.Open(path) && reader.NextElementToken() == UtQtXmlReader::cTT_START_ELEMENT)
            {
               UtPath      realPath = UtPath(path).GetRealPath();
               XML_Context context(&reader);
               context.SetFileName(realPath.GetSystemPath());
               projectPtr = new Project(mWorkspacePtr.get());
               projectPtr->FromXML(context);
            }
         }
      }
   }
   if (!projectPtr)
   {
      std::vector<UtPath> mainFiles;
      for (auto&& it : aFiles)
      {
         UtPath path(it.toStdString());
         path.MakeFullPath();
         if (path.Stat() == UtPath::cSTAT_ERROR)
         {
            QString message = "Could not find file: " + QString::fromStdString(path.GetSystemPath()) +
                              ".\nChoose Unlist to remove from your recent list or OK to create.";
            QMessageBox* box = new QMessageBox(QString("File not found"),
                                               message,
                                               QMessageBox::Question,
                                               QMessageBox::Ok,
                                               QMessageBox::Discard,
                                               QMessageBox::NoButton,
                                               wkfEnv.GetMainWindow());
            box->button(QMessageBox::Discard)->setText("Unlist");
            int b = box->exec();
            delete box;
            if (b == QMessageBox::Discard)
            {
               wizRunMgr.RemoveRecentScenario(aFiles);

               QStringList files;
               // launch dialog if no scenario was specified and -ups was not given.
               StartupDialog dialog(files, nullptr, Qt::MSWindowsFixedSizeDialogHint);
               if (dialog.exec() != QDialog::Rejected)
               {
                  return wizRunMgr.LoadScenario(files);
               }
            }
            else if (b == QMessageBox::Ok)
            {
               std::ofstream touchFile(path.GetSystemPath().c_str());
               touchFile.close();
            }
         }
         if (path.Stat() == UtPath::cFILE)
         {
            mainFiles.push_back(path);
         }
      }
      UtPath workingDir;
      if (!mainFiles.empty())
      {
         workingDir = mainFiles[0];
         // set the working directory to the start-up file's directory
         if (wizRunMgr.SetWorkingDirectoryToProject(workingDir))
         {
            workingDir.Up();

            projectPtr = new Project(mWorkspacePtr.get());
            projectPtr->SetProjectDirectory(workingDir);
            projectPtr->SetStartupFiles(mainFiles);
            wizSignals->ProjectOpen(*projectPtr);
            if (!mainFiles.empty())
            {
               projectPtr->GotoFile(mainFiles[0].GetSystemPath());
            }
         }
      }
   }
   PopulateRecents();
   wkfEnv.GetMainWindow()->setCursor(Qt::ArrowCursor);
   return (nullptr != projectPtr);
}

void wizard::Environment::FileOpen()
{
   QAction* sender = dynamic_cast<QAction*>(QObject::sender());
   if (!ProjectWorkspace::Instance()->GetExecutions().empty())
   {
      if (QMessageBox::Yes ==
          QMessageBox::warning(nullptr,
                               "Wizard Debugger still running",
                               "Opening a new project will cause the current run to stop. Do you wish to continue?",
                               QMessageBox::Yes,
                               QMessageBox::No))
      {
         ProjectWorkspace::Instance()->StopAllExecutions();
      }
      else
      {
         return;
      }
   }
   if ((nullptr == sender) || (sender->text() == "Open Project..."))
   {
      QStringList files =
         QFileDialog::getOpenFileNames(wkfEnv.GetMainWindow(),
                                       "Select Files",
                                       "",
                                       "AFSIM Input(*.afproj *.txt);; AFSIM Project(*.afproj);; All Files (*.*)");
      if (!files.isEmpty())
      {
         if (ProjectWorkspace::Instance()->RequestClose())
         {
            wizRunMgr.LoadScenario(files);
         }
      }
   }
   else
   {
      if (ProjectWorkspace::Instance()->RequestClose())
      {
         wizRunMgr.LoadScenario(sender->data().toStringList());
      }
   }
}

void wizard::Environment::OpenUrl(const QUrl& aUrl)
{
   if (aUrl.scheme() == "ide")
   {
      std::string foo = aUrl.toDisplayString().toStdString();
      // let our custom providers figure it out
      QString host = aUrl.host();
      if (host == "project")
      {
         LoadProject(aUrl);
      }
      else if (host == "documentation")
      {
         bool    returl;
         QString pageName;
         QString retData = DocumentationInterpretter::LookupDocumentationUrl(aUrl, returl, pageName);
         if (returl)
         {
            QDesktopServices::openUrl(retData);
         }
         // the alternative is not handled here... this is really for context documentation
      }
   }
   else
   {
      std::cout << aUrl.toDisplayString().toStdString() << std::endl;
      QDesktopServices::openUrl(aUrl.toDisplayString());
   }
}

void wizard::Environment::ProjectSettingsOpen(Project& aProject)
{
   ProjectSettingsDialog dlg(wkfEnv.GetMainWindow());
   dlg.Open(&aProject);
}

void wizard::Environment::MarginClickAction(Qt::MouseButton aButton, TextSource* aSource, size_t aLineNumber)
{
   emit MarginClick(aButton, aSource->GetSystemPath().c_str(), aLineNumber);
}

void wizard::Environment::MarginDraw(const QString& aSourcePath, QPainter& aPainter, const QMap<size_t, QPoint>& aVisibleLineMap)
{
   emit MarginDrawRequest(aSourcePath, aPainter, aVisibleLineMap);
}

void wizard::Environment::GetHighlightLines(const QString& aSourcePath, QList<size_t>& aLineList)
{
   emit HighlightLinesRequest(aSourcePath, aLineList);
}

void wizard::Environment::LoadProject(const QUrl& aUrl)
{
   QUrlQuery urlQuery(aUrl);
   if (urlQuery.hasQueryItem("open"))
   {
      QString projectPath = urlQuery.queryItemValue("path") + "/" + urlQuery.queryItemValue("startup");

      bool loaded = false;
      if (ProjectWorkspace::Instance()->RequestClose())
      {
         loaded = LoadScenario(QStringList({projectPath}));
      }
      if (loaded)
      {
         if (urlQuery.hasQueryItem("file"))
         {
            std::string filePath = QDir::toNativeSeparators(urlQuery.queryItemValue("file")).toStdString();

            ProjectWorkspace::Instance()->GotoFile(filePath);

            if (urlQuery.hasQueryItem("line"))
            {
               ProjectWorkspace::Instance()->GotoRange(filePath, urlQuery.queryItemValue("line").toInt() - 1);
            }
         }
      }
   }
   else if (urlQuery.hasQueryItem("copy"))
   {
      QFileDialog fileDialog;
      fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
      fileDialog.setFileMode(QFileDialog::Directory);
      fileDialog.setOption(QFileDialog::ShowDirsOnly);

      if (fileDialog.exec())
      {
         QDir sourceDir(urlQuery.queryItemValue("path"));

         QDir selectedDir(fileDialog.selectedFiles().first());

         QDir targetDir(selectedDir);
         targetDir.mkdir(sourceDir.dirName());
         targetDir.cd(sourceDir.dirName());

         // Project Contents

         QDirIterator sourceDirIter(sourceDir.path(),
                                    QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
                                    QDirIterator::Subdirectories);
         while (sourceDirIter.hasNext())
         {
            sourceDirIter.next();

            QString sourcePath = sourceDirIter.filePath();
            QString targetPath = targetDir.path() + "/" + sourceDir.relativeFilePath(sourcePath);
            if (sourceDirIter.fileInfo().isDir())
            {
               targetDir.mkpath(targetPath);
            }
            else
            {
               QFile::copy(sourcePath, targetPath);
            }
         }

         // Base and Site Types

         QString typesPath = urlQuery.queryItemValue("types_path");

#ifdef WIN32
         QProcess mklink;
         mklink.start("cmd.exe",
                      QStringList() << "/C"
                                    << "mklink"
                                    << "/J" << (QDir::toNativeSeparators(selectedDir.path()) + "\\base_types")
                                    << typesPath + "\\base_types");
         mklink.waitForFinished();
         mklink.start("cmd.exe",
                      QStringList() << "/C"
                                    << "mklink"
                                    << "/J" << (QDir::toNativeSeparators(selectedDir.path()) + "\\site_types")
                                    << typesPath + "\\site_types");
         mklink.waitForFinished();
#else
         QFile::link(typesPath + "/base_types", selectedDir.path() + "/base_types");
         QFile::link(typesPath + "/site_types", selectedDir.path() + "/site_types");
#endif

         QString projectPath = targetDir.path() + "/" + urlQuery.queryItemValue("startup");
         if (ProjectWorkspace::Instance()->RequestClose())
         {
            LoadScenario(QStringList({projectPath}));
         }
      }
   }
}

void wizard::Environment::CopyFocusedData()
{
   emit FocusedCopyRequest();
}

void wizard::Environment::UpdatePlatform(const QString& aPlatformName)
{
   emit PlatformUpdated(aPlatformName);
}

void wizard::Environment::DeletePlatform(const QString& aPlatformName)
{
   emit PlatformDeleted(aPlatformName);
}

WsfStandardApplication& wizard::Environment::GetWsfStandardApplication()
{
   if (!mWsfApplicationPtr)
   {
      mWsfApplicationPtr = new WsfStandardApplication("wiz_wsf_app", 0, nullptr);

      RegisterBuiltinExtensions(*mWsfApplicationPtr);
      RegisterOptionalExtensions(*mWsfApplicationPtr);
   }
   return *mWsfApplicationPtr;
}

void wizard::Environment::Undo()
{
   ProjectWorkspace::Instance()->GetEditorManager()->Undo();
}

void wizard::Environment::Redo()
{
   ProjectWorkspace::Instance()->GetEditorManager()->Redo();
}

void wizard::Environment::ClearUndo()
{
   ProjectWorkspace::Instance()->GetEditorManager()->ClearUndoStacks();
}

void wizard::Environment::BeginUndoCapture() noexcept
{
   ProjectWorkspace::Instance()->GetEditorManager()->BeginUndoCapture();
}

bool wizard::Environment::IsCapturingUndo() const noexcept
{
   return ProjectWorkspace::Instance()->GetEditorManager()->IsCapturingUndo();
}

void wizard::Environment::EndUndoCapture() noexcept
{
   ProjectWorkspace::Instance()->GetEditorManager()->EndUndoCapture();
}

void wizard::Environment::RequestSelectPlatform(const QString& aPlatformName)
{
   mVtkInterfacePtr->RequestSelectPlatform(aPlatformName);
}

void wizard::Environment::SetVtkMoveMode(wizard::VtkInterface::MoveMode aMoveMode) noexcept
{
   mVtkInterfacePtr->SetMoveMode(aMoveMode);
}

wizard::VtkInterface::MoveMode wizard::Environment::GetVtkMoveMode() const noexcept
{
   return mVtkInterfacePtr->GetMoveMode();
}

void wizard::Environment::SetMapCloseConnection(const QMetaObject::Connection& aConnection) noexcept
{
   mMapCloseConnection = aConnection;
}

QMetaObject::Connection wizard::Environment::GetMapCloseConnection() const noexcept
{
   return mMapCloseConnection;
}

bool wizard::ShutdownFilter::eventFilter(QObject* obj, QEvent* aEvent)
{
   if (aEvent->type() == QEvent::Close)
   {
      if (!ProjectWorkspace::Instance()->RequestClose())
      {
         aEvent->ignore();
         return true;
      }
   }
   return QObject::eventFilter(obj, aEvent);
}

void wizard::Environment::ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu)
{
   for (auto&& it : wkfEnv.GetPreferenceObject<ToolPrefObject>()->Tools())
   {
      if ((it.second.mExtensions.contains(aFileInfo.suffix())) ||
          (it.second.mExtensions.contains(QString('.') + aFileInfo.suffix())) ||
          (it.second.mExtensions.contains(QString("*.") + aFileInfo.suffix())))
      {
         ToolDef tool = it.second;
         connect(aMenu->addAction(QIcon::fromTheme("wrench"), "Open with " + it.second.mAlias),
                 &QAction::triggered,
                 [this, aFileInfo, tool]() { Execute(aFileInfo, tool); });
      }
   }
}

void wizard::Environment::Execute(const QFileInfo& aFile, const ToolDef& aTool)
{
   QString workingDir = aTool.mWorking;

   QString exePath = aTool.mPath;

   QString cmdLine = aTool.mCmdLine;

   ProcessTags(workingDir, aFile);
   ProcessTags(cmdLine, aFile);

   // construct the cmdLine
   QStringList argList = cmdLine.split(' ');

   if (MakeExecutable(exePath, argList))
   {
      // Note: this doesn't seem to work from Visual Studio, but does work for an install.
      if (!QProcess::startDetached(exePath, argList, workingDir))
      {
         QMessageBox::warning(nullptr, "Execution Failed", tr("Failed to start tool: ") + exePath);
      }
   }
   else
   {
      QMessageBox::information(nullptr, "Can't execute", "Cannot execute " + exePath + "; invalid file type.");
   }
}

void wizard::Environment::ProcessTags(QString& aString, const QFileInfo& aFile) const
{
   aString.replace(QString("$(ITEM_PATH)"), aFile.filePath());
   aString.replace("$(ITEM_FILE)", aFile.fileName());
   aString.replace("$(ITEM_DIRECTORY)", aFile.path());

   size_t      line(0), col(0);
   TextSource* src = ProjectWorkspace::Instance()->FindSource(aFile.filePath().toStdString(), false, false);
   if (src)
   {
      if (!src->GetViews().empty())
      {
         size_t  offset(0);
         Editor* editorPtr = src->GetViews()[0]->mEditorPtr;
         editorPtr->GetCurrentPosition(offset, line, col);
      }
   }
   aString.replace("$(CURRENT_LINE)", QString::number(line + 1));
   aString.replace("$(CURRENT_COLUMN)", QString::number(col + 1));

   Editor* currentEditor = ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
   if (currentEditor)
   {
      aString.replace("$(SELECTED_TEXT)", currentEditor->GetSelectedText().c_str());
   }
   QString  mainPaths, wsfPath, scenarioDir;
   Project* projectPtr = ProjectWorkspace::Instance()->GetProject();
   if (projectPtr)
   {
      std::vector<UtPath> paths = projectPtr->GetStartupFiles();
      for (size_t i = 0; i < paths.size(); ++i)
      {
         if (i > 0)
         {
            mainPaths += ' ';
         }
         mainPaths += Util::QuotePathWithSpaces(QString::fromStdString(paths[i].GetSystemPath()));
      }
      wsfPath     = Util::QuotePathWithSpaces(QString::fromStdString(projectPtr->GetExe()->GetPath().GetSystemPath()));
      scenarioDir = Util::QuotePathWithSpaces(QString::fromStdString(projectPtr->WorkingDirectory().GetSystemPath()));
   }
   aString.replace("$(SCENARIO_FILES)", mainPaths);
   aString.replace("$(WSF_BIN_PATH)", wsfPath);
   aString.replace("$(SCENARIO_DIRECTORY)", scenarioDir);
}

bool wizard::Environment::MakeExecutable(QString& aExe, QStringList& aArgs) const
{
   if (Util::IsExecutable(aExe.toStdString()))
   {
      return true;
   }
#ifdef _WIN32
   // Call .bat files with cmd.exe
   if (aExe.endsWith(".bat", Qt::CaseInsensitive))
   {
      char* windir = getenv("windir");
      if (windir)
      {
         QDir        d(windir);
         QString     cmdexe = d.filePath("system32/cmd.exe");
         QStringList newArgs;
         newArgs << "/C" << aExe;
         aArgs = newArgs + aArgs;
         aExe  = cmdexe;
      }
      return true;
   }
#else
   const char* shBin = "/bin/sh";
   if (QFile::exists(shBin) && (QFile::permissions(aExe) & QFile::ExeOwner))
   {
      QStringList newArgs;
      newArgs << aExe;
      aArgs = newArgs + aArgs;
      aExe = shBin;
      return true;
   }
#endif
   // Don't know how to execute this file
   return false;
}
