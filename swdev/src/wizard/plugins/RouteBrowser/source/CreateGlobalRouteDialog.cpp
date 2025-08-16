// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CreateGlobalRouteDialog.hpp"

#include <map>

#include <QMimeData>
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>
#include <QVariant>

#include "DragAndDrop.hpp"
#include "EditorPrefObject.hpp"
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

RouteBrowser::CreateGlobalRouteDialog::CreateGlobalRouteDialog(WsfPProxy* proxy, QWidget* parent)
   : QDialog(parent)
   , mProxy(proxy)
{
   mUI.setupUi(this);

   PopulateFiles();

   connect(mUI.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &CreateGlobalRouteDialog::deleteLater);
   connect(mUI.buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &CreateGlobalRouteDialog::accept);
}

/// Populate the save file combo box with all the files included in the scenario
/// and the number of routes contained in each file.
void RouteBrowser::CreateGlobalRouteDialog::PopulateFiles()
{
   FileHitMap  fileHitMap;
   FileHitList sortedFileHitList;

   InitFileHitMap(fileHitMap);
   BuildFileHitMap(fileHitMap);
   SortFileHitMap(fileHitMap, sortedFileHitList);

   for (auto& i : sortedFileHitList)
   {
      const QString fileName     = QString::fromStdString(i.first);
      const size_t  fileHitCount = i.second;

      QString itemText = fileName;
      if (fileHitCount > 0)
      {
         const bool plural = (fileHitCount > 1);
         itemText += QString(" (%1 route%2)").arg(fileHitCount).arg(plural ? "s" : "");
      }

      mUI.fileComboBox->addItem(itemText, QVariant(fileName));
   }
}

/// Set the file selection combo box to the file with the given file path
void RouteBrowser::CreateGlobalRouteDialog::SelectFile(const std::string& aFilePath)
{
   for (int i = 0; i < mUI.fileComboBox->count(); i++)
   {
      if (aFilePath == SaveFilePathAtIndex(i))
      {
         mUI.fileComboBox->setCurrentIndex(i);
         break;
      }
   }
}

/// Retrieve the save file path for the save file represented by the given combo
/// box index.
std::string RouteBrowser::CreateGlobalRouteDialog::SaveFilePathAtIndex(int aIndex)
{
   QVariant    fileData = mUI.fileComboBox->itemData(aIndex);
   std::string fileName = fileData.toString().toStdString();

   return UtPath(wizard::Project::Instance()->ProjectDirectory() + fileName).GetSystemPath();
}

/// Retrieve the save path for the currently selected combo box item.
std::string RouteBrowser::CreateGlobalRouteDialog::SelectedSaveFilePath()
{
   return SaveFilePathAtIndex(mUI.fileComboBox->currentIndex());
}

/// Given a text document, returns the path of the file that it represents
/// relative to the root project directory.
std::string RouteBrowser::CreateGlobalRouteDialog::SetRelativeDocPath(const UtTextDocument* aDoc)
{
   std::string path;

   if (aDoc)
   {
      const UtPath& filePath = aDoc->GetFilePath();
      path = wizard::Project::Instance()->ProjectDirectory().GetRelativePath(filePath).GetSystemPath();
   }

   return path;
}

/// Add an entry to the file hit map for every file in the scenario,
/// initializing all file hit counts to 0.
void RouteBrowser::CreateGlobalRouteDialog::InitFileHitMap(CreateGlobalRouteDialog::FileHitMap& aFileHitMap)
{
   wizard::ParseResults* parseResults = wizard::ProjectWorkspace::Instance()->GetProject()->WaitForParseResults();

   for (const auto& iter : parseResults->mFirstIncludes)
   {
      const std::string filePathStr = SetRelativeDocPath(iter.first);
      if (!filePathStr.empty())
      {
         aFileHitMap[filePathStr] = 0;
      }
   }
}

/// Scans the routes in the scenario for the files they were created in. We
/// keep a count of how many routes were created in each scenario file, so
/// that we can show this to the user when the user selects a save file.
void RouteBrowser::CreateGlobalRouteDialog::BuildFileHitMap(FileHitMap& aFileHitMap)
{
   WsfPM_RouteMap             routeMap = WsfPM_Root(mProxy).routes();
   std::vector<WsfPProxyNode> routes;

   routeMap.GetAllValues(routes);

   for (const auto& node : routes)
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
                  assert(aFileHitMap.find(filePathStr) != aFileHitMap.end());
                  aFileHitMap[filePathStr] = aFileHitMap[filePathStr] + 1;
                  break;
               }
            }
         }
      }
   }
}

/// Given a map of files and the amount of routes instantiated in those
/// files, create a sorted list.
void RouteBrowser::CreateGlobalRouteDialog::SortFileHitMap(const FileHitMap& aFileHitMap, FileHitList& aFileHitList)
{
   for (const auto& hit : aFileHitMap)
   {
      aFileHitList.emplace_back(hit);
   }

   // Sort the file list alphabetically, and then sort the list by the number
   // of routes created in each file. Stable sort ensures that files with an
   // equal amount of routes will stay alphabetical.
   std::sort(aFileHitList.begin(), aFileHitList.end(), SortAlphabetical);
   std::stable_sort(aFileHitList.begin(), aFileHitList.end(), SortMostHits);
}

bool RouteBrowser::CreateGlobalRouteDialog::SortAlphabetical(const FileHit& aEntry1, const FileHit& aEntry2)
{
   return aEntry1.first < aEntry2.first;
}

bool RouteBrowser::CreateGlobalRouteDialog::SortMostHits(const FileHit& aEntry1, const FileHit& aEntry2)
{
   return aEntry1.second > aEntry2.second;
}

const std::string RouteBrowser::CreateGlobalRouteDialog::GetSelectedFile()
{
   return SelectedSaveFilePath();
}
