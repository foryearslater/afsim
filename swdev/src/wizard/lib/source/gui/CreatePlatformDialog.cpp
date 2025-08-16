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

#include "CreatePlatformDialog.hpp"

#include <map>

#include <QMimeData>
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QVariant>

#include "DragAndDrop.hpp"
#include "EditorPrefObject.hpp"
#include "Environment.hpp"
#include "ParseResults.hpp"
#include "PasteContext.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "Signals.hpp"
#include "Undo.hpp"
#include "UtQt.hpp"
#include "Util.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxy.hpp"
#include "WsfPProxyIndex.hpp"
#include "WsfParseNode.hpp"
#include "WsfParseSourceInclude.hpp"

Q_DECLARE_METATYPE(WsfPProxyNode)

std::string wizard::CreatePlatformDialog::mPrevPlatTypeName;
std::string wizard::CreatePlatformDialog::mPrevSaveFilePath;

wizard::CreatePlatformDialog::CreatePlatformDialog(WsfPProxy* proxy, QWidget* parent)
   : QDialog(parent)
   , mPosValid(false)
   , mProxy(proxy)
   , mApplyToTextImmediately(true)
{
   mUi.setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);

   PopulateTypes();
   SelectPlatformType(mPrevPlatTypeName);

   PopulateFiles();
   SelectFile(mPrevSaveFilePath);

   mUi.platNameLineEdit->setValidator(new QRegExpValidator(Util::wsfValidInputRegexp, this));

   connect(mUi.platNameLineEdit, &QLineEdit::textChanged, this, &CreatePlatformDialog::OnNameTextChanged);
   connect(mUi.platTypeComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
           this,
           &CreatePlatformDialog::UpdateSavedPlatType);
   connect(mUi.fileComboBox,
           static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
           this,
           &CreatePlatformDialog::UpdateSavedFile);
   connect(mUi.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &CreatePlatformDialog::deleteLater);
   connect(mUi.buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &CreatePlatformDialog::CreatePlatform);
}

void wizard::CreatePlatformDialog::SetPlatformPosition(const UtLatPos& aLat, const UtLonPos& aLon)
{
   mPosValid = true;
   mPos.SetLatitude(aLat);
   mPos.SetLongitude(aLon);
}

// When the user clicks OK, try to create the platform and add it to the file.
void wizard::CreatePlatformDialog::CreatePlatform()
{
   if (!mUi.platNameLineEdit->text().isEmpty())
   {
      const WsfPProxyNode newPlatType = SelectedPlatType();
      const std::string   newPlatName = mUi.platNameLineEdit->text().toStdString();

      if (!WsfPM_Root(mProxy).platforms().Find(newPlatName).IsValid()) // if name is valid?  Look for name conflicts
                                                                       // the validator should prevent, but just in cast...
      {
         TextSource* src = ProjectWorkspace::Instance()->FindSource(SelectedSaveFilePath(), false, false);
         if (src)
         {
            std::string platDef = "\n";
            platDef += "platform " + newPlatName + " " + newPlatType.GetName() + "\n";
            if (mPosValid)
            {
               std::stringstream ss;
               ss << mPos.GetLatitude() << " " << mPos.GetLongitude();
               unsigned int tabSpace = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
               for (unsigned int i = 0; i < tabSpace; i++)
               {
                  platDef += " ";
               }
               platDef += "position " + ss.str() + "\n";
            }
            platDef += "end_platform\n";
            if (src->GetViews().empty())
            {
               src->NewView(wizard::Project::Instance());
            }
            src->Append(platDef);
            wizEnv.RequestSelectPlatform(QString::fromStdString(newPlatName));
            deleteLater();
         }
      }
      else
      {
         UtQtShowSimpleMsg("Platform name conflict",
                           "A platform named <b>" + QString::fromStdString(newPlatName) + "</b> already exists.",
                           "Please choose a different name or remove the existing platform.",
                           this);
      }
   }
   else
   {
      UtQtShowSimpleMsg("Name required", "Please enter a name for the platform.", "", this);
   }
}

/// When the user changes the selection in the combo box, save the selection for
/// the next time the user opens the dialog.
void wizard::CreatePlatformDialog::UpdateSavedPlatType(int index)
{
   if (index >= 0)
   {
      mPrevPlatTypeName = PlatTypeAtIndex(index).GetName();
   }
}

/// When the user changes the selection in the combo box, save the selection for
/// the next time the user opens the dialog.
void wizard::CreatePlatformDialog::UpdateSavedFile(int index)
{
   if (index >= 0)
   {
      mPrevSaveFilePath = SaveFilePathAtIndex(index);
   }
}

/// Populate the platform types combo box with all the platform types available
/// to use in the scenario.
void wizard::CreatePlatformDialog::PopulateTypes()
{
   WsfPM_PlatformMap          platTypesMap = WsfPM_Root(mProxy).platformTypes();
   std::vector<WsfPProxyNode> types;

   platTypesMap.GetAllValues(types);
   std::sort(types.begin(), types.end());

   for (const auto& node : types)
   {
      const QString itemText = QString::fromStdString(node.GetName());
      mUi.platTypeComboBox->addItem(itemText, QVariant::fromValue<WsfPProxyNode>(node));
   }
}

/// Populate the save file combo box with all the files included in the scenario
/// and the number of platforms contained in each file.
void wizard::CreatePlatformDialog::PopulateFiles()
{
   FileHitMap  fileHitMap;
   FileHitList sortedFileHitList;

   InitFileHitMap(fileHitMap);
   BuildFileHitMap(fileHitMap);
   SortFileHitMap(fileHitMap, sortedFileHitList);

   for (size_t i = 0; i < sortedFileHitList.size(); i++)
   {
      const QString fileName     = QString::fromStdString(sortedFileHitList[i].first);
      const size_t  fileHitCount = sortedFileHitList[i].second;

      QString itemText = fileName;
      if (fileHitCount > 0)
      {
         const bool plural = (fileHitCount > 1);
         itemText += QString(" (%1 platform%2)").arg(fileHitCount).arg(plural ? "s" : "");
      }

      mUi.fileComboBox->addItem(itemText, QVariant(fileName));
   }
}

/// Use the static member variable to prepopulate the combo box selection based
/// on what the user had selected the last time the dialog was used.
void wizard::CreatePlatformDialog::SelectPlatformType(const std::string& aName)
{
   for (int i = 0; i < mUi.platTypeComboBox->count(); i++)
   {
      if (aName == PlatTypeAtIndex(i).GetName())
      {
         mUi.platTypeComboBox->setCurrentIndex(i);
         break;
      }
   }
}

/// Use the static member variable to prepopulate the combo box selection based
/// on what the user had selected the last time the dialog was used.
void wizard::CreatePlatformDialog::SelectFile(const std::string& aFilePath)
{
   for (int i = 0; i < mUi.fileComboBox->count(); i++)
   {
      if (aFilePath == SaveFilePathAtIndex(i))
      {
         mUi.fileComboBox->setCurrentIndex(i);
         break;
      }
   }
}

/// Retrieve the proxy node for the platform type represented by the given combo
/// box index.
WsfPProxyNode wizard::CreatePlatformDialog::PlatTypeAtIndex(int index)
{
   QVariant platTypeData = mUi.platTypeComboBox->itemData(index);
   return platTypeData.value<WsfPProxyNode>();
}

/// Retrieve the proxy node for the currently selected combo box item.
WsfPProxyNode wizard::CreatePlatformDialog::SelectedPlatType()
{
   return PlatTypeAtIndex(mUi.platTypeComboBox->currentIndex());
}

/// Retrieve the save file path for the save file represented by the given combo
/// box index.
std::string wizard::CreatePlatformDialog::SaveFilePathAtIndex(int index)
{
   QVariant    fileData = mUi.fileComboBox->itemData(index);
   std::string fileName = fileData.toString().toStdString();

   return UtPath(Project::Instance()->ProjectDirectory() + fileName).GetSystemPath();
}

/// Retrieve the save path for the currently selected combo box item.
std::string wizard::CreatePlatformDialog::SelectedSaveFilePath()
{
   return SaveFilePathAtIndex(mUi.fileComboBox->currentIndex());
}

/// Given a text document, returns the path of the file that it represents
/// relative to the root project directory.
std::string wizard::CreatePlatformDialog::SetRelativeDocPath(const UtTextDocument* doc)
{
   std::string path;

   if (doc)
   {
      const UtPath& filePath = doc->GetFilePath();
      path                   = Project::Instance()->ProjectDirectory().GetRelativePath(filePath).GetSystemPath();
   }

   return path;
}

/// Add an entry to the file hit map for every file in the scenario,
/// initializing all file hit counts to 0.
void wizard::CreatePlatformDialog::InitFileHitMap(CreatePlatformDialog::FileHitMap& fileHitMap)
{
   ParseResults* parseResults = ProjectWorkspace::Instance()->GetProject()->WaitForParseResults();

   for (const auto& iter : parseResults->mFirstIncludes)
   {
      const std::string filePathStr = SetRelativeDocPath(iter.first);
      if (!filePathStr.empty())
      {
         fileHitMap[filePathStr] = 0;
      }
   }
}

/// Scans the platforms in the scenario for the files they were created in. We
/// keep a count of how many platforms were created in each scenario file, so
/// that we can show this to the user when the user selects a save file.
void wizard::CreatePlatformDialog::BuildFileHitMap(FileHitMap& fileHitMap)
{
   WsfPM_PlatformMap          platMap = WsfPM_Root(mProxy).platforms();
   std::vector<WsfPProxyNode> plats;

   platMap.GetAllValues(plats);

   for (const auto& node : plats)
   {
      const WsfPProxyIndexNode* indNode = node.FindIndexNode();

      if (indNode)
      {
         for (const auto& indEntry : indNode->mEntries)
         {
            if (indEntry.mEntryType == WsfPProxyIndex::cADDED)
            {
               const WsfParseNode* pNode       = indEntry.mNodePtr->PreviousLeaf();
               const std::string   filePathStr = SetRelativeDocPath(pNode->mValue.mSource);
               if (!filePathStr.empty())
               {
                  assert(fileHitMap.find(filePathStr) != fileHitMap.end());
                  fileHitMap[filePathStr] = fileHitMap[filePathStr] + 1;
                  break;
               }
            }
         }
      }
   }
}

/// Given a map of files and the amount of platforms instantiated in those
/// files, create a sorted list.
void wizard::CreatePlatformDialog::SortFileHitMap(const FileHitMap& fileHitMap, FileHitList& fileHitList)
{
   for (const auto& hit : fileHitMap)
   {
      fileHitList.push_back(hit);
   }

   // Sort the file list alphabetically, and then sort the list by the number
   // of platforms created in each file. Stable sort ensures that files with an
   // equal amount of platforms will stay alphabetical.
   std::sort(fileHitList.begin(), fileHitList.end(), SortAlphabetical);
   std::stable_sort(fileHitList.begin(), fileHitList.end(), SortMostHits);
}

bool wizard::CreatePlatformDialog::SortAlphabetical(const FileHit& entry1, const FileHit& entry2)
{
   return entry1.first < entry2.first;
}

bool wizard::CreatePlatformDialog::SortMostHits(const FileHit& entry1, const FileHit& entry2)
{
   return entry1.second > entry2.second;
}

void wizard::CreatePlatformDialog::OnNameTextChanged(const QString& aName)
{
   bool validName = !aName.isEmpty() && !WsfPM_Root(mProxy).platforms().Exists(aName.toStdString());
   mUi.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validName);
   if (validName)
   {
      UtQtSetTextColor(mUi.platNameLineEdit, QApplication::palette().text().color());
      mUi.platNameLineEdit->setToolTip("");
   }
   else
   {
      UtQtSetTextColor(mUi.platNameLineEdit, Qt::red);
      mUi.platNameLineEdit->setToolTip("Platform with this name already exists");
   }
}

void wizard::CreatePlatformDialog::ChooseFileFromType()
{
   std::string                type = mUi.platTypeComboBox->currentText().toStdString();
   std::map<std::string, int> fileHitMap;
   std::string                maxFile;
   int                        maxInst = 0;

   WsfPM_PlatformMap          platMap = WsfPM_Root(mProxy).platforms();
   std::vector<WsfPProxyNode> plats;

   platMap.GetAllValues(plats);

   for (const auto& node : plats)
   {
      if (node.IsOfType(type))
      {
         const WsfPProxyIndexNode* indNode = node.FindIndexNode();

         if (indNode)
         {
            for (const auto& indEntry : indNode->mEntries)
            {
               if (indEntry.mEntryType == WsfPProxyIndex::cADDED)
               {
                  const WsfParseNode* pNode       = indEntry.mNodePtr->PreviousLeaf();
                  const std::string   filePathStr = SetRelativeDocPath(pNode->mValue.mSource);
                  if (!filePathStr.empty())
                  {
                     fileHitMap[filePathStr] = fileHitMap[filePathStr] + 1;
                     if (fileHitMap[filePathStr] > maxInst)
                     {
                        maxInst = fileHitMap[filePathStr];
                        maxFile = filePathStr;
                     }
                     break;
                  }
               }
            }
         }
      }
   }
   if (maxInst > 0)
   {
      int index = mUi.fileComboBox->findText(maxFile.c_str(), Qt::MatchContains);
      if (index >= 0)
      {
         mUi.fileComboBox->setCurrentIndex(index);
      }
   }
}

void wizard::CreatePlatformDialog::PasteHandler(const QMimeData* aMime, const PasteContext& aContext, bool aExecute)
{
   if (aExecute && !aContext.IsActionChosen("create-platform"))
   {
      return;
   }
   if (!aMime || !aMime->hasFormat(DragAndDrop::cPROPERTY_SET_MIME_TYPE))
   {
      return;
   }
   if (!ProxyWatcher::GetActiveProxy())
   {
      return;
   }
   DragAndDrop::PropertyList properties = DragAndDrop::ReadPropertySetMime(aMime);
   if (properties.empty())
   {
      return;
   }
   if (properties.size() != 1)
   {
      return;
   }
   DragAndDrop::PropertyMap pMap = properties[0];
   if (!pMap.contains("proxy-node"))
   {
      return;
   }
   WsfPProxyNode node = DragAndDrop::ToProxyNode(pMap["proxy-node"].toString());
   WsfPM_Root    proxyRoot(ProxyWatcher::GetActiveProxy());

   if (node.GetParent() == proxyRoot.platformTypes())
   {
      if (aExecute)
      {
         CreatePlatformDialog* dlg = new CreatePlatformDialog(node.GetProxy(), aContext.GetWidget());
         if (aContext.HasPosition())
         {
            dlg->SetPlatformPosition(aContext.GetLat(), aContext.GetLon());
         }

         dlg->SelectPlatformType(node.GetName());
         dlg->SelectFile(aContext.GetFilePath());
         dlg->mApplyToTextImmediately = aContext.ApplyToTextImmediately();
         dlg->show();
      }
      else
      {
         QString label = "New Platform(";
         label         = label + node.GetName().c_str() + ")";
         aContext.AddAvailableAction("create-platform", label);
      }
   }
}
