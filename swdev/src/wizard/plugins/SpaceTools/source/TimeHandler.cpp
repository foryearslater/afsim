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

#include "TimeHandler.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

#include <QString>
#include <QStringList>

#include "Editor.hpp"
#include "InputReader.hpp"
#include "InputUtils.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "SpaceToolsScenario.hpp"
#include "TextSource.hpp"
#include "TextSourceCache.hpp"
#include "UtCalendar.hpp"
#include "UtPath.hpp"
#include "UtTextDocument.hpp"
#include "UtTwoLineElement.hpp"
#include "WkfEnvironment.hpp"
#include "WsfDateTime.hpp"
#include "WsfSpaceMoverBase.hpp"

namespace // anonymous
{
std::string GetReplacementText(const UtCalendar& aNewCalendar, int aOutputType)
{
   UtCalendar prevCal{wizard::Project::Instance()->GetStartDateTime()};

   std::stringstream retval{};
   int               year{prevCal.GetYear()};
   int               month{prevCal.GetMonth()};
   std::string       monthStr = UtCalendar::cMONTH_NAMES[month];
   int               day{prevCal.GetDay()};
   int               hour{prevCal.GetHour()};
   int               min{prevCal.GetMinute()};
   double            sec{prevCal.GetSecond()};

   retval << std::setfill('0');
   retval << "\n# Previous start date and time:\n# start_date ";
   retval << monthStr << ' ' << day << ' ' << year;
   retval << "\n# start_time " << std::setw(2) << hour;
   retval << ':' << std::setw(2) << min;
   retval << ':' << std::setw(2) << std::setprecision(6) << sec << '\n';

   switch (aOutputType)
   {
   case SpaceTools::TimeHandler::cOUTPUT_TIME_NOW:
      retval << "start_time_now\n";
      break;
   case SpaceTools::TimeHandler::cOUTPUT_EPOCH:
      retval << "start_epoch " << std::setprecision(13) << aNewCalendar.GetEpoch() << '\n';
      break;
   case SpaceTools::TimeHandler::cOUTPUT_DATE_TIME:
      aNewCalendar.GetDate(year, month, day);
      aNewCalendar.GetTime(hour, min, sec);
      monthStr = UtCalendar::cMONTH_NAMES[month];
      retval << std::setfill('0');
      retval << "start_date " << monthStr << ' ' << day << ' ' << year;
      retval << "\nstart_time " << std::setw(2) << hour;
      retval << ':' << std::setw(2) << min;
      retval << ':' << std::setw(2) << std::setprecision(6) << sec << '\n';
      break;
   }
   return retval.str();
}

void RemoveAllStartDateTimeEpoch()
{
   static std::string cSTART_TIME{"start_time"};
   static std::string cSTART_DATE{"start_date"};
   static std::string cSTART_EPOCH{"start_epoch"};
   static std::string cSTART_NOW{"start_time_now"};
   static std::string cEND_LINE{"\n"};


   const wizard::TextSourceCache::SourceMap& sources =
      wizard::ProjectWorkspace::Instance()->GetSourceCache()->GetSources();
   for (const auto& source : sources)
   {
      std::vector<UtTextDocumentRange> erasures{};
      wizard::TextSource*              sourcePtr = source.second;
      UtTextDocument*                  docPtr    = sourcePtr->GetSource();
      if (docPtr->GetText().Empty())
      {
         continue;
      }
      UtTextDocumentRange sourceRange{docPtr, 0, docPtr->Size()};

      std::string sourceText = SpaceTools::InputUtils::HandleComments(sourceRange.Text());

      size_t startTimePos{sourceText.find(cSTART_TIME)};
      size_t startDatePos{sourceText.find(cSTART_DATE)};
      size_t startEpochPos{sourceText.find(cSTART_EPOCH)};
      size_t startNowPos{sourceText.find(cSTART_NOW)};
      size_t startPos{std::min<size_t>({startTimePos, startDatePos, startEpochPos, startNowPos})};
      size_t column{0};
      size_t line{};

      while (startPos != std::string::npos)
      {
         size_t endPos = sourceText.find(cEND_LINE, startPos);
         if (endPos == std::string::npos)
         {
            endPos = sourceText.size() - 3;
         }
         sourceRange.mSource->PositionToLineColumn(sourceRange.GetBegin() + startPos, line, column);
         UtTextDocumentRange range{sourceRange.mSource,
                                   sourceRange.GetBegin() + startPos - column,
                                   sourceRange.GetBegin() + endPos + cEND_LINE.size() - 1};
         erasures.emplace_back(range);

         startTimePos  = sourceText.find(cSTART_TIME, endPos - 1);
         startDatePos  = sourceText.find(cSTART_DATE, endPos - 1);
         startEpochPos = sourceText.find(cSTART_EPOCH, endPos - 1);
         startNowPos   = sourceText.find(cSTART_NOW, endPos - 1);
         startPos      = std::min<size_t>({startTimePos, startDatePos, startEpochPos, startNowPos});
      }

      for (auto iter = erasures.rbegin(); iter != erasures.rend(); ++iter)
      {
         sourcePtr->Replace(*iter, "");
      }
   }
}

void ReplaceText(const std::string& aText)
{
   RemoveAllStartDateTimeEpoch();
   // Write the new start date/time at the end of the first startup file
   auto startupFile = wizard::Project::Instance()->GetStartupFiles().at(0);
   auto editorPtr   = wizard::ProjectWorkspace::Instance()->GotoFile(startupFile.GetNormalizedPath());
   editorPtr->moveCursor(QTextCursor::End);
   editorPtr->textCursor().insertText(QString::fromStdString(aText));
   editorPtr->centerCursor();
}

} // namespace
//! Update the start date/time/epoch of the scenario. Removes all other instances relating to the start date/time/epoch
//!
//! @param aNewCalendar The calendar holding the new date/time/epoch
//! @param aOutputType The type of output.
void SpaceTools::TimeHandler::UpdateStartDateTime(const UtCalendar& aNewCalendar, int aOutputType)
{
   std::string replacementText{GetReplacementText(aNewCalendar, aOutputType)};
   ReplaceText(replacementText);
}

UtCalendar SpaceTools::TimeHandler::GetLatestEpoch()
{
   UtCalendar latestEpochCal{};
   WsfPProxy* proxyPtr = wizard::ProxyWatcher::GetActiveProxy();
   if (proxyPtr)
   {
      Scenario    scenario;
      InputReader reader(scenario.GetScenario());
      double      latestEpoch{-1};
      QStringList spacePlatforms = reader.GetAllSpacePlatforms(false);

      for (const auto& spacePlatform : spacePlatforms)
      {
         auto platformPtr = reader.GetPlatformSpaceMover(spacePlatform.toStdString());
         if (platformPtr)
         {
            auto epoch = platformPtr->GetInitialOrbitalState().GetEpoch().GetEpoch();
            if (epoch > latestEpoch)
            {
               latestEpoch = epoch;
            }
         }
      }

      if (latestEpoch < 0)
      {
         UtCalendar scenarioDateTime{wizard::Project::Instance()->GetStartDateTime()};
         UtCalendar now{};
         now.SetCurrentDateAndTime();
         latestEpochCal = scenarioDateTime < now ? now : scenarioDateTime;
      }
      else
      {
         latestEpochCal.SetEpoch(latestEpoch);
      }
   }
   return latestEpochCal;
}
