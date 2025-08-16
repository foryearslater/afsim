// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
/*******************************************************************************
*
*   File: TimeUtilities.cpp
*
*   Classification: UNCLASSIFIED
*
*   Details:
*
*   Contract Number:
*   Contractor Name: Radiance Technologies
*   Address:         350 Wynn Drive
*                    Huntsville, AL 35805
*
*   Abstract:
*
*******************************************************************************/


#include <string>
#include <vector>

#include "TimeUtilities.hpp"

namespace // {anonymous}
{
QString convertSecondsToValidTime(unsigned int totalSeconds)
{
   std::vector<unsigned int> components;

   // seconds
   components.push_back(totalSeconds % 60);
   totalSeconds -= components.back();
   // minutes
   components.push_back((totalSeconds % 3600) / 60);
   totalSeconds -= components.back() * 60;
   // hours
   components.push_back((totalSeconds % 86400) / 3600);
   totalSeconds -= components.back() * 3600;
   // days
   components.push_back(totalSeconds / 86400);

   std::string timeString;
   while (components.size() != 0)
   {
      unsigned int c = components.back();
      if (c < 10)
      {
         timeString += '0';
      }
      timeString += std::to_string(c) + ':';
      components.pop_back();
   }
   // remove extra colon
   timeString = timeString.substr(0, timeString.size() - 1);
   return QString::fromLocal8Bit(timeString.c_str());
}
}

QValidator::State ScenarioAnalyzer::TimeValidator::validate(QString& input, int& pos) const
{
   const QRegExp format1("([0-5]?[0-9]:)?[0-5]?[0-9]");  // sec <= 59 and optional minutes <= 59
   const QRegExp format2("([0-9]*:)?([0-1]?[0-9])|(2[0-3])(:[0-5]?[0-9]){2}"); // any number of days, hours <= 23, minutes <= 59, seconds <= 59
   const QRegExp format3("[0-9]{0,6}"); // an integer will be interpreted as seconds. 6 digits allows for 11 days.
   const QRegExp intermediate("([0-9]{1,2}:){0,3}[0-9]{0,2}");

   std::string inputString = input.toLocal8Bit().data();
   if (format1.exactMatch(input) || format2.exactMatch(input) || format3.exactMatch(input))
   {
      return QValidator::Acceptable;
   }
   else if (intermediate.exactMatch(input))
   {
      return QValidator::Intermediate;
   }
   else
   {
      return QValidator::Invalid;
   }
}

void ScenarioAnalyzer::TimeLabel::setTextAsValidTime(const QString& input)
{
   unsigned seconds = convertTimeToSeconds(input);
   QString validTime = convertSecondsToValidTime(seconds);
   setText(validTime);
}

// Assumes that TimeLineEdit's text contains only integers
// and colons. (The custom QVerifier ensure this is true.)
unsigned int ScenarioAnalyzer::TimeLabel::convertTimeToSeconds(const QString& input)
{
   // seconds per: second, minute, hour, day
   std::vector<unsigned int> multipliers{ 1, 60, 3600, 86400 };
   unsigned int seconds = 0;

   std::string timeString = input.toLocal8Bit().data();
   if (timeString.empty())
   {
      return 0;
   }

   if (timeString.back() == ':')
   {
      std::string front = timeString.substr(0, timeString.size() - 1);
      return convertTimeToSeconds(QString::fromLocal8Bit(front.c_str()));
   }

   for (const auto m : multipliers)
   {
      auto lastColon = timeString.find_last_of(':');
      if (lastColon == std::string::npos && !timeString.empty())
      {
         seconds += m * stoi(timeString);
         return seconds;
      }
      std::string str = timeString.substr(lastColon + 1);
      if (!str.empty())
      {
         seconds += m * stoi(str);
         timeString = timeString.substr(0, lastColon);
      }

   }
   return seconds;
}
