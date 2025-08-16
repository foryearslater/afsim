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

#include "SatelliteInserterHandler.hpp"

#include <string>

#include <QMessageBox>
#include <QModelIndexList>

#include "Editor.hpp"
#include "EditorManager.hpp"
#include "InputReader.hpp"
#include "InputUtils.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "SatelliteInserterModel.hpp"
#include "SpaceToolsScenario.hpp"
#include "TextSourceCache.hpp"
#include "UtTwoLineElement.hpp"
#include "WsfPM_Root.hpp"
#include "WsfSpaceMoverBase.hpp"

//! Sets the insert file to the given file. If the given file is empty then the file is set to:
//! 1. The last file that the tool inserted satellites in,
//! 2. The file that the current editor is showing,
//! 3. The first startup file
//!
//! @param aFile The file path that the tool should insert the satellites into.
void SpaceTools::SatelliteInserterHandler::SetInsertFile(const QString& aFile)
{
   if (aFile.isEmpty())
   {
      if (mInsertFile.isEmpty())
      {
         wizard::Editor* editorPtr = wizard::ProjectWorkspace::Instance()->GetEditorManager()->GetCurrentEditor();
         if (editorPtr)
         {
            mInsertFile = QString::fromStdString(editorPtr->GetSource()->GetFilePath().GetNormalizedPath());
         }
         else if (!wizard::Project::Instance()->GetStartupFiles().empty())
         {
            auto startupFile = wizard::Project::Instance()->GetStartupFiles().at(0);
            mInsertFile      = QString::fromStdString(startupFile.GetNormalizedPath());
         }
      }
   }
   else
   {
      mInsertFile = aFile;
   }
}

std::map<QString, std::pair<UtPath, std::vector<QString>>>
SpaceTools::SatelliteInserterHandler::GetInsertPlatforms(const QAbstractItemModel* aModelPtr,
                                                         const QModelIndexList&    aIndexList)
{
   std::map<QString, std::pair<UtPath, std::vector<QString>>> filePlatName;
   QSet<QString>                                              insertedPlatforms{};
   QSet<QString>                                              duplicatePlatforms{};
   QSet<QString>                                              existingPlatforms{};
   QSet<QString>                                              existingDesignators{};
   QSet<QString>                                              allDesignators{GetDesignators()};

   WsfPProxy* proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   WsfPM_Root root(proxyPtr);

   auto platformMap = root.platforms();

   // Insert only selected platforms
   for (auto platform : aIndexList)
   {
      int     row = platform.row();
      QString platName =
         aModelPtr->data(aModelPtr->index(row, SatelliteInserterModel::FieldColumn::cNAME), Qt::DisplayRole).toString();
      QString platDesignator =
         aModelPtr->data(aModelPtr->index(row, SatelliteInserterModel::FieldColumn::cDESIGNATOR), Qt::DisplayRole).toString();

      if (platformMap.FindT(platName.toStdString()))
      {
         existingPlatforms.insert(platName);
         continue;
      }
      if (!platDesignator.isEmpty())
      {
         if (allDesignators.contains(platDesignator))
         {
            existingDesignators.insert(platDesignator);
         }
         else
         {
            allDesignators.insert(platDesignator);
         }
      }

      if (insertedPlatforms.contains(platName))
      {
         duplicatePlatforms.insert(platName);
         continue;
      }

      // If there is no platform with the same name, add the definition location and platform name to the map.
      QString fileLoc{
         aModelPtr->data(aModelPtr->index(row, SatelliteInserterModel::FieldColumn::cFILE), Qt::DisplayRole).toString()};

      auto it = filePlatName.find(fileLoc);
      if (it != filePlatName.end())
      {
         it->second.second.push_back(platName);
      }
      else
      {
         UtPath basePath{
            aModelPtr->data(aModelPtr->index(row, SatelliteInserterModel::FieldColumn::cDATABASE), Qt::UserRole)
               .toString()
               .toStdString()};
         basePath.Up();
         std::vector<QString>                    newVec = {platName};
         std::pair<UtPath, std::vector<QString>> pair{basePath, newVec};
         filePlatName[fileLoc] = pair;
      }
      insertedPlatforms.insert(platName);
   }

   if (!duplicatePlatforms.empty())
   {
      QString duplicates = duplicatePlatforms.toList().join(", ");
      QMessageBox::information(nullptr,
                               "Multiple Name Insertion",
                               "Warning: The following platforms that you are trying to insert have the same name. "
                               "Only the first instance found will be inserted:\n\n" +
                                  duplicates);
   }

   if (!existingPlatforms.empty())
   {
      auto platforms = existingPlatforms.toList().join(", ");
      QMessageBox::information(nullptr,
                               "Platforms Already Exist",
                               "Cannot insert already existing platforms:\n\n" + platforms);
   }

   if (!existingDesignators.empty())
   {
      QString designators = existingDesignators.toList().join(", ");
      QMessageBox::information(nullptr,
                               "Designators Already Exist",
                               "Warning: The following space movers already exist in the scenario:\n\n" + designators);
   }
   return filePlatName;
}

void SpaceTools::SatelliteInserterHandler::InsertSatellites(const QAbstractItemModel* aModelPtr,
                                                            const QModelIndexList&    aIndexList)
{
   auto platformMap = GetInsertPlatforms(aModelPtr, aIndexList);
   if (!platformMap.empty()) // if there are platforms that need to be added
   {
      QSet<QString> invalidFiles;
      QFile         file(mInsertFile);
      if (file.open(QIODevice::WriteOnly | QIODevice::Append))
      {
         QString definitions;
         for (auto& inFile : platformMap) // for every file that has a platform definition
         {
            auto insertFile   = inFile.first;
            auto basePath     = inFile.second.first;
            auto platformList = inFile.second.second;
            definitions.append(FindDefinitions(insertFile, basePath, platformList, invalidFiles));
         } // for infile

         if (!definitions.isEmpty())
         {
            auto editorPtr = wizard::ProjectWorkspace::Instance()->GotoFile(mInsertFile.toStdString());
            editorPtr->moveCursor(QTextCursor::End);
            editorPtr->textCursor().insertText("\n" + definitions);
            editorPtr->centerCursor();
         }
      } // file.open
      else
      {
         QMessageBox::warning(nullptr,
                              "Invalid File",
                              "Cannot open " + mInsertFile + " to insert definitions. Try another file.");
         return;
      }

      if (!invalidFiles.empty())
      {
         QString files = invalidFiles.toList().join(", ");
         QMessageBox::information(nullptr, "Invalid Files", "Could not open the following files:\n\n" + files);
      }
   }
}

QString SpaceTools::SatelliteInserterHandler::FindDefinitions(const QString&        aFile,
                                                              const UtPath&         aBasePath,
                                                              std::vector<QString>& aPlatforms,
                                                              QSet<QString>&        aInvalidFilesList)
{
   QString definitionFilePath{QString::fromStdString(aBasePath.GetNormalizedPath()) + '/' + aFile};
   QFile   inputFile(definitionFilePath);
   QString definitions = "";
   if (inputFile.open(QIODevice::ReadOnly))
   {
      QTextStream in(&inputFile);

      bool insertDefinition     = false;
      bool inPlatformDefinition = false;
      while (!in.atEnd())
      {
         QString line     = in.readLine();
         auto    splitStr = line.split(" ");
         // Only add include files that are not in platform definitions.
         if ((splitStr.at(0) == "include" || splitStr.at(0) == "include_once") && !inPlatformDefinition)
         {
            QString includeString("include ");
            if (line.contains("include_once"))
            {
               includeString = "include_once ";
            }

            // includePath is the path to the platform type definition
            // line.right removes the "include " or "include_once "from the string
            UtPath includePath(aBasePath.GetNormalizedPath() + '/' +
                               line.right(line.size() - includeString.size()).toStdString());
            auto   sources         = wizard::Project::Instance()->GetSourceCache().GetSources();
            bool   alreadyIncluded = false;
            includePath            = includePath.GetNormalizedPath();
            // checks if the new include file is already included in the scenario. If it isn't, include it.
            for (const auto& source : sources)
            {
               UtPath sourcePath(source.first);
               if (includePath == sourcePath.GetNormalizedPath())
               {
                  alreadyIncluded = true;
                  break;
               }
            }
            if (alreadyIncluded)
            {
               continue;
            }

            QString includeLine =
               includeString + QString::fromStdString(includePath.GetRealPath().GetNormalizedPath()) + "\n\n";
            definitions.append(includeLine);
         }
         else if ((splitStr.at(0) == "platform" || splitStr.at(0) == "radar_signature") && !insertDefinition)
         {
            inPlatformDefinition = true;
            // Check if the platform needs to be added.
            if (splitStr.size() < 2)
            {
               std::cout << "*****WARNING: Invalid platform definition\n";
            }
            else
            {
               auto it =
                  std::find_if(aPlatforms.begin(),
                               aPlatforms.end(),
                               [&](const QString& aPlatform)
                               { return splitStr[1] == aPlatform || splitStr[1] == (aPlatform + "_RADAR_SIGNATURE"); });
               if (it != aPlatforms.end())
               {
                  insertDefinition = true;
                  aPlatforms.erase(it);
               }
            }
         }
         else if (splitStr.at(0) == "end_platform" || splitStr.at(0) == "end_radar_signature")
         {
            inPlatformDefinition = false;
         }

         if (insertDefinition)
         {
            definitions.append(line + '\n');
            if (splitStr.at(0) == "end_platform")
            {
               definitions.append("\n");
               insertDefinition = false;
            }
         }
      } // while !in.atEnd
      inputFile.close();
   } // inputFile.open
   else
   {
      aInvalidFilesList.insert(definitionFilePath);
   }
   return definitions;
}

const QSet<QString> SpaceTools::SatelliteInserterHandler::GetDesignators()
{
   QSet<QString> allDesignator{};
   Scenario      scenario;
   InputReader   reader{scenario.GetScenario()};
   QStringList   spacePlatforms{reader.GetAllSpacePlatforms()};
   QString       designator{};
   for (const auto& platform : spacePlatforms)
   {
      auto platformProxy = InputUtils::GetPlatformProxy(platform.toStdString());
      if (platformProxy.Mover().GetValue().GetAttr("has_tle").GetBoolValue())
      {
         auto spaceMoverPtr = reader.GetPlatformSpaceMover(platformProxy.GetName());
         if (spaceMoverPtr)
         {
            designator = QString::fromStdString(spaceMoverPtr->GetDesignator());
         }
      }
      else
      {
         designator = QString::fromStdString(platformProxy.Mover().GetValue().GetAttr("designator").ValueToString());
      }
      if (!designator.isEmpty())
      {
         allDesignator.insert(designator.trimmed());
      }
   }
   return allDesignator;
}
