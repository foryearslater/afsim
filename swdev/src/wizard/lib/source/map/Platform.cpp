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

#include "Platform.hpp"

#include <unordered_set>

#include <QTextBlock>
#include <QTextDocumentFragment>

#include "Editor.hpp"
#include "EditorHelpers.hpp"
#include "EditorPrefObject.hpp"
#include "Environment.hpp"
#include "FindReferencesResultDialog.hpp"
#include "FindTextRequest.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProxyChange.hpp"
#include "ProxyWatcher.hpp"
#include "UtEllipsoidalEarth.hpp"
#include "UtLLAPos.hpp"
#include "UtSphericalEarth.hpp"
#include "VaViewerManager.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPProxyNode.hpp"
#include "zone/WkfAttachmentZonePolygon.hpp"
#include "zone/WkfZonePoint.hpp"

namespace std
{
template<>
struct hash<QString>
{
   std::size_t operator()(const QString& s) const { return qHash(s); }
};
} // namespace std
namespace
{
QString AltitudeReferenceToString(int aAltRef)
{
   switch (aAltRef)
   {
   case 1:
      return " msl";
   case 2:
      return " agl";
   default:
      return "";
   }
}
} // namespace

wizard::Platform::Platform(const std::string& aName, unsigned int aId, WsfPM_Platform& aPlatform)
   : wkf::Platform(aName)
   , mId(aId)
   , mPlatform(aPlatform)
   , mDirty(true)
   , mWrapped(false)
   , mSimTime(-1.0)
   , mWaitingForDelete(false)
{
   SetType<Platform>();

   mProxyCallbacks.Add(WatchProxy(aPlatform.GetPath()).AnyUpdate.Connect(&Platform::OnProxyModified, this));
   mProxyCallbacks.Add(wizard::WatchProxy(WsfPM_Root(aPlatform.Root()).routes().GetPath())
                          .AnyUpdate.Connect(&Platform::OnGlobalRouteListChange, this));

   auto* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      vaEnv.GetStandardScenario()->AddPlatformWithIndex(this, mId);
   }

   PlatformInitialize();
   wkf::PlatformList temp{this};
   wkf::Observer::PlatformsAdded(temp);
   wkf::Observer::PlatformConfigured(this);
}

wizard::Platform::~Platform()
{
   wizEnv.DeletePlatform(GetName().c_str());
   wkfEnv.GetResourceManager().ReleaseRequests(mDtedId);
}


void wizard::Platform::PlatformInitialize()
{
   mDirty              = true;
   std::string newName = mPlatform.GetName();

   SetName(newName);
   SetIcon(mPlatform.Icon());
   SetSide(mPlatform.Side());
   SetIndex(mId);
   SetupGroupings();

   Initialize();
   mDtedId = wkfEnv.GetResourceManager().Register();
   wkf::Observer::PlatformPropertiesChanged(this);

   wizEnv.UpdatePlatform(GetName().c_str());
   mLastLat = GetLat();
   mLastLon = GetLon();
   mDirty   = false;
}

void wizard::Platform::UpdateRoute(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
{
   if (mPlatform.IsValid() && mPlatform.Mover().IsValid())
   {
      WsfPM_Route route = mPlatform.Mover().InitialRoute();
      if (route.IsValid())
      {
         int wayptCount = static_cast<int>(route.WaypointCount());
         // store off the initial waypoint ranges, these can be modified if information is added to waypoints
         std::vector<UtTextDocumentRange> initialRanges;
         for (int idx = 0; idx < wayptCount; ++idx)
         {
            WsfPM_Waypoint      curwayptNode = route.GetWaypoint(idx);
            UtTextDocumentRange curNodeRange = GetAttributeDocumentRange(curwayptNode);
            initialRanges.emplace_back(curNodeRange);
         }

         // Get the entire initial route and store it in a QString
         std::vector<std::pair<int, int>> offsetRanges;
         QTextCursor                      cursor = GetEditor()->textCursor();
         cursor.setPosition(initialRanges[0].GetBegin(), QTextCursor::MoveAnchor);
         cursor.setPosition(initialRanges[initialRanges.size() - 1].GetEnd() + 1, QTextCursor::KeepAnchor);
         QString routeCopy  = cursor.selection().toPlainText();
         QString routeCopy2 = routeCopy;
         // Convert text document range to routeCopy QString range
         for (const auto& it : initialRanges)
         {
            int begin = it.GetBegin() - initialRanges[0].GetBegin();
            int end   = it.GetEnd() - initialRanges[0].GetBegin();
            offsetRanges.emplace_back(std::pair<int, int>(begin, end));
         }

         // if the user adds/removes waypoints in the text editor while the route browser is open, handle separately
         bool userDataCorruption = false;
         int  userModifiedSize   = offsetRanges.size();
         for (const auto& it : mQueuedOperations)
         {
            (it.second) ? userModifiedSize++ : userModifiedSize--;
            if (it.first < 0 || it.first > userModifiedSize)
            {
               userDataCorruption = true;
               break;
            }
         }
         if (userModifiedSize != aRouteInfo.mWaypoints.size())
         {
            userDataCorruption = true;
         }
         if (!userDataCorruption)
         {
            // Handle Insertion/Removal of Waypoints, using the copied route string (first = index, second = insertion/removal)
            for (const auto& it : mQueuedOperations)
            {
               if (it.second) // insertion
               {
                  // update the route string
                  QString placeholder = "\nposition 0n 0w\n";
                  if (it.first == static_cast<int>(offsetRanges.size()))
                  {
                     int beginIndex = offsetRanges[it.first - 1].second + 1;
                     offsetRanges.emplace_back(std::pair<int, int>(beginIndex, beginIndex + placeholder.size() - 1));
                     routeCopy += placeholder;
                  }
                  else
                  {
                     int textIndex = offsetRanges[it.first].first;
                     routeCopy.insert(textIndex, placeholder);
                     // add the range of the new waypoint
                     offsetRanges.insert(offsetRanges.begin() + it.first,
                                         std::pair<int, int>(textIndex, textIndex + placeholder.size() - 1));
                     // update the ranges of all subsequent waypoints
                     for (unsigned int i = it.first + 1; i < offsetRanges.size(); ++i)
                     {
                        offsetRanges[i].first += placeholder.size();
                        offsetRanges[i].second += placeholder.size();
                     }
                  }
               }
               else // removal
               {
                  // update the route string
                  int length = offsetRanges[it.first].second - offsetRanges[it.first].first + 1;
                  routeCopy.remove(offsetRanges[it.first].first, length);
                  offsetRanges.erase(offsetRanges.begin() + it.first);
                  // remove the length from subsequent waypoints
                  for (auto i = ut::cast_to_size_t(it.first); i < offsetRanges.size(); ++i)
                  {
                     offsetRanges[i].first -= length;
                     offsetRanges[i].second -= length;
                  }
               }
            }
         }
         else // when user messes with data, omit formatting and comments, prioritize having correct route data and
              // avoiding crashes
         { // TODO: a complete replacement of the route can potentially also be used to modify initially invalid routes
            offsetRanges.clear();
            routeCopy = "";
            for (int i = 0; i < aRouteInfo.mWaypoints.size(); ++i)
            {
               QString placeholder = "\nposition 0n 0w\n";
               routeCopy += placeholder;
               int start = i * placeholder.size();
               int end   = start + placeholder.size() - 1;
               offsetRanges.emplace_back(std::pair<int, int>(start, end));
            }
         }
         mQueuedOperations.clear();

         // retain comments, misc. commands that may be placed between/after waypoints
         std::vector<QString> afterWaypoints;
         for (unsigned int y = 0; y < offsetRanges.size() - 1; ++y)
         {
            int startPos = offsetRanges[y].second + 1;
            int endPos   = offsetRanges[y + 1].first;
            if (startPos != endPos)
            {
               QString selectedStr = routeCopy.mid(startPos, endPos - startPos);
               // get rid of whitespace after last non whitespace character
               int lastIndex = selectedStr.lastIndexOf(QRegExp("\\S"));
               selectedStr   = selectedStr.mid(0, lastIndex + 1);
               afterWaypoints.push_back(selectedStr);
            }
            else
            {
               afterWaypoints.emplace_back("");
            }
         }

         const QString cSLASH_COMMENT = "SLASHCOMMENTIDENTIFIER";   // /* some comment */
         const QString cHASH_COMMENT  = "HASHTAGCOMMENTIDENTIFIER"; // # some comment
         // list of commands that are modified by the route browser
         const std::unordered_set<QString> keyCommands = {"label",
                                                          "position",
                                                          "altitude",
                                                          "turn",
                                                          "speed",
                                                          "linear_acceleration",
                                                          "radial_acceleration",
                                                          "bank_angle_limit",
                                                          "turn_g_limit",
                                                          "climb_rate",
                                                          "switch_on_passing",
                                                          "switch_on_approach",
                                                          "extrapolate",
                                                          "stop",
                                                          "remove"};
         // list of commands that are not modified by the route browser, but should not be overwritten
         const std::unordered_set<QString> otherCommands = {"aux_data",
                                                            "end_aux_data",
                                                            "node_id",
                                                            "time",
                                                            "time_to_point",
                                                            "distance",
                                                            "execute",
                                                            "dive_rate",
                                                            "pause_time",
                                                            "depth"};

         // lambda to find associated data following a position, label, speed, etc. command
         auto GetNextRealTokenLambda =
            [cSLASH_COMMENT, cHASH_COMMENT, keyCommands, otherCommands](int aIndex, const QStringList& aTokenList)
         {
            if (aIndex >= 0)
            {
               for (int i = aIndex + 1; i < aTokenList.size(); ++i)
               {
                  if (keyCommands.find(aTokenList[i]) != keyCommands.end() ||
                      otherCommands.find(aTokenList[i]) != otherCommands.end())
                  {
                     return -1;
                  }
                  if (!aTokenList[i].contains(QRegExp("\\r\\n|\\r|\\n")) && aTokenList[i] != cSLASH_COMMENT &&
                      aTokenList[i] != cHASH_COMMENT)
                  {
                     return i;
                  }
               }
            }
            return -1;
         };
         // lambda for removal that deletes the next content token and skips format tokens, other commands and comments
         auto RemoveDataLambda = [&](int aIndex, QStringList& aTokenList)
         {
            int dataIndex = GetNextRealTokenLambda(aIndex, aTokenList);
            if (dataIndex >= 0 && dataIndex < aTokenList.size())
            {
               aTokenList.removeAt(dataIndex);
               return true;
            }
            return false;
         };
         // lambda for removing an entire command (with units), count corresponds to number of data fields appearing
         // after the command name
         auto RemoveCommandLambda = [&](int aIndex, QStringList& aTokenList)
         {
            // keep removing tokens until another command is hit
            while (RemoveDataLambda(aIndex, aTokenList))
            {
            }
            aTokenList.removeAt(aIndex);
         };
         // lambda for removing earlier commands if they are defined more than once
         auto RemoveDuplicateLambda = [&](const QStringList& aCommandNameList, QStringList& aTokenList)
         {
            // get all indices that match the command name
            std::vector<int> indexMatches;
            for (const auto& it : aCommandNameList)
            {
               int i = 0;
               while (i != -1)
               {
                  i = aTokenList.indexOf(it, i);
                  if (i != -1)
                  {
                     indexMatches.emplace_back(i++);
                  }
               }
            }
            if (indexMatches.size() > 1)
            {
               std::sort(indexMatches.begin(), indexMatches.end());
               for (int j = indexMatches.size() - 2; j >= 0; --j)
               {
                  RemoveCommandLambda(indexMatches[j], aTokenList);
               }
            }
         };

         // lambda for modifying route information that has units
         auto ModifyRouteCommandTextLambda =
            [&](int aIndex, QStringList& aTokenList, const QString& aDataValue, const QString& aStandardUnit)
         {
            int valueIndex = GetNextRealTokenLambda(aIndex, aTokenList);
            if (valueIndex >= 0)
            {
               QStringList dataStrList = aDataValue.split(QRegExp("[ \t]{1,}"));
               if (!dataStrList.empty())
               {
                  aTokenList[valueIndex] = dataStrList[0];
                  int unitIndex          = GetNextRealTokenLambda(valueIndex, aTokenList);
                  if (unitIndex >= 0) // if no unit is defined in the route browser, use the standard unit
                  {
                     (dataStrList.size() > 1) ? aTokenList[unitIndex] = dataStrList[1] :
                                                aTokenList[unitIndex] = aStandardUnit;
                  }
                  return true;
               }
            }
            return false;
         };
         // lambda for adding new information to the end of the token list
         auto AddRouteCommandTextLambda =
            [](QStringList& aTokenList, const QString& aDataCommand, const QString& aData, const QString& aStandardUnit)
         {
            QStringList dataStrList = aData.split(QRegExp("[ \t]{1,}"));
            if (!dataStrList.empty())
            {
               aTokenList.push_back("\n");
               aTokenList.push_back(aDataCommand);
               aTokenList.push_back(dataStrList[0]);
               (dataStrList.size() > 1) ? aTokenList.push_back(dataStrList[1]) : aTokenList.push_back(aStandardUnit);
            }
         };
         // lambda to check the various wkf::WaypointValueType
         auto CheckValueTypeLambda = [](double aValue)
         {
            return (aValue != wkf::WaypointValueType::cNOT_SET && aValue != wkf::WaypointValueType::cUSE_PREVIOUS &&
                    aValue != wkf::WaypointValueType::cUSE_DEFAULT);
         };
         // lambda to execute text editor insertion/deletion/update logic
         auto UpdateDataLambda = [&](int            aVarLoc,
                                     QStringList&   aTokenList,
                                     const QString& aCommandName,
                                     const double&  aRawData,
                                     const QString& aDataStr,
                                     const QString& aStandardUnit)
         {
            bool updatedData = false;
            if (aVarLoc != -1)
            {
               if (CheckValueTypeLambda(aRawData))
               { // if the data is valid and the command name exists in the text editor, update the data
                  updatedData = ModifyRouteCommandTextLambda(aVarLoc, aTokenList, aDataStr, aStandardUnit);
               }
               else
               { // if the data is invalid and the command name exists, delete the command entirely
                  int index = aTokenList.indexOf(aCommandName);
                  RemoveCommandLambda(index, aTokenList);
               }
            }
            else if (!updatedData && CheckValueTypeLambda(aRawData))
            { // if the data is valid, but the command name doesn't exist, add it to the end of the waypoint definition
               AddRouteCommandTextLambda(aTokenList, aCommandName, aDataStr, aStandardUnit);
            }
         };

         // find the number of spaces to indent based on where the first waypoint command starts
         // note: if this waypoint is unusually spaced, all future waypoints in the route will be too, but this is on the user
         cursor = GetEditor()->textCursor();
         cursor.setPosition(initialRanges[0].GetBegin(), QTextCursor::MoveAnchor);
         QString firstLine     = cursor.block().text();
         int     defaultIndent = 0;

         // Calculate how large the indents on each new line should be based on the position of the first indentation
         std::vector<int> defaultIndentCommands = {firstLine.indexOf("label"),
                                                   firstLine.indexOf("position"),
                                                   firstLine.indexOf("mgrs_coordinate")};
         defaultIndentCommands.erase(std::remove_if(defaultIndentCommands.begin(),
                                                    defaultIndentCommands.end(),
                                                    [](int aDefault) { return aDefault < 0; }),
                                     defaultIndentCommands.end());
         defaultIndent = (defaultIndentCommands.size() > 0) ?
                            *std::min_element(defaultIndentCommands.begin(), defaultIndentCommands.end()) :
                            0;

         const int         cTABLENGTH = 3;
         const std::string cINDENT(defaultIndent, ' ');
         const std::string cINDENT_TAB(defaultIndent + cTABLENGTH, ' ');

         QStringList newWaypointText;
         for (int i = 0; i < aRouteInfo.mWaypoints.size(); ++i)
         {
            UtLLAPos curPos;
            double   newLatPos = aRouteInfo.mWaypoints[i].mLat;
            double   newLonPos = aRouteInfo.mWaypoints[i].mLon;
            double   newAlt    = aRouteInfo.mWaypoints[i].mAlt;

            int     length      = offsetRanges[i].second - offsetRanges[i].first + 1;
            QString selectedStr = routeCopy.mid(offsetRanges[i].first, length);
            selectedStr         = selectedStr.replace(QRegExp("\\r\\n|\\r|\\n"), "\n");

            // Extract all slash comments from selection, save them, and replace them with a constant token
            // if nested comments exist, wizard only interprets the outermost comment (the rest is interpreted as plain text)
            int                 startIndex = -1;
            std::queue<QString> slashComments;
            std::queue<QString> hashComments;
            bool                isSlashComment = true;
            for (int i = 0; i < selectedStr.size() - 1; ++i)
            {
               if (startIndex == -1)
               {
                  if (selectedStr[i] == "/" && selectedStr[i + 1] == "*")
                  {
                     startIndex     = i++;
                     isSlashComment = true;
                  }
                  else if (selectedStr[i] == "#")
                  {
                     startIndex     = i;
                     isSlashComment = false;
                  }
               }
               else
               {
                  if (isSlashComment && selectedStr[i] == "*" && selectedStr[i + 1] == "/")
                  {
                     int     length  = (i + 2) - startIndex;
                     QString comment = selectedStr.mid(startIndex, length);
                     slashComments.push(comment);
                     QString insertStr = " " + cSLASH_COMMENT + " ";
                     selectedStr.replace(startIndex, length, insertStr);
                     i          = startIndex - 1 + insertStr.size();
                     startIndex = -1;
                  }
                  else if (!isSlashComment && selectedStr[i] == "\n")
                  {
                     int     length  = i - startIndex; //(dont include the newline in the token)
                     QString comment = selectedStr.mid(startIndex, length);
                     hashComments.push(comment);
                     QString insertStr = " " + cHASH_COMMENT + " ";
                     selectedStr.replace(startIndex, length, cHASH_COMMENT);
                     i          = startIndex - 1 + insertStr.size();
                     startIndex = -1;
                  }
               }
            }

            // add spaces before/after newlines to guarantee they act as their own separate token
            selectedStr = selectedStr.replace("\n", " \n ");
            // create tokens using whitespace (excluding newlines) as separators: occasionally creates empty strings as
            // tokens so remove those
            QStringList tokenList = selectedStr.split(QRegExp("[ \t]{1,}"));
            tokenList.removeAll(QString(""));
            if (!tokenList.isEmpty())
            {
               // Remove duplicate single Commands
               for (const auto& it : keyCommands)
               {
                  QStringList singleCommand;
                  singleCommand.push_back(it);
                  RemoveDuplicateLambda(singleCommand, tokenList);
               }
               // Remove duplicate commands that are mutually exclusive
               QStringList multiCommand;
               multiCommand.push_back("msl");
               multiCommand.push_back("agl");
               RemoveDuplicateLambda(multiCommand, tokenList);
               multiCommand.clear();
               multiCommand.push_back("radial_acceleration");
               multiCommand.push_back("bank_angle_limit");
               multiCommand.push_back("turn_g_limit");
               RemoveDuplicateLambda(multiCommand, tokenList);
               multiCommand.clear();
               multiCommand.push_back("switch_on_passing");
               multiCommand.push_back("switch_on_approach");
               RemoveDuplicateLambda(multiCommand, tokenList);
               multiCommand.clear();
               multiCommand.push_back("extrapolate");
               multiCommand.push_back("remove");
               multiCommand.push_back("stop");
               RemoveDuplicateLambda(multiCommand, tokenList);

               int varLoc;
               // Update or remove existing labels
               varLoc = tokenList.indexOf("label");
               if (varLoc != -1)
               {
                  // update waypoint label, or remove the existing one if route browser field is blank
                  if (!aRouteInfo.mWaypoints[i].mLabel.isEmpty() && varLoc + 1 < tokenList.size())
                  {
                     int valueIndex = GetNextRealTokenLambda(varLoc, tokenList);
                     if (valueIndex != -1)
                     {
                        tokenList[valueIndex] = aRouteInfo.mWaypoints[i].mLabel;
                     }
                  }
                  else
                  {
                     int labelIndex = tokenList.indexOf("label");
                     RemoveDataLambda(labelIndex, tokenList);
                     tokenList.removeAt(labelIndex);
                  }
               }
               else
               { // add label if one didn't already exist
                  if (!aRouteInfo.mWaypoints[i].mLabel.isEmpty())
                  {
                     tokenList.push_front("\n");
                     tokenList.push_front(aRouteInfo.mWaypoints[i].mLabel);
                     tokenList.push_front("label");
                  }
               }
               // TODO: Do we need to handle overwriting existing routes with mgrs, offset, etc. commands?
               // update existing positions
               VaPosition newPos;
               newPos.SetLat(newLatPos);
               newPos.SetLon(newLonPos);
               newPos.SetAlt(newAlt);

               varLoc = tokenList.indexOf("position");
               if (varLoc != -1)
               {
                  int latIndex = GetNextRealTokenLambda(varLoc, tokenList);
                  int lonIndex = GetNextRealTokenLambda(latIndex, tokenList);
                  if (latIndex >= 0)
                  {
                     std::ostringstream ossLat;
                     ossLat << newPos.GetLat();
                     tokenList[latIndex] = QString(ossLat.str().c_str());
                  }
                  if (lonIndex >= 0)
                  {
                     std::ostringstream ossLon;
                     ossLon << newPos.GetLon();
                     tokenList[lonIndex] = QString(ossLon.str().c_str());
                  }
               }
               // There is no MGRS field in the route browser itself. Convert the coordinates to MGRS prior to modifying
               // the text editor
               varLoc = tokenList.indexOf("mgrs_coordinate");
               if (varLoc != -1)
               {
                  int coordIndex = GetNextRealTokenLambda(varLoc, tokenList);
                  if (coordIndex >= 0)
                  {
                     std::string mgrsPos;
                     UtEllipsoidalEarth::ConvertLLToMGRS(newLatPos, newLonPos, mgrsPos);
                     tokenList[coordIndex] = QString::fromStdString(mgrsPos);
                  }
               }

               // update altitude
               varLoc = tokenList.lastIndexOf("altitude");
               std::ostringstream ossAlt;
               UtLengthValue      altCopy = aRouteInfo.mWaypoints[i].mAlt;
               if (i == 0 && !CheckValueTypeLambda(aRouteInfo.mWaypoints[0].mAlt))
               {
                  altCopy = 0;
               }
               ossAlt << newPos.GetAlt();
               QString altStr = QString(ossAlt.str().c_str());
               UpdateDataLambda(varLoc,
                                tokenList,
                                "altitude",
                                altCopy,
                                altStr,
                                QString::fromStdString(UtUnitLength::FindUnitName(UtUnitLength::cSTANDARD_UNIT_ID)));

               // update altitude type
               int mslIndex      = tokenList.lastIndexOf("msl");
               int aglIndex      = tokenList.lastIndexOf("agl");
               varLoc            = std::max(mslIndex, aglIndex);
               int     altRef    = aRouteInfo.mWaypoints[i].mAltRef;
               QString newAltRef = "";
               if (altRef == 1)
               {
                  newAltRef = "msl";
               }
               else if (altRef == 2)
               {
                  newAltRef = "agl";
               }
               if (varLoc != -1)
               {
                  if (newAltRef.isEmpty())
                  {
                     tokenList.removeAt(varLoc);
                  }
                  else
                  {
                     tokenList[varLoc] = newAltRef;
                  }
               }
               else if (!newAltRef.isEmpty() && CheckValueTypeLambda(aRouteInfo.mWaypoints[i].mAlt))
               {
                  int altIndex  = tokenList.lastIndexOf("altitude");
                  int dataIndex = GetNextRealTokenLambda(altIndex, tokenList);
                  int unitIndex = GetNextRealTokenLambda(dataIndex, tokenList);
                  (unitIndex + 1 < tokenList.size()) ? tokenList.insert(unitIndex + 1, newAltRef) :
                                                       tokenList.push_back(newAltRef);
               }

               // update speed
               varLoc                      = tokenList.lastIndexOf("speed");
               double       convertedSpeed = UtUnitSpeed::ConvertFromStandard(aRouteInfo.mWaypoints[i].mSpeed,
                                                                        aRouteInfo.mWaypoints[i].mSpeed.GetUnit());
               UtSpeedValue speedCopy      = aRouteInfo.mWaypoints[i].mSpeed;
               if (i == 0 && !CheckValueTypeLambda(convertedSpeed))
               { // for the route to show up, the first point must have a defined speed
                  convertedSpeed = 50;
                  speedCopy      = 50;
               }
               QString speedStr = QString::number(convertedSpeed);
               speedStr += (" " + QString::fromStdString(aRouteInfo.mWaypoints[i].mSpeed.GetUnitName()));
               UpdateDataLambda(varLoc,
                                tokenList,
                                "speed",
                                speedCopy,
                                speedStr,
                                QString::fromStdString(UtUnitSpeed::FindUnitName(UtUnitSpeed::cSTANDARD_UNIT_ID)));

               // update climb rate
               varLoc                     = tokenList.lastIndexOf("climb_rate");
               double  convertedClimbRate = UtUnitSpeed::ConvertFromStandard(aRouteInfo.mWaypoints[i].mClimbRate,
                                                                            aRouteInfo.mWaypoints[i].mClimbRate.GetUnit());
               QString climbRateStr       = QString::number(convertedClimbRate);
               climbRateStr += (" " + QString::fromStdString(aRouteInfo.mWaypoints[i].mClimbRate.GetUnitName()));
               UpdateDataLambda(varLoc,
                                tokenList,
                                "climb_rate",
                                aRouteInfo.mWaypoints[i].mClimbRate,
                                climbRateStr,
                                QString::fromStdString(UtUnitSpeed::FindUnitName(UtUnitSpeed::cSTANDARD_UNIT_ID)));

               // update linear acceleration
               varLoc = tokenList.lastIndexOf("linear_acceleration");
               double convertedAccel =
                  UtUnitAcceleration::ConvertFromStandard(aRouteInfo.mWaypoints[i].mLinAccel,
                                                          aRouteInfo.mWaypoints[i].mLinAccel.GetUnit());
               QString linAccelStr = QString::number(convertedAccel);
               linAccelStr += (" " + QString::fromStdString(aRouteInfo.mWaypoints[i].mLinAccel.GetUnitName()));
               UpdateDataLambda(varLoc,
                                tokenList,
                                "linear_acceleration",
                                aRouteInfo.mWaypoints[i].mLinAccel,
                                linAccelStr,
                                QString::fromStdString(
                                   UtUnitAcceleration::FindUnitName(UtUnitAcceleration::cSTANDARD_UNIT_ID)));

               // update radial acceleration
               // treat these 3 commands as mutually exclusive
               int radAccelIndex = tokenList.lastIndexOf("radial_acceleration");
               int angleIndex    = tokenList.lastIndexOf("bank_angle_limit");
               int gIndex        = tokenList.lastIndexOf("turn_g_limit");

               double convertedRadAccel;
               // distinguish which command the acceleration value belongs to
               std::string accelUnit = aRouteInfo.mWaypoints[i].mRadAccel.GetUnitName();
               if (UtUnitAcceleration::ReadUnit(accelUnit) >= 0)
               {
                  convertedRadAccel =
                     UtUnitAcceleration::ConvertFromStandard(aRouteInfo.mWaypoints[i].mRadAccel,
                                                             aRouteInfo.mWaypoints[i].mLinAccel.GetUnit());
                  QString radAccelStr = QString::number(convertedRadAccel);
                  radAccelStr += (" " + QString::fromStdString(aRouteInfo.mWaypoints[i].mRadAccel.GetUnitName()));
                  UpdateDataLambda(radAccelIndex,
                                   tokenList,
                                   "radial_acceleration",
                                   aRouteInfo.mWaypoints[i].mRadAccel,
                                   radAccelStr,
                                   QString::fromStdString(
                                      UtUnitAcceleration::FindUnitName(UtUnitAcceleration::cSTANDARD_UNIT_ID)));
                  if (gIndex >= 0)
                  {
                     gIndex = tokenList.lastIndexOf("turn_g_limit");
                     RemoveCommandLambda(gIndex, tokenList);
                  }
                  if (angleIndex >= 0)
                  {
                     angleIndex = tokenList.lastIndexOf("bank_angle_limit");
                     RemoveCommandLambda(angleIndex, tokenList);
                  }
               }
               // TODO: Right now, the route browser has a single-line option to set bank angle, max g's or radial
               // acceleration. No matter which one is specified and what unit is used, it will get converted to m/s2
               // prior to reaching the route browser interface, so there is no way to distinguish between what command
               // the user wants. As a result, for now, we will always use radial_acceleration until the route browser
               // is potentially changed.

               // else if (accelUnit == "g")
               //{
               //    convertedRadAccel = UtUnitAcceleration::ConvertFromStandard(aRouteInfo.mWaypoints[i].mRadAccel,
               //    aRouteInfo.mWaypoints[i].mLinAccel.GetUnit()); QString radAccelStr =
               //    QString::number(convertedRadAccel); radAccelStr += (" " +
               //    QString::fromStdString(aRouteInfo.mWaypoints[i].mRadAccel.GetUnitName())); UpdateDataLambda(gIndex,
               //    tokenList, "turn_g_limit", aRouteInfo.mWaypoints[i].mRadAccel, radAccelStr,
               //    QString::fromStdString(UtUnitAcceleration::FindUnitName(UtUnitAcceleration::cSTANDARD_UNIT_ID)));
               //    if (radAccelIndex >= 0)
               //    {
               //       radAccelIndex = tokenList.lastIndexOf("radial_acceleration");
               //       RemoveCommandLambda(radAccelIndex, tokenList);
               //    }
               //    if (angleIndex >= 0)
               //    {
               //       angleIndex = tokenList.lastIndexOf("bank_angle_limit");
               //       RemoveCommandLambda(angleIndex, tokenList);
               //    }
               // }
               // else if (UtUnitAngle::ReadUnit(accelUnit) >= 0)
               //{
               //    convertedRadAccel = UtUnitAngle::ConvertFromStandard(aRouteInfo.mWaypoints[i].mRadAccel,
               //    aRouteInfo.mWaypoints[i].mLinAccel.GetUnit()); QString radAccelStr =
               //    QString::number(convertedRadAccel); radAccelStr += (" " +
               //    QString::fromStdString(aRouteInfo.mWaypoints[i].mRadAccel.GetUnitName()));
               //    UpdateDataLambda(angleIndex, tokenList, "bank_angle_limit", aRouteInfo.mWaypoints[i].mRadAccel,
               //    radAccelStr, QString::fromStdString(UtUnitAngle::FindUnitName(UtUnitAngle::cSTANDARD_UNIT_ID))); if
               //    (radAccelIndex >= 0)
               //    {
               //       radAccelIndex = tokenList.lastIndexOf("radial_acceleration");
               //       RemoveCommandLambda(radAccelIndex, tokenList);
               //    }
               //    if (gIndex >= 0)
               //    {
               //       gIndex = tokenList.lastIndexOf("turn_g_limit");
               //       RemoveCommandLambda(angleIndex, tokenList);
               //    }
               // }

               // update switch type
               int passingIndex      = tokenList.lastIndexOf("switch_on_passing");
               int approachIndex     = tokenList.lastIndexOf("switch_on_approach");
               varLoc                = std::max(passingIndex, approachIndex);
               int     switchType    = aRouteInfo.mWaypoints[i].mSwitchType;
               QString switchTypeStr = "";
               if (switchType == 1)
               {
                  switchTypeStr = "switch_on_passing";
               }
               else if (switchType == 2)
               {
                  switchTypeStr = "switch_on_approach";
               }
               if (varLoc != -1)
               {
                  if (!switchTypeStr.isEmpty())
                  {
                     tokenList[varLoc] = switchTypeStr;
                  }
                  else
                  {
                     tokenList.removeAt(varLoc);
                     int lowIndex = std::min(passingIndex, approachIndex);
                     if (lowIndex >= 0)
                     {
                        tokenList.removeAt(lowIndex);
                     }
                  }
               }
               else if (!switchTypeStr.isEmpty())
               {
                  tokenList.push_back("\n");
                  tokenList.push_back(switchTypeStr);
               }

               // update turn direction
               varLoc        = tokenList.lastIndexOf("turn");
               int     dirId = aRouteInfo.mWaypoints[i].mTurnDir;
               QString dir   = "";
               if (dirId == -1)
               {
                  dir = "left";
               }
               else if (dirId == 1)
               {
                  dir = "right";
               }
               if (varLoc != -1)
               {
                  int dirIndex = GetNextRealTokenLambda(varLoc, tokenList);
                  if (dirIndex >= 0)
                  {
                     if (!dir.isEmpty())
                     {
                        tokenList[dirIndex] = dir;
                     }
                     else
                     {
                        tokenList.removeAt(dirIndex);
                        tokenList.removeAt(varLoc);
                     }
                  }
               }
               else
               {
                  if (!dir.isEmpty())
                  {
                     tokenList.push_back("\n");
                     tokenList.push_back("turn");
                     tokenList.push_back(dir);
                  }
               }

               // update end of path option (this should only be applied to the last defined waypoint)
               if (i == aRouteInfo.mWaypoints.size() - 1)
               {
                  int extrapolateIndex   = tokenList.lastIndexOf("extrapolate");
                  int stopIndex          = tokenList.lastIndexOf("stop");
                  int removeIndex        = tokenList.lastIndexOf("remove");
                  varLoc                 = std::max(extrapolateIndex, std::max(stopIndex, removeIndex));
                  int     endPathId      = aRouteInfo.mEndPathType;
                  QString endPathCommand = "";
                  if (endPathId == 1)
                  {
                     endPathCommand = "extrapolate";
                  }
                  else if (endPathId == 2)
                  {
                     endPathCommand = "stop";
                  }
                  else if (endPathId == 3)
                  {
                     endPathCommand = "remove";
                  }
                  if (varLoc != -1)
                  {
                     if (!endPathCommand.isEmpty())
                     {
                        tokenList[varLoc] = endPathCommand;
                     }
                     else
                     {
                        tokenList.removeAt(varLoc);
                     }
                  }
                  else
                  {
                     if (!endPathCommand.isEmpty())
                     {
                        tokenList.push_back("\n");
                        tokenList.push_back(endPathCommand);
                     }
                  }
               }

               // update "goto"
               varLoc         = tokenList.lastIndexOf("goto");
               bool goToIsSet = false;
               if (varLoc != -1)
               {
                  QString gotoStr    = aRouteInfo.mWaypoints[i].mGoTo;
                  int     valueIndex = GetNextRealTokenLambda(varLoc, tokenList);
                  if (!gotoStr.isEmpty() && valueIndex >= 0)
                  {
                     tokenList[valueIndex] = gotoStr;
                     goToIsSet             = true;
                  }
               }
               if (!goToIsSet)
               {
                  if (aRouteInfo.mWaypoints[i].mGoTo.isEmpty())
                  {
                     RemoveDataLambda(tokenList.indexOf("goto"), tokenList);
                     tokenList.removeAt(tokenList.indexOf("goto"));
                  }
                  else
                  {
                     tokenList.push_back("\n");
                     tokenList.push_back("goto");
                     tokenList.push_back(aRouteInfo.mWaypoints[i].mGoTo);
                  }
               }

               // format the waypoint string that will be placed in the text editor
               QString newDataStr = "";
               if (i != 0) // the first waypoint is already indented, so don't double the indentation
               {
                  newDataStr += QString::fromStdString(cINDENT);
               }
               // get rid of leading newlines
               int j = 0;
               while (j < tokenList.size() && tokenList[j] == "\n")
               {
                  tokenList.removeAt(j);
               }
               // get rid of trailing newlines
               int k = tokenList.size() - 1;
               while (k > 0 && tokenList[k] == "\n")
               {
                  tokenList.removeAt(k--);
               }
               // get rid of consecutive newlines
               j = 0;
               while (j + 1 < tokenList.size())
               {
                  if (tokenList[j] == "\n" && tokenList[j + 1] == "\n")
                  {
                     tokenList.removeAt(j + 1);
                  }
                  else
                  {
                     j++;
                  }
               }

               // if goto isn't at the end of the definition AFSIM is not happy.  Force goto to the bottom of the definition
               if (goToIsSet)
               {
                  int gotoIdx       = tokenList.indexOf("goto");
                  int tokenListSize = tokenList.size();
                  if (gotoIdx != tokenListSize - 2)
                  {
                     // goto needs to be moved to the end.  Swap it with the last element and its value
                     int lastNewLine = tokenList.lastIndexOf("\n");

                     QString lastAttrib = tokenList[lastNewLine + 1];
                     QString lastValue  = "\n";
                     // some attributes don't have associated values e.g. 'switch_on_passing'
                     if (lastNewLine + 1 != tokenListSize - 1)
                     {
                        lastValue = tokenList[tokenListSize - 1];
                     }
                     QString gotoValue            = tokenList[gotoIdx + 1];
                     tokenList[gotoIdx]           = lastAttrib;
                     tokenList[gotoIdx + 1]       = lastValue;
                     tokenList[tokenListSize - 2] = "goto";
                     tokenList[tokenListSize - 1] = gotoValue;
                  }
               }
               for (const auto& tokenIt : tokenList)
               {
                  // indent after newlines, using the first command of the selected section as reference
                  if (tokenIt == "\n")
                  { // get rid of added space before newline
                     newDataStr.replace(newDataStr.size() - 1, 1, tokenIt);
                     newDataStr += QString::fromStdString(cINDENT_TAB);
                  }
                  else
                  {
                     if (tokenIt == cHASH_COMMENT)
                     {
                        newDataStr += hashComments.front();
                        hashComments.pop();
                     }
                     else if (tokenIt == cSLASH_COMMENT)
                     {
                        newDataStr += slashComments.front();
                        slashComments.pop();
                     }
                     else
                     {
                        newDataStr += tokenIt;
                     }
                     newDataStr += " ";
                  }
               }
               if (i < aRouteInfo.mWaypoints.size() - 1)
               {
                  newDataStr += afterWaypoints[i];
               }
               newWaypointText.push_back(newDataStr);
            }
         }
         QTextCursor waypointCursor = GetEditor()->textCursor();
         waypointCursor.setPosition(initialRanges[0].GetBegin(), QTextCursor::MoveAnchor);
         waypointCursor.setPosition(initialRanges[initialRanges.size() - 1].GetEnd() + 1, QTextCursor::KeepAnchor);
         QString combinedStr = newWaypointText.join("\n");
         waypointCursor.insertText(combinedStr);
         UpdateTextRange();
      }
   }
}


void wizard::Platform::RemoveWaypointFromEditor(wkf::AttachmentRoute* aRoutePtr, int aWaypointIndex)
{
   if (mPlatform.IsValid() && mPlatform.Mover().IsValid())
   {
      if (aWaypointIndex != -1)
      {
         bool                removedWaypointSelected = false;
         WsfPM_Route         route                   = mPlatform.Mover().InitialRoute();
         WsfPM_Waypoint      wayptNode               = route.GetWaypoint(aWaypointIndex);
         UtTextDocumentRange wayPtRange              = GetAttributeDocumentRange(wayptNode);
         QTextCursor         cursor                  = GetEditor()->textCursor();
         cursor.setPosition(wayPtRange.GetBegin());
         GetEditor()->setTextCursor(cursor);
         GetEditor()->Replace(wayPtRange, "");
         GetEditor()->DeleteLine();
         UpdateTextRange();
         vespa::VaAttachment::ShapeList sList;
         aRoutePtr->GetShapes(sList);
         if (!sList.empty() && sList.size() > static_cast<size_t>(aWaypointIndex))
         {
            removedWaypointSelected = aRoutePtr->GetWaypoint(aWaypointIndex)->IsSelected();
            ;
            aRoutePtr->DeleteWaypoint(aWaypointIndex);
         }
         UpdateSelectedWaypoints(aRoutePtr);
         // update the selected waypoint indices if the waypoint will be deleted later
         if (!removedWaypointSelected)
         {
            mWaitingForDelete = aWaypointIndex;
            HandleQueuedDeleteWaypoint();
         }
         // if the waypoint was already removed prior to checking for selections, select the previous waypoint and add it to the list
         else
         {
            (aWaypointIndex > 0) ? mSelectedWaypoints.emplace_back(aWaypointIndex - 1) :
                                   mSelectedWaypoints.emplace_back(0);
         }
      }
   }
}

void wizard::Platform::HandleQueuedDeleteWaypoint()
{
   if (mWaitingForDelete >= 0)
   {
      for (auto& it : mSelectedWaypoints)
      {
         if (it > 0 && static_cast<unsigned int>(mWaitingForDelete) <= it)
         {
            it--;
         }
      }
   }
}

void wizard::Platform::AddWaypointToEditor(const wkf::RouteBrowserInterface::WaypointInfo& aWaypointInfo,
                                           const int                                       aWaypointIndex)
{
   // for the moment, this is the only way to enter this method, so add to handle the others when we support them in the
   // UI we are also ignoring a lot of the values in aWaypointInfo, so that should be added if we need to access this
   // method from anywhere else in code.
   if (aWaypointInfo.mPointType == wkf::RouteBrowserInterface::WaypointInfo::cABSOLUTE)
   {
      int idx = aWaypointIndex;
      if (aWaypointIndex == -1)
      {
         idx = mPlatform.Mover().InitialRoute().WaypointCount() - 1;
      }
      if (aWaypointIndex == static_cast<int>(mPlatform.Mover().InitialRoute().WaypointCount()))
      {
         idx--;
      }

      WsfPM_Route route = mPlatform.Mover().InitialRoute();
      if (!route.FindIndexNode()) // there is no route on the platform
      {
         wkf::RouteBrowserInterface::RouteInfo routeInfo;
         routeInfo.mWaypoints.push_back(aWaypointInfo);
         AddNewRouteInText(routeInfo);
         return;
      }
      WsfPM_Waypoint wayptNode = route.GetWaypoint(idx);
      if (!wayptNode.IsValid()) // there is an empty route on the platform
      {
         UtTextDocumentRange platRange    = mPlatform.GetSourceDocumentRange();
         QTextCursor         attributeLoc = GetEditor()->textCursor();
         attributeLoc.setPosition(platRange.GetBegin());
         attributeLoc.setPosition(platRange.GetEnd() + 1, QTextCursor::KeepAnchor);
         QString     text      = attributeLoc.selection().toPlainText();
         QStringList tokenlist = Editor::Split(text);

         int ptok = tokenlist.indexOf("end_route");

         QString           waypointString = "\n   position ";
         std::stringstream osslat;
         osslat << aWaypointInfo.mLat;
         waypointString += osslat.str().c_str();

         waypointString += " ";
         std::stringstream osslon;
         osslon << aWaypointInfo.mLon;
         waypointString += osslon.str().c_str();

         waypointString += " altitude ";
         std::ostringstream ossAlt;
         ossAlt << aWaypointInfo.mAlt;
         waypointString += ossAlt.str().c_str();

         // append the altitude reference string
         waypointString += AltitudeReferenceToString(aWaypointInfo.mAltRef);

         waypointString += " speed ";
         if (aWaypointInfo.mSpeed == wkf::WaypointValueType::cNOT_SET)
         {
            if (mPlatform.IsValid() && mPlatform.Mover().IsValid())
            {
               if (mPlatform.Mover().IsOfType("WSF_GROUND_MOVER"))
               {
                  waypointString += "50 mph";
               }
               else
               {
                  waypointString += "450 nm/h";
               }
            }
            else
            {
               waypointString += "450 nm/h";
            }
         }
         else
         {
            std::stringstream ossspd;
            ossspd << aWaypointInfo.mSpeed;
            waypointString += ossspd.str().c_str();
         }
         waypointString += "\n";

         tokenlist.insert(ptok, waypointString);
         GetEditor()->setTextCursor(attributeLoc);
         attributeLoc.insertText(tokenlist.join(""));
      }
      else
      {
         UtTextDocumentRange wayPtRange = GetAttributeDocumentRange(wayptNode);

         // insert waypoint after the given waypoint index
         QTextCursor waypointCursor = GetEditor()->textCursor();
         int         oldPos         = wayPtRange.GetEnd() + 2;

         waypointCursor.setPosition(wayPtRange.GetBegin());
         waypointCursor.setPosition(wayPtRange.GetEnd() + 1, QTextCursor::KeepAnchor);
         QString text   = waypointCursor.selection().toPlainText();
         QString output = "\n";

         // get the indentation
         waypointCursor.setPosition(wayPtRange.GetBegin());
         QString positionTextLine = waypointCursor.block().text();
         int     numLeadingSpaces =
            positionTextLine.indexOf(QRegExp("[^ ]")); // capture leading white space... this will miss tabs currently
         output += positionTextLine.mid(0, numLeadingSpaces);

         // move to the end of the waypoint
         waypointCursor.setPosition(wayPtRange.GetEnd() + 1);

         // update the data, if we handled other kinds of points, this would change
         output += "position ";
         std::stringstream osslat;
         osslat << aWaypointInfo.mLat;
         output += osslat.str().c_str();
         output += " ";
         std::stringstream osslon;
         osslon << aWaypointInfo.mLon;
         output += osslon.str().c_str();

         // append the altitude reference string
         output += AltitudeReferenceToString(aWaypointInfo.mAltRef);

         // write it in
         GetEditor()->textCursor().clearSelection();
         waypointCursor.insertText(output);
         waypointCursor.clearSelection();
         waypointCursor.setPosition(oldPos);
      }
   }
}

void wizard::Platform::GotoDefinition()
{
   if (Project::Instance() && Project::Instance()->TinyParseResults())
   {
      ParseResultsTiny* resultsPtr = Project::Instance()->TinyParseResults();
      WsfPProxyIndex*   indexPtr   = resultsPtr->ProxyIndex();

      if (indexPtr)
      {
         WsfPProxyPath                    endPath = mPlatform.GetPath();
         std::vector<UtTextDocumentRange> ranges;
         resultsPtr->FindModificationLocationsExtended(endPath, ranges);
         FindReferencesResultDialog::GoToLocationList(Project::Instance(), ranges);
      }
   }
}

void wizard::Platform::OnProxyModified(const ProxyChange& aProxyChange)
{
   ProxyUpdate();
}

void wizard::Platform::ProxyUpdate()
{
   // update this platform's text editor information
   mPlatformTextRange = GetPlatform().GetSourceDocumentRange();
   mDirty             = true;

   std::string newName = mPlatform.GetName();

   SetName(newName);
   SetIcon(mPlatform.Icon());
   SetSide(mPlatform.Side());
   SetIndex(mId);
   SetupGroupings();

   Initialize();
   wkf::Observer::PlatformPropertiesChanged(this);


   wizEnv.UpdatePlatform(GetName().c_str());
   mLastLat = GetLat();
   mLastLon = GetLon();
   mDirty   = false;
}

void wizard::Platform::SetupGroupings()
{
   // NOTE: groupings as currently written will not robustly nor immediately handle re-categorization, re-typing, or
   // siding of platforms.
   wkfEnv.ClearPlatformGroupings(mId);

   WsfPM_ObjectMap cat = mPlatform.Categories();
   if (cat.IsValid())
   {
      std::vector<WsfPProxyNode> cats;
      cat.GetAllValues(cats);
      std::vector<WsfPProxyNode>::iterator catit;
      for (catit = cats.begin(); catit != cats.end(); ++catit)
      {
         if (catit->IsValid())
         {
            wkfEnv.SetPlatformGrouping(mId, wkf::PlatformGrouping(wkf::PlatformGrouping::eCategory, catit->GetName()));
         }
      }
   }
   wkfEnv.SetPlatformGrouping(mId, wkf::PlatformGrouping(wkf::PlatformGrouping::eSide, mPlatform.Side()));

   WsfPProxyNode          inh = mPlatform.GetInherited();
   std::list<std::string> inheritedTypesList;

   while (inh.IsValid())
   {
      auto inheritedNameStr = inh.GetName();
      if (std::find(inheritedTypesList.begin(), inheritedTypesList.end(), inheritedNameStr) != inheritedTypesList.end())
      {
         break; // break to avoid potential infinite loop of inherited types
      }
      else
      {
         inheritedTypesList.emplace_back(inheritedNameStr);
         wkfEnv.SetPlatformGrouping(mId, wkf::PlatformGrouping(wkf::PlatformGrouping::eType, inheritedNameStr));
         inh = inh.GetInherited();
      }
   }
}

//! This method is called to compare data on this platform with the data associated with it in the text editor
//! if the data is different, it is updated in the text editor
//! currently aDataName should be one of the following: "type","side","icon", "latitude", longitude", "altitude msl",
//! "yaw","pitch", and "roll"
bool wizard::Platform::CheckPlatformDataChange(const QString& aDataName, const QString& aDataValue)
{
   if (!GetPlatform().IsValid() || mDirty)
   {
      return false;
   }

   if (aDataName != "type" && aDataName != "side" && aDataName != "icon" && aDataName != "latitude" &&
       aDataName != "longitude" && aDataName != "altitude agl" && aDataName != "altitude msl" &&
       aDataName != "heading" && aDataName != "pitch" && aDataName != "roll")
   {
      return false;
   }


   // first check to see if the editor info for this platform has been set, if not, set it.
   // update this platform's text editor information
   UpdateEditorInfo();
   mEditorPtr->GoToRange(mPlatformTextRange);

   // now check the data
   if (aDataName == "latitude")
   {
      CheckLatitudeChanged(aDataValue);
   }
   else if (aDataName == "longitude")
   {
      CheckLongitudeChanged(aDataValue);
   }
   else if (aDataName == "altitude agl" || aDataName == "altitude msl")
   {
      CheckAltitudeChanged(aDataValue);
   }
   else if (aDataName == "type")
   {
      CheckTypeChanged(aDataValue);
   }
   else
   {
      QString attribName = aDataName;
      if (attribName == "yaw")
      {
         attribName = "heading";
      }
      AttributeChanged(attribName, aDataValue);

      wizard::ProxyWatcher::Instance()->EmitOptionChange(GetPlatform());
   }
   return true;
}

bool wizard::Platform::CheckLatitudeChanged(const QString& aDataValue)
{
   double lat, lon, alt;
   if (GetPlatform().Mover().IsValid())
   {
      WsfPM_WaypointMover wpm = GetPlatform().Mover();
      if (WsfPM_WaypointMover::IsWaypointMover(wpm))
      {
         // update the first waypoint position
         WsfPM_Route route = wpm.InitialRoute();
         if ((route.IsValid()) && (route.WaypointCount() > 0))
         {
            WsfPM_Waypoint wp = route.GetWaypoint(0);
            lat               = wp.Position().GetLatitude();
         }
      }
      else
      {
         GetPlatform().LocationLLA(lat, lon, alt);
      }
   }

   else
   {
      GetPlatform().LocationLLA(lat, lon, alt);
   }

   UtTextDocumentRange attribRange = GetAttributeRangeByName("position");
   if (attribRange.Valid())
   {
      QTextCursor attributeLoc = GetEditor()->textCursor();
      attributeLoc.setPosition(attribRange.GetBegin());
      attributeLoc.setPosition(attribRange.GetEnd() + 1, QTextCursor::KeepAnchor);
      QString     text      = attributeLoc.selection().toPlainText();
      QStringList tokenlist = Editor::Split(text);

      int ptok = tokenlist.indexOf("position");
      if (ptok >= 0)
      {
         ptok = Editor::FindNextValidToken(tokenlist, ptok + 1);
         if (ptok >= 0)
         {
            UtLatPos           newPos;
            QString            latValStr = aDataValue.split(" ")[0];
            std::istringstream iss(latValStr.toStdString());
            std::ostringstream oss;
            iss >> newPos;
            oss << newPos;
            std::string lat = oss.str();
            if (lat == "n" || lat == "s")
            {
               lat = '0' + lat;
            }
            if (lat == "")
            {
               lat = "0n";
            }
            tokenlist[ptok] = QString::fromStdString(lat);
            GetEditor()->setTextCursor(attributeLoc);
            attributeLoc.insertText(tokenlist.join(""));
         }
      }
   }
   // there was no position attribute for this platform, create one
   else
   {
      QString            latValStr = aDataValue.split(" ")[0];
      UtLatPos           newPos;
      std::istringstream iss(latValStr.toStdString());
      std::ostringstream oss;
      // UtLatPos will convert the latitude to 0 if the input is invalid
      iss >> newPos;
      oss << newPos;
      QString newDataStr = QString(oss.str().c_str()) + " 0e";

      AddNewAttributeInText("position", newDataStr);
   }
   return true;
}

bool wizard::Platform::CheckLongitudeChanged(const QString& aDataValue)
{
   double lat, lon, alt;
   if (GetPlatform().Mover().IsValid())
   {
      WsfPM_WaypointMover wpm = GetPlatform().Mover();
      if (WsfPM_WaypointMover::IsWaypointMover(wpm))
      {
         // update the first waypoint position
         WsfPM_Route route = wpm.InitialRoute();
         if ((route.IsValid()) && (route.WaypointCount() > 0))
         {
            WsfPM_Waypoint wp = route.GetWaypoint(0);
            lon               = wp.Position().GetLongitude();
         }
      }

      else
      {
         GetPlatform().LocationLLA(lat, lon, alt);
      }
   }
   else
   {
      GetPlatform().LocationLLA(lat, lon, alt);
   }

   UtTextDocumentRange attribRange = GetAttributeRangeByName("position");
   if (attribRange.Valid())
   {
      QTextCursor attributeLoc = GetEditor()->textCursor();
      attributeLoc.setPosition(attribRange.GetBegin());
      attributeLoc.setPosition(attribRange.GetEnd() + 1, QTextCursor::KeepAnchor);
      QString     text      = attributeLoc.selection().toPlainText();
      QStringList tokenlist = Editor::Split(text);

      int ptok = tokenlist.indexOf("position");
      if (ptok >= 0)
      {
         ptok = Editor::FindNextValidToken(tokenlist, ptok + 1);
         if (ptok >= 0)
         {
            ptok = Editor::FindNextValidToken(tokenlist, ptok + 1);
            if (ptok >= 0)
            {
               UtLonPos           newPos;
               QString            latValStr = aDataValue.split(" ")[0];
               std::istringstream iss(latValStr.toStdString());
               std::ostringstream oss;
               iss >> newPos;
               oss << newPos;
               std::string lon = oss.str();
               if (lon == "e" || lon == "w")
               {
                  lon = '0' + lon;
               }
               if (lon == "")
               {
                  lon = "0e";
               }
               tokenlist[ptok] = QString::fromStdString(lon);
               GetEditor()->setTextCursor(attributeLoc);
               attributeLoc.insertText(tokenlist.join(""));
            }
         }
      }
   }

   // there was no position attribute for this platform, create one
   else
   {
      QString            lonValStr = aDataValue.split(" ")[0];
      UtLonPos           newPos;
      std::istringstream iss(lonValStr.toStdString());
      std::ostringstream oss;
      // UtLatPos will convert the latitude to 0 if the input is invalid
      iss >> newPos;
      oss << newPos;
      QString newDataStr = " 0n " + QString(oss.str().c_str());

      AddNewAttributeInText("position", newDataStr);
   }
   return true;
}

bool wizard::Platform::CheckPositionChanged(const UtLatPos& aLatValue, const UtLonPos& aLonValue)
{
   std::ostringstream ossLat;
   ossLat << aLatValue;
   std::ostringstream ossLon;
   ossLon << aLonValue;
   UtTextDocumentRange attribRange = GetAttributeRangeByName("position");
   UtStringRef         attribText;
   attribRange.Text(attribText);
   if (attribRange.Valid())
   {
      QTextCursor attributeLoc = GetEditor()->textCursor();
      attributeLoc.setPosition(attribRange.GetBegin());
      attributeLoc.setPosition(attribRange.GetEnd() + 1, QTextCursor::KeepAnchor);
      QString     text      = attributeLoc.selection().toPlainText();
      QStringList tokenlist = Editor::Split(text);

      int ptok = tokenlist.indexOf("position");
      if (ptok >= 0)
      {
         ptok = Editor::FindNextValidToken(tokenlist, ptok + 1);
         if (ptok >= 0)
         {
            tokenlist[ptok] = QString::fromStdString(ossLat.str());
            ptok            = Editor::FindNextValidToken(tokenlist, ptok + 1);
            if (ptok >= 0)
            {
               tokenlist[ptok] = QString::fromStdString(ossLon.str());
               GetEditor()->setTextCursor(attributeLoc);
               attributeLoc.insertText(tokenlist.join(""));
            }
         }
         return true;
      }
      int mgrs_tok = tokenlist.indexOf("mgrs_coordinate");
      if (mgrs_tok >= 0)
      {
         mgrs_tok = Editor::FindNextValidToken(tokenlist, mgrs_tok + 1);
         if (mgrs_tok >= 0)
         {
            std::string posMGRS;
            UtEllipsoidalEarth::ConvertLLToMGRS(aLatValue, aLonValue, posMGRS);
            tokenlist[mgrs_tok] = QString::fromStdString(posMGRS);
            GetEditor()->setTextCursor(attributeLoc);
            attributeLoc.insertText(tokenlist.join(""));
         }
      }
   }
   else
   {
      QStringList tokenList;
      // position wasn't found add it to the platform definition
      tokenList.append(QString::fromStdString(ossLat.str()));
      tokenList.append(QString::fromStdString(ossLon.str()));
      AddNewAttributeInText("position", tokenList.join(" "));
   }
   return true;
}

bool wizard::Platform::CheckAltitudeChanged(const QString& aDataValue)
{
   bool                found       = false;
   UtTextDocumentRange attribRange = GetAttributeRangeByName("altitude");
   if (attribRange.Valid())
   {
      QTextCursor attributeLoc = GetEditor()->textCursor();
      attributeLoc.setPosition(attribRange.GetBegin());
      attributeLoc.setPosition(attribRange.GetEnd() + 1, QTextCursor::KeepAnchor);
      QString     text      = attributeLoc.selection().toPlainText();
      QStringList tokenlist = Editor::Split(text);

      int ptok = tokenlist.indexOf("altitude");
      if (ptok >= 0)
      {
         int valueLoc = ptok = Editor::FindNextValidToken(tokenlist, ptok + 1);
         if (ptok >= 0)
         {
            int unitLoc = ptok = Editor::FindNextValidToken(tokenlist, ptok + 1);
            if (ptok >= 0)
            {
               found                     = true;
               QStringList altStringList = aDataValue.split(" ", QString::SkipEmptyParts);
               QString     altValStr     = "";
               QString     altType       = "meters";
               if (!altStringList.isEmpty())
               {
                  altValStr = altStringList[0];
               }
               if (altStringList.size() > 1)
               {
                  altType = altStringList[1];

                  int unitId = UtUnitLength::ReadUnit(altStringList[1].toStdString());
                  if (!UtUnitLength::IsUnitValid(unitId) || (unitId < 0))
                  {
                     return false; // we can't work with bad units
                  }
               }
               tokenlist[valueLoc] = altValStr;
               tokenlist[unitLoc]  = altType;
               GetEditor()->setTextCursor(attributeLoc);
               attributeLoc.insertText(tokenlist.join(""));
            }
         }
      }
   }
   if (!found)
   {
      QString     newDataStr    = aDataValue;
      QStringList dataValueList = newDataStr.split(" ");
      if (dataValueList.size() == 1)
      {
         // see if the data value is missing a unit
         bool isDouble = false;
         dataValueList[0].toDouble(&isDouble);
         if (isDouble)
         {
            // there is a value, but no units, assume feet
            newDataStr.append(" feet");
         }
      }
      AddNewAttributeInText("altitude", newDataStr);
   }
   return true;
}

bool wizard::Platform::CheckHeadingChanged()
{
   bool                found       = false;
   UtTextDocumentRange attribRange = GetAttributeRangeByName("heading");
   if (attribRange.Valid())
   {
      QTextCursor attributeLoc = GetEditor()->textCursor();
      attributeLoc.setPosition(attribRange.GetBegin() + 1);
      QString     curTxt    = attributeLoc.block().text();
      QStringList tokenlist = Editor::Split(curTxt);

      int ptok = tokenlist.indexOf("heading");
      if (ptok >= 0)
      {
         int valueLoc = ptok = Editor::FindNextValidToken(tokenlist, ptok + 1);
         if (ptok >= 0)
         {
            int unitLoc = ptok = Editor::FindNextValidToken(tokenlist, ptok + 1);
            if (ptok >= 0)
            {
               found = true;

               UtAngleValue headingVal{GetHeading()};
               double       curHeading = tokenlist[valueLoc].toDouble();
               // correct the heading if the user is using a positive value but the vaEntity returned the negative version of it
               if (curHeading > 0 && headingVal < 0)
               {
                  headingVal = headingVal + UtMath::cTWO_PI;
               }
               // correct the heading if the user is using a negative value but the vaEntity returned the positive version of it
               else if (curHeading < 0 && headingVal > 0)
               {
                  headingVal = headingVal - UtMath::cTWO_PI;
               }


               QString headingValStr = QString::number(headingVal);
               QString headingType   = tokenlist[unitLoc];

               // correct the "mismatch bug"
               if (!IsValidUnitForHeading(tokenlist[unitLoc]))
               {
                  int     extra = 0;
                  QString text  = attributeLoc.selection().toPlainText();
                  do
                  {
                     attributeLoc.setPosition(attribRange.GetEnd() + extra, QTextCursor::KeepAnchor);
                     text      = attributeLoc.selection().toPlainText();
                     tokenlist = Editor::Split(text);
                     ++extra;
                  } while (!IsValidUnitForHeading(tokenlist[unitLoc]));
                  attributeLoc.setPosition(attribRange.GetEnd() + extra - 1, QTextCursor::KeepAnchor);
                  text      = attributeLoc.selection().toPlainText();
                  tokenlist = Editor::Split(text);
               }
               headingType = tokenlist[unitLoc];

               if (headingType == "rad" || headingType == "radian" || headingType == "radians")
               {
                  headingValStr = QString::number(headingVal);
               }
               else if (headingType == "deg" || headingType == "degree" || headingType == "degrees")
               {
                  // convert to degrees because it is stored as radians
                  headingValStr = QString::number(headingVal.GetAsUnit("degrees"));
               }
               else if (headingType == "mil" || headingType == "mils")
               {
                  // convert to mils because it is stored as radians
                  headingValStr = QString::number(headingVal.GetAsUnit("mils"));
               }
               else if (headingType == "arcsecond" || headingType == "arcseconds")
               {
                  // convert to arcseconds because it is stored as radians
                  headingValStr = QString::number(headingVal.GetAsUnit("arcseconds"));
               }
               tokenlist[valueLoc] = headingValStr;
               tokenlist[unitLoc]  = headingType;

               int oldPos = attributeLoc.position();
               attributeLoc.setPosition(attributeLoc.block().position());
               attributeLoc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
               attributeLoc.insertText(tokenlist.join(""));
               attributeLoc.clearSelection();
               attributeLoc.setPosition(oldPos);
               GetEditor()->setTextCursor(attributeLoc);
            }
         }
      }
   }
   if (!found)
   {
      // convert to degrees
      double  degHeading = GetHeading() * UtMath::cDEG_PER_RAD;
      QString newDataStr = QString::number(degHeading);
      newDataStr.append(" degrees");
      AddNewAttributeInText("heading", newDataStr);
   }
   return true;
}

bool wizard::Platform::IsValidUnitForHeading(const QString& aUnitStr) const noexcept
{
   return UtUnitAngle::ReadUnit(aUnitStr.toStdString()) != -1;
}

bool wizard::Platform::CheckTypeChanged(const QString& aDataValue)
{
   mPlatformTextRange             = GetPlatform().GetSourceDocumentRange();
   QTextCursor platformTextCursor = GetEditor()->textCursor();
   platformTextCursor.setPosition(mPlatformTextRange.GetBegin());
   platformTextCursor.setPosition(mPlatformTextRange.GetEnd(), QTextCursor::KeepAnchor);
   GetEditor()->setTextCursor(platformTextCursor);

   QStringList curLine = Editor::Split(platformTextCursor.selection().toPlainText());
   int         ptok    = Editor::FindNextValidToken(curLine, 0); // "platform"
   if (ptok >= 0)
   {
      ptok = Editor::FindNextValidToken(curLine, ptok + 1); // <platform name>
      if (ptok >= 0)
      {
         ptok = Editor::FindNextValidToken(curLine, ptok + 1); // <platform type>
         if (ptok >= 0)
         {
            curLine[ptok] = aDataValue;

            QString newLine = curLine.join("");
            int     oldPos  = mPlatformTextRange.GetBegin();
            platformTextCursor.insertText(newLine);
            platformTextCursor.clearSelection();
            platformTextCursor.setPosition(oldPos);
            GetEditor()->setTextCursor(platformTextCursor);
            UpdateTextRange();
            return true;
         }
      }
   }
   return false;
}

void wizard::Platform::AttributeChanged(const QString& aDataName, const QString& aDataValue)
{
   WsfPProxyNode       attribNode  = GetPlatform() + aDataName.toStdString();
   UtTextDocumentRange attribRange = GetAttributeDocumentRange(attribNode);
   if (attribRange.Valid())
   {
      QTextCursor cursor = GetEditor()->textCursor();
      cursor.setPosition(attribRange.GetBegin());
      cursor.setPosition(attribRange.GetEnd() + 1, QTextCursor::KeepAnchor);
      GetEditor()->setTextCursor(cursor);
      QStringList tokenList = Editor::Split(cursor.selection().toPlainText());
      if (!tokenList.isEmpty())
      {
         int ptok = tokenList.indexOf(aDataName);
         if (ptok >= 0)
         {
            if (aDataValue.isEmpty())
            {
               // we should probably clear the line in this situation, this code doesn't do that.
               return;
            }
            else
            {
               int dataLoc = ptok = Editor::FindNextValidToken(tokenList, ptok + 1);

               QStringList dataValueList = aDataValue.split(" ");
               if (!dataValueList.isEmpty())
               {
                  QString dataVal = dataValueList[0];
                  if (dataValueList.size() > 1)
                  {
                     QString dataUnit = dataValueList[1];

                     if (aDataName == "heading" || aDataName == "pitch" || aDataName == "roll")
                     {
                        auto CheckBaseUnitLambda = [](UtUnitAngle::BaseUnit aUnitType) {
                           return (aUnitType >= UtUnitAngle::BaseUnit::cRADIANS &&
                                   aUnitType <= UtUnitAngle::BaseUnit::cMILS);
                        };
                        bool   isDouble = false;
                        double angleVal = dataVal.toDouble(&isDouble);

                        int unitLoc = Editor::FindNextValidToken(tokenList, ptok + 1);
                        if (isDouble && (unitLoc >= 0))
                        {
                           UtUnitAngle::BaseUnit dataUnitType =
                              static_cast<UtUnitAngle::BaseUnit>(UtUnitAngle::ReadUnit(dataUnit.toStdString()));
                           UtUnitAngle::BaseUnit curUnitType =
                              static_cast<UtUnitAngle::BaseUnit>(UtUnitAngle::ReadUnit(tokenList[unitLoc].toStdString()));
                           if (CheckBaseUnitLambda(dataUnitType) && CheckBaseUnitLambda(curUnitType))
                           {
                              double standardUnitValue = UtUnitAngle::ConvertToStandard(angleVal, dataUnitType);
                              double convertedValue = UtUnitAngle::ConvertFromStandard(standardUnitValue, curUnitType);
                              tokenList[dataLoc]    = QString::number(convertedValue);
                           }
                        }
                     }
                     else
                     {
                        return; // we don't know how to handle this
                     }
                  }
                  else
                  {
                     if ((aDataName == "heading" || aDataName == "pitch" || aDataName == "roll"))
                     {
                        // default to deg if no unit specified
                        tokenList[ptok] = dataVal + " deg";
                     }
                     else if (aDataName == "icon" || aDataName == "side")
                     {
                        // Allows values including special characters to be overwritten for icon and side
                        tokenList[ptok] = dataVal;
                        for (int i = ptok + 1; i < tokenList.size();)
                        {
                           tokenList.erase(tokenList.begin() + i);
                        }
                     }
                     else
                     {
                        tokenList[ptok] = dataVal;
                     }
                  }
                  // assemble and dispatch
                  QString newLine = tokenList.join("");
                  int     oldPos  = mPlatformTextRange.GetBegin();
                  cursor.insertText(newLine);
                  cursor.clearSelection();
                  cursor.setPosition(oldPos);
                  GetEditor()->setTextCursor(cursor);
                  UpdateTextRange();
               }
            }
         }
      }
   }
   else
   {
      // the data item wasn't found, but the platform does exist, add it to this platform, but only if there is a valid data value
      QString     newDataStr    = aDataValue;
      QStringList dataValueList = newDataStr.split(" ");
      if (dataValueList.size() == 1)
      {
         // see if the data value is missing a unit
         bool isDouble = false;
         dataValueList[0].toDouble(&isDouble);
         if (isDouble)
         {
            if (aDataName == "heading" || aDataName == "pitch" || aDataName == "roll")
            {
               newDataStr.append(" degrees");
            }
         }
      }
      AddNewAttributeInText(aDataName, newDataStr);

      // update the proxy node
      WsfPProxyNode attribNode = GetPlatform() + aDataName.toStdString();
      if (aDataValue == "")
      {
         attribNode.GetValue().SetUnset();
      }
      else
      {
         attribNode.GetValue().SetValue(aDataValue.toStdString());
      }
   }
}

UtTextDocumentRange wizard::Platform::GetAttributeRangeByName(const QString& aAttribute)
{
   UtTextDocumentRange attribRange;

   // if this platform has a mover and a route, use its proxy node
   if (PlatformHasRoute() && !IsUsingGlobalRoute() && aAttribute == "position")
   {
      WsfPM_Waypoint wayptNode = mPlatform.Mover().InitialRoute().GetWaypoint(0);
      // if the waypoint is not a position but an offset, use the platform's position
      if (wayptNode.GetWaypointType() == WsfPM_Waypoint::cOFFSET)
      {
         WsfPProxyNode attribNode = GetPlatform() + "position";
         attribRange              = GetAttributeDocumentRange(attribNode);
      }
      else
      {
         attribRange = GetAttributeDocumentRange(wayptNode);
      }
   }
   else
   {
      WsfPProxyNode attribNode = GetPlatform() + aAttribute.toStdString();
      attribRange              = GetAttributeDocumentRange(attribNode);
   }
   return attribRange;
}

void wizard::Platform::UpdatePlatformInTextEditor(const QString& aNewLine)
{
   if (GetEditor() != nullptr)
   {
      if (aNewLine.isEmpty())
      {
         // if it's an empty line, delete it
         GetEditor()->DeleteLine();
         UpdateTextRange();
      }
      else
      {
         QString attribStr = aNewLine.trimmed().split(" ")[0];
         if (attribStr == "label")
         {
            // check to see if there is a 'position' on the same line as this label
            if (aNewLine.contains("position"))
            {
               attribStr = "position";
            }
         }
         WsfPProxyNode attribNode = GetPlatform() + attribStr.toStdString();

         // handle a 'position' attribute differently since it can have a route and waypoints
         if (attribStr == "position")
         {
            QTextCursor cursor = GetEditor()->textCursor();
            // if this platform has a mover and a route, use its document range
            if (mPlatform.Mover().IsValid() && mPlatform.Mover().InitialRoute().WaypointCount() > 0)
            {
               WsfPM_Waypoint      wayptNode   = mPlatform.Mover().InitialRoute().GetWaypoint(0);
               UtTextDocumentRange attribRange = GetAttributeDocumentRange(wayptNode);
               cursor.setPosition(attribRange.GetBegin());
               QString curTxt = cursor.block().text();
               while (!curTxt.contains("position"))
               {
                  cursor.setPosition(cursor.block().next().position());
                  curTxt = cursor.block().text();
               }
               GetEditor()->setTextCursor(cursor);
            }
            else
            {
               UtTextDocumentRange attribRange = GetAttributeDocumentRange(attribNode);
               cursor.setPosition(attribRange.GetBegin());
               GetEditor()->setTextCursor(cursor);
            }
            // store off the cursor position..
            int oldPos = cursor.position();
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            cursor.insertText(aNewLine);
            cursor.clearSelection();
            cursor.setPosition(oldPos);
            GetEditor()->setTextCursor(cursor);
            UpdateTextRange();
            return;
         }
         else
         {
            UtTextDocumentRange attribRange = GetAttributeDocumentRange(attribNode);

            QTextCursor cursor = GetEditor()->textCursor();
            cursor.setPosition(attribRange.GetBegin());
            GetEditor()->setTextCursor(cursor);

            // store off the cursor position..
            int oldPos = cursor.position();
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            cursor.insertText(aNewLine);
            cursor.clearSelection();
            cursor.setPosition(oldPos);
            GetEditor()->setTextCursor(cursor);
            UpdateTextRange();
         }
      }
   }
}

UtTextDocumentRange wizard::Platform::GetAttributeDocumentRange(WsfPProxyNode aAttrNode)
{
   UtTextDocumentRange retRange       = aAttrNode.GetSourceDocumentRange();
   WsfPProxyIndexNode* proxyIndexNode = aAttrNode.FindIndexNode();
   mEditPlatformEditorPtr             = nullptr;
   if (proxyIndexNode && !proxyIndexNode->mEntries.empty())
   {
      if (proxyIndexNode->mEntries.size() ==
          1) // there is only 1 and it's in the platform definition.  No need to look for an 'edit'
      {
         return retRange;
      }
      else
      {
         for (auto&& proxyIndexEntry : proxyIndexNode->mEntries)
         {
            const WsfParseNode* parseNode = proxyIndexEntry.mNodePtr;
            if (parseNode && parseNode->GetParent())
            {
               WsfParseNode* parent = parseNode->GetParent();
               retRange             = parent->SubtreeRange();
               mFileName            = retRange.mSource->GetFilePath().GetNormalizedPath();

               mEditPlatformEditorPtr = Project::Instance()->GotoFile(mFileName);

               QTextCursor tmpCursor = mEditPlatformEditorPtr->textCursor();
               tmpCursor.setPosition(retRange.GetBegin());
               mEditPlatformEditorPtr->setTextCursor(tmpCursor);
               QString curBlockStr = mEditPlatformEditorPtr->textCursor().block().text();
               curBlockStr         = curBlockStr.trimmed();
               // loop through block().previous() until we get to platform definition or edit platform
               // if edit platform is found, mark this as the document range we want to deal with...
               while (!curBlockStr.startsWith("platform") && !curBlockStr.startsWith("edit platform"))
               {
                  QTextBlock previousBlock   = mEditPlatformEditorPtr->textCursor().block().previous();
                  curBlockStr                = previousBlock.text();
                  QTextCursor curBlockCursor = mEditPlatformEditorPtr->textCursor();
                  curBlockCursor.setPosition(previousBlock.position());
                  mEditPlatformEditorPtr->setTextCursor(curBlockCursor);
               }
               if (curBlockStr.startsWith("edit platform"))
               {
                  return retRange;
               }
            }
         }
      }
   }
   return retRange;
}

void wizard::Platform::AddNewAttributeInText(const QString& aAttributeName, const QString& aDataValue)
{
   if (!aDataValue.isEmpty())
   {
      // Get the cursors pointing to the start and end of the platform definition
      auto platformCursor    = wizard::EditorHelpers::FindStartCursor(GetPlatform());
      auto endPlatformCursor = wizard::EditorHelpers::FindEndCursor(GetPlatform());

      auto textToInsert = ConstructTextToAppendForPlatform(platformCursor, endPlatformCursor, aAttributeName, aDataValue);
      // Consume the whitespace of between "end_platform" and the last attribute
      wizard::EditorHelpers::MoveToPreviousNonWhitespaceToken(endPlatformCursor, QTextCursor::KeepAnchor);
      endPlatformCursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
      auto text0 = endPlatformCursor.selection().toPlainText();
      // APPEND the attribute to the platform block
      endPlatformCursor.insertText(textToInsert);

      // Set the cursor to the beginning of the newly-added text
      GetEditor()->setTextCursor(endPlatformCursor);
      GetEditor()->textCursor().movePosition(QTextCursor::StartOfLine);
      GetEditor()->textCursor().clearSelection();
      int curPos = GetEditor()->textCursor().blockNumber();
      GetEditor()->GoToLine(curPos - 1);
      // Update the text range after the modification
      UpdateTextRange();
   }
}

QString wizard::Platform::ConstructTextToAppendForPlatform(const QTextCursor& aStartCursor,
                                                           const QTextCursor& aEndCursor,
                                                           const QString&     aAttributeName,
                                                           const QString&     aDataValue) const noexcept
{
   // Get the text of the current and start of definition QTextBlocks
   auto currentBlockText = aEndCursor.block().text();
   auto startBlockText   = aStartCursor.block().text();

   // Acquire the indent level of the start of definition QTextBlock
   auto startIndentLevel = wizard::EditorHelpers::GetIndentLevel(startBlockText);

   // Construct the text-to-insert
   QString textToInsert{'\n'};
   // Acquire the indent preference
   auto indentPref = ut::safe_cast<int>(wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace());
   textToInsert.append(QString{QString{startIndentLevel + indentPref, ' '} + "%1 %2\n" + QString{startIndentLevel, ' '}}
                          .arg(aAttributeName)
                          .arg(aDataValue));
   return textToInsert;
}

void wizard::Platform::AddNewRouteInText(wkf::AttachmentRoute* aRoutePtr)
{
   if (aRoutePtr != nullptr)
   {
      // insert at bottom of platform def
      QTextCursor bottomCursor = GetEditor()->textCursor();
      bottomCursor.setPosition(mPlatformTextRange.GetEnd());
      bottomCursor.setPosition(bottomCursor.block().position());

      QString routeLine    = "   route";
      int     numWaypoints = aRoutePtr->GetWaypoints().size();
      for (int i = 0; i < numWaypoints; ++i)
      {
         wkf::Waypoint* curWayPt = aRoutePtr->GetWaypoint(i);
         UtLLAPos       pos;
         curWayPt->GetLLA(pos);

         VaPosition newPos(pos.GetLat(), pos.GetLon(), pos.GetAlt());
         // insert waypoint after the given waypoint index
         QString leadingSpacesStr = "      ";
         QString posStr           = "\n";
         posStr.append(leadingSpacesStr);
         posStr.append("position ");

         std::ostringstream ossLat;
         ossLat << newPos.GetLat();
         std::ostringstream ossLon;
         ossLon << newPos.GetLon();
         std::ostringstream ossAlt;
         ossAlt << newPos.GetAlt();

         posStr.append(QString(ossLat.str().c_str()));
         posStr.append(" ");
         posStr.append(QString(ossLon.str().c_str()));
         posStr.append(" altitude ");
         posStr.append(QString(ossAlt.str().c_str()));

         routeLine += posStr;
      }
      routeLine += "\n   end_route\n";
      bottomCursor.setPosition(bottomCursor.block().previous().position());
      bottomCursor.movePosition(QTextCursor::EndOfLine);
      GetEditor()->setTextCursor(bottomCursor);
      GetEditor()->textCursor().insertText(routeLine);
      GetEditor()->textCursor().movePosition(QTextCursor::StartOfLine);
      GetEditor()->textCursor().clearSelection();
      // since a new line was added, the text range of the platform needs to be updated...
      UpdateTextRange();
      int curPos = GetEditor()->textCursor().blockNumber();
      GetEditor()->GoToLine(curPos - 1);
   }
}

void wizard::Platform::AddNewRouteInText(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
{
   QTextCursor bottomCursor = GetEditor()->textCursor();
   bottomCursor.setPosition(mPlatformTextRange.GetEnd());

   QString routeLine = "   route";
   routeLine.prepend("\n");
   bottomCursor.setPosition(bottomCursor.block().previous().position());
   bottomCursor.movePosition(QTextCursor::EndOfLine);

   QString leadingSpacesStr = "      ";
   int     curWayptIdx      = 0;
   for (auto curWaypt : aRouteInfo.mWaypoints)
   {
      // check for a label first
      if (curWaypt.mLabel != "")
      {
         // label is set
         QString labelStr = "\n" + leadingSpacesStr + "label ";
         labelStr += curWaypt.mLabel;
         routeLine += labelStr;
      }

      // write out the position
      VaPosition newPos(curWaypt.mLat, curWaypt.mLon, curWaypt.mAlt);
      QString    posStr = "\n" + leadingSpacesStr + "position ";

      std::ostringstream ossLat;
      ossLat << newPos.GetLat();
      std::ostringstream ossLon;
      ossLon << newPos.GetLon();
      std::ostringstream ossAlt;
      ossAlt << newPos.GetAlt();

      posStr.append(QString(ossLat.str().c_str()));
      posStr.append(" ");
      posStr.append(QString(ossLon.str().c_str()));
      posStr.append(" altitude ");
      posStr.append(QString(ossAlt.str().c_str()));

      // append the altitude reference string
      posStr += AltitudeReferenceToString(curWaypt.mAltRef);

      routeLine += posStr;

      if (curWaypt.mSpeed != wkf::WaypointValueType::cNOT_SET && curWaypt.mSpeed != wkf::WaypointValueType::cUSE_PREVIOUS &&
          curWaypt.mSpeed != wkf::WaypointValueType::cUSE_DEFAULT)
      {
         // speed is set
         QString speedStr = "\n" + leadingSpacesStr + "   speed ";
         speedStr += QString::number(curWaypt.mSpeed.GetAsUnit());
         speedStr += " ";
         speedStr += curWaypt.mSpeed.GetUnitName().c_str();
         routeLine += speedStr;
      }
      if (curWayptIdx == 0 && (curWaypt.mSpeed == wkf::WaypointValueType::cNOT_SET ||
                               curWaypt.mSpeed == wkf::WaypointValueType::cUSE_PREVIOUS ||
                               curWaypt.mSpeed == wkf::WaypointValueType::cUSE_DEFAULT))
      {
         // speed is not set, add a default value so route computer will correctly construct the path
         QString speedStr = "\n" + leadingSpacesStr + "   speed ";

         if (mPlatform.IsValid() && mPlatform.Mover().IsValid())
         {
            if (mPlatform.Mover().IsOfType("WSF_GROUND_MOVER"))
            {
               speedStr += "50 mph";
            }
            else
            {
               speedStr += "500 mph";
            }
         }
         else
         {
            speedStr += "500 mph";
         }
         routeLine += speedStr;
      }
      if (curWaypt.mClimbRate != wkf::WaypointValueType::cNOT_SET &&
          curWaypt.mClimbRate != wkf::WaypointValueType::cUSE_PREVIOUS &&
          curWaypt.mClimbRate != wkf::WaypointValueType::cUSE_DEFAULT)
      {
         // climbe_rate is set
         QString climbRateStr = "\n" + leadingSpacesStr + "   climb_rate ";
         climbRateStr += QString::number(curWaypt.mClimbRate.GetAsUnit());
         climbRateStr += " ";
         climbRateStr += curWaypt.mClimbRate.GetUnitName().c_str();
         routeLine += climbRateStr;
      }
      if (curWaypt.mLinAccel != wkf::WaypointValueType::cNOT_SET &&
          curWaypt.mLinAccel != wkf::WaypointValueType::cUSE_PREVIOUS &&
          curWaypt.mLinAccel != wkf::WaypointValueType::cUSE_DEFAULT)
      {
         // linear_acceleration is set
         QString linAccelStr = "\n" + leadingSpacesStr + "   linear_acceleration ";
         linAccelStr += QString::number(curWaypt.mLinAccel.GetAsUnit());
         linAccelStr += " ";
         linAccelStr += curWaypt.mLinAccel.GetUnitName().c_str();
         routeLine += linAccelStr;
      }
      // radial_acceleration/Bank Angle (bank_angle_limit)/ Maximum G's (turn_g_limit)
      if (curWaypt.mRadAccel != wkf::WaypointValueType::cNOT_SET &&
          curWaypt.mRadAccel != wkf::WaypointValueType::cUSE_PREVIOUS &&
          curWaypt.mRadAccel != wkf::WaypointValueType::cUSE_DEFAULT)
      {
         // radial_acceleration is set
         QString radAccelStr = "\n" + leadingSpacesStr + "   radial_acceleration ";
         radAccelStr += QString::number(curWaypt.mRadAccel.GetAsUnit());
         radAccelStr += " ";
         radAccelStr += curWaypt.mRadAccel.GetUnitName().c_str();
         routeLine += radAccelStr;
      }

      // Turn Direction (turn left, turn right, turn_to_heading)
      if (curWaypt.mTurnDir != 0)
      {
         // turn direction is set
         QString turnDirStr = "\n" + leadingSpacesStr;
         if (curWaypt.mTurnDir == -1)
         {
            turnDirStr += "   turn left";
         }
         else if (curWaypt.mTurnDir == 1)
         {
            turnDirStr += "   turn right";
         }
         routeLine += turnDirStr;
      }

      // Goto needs to be the last attribute added to avoid errors
      if (curWaypt.mGoTo != "")
      {
         // goto is set
         QString gotoStr = "\n" + leadingSpacesStr + "   goto ";
         gotoStr += curWaypt.mGoTo;
         routeLine += gotoStr;
      }
      ++curWayptIdx;
   }
   routeLine += "\n   end_route\n";
   bottomCursor.insertText(routeLine);
   GetEditor()->setTextCursor(bottomCursor);
   GetEditor()->textCursor().movePosition(QTextCursor::StartOfLine);
   GetEditor()->textCursor().clearSelection();
   // since a new line was added, the text range of the platform needs to be updated...
   UpdateTextRange();
   int curPos = GetEditor()->textCursor().blockNumber();
   GetEditor()->GoToLine(curPos - 1);
}

void wizard::Platform::UpdateTextRange()
{
   mPlatformTextRange = GetPlatform().GetSourceDocumentRange();
}

void wizard::Platform::ReplaceTabs(QStringList& aList)
{
   // trim the strings of any whitespace characters like '\t'
   std::vector<int> tabIdx;
   int              curIdx = 0;
   for (auto& curStr : aList)
   {
      if (curStr.contains("\t"))
      {
         tabIdx.push_back(curIdx);
      }
      curStr = curStr.trimmed();
      ++curIdx;
   }
   for (auto idx : tabIdx)
   {
      aList.insert(idx, "   ");
   }
}

void wizard::Platform::UpdateSelectedWaypoints(wkf::AttachmentRoute* aRoute) noexcept
{
   mSelectedWaypoints.clear();
   auto waypointList = aRoute->GetWaypoints();
   for (unsigned int i = 0; i < waypointList.size(); ++i)
   {
      if (waypointList[i]->IsSelected())
      {
         mSelectedWaypoints.emplace_back(i);
      }
   }
}

void wizard::Platform::RemovePlatformFromEditor()
{
   // many changes to our documents are about to happen.  The proxy informs these changes by finding the nodes of values
   // we are about to change and telling us where we can find them in file.  Unfortunately once we make a change in the
   // file, proxy nodes that point to the same file may be invalidated.  To avoid this, we sort the necessary changes
   // from the bottom of the file to the top, so that early changes never effect the location of later changes.

   // this class holds onto the deletion data.  This lets us sort everything in a single set.
   class DeleteWrapper
   {
   public:
      DeleteWrapper(UtTextDocumentRange aRange) { mRange = aRange; }
      bool operator<(const DeleteWrapper& aRHS) const
      {
         if (mRange.mSource->GetFilePath() == aRHS.mRange.mSource->GetFilePath())
         {
            if (mRange.GetBegin() == aRHS.mRange.GetBegin())
            {
               return mRange.GetEnd() >= aRHS.mRange.GetEnd();
            }
            else
            {
               return mRange.GetBegin() > aRHS.mRange.GetBegin();
            }
         }
         else
         {
            return mRange.mSource->GetFilePath() < aRHS.mRange.mSource->GetFilePath();
         }
      }
      UtTextDocumentRange GetRange() { return mRange; }

   private:
      UtTextDocumentRange mRange;
   };


   // need to check to see if there is an edit block for this platform, clear those up first, then clear out the base definition
   UtTextDocumentRange retRange       = GetPlatform().GetSourceDocumentRange();
   WsfPProxyIndexNode* proxyIndexNode = GetPlatform().FindIndexNode();
   mEditPlatformEditorPtr             = nullptr;
   if (proxyIndexNode && !proxyIndexNode->mEntries.empty())
   {
      if (proxyIndexNode->mEntries.size() ==
          1) // there is only 1 and it's in the platform definition.  No need to look for an 'edit'
      {
         mPlatformTextRange = GetPlatform().GetSourceDocumentRange();
         GetEditor()->Replace(mPlatformTextRange, "");
         return;
      }
      else
      {
         // get a list of all documents and the ranges associated with this platform
         std::map<Editor*, std::set<DeleteWrapper>> platformEditorRangeMap;
         for (auto&& proxyIndexEntry : proxyIndexNode->mEntries)
         {
            const WsfParseNode* parseNode = proxyIndexEntry.mNodePtr;
            if (parseNode && parseNode->GetParent())
            {
               WsfParseNode* parent = parseNode->GetParent();
               retRange             = parent->SubtreeRange();
               mFileName            = retRange.mSource->GetFilePath().GetNormalizedPath();

               Editor* curEditorPtr = Project::Instance()->GotoFile(mFileName);
               platformEditorRangeMap[curEditorPtr].insert(DeleteWrapper(retRange));
            }
         }
         // now apply the deletions
         for (auto&& it : platformEditorRangeMap)
         {
            std::set<DeleteWrapper> editorSet = it.second;
            for (auto curSet : editorSet)
            {
               it.first->Replace(curSet.GetRange(), "");
            }
         }
      }
   }
}

const std::vector<std::pair<wizard::Editor*, UtTextDocumentRange>> wizard::Platform::GetEditRanges()
{
   // need to check to see if there is an edit block for this platform, clear those up first, then clear out the base definition
   UtTextDocumentRange retRange       = GetPlatform().GetSourceDocumentRange();
   WsfPProxyIndexNode* proxyIndexNode = GetPlatform().FindIndexNode();
   mEditPlatformEditorPtr             = nullptr;
   std::vector<std::pair<Editor*, UtTextDocumentRange>> retval;

   if (proxyIndexNode && !proxyIndexNode->mEntries.empty())
   {
      if (proxyIndexNode->mEntries.size() ==
          1) // there is only 1 and it's in the platform definition.  No need to look for an 'edit'
      {
         mPlatformTextRange = GetPlatform().GetSourceDocumentRange();
         retval.emplace_back(std::make_pair(GetEditor(), mPlatformTextRange));
      }
      else
      {
         // get a list of all documents and the ranges associated with this platform
         for (auto&& proxyIndexEntry : proxyIndexNode->mEntries)
         {
            const WsfParseNode* parseNode = proxyIndexEntry.mNodePtr;
            if (parseNode && parseNode->GetParent())
            {
               WsfParseNode* parent = parseNode->GetParent();
               retRange             = parent->SubtreeRange();
               mFileName            = retRange.mSource->GetFilePath().GetNormalizedPath();

               Editor* curEditorPtr = Project::Instance()->GotoFile(mFileName);
               retval.emplace_back(std::make_pair(curEditorPtr, retRange));
            }
         }
      }
   }
   return retval;
}

void wizard::Platform::UpdateEditorInfo() noexcept
{
   mPlatformTextRange = GetPlatform().GetSourceDocumentRange();
   mFileName          = mPlatformTextRange.mSource->GetFilePath().GetNormalizedPath();
   mEditorPtr         = Project::Instance()->GotoFile(mFileName);
}

wizard::Editor* wizard::Platform::GetEditor() noexcept
{
   if (mEditPlatformEditorPtr != nullptr)
   {
      return mEditPlatformEditorPtr;
   }

   UpdateEditorInfo();
   return mEditorPtr;
}

bool wizard::Platform::PlatformHasMover() const noexcept
{
   return (GetPlatform().IsValid() && GetPlatform().Mover().IsValid());
}

bool wizard::Platform::IsRouteable() const noexcept
{
   if (PlatformHasMover())
   {
      return (GetPlatform().Mover().IsOfType("WSF_ROUTE_MOVER"));
   }
   return false;
}

bool wizard::Platform::PlatformHasRoute() const noexcept
{
   if (PlatformHasMover())
   {
      return GetProxyRoute().WaypointCount() > 0;
   }
   return false;
}

bool wizard::Platform::IsUsingGlobalRoute() const noexcept
{
   return PlatformHasMover() && !(GetPlatform().Mover().UsingRoute()).empty();
}

WsfPM_Route wizard::Platform::GetProxyRoute() const noexcept
{
   WsfPM_Platform platformNode{GetPlatform()};
   if (platformNode)
   {
      WsfPM_Mover moverNode{platformNode.Mover()};
      if (moverNode)
      {
         WsfPM_Route routeNode{moverNode.InitialRoute()};
         if (routeNode)
         {
            return routeNode;
         }
      }
   }
   return WsfPM_Route{};
}

void wizard::Platform::Wrap() noexcept
{
   mWrapped = true;
}

void wizard::Platform::Unwrap() noexcept
{
   mWrapped = false;
}

bool wizard::Platform::IsWrapped() const noexcept
{
   return mWrapped;
}

void wizard::Platform::OnGlobalRouteListChange(const wizard::ProxyChange& aProxyChange)
{
   if (IsUsingGlobalRoute())
   {
      wizEnv.UpdatePlatform(GetName().c_str());
   }
}
