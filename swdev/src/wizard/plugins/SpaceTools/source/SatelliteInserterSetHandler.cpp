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

#include "SatelliteInserterSetHandler.hpp"

#include <sstream>
#include <utility>

#include "EditorPrefObject.hpp"
#include "InputReader.hpp"
#include "InputUtils.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "SpaceToolsScenario.hpp"
#include "TextSource.hpp"
#include "UtInputString.hpp"
#include "WkfEnvironment.hpp"
#include "WsfSpaceMoverBase.hpp"

SpaceTools::SatelliteInserterSetHandler::SatelliteInserterSetHandler(const QStringList& aFileList)
{
   for (const auto& file : aFileList)
   {
      LoadPatchFile(file.toStdString());
   }
}

//! Update the TLEs in the current scenario
void SpaceTools::SatelliteInserterSetHandler::UpdateTLEs()
{
   WsfPProxy* proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   if (proxyPtr)
   {
      Scenario    scenario;
      InputReader reader(scenario.GetScenario());

      QStringList spaceTLE_Platforms = reader.GetAllSpacePlatforms(true);
      auto        spacePlatforms     = SortByLatestStartingPosition(spaceTLE_Platforms);
      for (const auto& spacePlatform : spacePlatforms)
      {
         auto spaceMoverPtr = reader.GetPlatformSpaceMover(spacePlatform.toStdString());
         if (spaceMoverPtr)
         {
            std::string designator = spaceMoverPtr->GetDesignator();
            UpdatePlatformTLE(designator, spacePlatform);
         }
      }
   }
}

QStringList SpaceTools::SatelliteInserterSetHandler::SortByLatestStartingPosition(const QStringList& aSpacePlatformsList)
{
   std::vector<std::pair<size_t, QString>> sizeSet;

   for (const auto& platform : aSpacePlatformsList)
   {
      WsfPM_Platform plat = InputUtils::GetPlatformProxy(platform.toStdString());
      if (!plat.IsNull())
      {
         UtTextDocumentRange sourceRange = plat.GetSourceDocumentRange();
         sizeSet.emplace_back(sourceRange.GetBegin(), platform);
      }
   }
   std::sort(sizeSet.begin(),
             sizeSet.end(),
             [&](std::pair<size_t, QString> a, std::pair<size_t, QString> b) { return a.first > b.first; });

   QStringList sortedList;
   for (const auto& pair : sizeSet)
   {
      sortedList << pair.second;
   }
   return sortedList;
}

void SpaceTools::SatelliteInserterSetHandler::UpdatePlatformTLE(const std::string& aDesignator, const QString& aPlatformName)
{
   auto iter = mUpdatedTLEs.find(aDesignator);
   if (iter != mUpdatedTLEs.end())
   {
      UtTextDocumentRange range;
      size_t              column;
      std::tie(range, column) = GetSourceRange(aPlatformName.toStdString());

      if (range.Valid())
      {
         std::string replacement = GetReplacementText(iter->second, column);
         ReplaceText(range, replacement);
      }
   }
}

std::pair<UtTextDocumentRange, size_t> SpaceTools::SatelliteInserterSetHandler::GetSourceRange(const std::string& aPlatformName)
{
   static std::string cBLOCK_OPEN{"orbit"};
   static std::string cBLOCK_CLOSE{"end_orbit"};

   WsfPM_Platform      platNode = InputUtils::GetPlatformProxy(aPlatformName);
   UtTextDocumentRange range{};
   size_t              column{0};
   if (!platNode.IsNull())
   {
      UtTextDocumentRange sourceRange = platNode.GetSourceDocumentRange();
      std::string         sourceText  = InputUtils::HandleComments(sourceRange.Text());

      size_t startPos = sourceText.find(cBLOCK_OPEN);
      if (startPos != std::string::npos)
      {
         size_t endPos = sourceText.find(cBLOCK_CLOSE, startPos);
         if (endPos != std::string::npos)
         {
            size_t line{};
            sourceRange.mSource->PositionToLineColumn(sourceRange.GetBegin() + startPos, line, column);
            startPos -= column;
            range = UtTextDocumentRange{sourceRange.mSource,
                                        sourceRange.GetBegin() + startPos,
                                        sourceRange.GetBegin() + endPos + cBLOCK_CLOSE.size() - 1};
         }
      }
   }

   return std::make_pair(range, column);
}
std::string SpaceTools::SatelliteInserterSetHandler::GetReplacementText(const UtTwoLineElement& aTLE, size_t aColumn)
{
   std::stringstream ss{};
   ss << aTLE;
   std::string descrip{};
   std::string line1{};
   std::string line2{};

   if (!aTLE.GetDescriptor().empty())
   {
      std::getline(ss, descrip);
   }
   std::getline(ss, line1);
   std::getline(ss, line2);

   size_t indentSize = ut::cast_to_size_t(wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace());

   std::string spaces(aColumn, ' ');
   std::string indented(aColumn + indentSize, ' ');

   std::stringstream retval{};
   retval << spaces << "orbit\n";
   if (!descrip.empty())
   {
      retval << indented << descrip << "\n";
   }
   retval << indented << line1 << "\n";
   retval << indented << line2 << "\n";
   retval << spaces << "end_orbit";

   return retval.str();
}

void SpaceTools::SatelliteInserterSetHandler::ReplaceText(const UtTextDocumentRange& aRange, const std::string& aText)
{
   std::string         sourcePath        = aRange.mSource->GetFilePath().GetSystemPath();
   wizard::TextSource* platformSourcePtr = wizard::ProjectWorkspace::Instance()->FindSource(sourcePath, false, true);
   platformSourcePtr->Replace(aRange, aText);
}

void SpaceTools::SatelliteInserterSetHandler::LoadPatchFile(const std::string& aFileName)
{
   std::ifstream     in{aFileName};
   std::stringstream ss{};
   ss << "\n" << in.rdbuf(); // The extra blank line is needed because of the implementation of ReadOrbitalData.
   UtInput input{};
   input.PushInput(ut::make_unique<UtInputString>(ss.str()));

   while (true)
   {
      try
      {
         UtTwoLineElement tle{};
         tle.ReadOrbitalData(input);

         auto iter = mUpdatedTLEs.find(tle.GetDesignator());
         if (iter != mUpdatedTLEs.end())
         {
            // The designator is already available, so we compare epoch, and keep the latest.
            if (iter->second.GetEpoch() < tle.GetEpoch())
            {
               iter->second = tle;
            }
         }
         else
         {
            // No current record, so add it.
            mUpdatedTLEs[tle.GetDesignator()] = tle;
         }
      }
      catch (std::exception&)
      {
         break;
      }
   }
}
