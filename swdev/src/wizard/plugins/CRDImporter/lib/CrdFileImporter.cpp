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

#include "CrdFileImporter.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include <QFile>
#include <QStandardPaths>
#include <QString>
#include <QXmlStreamReader>

#include "CrdState.hpp"
#include "CrdTransition.hpp"
#include "Intent.hpp"
#include "OrbitIntent.hpp"
#include "Path.hpp"
#include "Point.hpp"
#include "Route.hpp"
#include "RoutePoint.hpp"
#include "UtAirspeed.hpp"
#include "UtEarth.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "UtPath.hpp"
#include "UtSphericalEarth.hpp"
#include "Vehicle.hpp"

namespace CrdImporter
{
CrdFileImporter::CrdFileImporter()
{
   mStartDate.SetDate(cINVALID_YEAR, 1, 1); // Set to a really high value
   mStartDate.SetTime(0);

   QString     openLocationDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
   std::string logfileName     = openLocationDir.toStdString() + "/crdlogfile.txt";
   mLogfileStream.open(logfileName);
}

void CrdFileImporter::Reset()
{
   mErrorCount = 0;
   mMissionList.clear();
   mStartDate.SetDate(cINVALID_YEAR, 1, 1); // Set to a really high value
   mStartDate.SetTime(0);
}

double CrdFileImporter::CalculateSpeedOfSound(double aAltitude)
{
   double speedOfSoundKtas = 60 * 60 * UtAirspeed::GetSpeedOfSoundAtAltitude_fps(aAltitude) /
                             UtMath::cFT_PER_NM; // converting to NM/hour = knots
   return speedOfSoundKtas;
}

std::string CrdFileImporter::ConvertToDms(double aValue)
{
   int sign = (aValue >= 0) ? 1 : -1;

   double absVal        = sign * aValue;
   int    degrees       = static_cast<int>(absVal);
   double degreeFrac    = absVal - degrees;
   double minutesDouble = degreeFrac * 60;
   int    minutes       = static_cast<int>(minutesDouble);
   double secDouble     = (minutesDouble - minutes) * 60;

   std::string retVal = std::to_string(degrees) + ":" + std::to_string(minutes) + ":" + std::to_string(secDouble);
   if (sign < 0)
   {
      retVal = "-" + retVal;
   }
   return retVal;
}

std::string CrdFileImporter::ConvertToUnsignedDms(double aValue)
{
   int sign = (aValue >= 0) ? 1 : -1;

   double absVal        = sign * aValue;
   int    degrees       = static_cast<int>(absVal);
   double degreeFrac    = absVal - degrees;
   double minutesDouble = degreeFrac * 60;
   int    minutes       = static_cast<int>(minutesDouble);
   double secDouble     = (minutesDouble - minutes) * 60;

   return std::to_string(degrees) + ":" + std::to_string(minutes) + ":" + std::to_string(secDouble);
}

double CrdFileImporter::ConvertToDecimalLat(const std::string& aLatitudeString)
{
   double      doubleMaxValue = std::numeric_limits<double>::max();
   double      decimalDegrees = doubleMaxValue;
   std::string direction      = aLatitudeString.substr(0, 1);
   std::string degrees        = aLatitudeString.substr(1, 2);
   std::string minutes        = aLatitudeString.substr(4, 2);
   std::string seconds        = aLatitudeString.substr(6);
   int         min            = stoi(minutes);
   double      sec            = stod(seconds);
   decimalDegrees             = stoi(degrees) + (min / 60.0) + (sec / 3600.0);
   if (decimalDegrees > 90.0)
   {
      decimalDegrees = 180 - decimalDegrees;
   }
   if (direction == "S")
   {
      decimalDegrees = -decimalDegrees; // negative for southern hemisphere
   }
   return decimalDegrees;
}

double CrdFileImporter::ConvertToDecimalLon(const std::string& aLongitudeString)
{
   double      doubleMaxValue = std::numeric_limits<double>::max();
   double      decimalDegrees = doubleMaxValue;
   std::string direction      = aLongitudeString.substr(0, 1);
   std::string degrees        = aLongitudeString.substr(1, 3);
   std::string minutes        = aLongitudeString.substr(4, 2);
   std::string seconds        = aLongitudeString.substr(6);
   int         min            = stoi(minutes);
   double      sec            = stod(seconds);
   decimalDegrees             = stoi(degrees) + (min / 60.0) + (sec / 3600.0);
   bool reversedHemisphere    = false;
   if (decimalDegrees > 180.0)
   {
      decimalDegrees     = 360 - decimalDegrees;
      reversedHemisphere = true;
   }
   if (direction == "W")
   {
      decimalDegrees = -decimalDegrees; // negative for western hemisphere
   }
   if (reversedHemisphere)
   {
      decimalDegrees = -decimalDegrees;
   }
   return decimalDegrees;
}

bool CrdFileImporter::PointsEqualWithinTolerance(const Point& aPoint1, const Point& aPoint2) const
{
   return std::abs(aPoint1.GetLat() - aPoint2.GetLat()) < mEqualityTolerance &&
          std::abs(aPoint1.GetLon() - aPoint2.GetLon()) < mEqualityTolerance;
}

int isNotAlphaNum(char c)
{
   return !isalnum(c);
}

void CrdFileImporter::ReplaceIllegalCharacters(std::string& aString)
{
   mLogfileStream << aString << ":";
   std::replace_if(aString.begin(), aString.end(), isNotAlphaNum, '_');
   std::string notLeadCharacters = "_0123456789";
   aString.erase(0, aString.find_first_not_of(notLeadCharacters));
   mLogfileStream << aString << std::endl;
}

void CrdFileImporter::OutputPointString(std::ofstream&     aOutputRouteFile,
                                        const Point*       aEntryPoint,
                                        double             aEntryPointAlt,
                                        const std::string& aEntryAltType,
                                        const Intent*      aCurrIntent,
                                        const std::string& aIndentString)
{
   if (aEntryPoint == nullptr)
   {
      return;
   }

   double doubleMaxValue = std::numeric_limits<double>::max();

   std::string pointString = aIndentString + "position ";
   pointString += aEntryPoint->GetLatString() + " ";
   pointString += aEntryPoint->GetLonString();
   // units of altitude are assumed to be feet (appendix A.37 in CRD PDF reference)

   // altitude strings for entry point
   std::string altString    = " altitude " + std::to_string(aEntryPointAlt) + " ft ";
   std::string altInputType = aEntryAltType;
   transform(altInputType.begin(), altInputType.end(), altInputType.begin(), ::tolower); // applies tolower to every
                                                                                         // char (MSG or AGL)

   altString += altInputType;
   // units of speed in crd pdf reference:
   //<xsd:enumeration value = "KTAS" / >
   //<xsd:enumeration value = "KCAS" / >
   //<xsd:enumeration value = "MACH" / >
   //<xsd:enumeration value = "KGSP" / >
   //<xsd:enumeration value = "KIAS" / >
   // looks like knots is all the same to AFSIM, so MACH is the only value that needs to get
   // converted to afsim's "kts" units
   // TODO - see if afsim has mach to knots conversion
   // in the meantime, web reference (downloaded from nipr to jmpscrdproject/mach-as.pdf) shows speed of sound
   // in knots true airspeed as 29.06*sqrt(518.7-(3.57*altitude)), a is altitude in Kft
   double speed = aCurrIntent->GetCommandedAirspeedValue();
   if (speed != doubleMaxValue && aCurrIntent->GetAirspeedType() == "MACH")
   {
      double speedOfSoundKtas = CalculateSpeedOfSound(aEntryPointAlt); // converting to NM/hour = knots
      speed                   = speed * speedOfSoundKtas;
   }

   std::string speedString;
   if (speed < doubleMaxValue)
   {
      speedString = " speed " + std::to_string(speed) + " kts";
   }

   pointString += altString + speedString; // entry point of intent
   if (mPrintAltCoordinateFormat)
   {
      if (mPointFormat == cDECIMAL_FORMAT)
      {
         // slap the lat/lon on the end of the string in DMS so the user/analyst can check against the original CRD points
         std::string crdLat = ConvertToDms(aEntryPoint->GetLat());
         std::string crdLon = ConvertToDms(aEntryPoint->GetLon());
         pointString        = pointString + "   // (" + crdLat + ", " + crdLon + ")";
      }
      else
      {
         pointString = pointString + "   //  (" + std::to_string(aEntryPoint->GetLat()) + "," +
                       std::to_string(aEntryPoint->GetLon()) + ")";
      }
   }

   SimulateOrbitIfOrbitIntent(pointString, aCurrIntent, *aEntryPoint, speed);

   aOutputRouteFile << pointString << std::endl;
}

bool CrdFileImporter::OutputRoutePoints(std::ofstream&     aOutputRouteFile,
                                        const Route*       aRoute,
                                        Path*              aCurrPath,
                                        const PointList&   aMissionPoints,
                                        double             aPathInitialAltitude,
                                        const std::string& aMsgOrAgl,
                                        int                aIndent)
{
   bool retVal = true; // assume success, till one of the error conditions requires otherwise
   // walk through the transitions, get the intent for each transition, get the entry and exit route point for the
   // intent, find the Points for the entry and exit Route Points, and get their Latitude and Longitude strings.  Then
   // compose the position tag for the AFSIM route.  The intent should have the speed and altitude
   std::vector<std::unique_ptr<CrdTransition>>* transitions = nullptr;
   ElementBase* topElement = aCurrPath; // this is the node to go back to after a sublist is finished
   if (aCurrPath != nullptr)
   {
      transitions = aCurrPath->GetTransitionList();
   }
   else
   {
      // no transitions!
      return false;
   }
   double                doubleMaxValue = std::numeric_limits<double>::max();
   CrdTransition*        currTransition = nullptr;
   const IntentList&     intents        = aRoute->GetIntentList();
   const RoutePointList& rtePoints      = aRoute->GetRoutePointList();
   const PointList&      points         = aRoute->GetPointList();
   bool                  searchError    = false;
   // CRD says that intent speed and alt values are at that exit point.
   // TODO - find what the speed and alt are for the first intent (looks like
   // altitude is in initial_intent inside Path, but don't see speed).
   // For the first pass, we'll set the alt and airspeed the same for the first
   // intent/transition, then only set the exit alt and airspeed.
   // More info - Transition has a start state reference, which has a calculated
   // true airspeed and altitude. What if the State values are inconsistent with
   // the Intent values?  Do we need to check that consistency?
   double      entryPointAlt                 = aPathInitialAltitude;
   long        nextEntryPointRef             = -1;
   long        nextExitPointRef              = -1;
   long        prevPointRef                  = -1;
   bool        descendingTransitionHierarchy = false;
   bool        ascendingTransitionHierarchy  = false;
   std::string indentString;
   for (int i = 0; i < aIndent; i++)
   {
      indentString += " ";
   }
   // use index based loop instead, since we need to look ahead to collect duplicate point data
   CrdTransition* nextTransition              = nullptr;
   size_t         index                       = 0;
   bool           finishedTransitionTraversal = false;
   if (transitions == nullptr || transitions->empty())
   {
      return false;
   }
   currTransition = transitions->at(0).get();

   std::vector<ElementBase*> transitionListOwnerStack;
   while (!finishedTransitionTraversal)
   {
      std::vector<std::unique_ptr<CrdTransition>>* subordinateTransitions = currTransition->GetTransitionList();
      double                                       exitPointAltValue      = doubleMaxValue;
      if (!ascendingTransitionHierarchy && subordinateTransitions != nullptr && !subordinateTransitions->empty())
      {
         transitionListOwnerStack.push_back(topElement);
         transitions = subordinateTransitions;
         topElement->SetNextTraversalIndex(index);
         topElement                    = currTransition;
         index                         = 0; // reset the next index to be the first in this subordinate list
         nextTransition                = subordinateTransitions->at(0).get();
         descendingTransitionHierarchy = true;
         ascendingTransitionHierarchy  = false;
      }
      else if (index + 1 < transitions->size())
      {
         nextTransition = transitions->at(index + 1).get();
      }
      else
      {
         nextTransition = nullptr;
      }

      long transitionRef = currTransition->GetId();
      long intentRef     = currTransition->GetIntentReference();
      if (intents->find(intentRef) != intents->end())
      {
         Intent* currIntent         = intents->at(intentRef).get();
         long    entryRoutePointRef = currIntent->GetEntryPointRef();
         long    exitRoutePointRef  = currIntent->GetExitPointRef();
         if (rtePoints->find(entryRoutePointRef) != rtePoints->end() &&
             rtePoints->find(exitRoutePointRef) != rtePoints->end())
         {
            long   entryPointRef = rtePoints->at(entryRoutePointRef)->GetReferencePointId();
            long   exitPointRef  = rtePoints->at(exitRoutePointRef)->GetReferencePointId();
            Point* entryPoint    = nullptr;
            Point* exitPoint     = nullptr;
            if (points.find(entryPointRef) != points.end())
            {
               entryPoint = points.at(entryPointRef).get();
            }
            if (entryPoint == nullptr)
            {
               if (aMissionPoints.find(entryPointRef) != aMissionPoints.end())
               {
                  entryPoint = aMissionPoints.at(entryPointRef).get();
               }
            }
            if (points.find(exitPointRef) != points.end())
            {
               exitPoint = points.at(exitPointRef).get();
            }
            if (exitPoint == nullptr)
            {
               if (aMissionPoints.find(exitPointRef) != aMissionPoints.end())
               {
                  exitPoint = aMissionPoints.at(exitPointRef).get();
               }
            }
            if (entryPoint != nullptr && exitPoint != nullptr)
            {
               Point* prevPoint = nullptr;
               if (prevPointRef != -1)
               {
                  if (points.find(prevPointRef) != points.end())
                  {
                     prevPoint = points.at(prevPointRef).get();
                  }
                  else
                  {
                     if (aMissionPoints.find(prevPointRef) != aMissionPoints.end())
                     {
                        prevPoint = aMissionPoints.at(prevPointRef).get();
                     }
                  }
               }

               std::string pointString   = indentString + "position ";
               long        maxLongValue  = std::numeric_limits<long>::max();
               long        nextIntentRef = maxLongValue;
               // Next Level of Detail (NLD) TBD
               // Intents are very complicated, and to get an accurate reproduction of mission rehearsal, we may have to
               // implement orbits, delays, speed intents, winds, and more.  The nextIntentSpeed will support that.
               // Currently the code commented out involving it will just move the commanded speed to the exit point
               // instead of the entry point.
               // double nextIntentSpeed = doubleMaxValue;
               long nextTransitionRef = maxLongValue;
               nextEntryPointRef      = maxLongValue;
               nextExitPointRef       = maxLongValue;
               Intent* nextIntent     = nullptr;
               if (nextTransition != nullptr)
               {
                  nextTransitionRef = nextTransition->GetId();
                  nextIntentRef     = nextTransition->GetIntentReference();
                  if (intents->find(nextIntentRef) != intents->end())
                  {
                     nextIntent = intents->at(nextIntentRef).get();
                     // NLD TBD
                     // if (nextIntent != nullptr) {
                     //   nextIntentSpeed = nextIntent->GetCommandedAirspeedValue();
                     //   const std::string& nextIntentSpeedType = nextIntent->GetAirspeedType();
                     //   if (nextIntentSpeed != doubleMaxValue){
                     //      double speedOfSoundKtas = 60 * 60 * UtAirspeed::GetSpeedOfSoundAtAltitude_fps(entryPointAlt)
                     //      / UtMath::cFT_PER_NM;  //converting to NM/hour = knots if (nextIntentSpeedType == "MACH") {
                     //         nextIntentSpeed = nextIntentSpeed * speedOfSoundKtas;
                     //      }
                     //   }
                     //}
                     long nextEntryRoutePointRef = nextIntent->GetEntryPointRef();
                     long nextExitRoutePointRef  = nextIntent->GetExitPointRef();
                     if (rtePoints->find(nextEntryRoutePointRef) != rtePoints->end() &&
                         rtePoints->find(nextExitRoutePointRef) != rtePoints->end())
                     {
                        nextEntryPointRef = rtePoints->at(nextEntryRoutePointRef)->GetReferencePointId();
                        nextExitPointRef  = rtePoints->at(nextExitRoutePointRef)->GetReferencePointId();
                     }
                  }
               }
               std::string altInputType = aMsgOrAgl;

               if (!descendingTransitionHierarchy)
               { //! ascendingTransitionHierarchy
                  pointString += entryPoint->GetLatString() + " ";
                  pointString += entryPoint->GetLonString();
                  // units of altitude are assumed to be feet (appendix A.37 in CRD PDF reference)

                  // altitude strings for entry point
                  std::string altString;
                  if (entryPointAlt != doubleMaxValue)
                  {
                     altString = " altitude " + std::to_string(entryPointAlt) + " ft ";
                  }

                  transform(altInputType.begin(),
                            altInputType.end(),
                            altInputType.begin(),
                            ::tolower); // applies tolower to every char (MSG or AGL)

                  altString += altInputType;
                  // units of speed in crd pdf reference:
                  //<xsd:enumeration value = "KTAS" / >
                  //<xsd:enumeration value = "KCAS" / >
                  //<xsd:enumeration value = "MACH" / >
                  //<xsd:enumeration value = "KGSP" / >
                  //<xsd:enumeration value = "KIAS" / >
                  // looks like knots is all the same to AFSIM, so MACH is the only value that needs to get
                  // converted to AFSIM's "kts" units
                  const std::string& speedType = currIntent->GetAirspeedType();
                  double             speed     = currIntent->GetCommandedAirspeedValue();
                  if (speed != doubleMaxValue && speedType == "MACH")
                  {
                     double speedOfSoundKtas = CalculateSpeedOfSound(entryPointAlt); // converting to NM/hour = knots
                     speed                   = speed * speedOfSoundKtas;
                  }

                  std::string speedString;
                  if (speed < doubleMaxValue)
                  {
                     speedString = " speed " + std::to_string(speed) + " kts";
                  }

                  // TODO - add indent (as in spaces, not intent)

                  if (mDebugFlag)
                  {
                     aOutputRouteFile << std::endl
                                      << "//Processing Intent " << intentRef << " from Transition " << transitionRef
                                      << std::endl;
                  }

                  if (entryPointRef != exitPointRef)
                  {
                     if ((nextEntryPointRef == entryPointRef && nextExitPointRef == exitPointRef))
                     { // same entry/exit pair as last intent
                       // need to combine data from the two intents.
                       // one pair might have alt/speed, other has turn or climb intent, but don't
                       // write both
                        exitPointAltValue = currIntent->GetCommandedAltitudeValue();
                        altInputType      = currIntent->GetAltitudeInputType();
                        if (exitPointAltValue == doubleMaxValue && nextIntent != nullptr)
                        {
                           // see it the next intent has the alt value
                           exitPointAltValue = nextIntent->GetCommandedAltitudeValue();
                           altInputType      = nextIntent->GetAltitudeInputType();
                        }
                        speed = currIntent->GetCommandedAirspeedValue();
                        if (speed == doubleMaxValue && nextIntent != nullptr)
                        {
                           speed = nextIntent->GetCommandedAirspeedValue();
                           if (speed != doubleMaxValue && speedType == "MACH")
                           {
                              double speedOfSoundKtas =
                                 CalculateSpeedOfSound(exitPointAltValue); // converting to NM/hour = knots
                              speed = speed * speedOfSoundKtas;
                           }
                           speedString.clear();
                           if (speed < doubleMaxValue)
                           {
                              speedString = " speed " + std::to_string(speed) + " kts";
                           }
                        }
                        if (entryPointAlt != doubleMaxValue)
                        {
                           altString = " altitude " + std::to_string(entryPointAlt) + " ft " + altInputType;
                        }
                        else
                        {
                           altString.clear();
                        }
                        pointString += altString + speedString; // entry point of intent

                        //...
                        // after combining, write the route record for the two points
                        // if (prevPointRef != entryPointRef) { // otherwise it was already written (like the (1,1) example intent)
                        if (prevPoint == nullptr || !PointsEqualWithinTolerance(*entryPoint, *prevPoint))
                        {
                           if (mDebugFlag)
                           {
                              std::string latString = ConvertToDms(entryPoint->GetLat());
                              std::string lonString = ConvertToDms(entryPoint->GetLon());
                              aOutputRouteFile << std::endl
                                               << "//Entry Point " << entryPointRef << " lat =  " << latString
                                               << " lon = " << lonString << std::endl;
                           }

                           if (mPrintAltCoordinateFormat)
                           {
                              if (mPointFormat == cDECIMAL_FORMAT)
                              {
                                 // slap the lat/lon on the ent of the string in DMS so the user/analyst can check
                                 // against the original CRD points
                                 std::string crdLat = ConvertToDms(entryPoint->GetLat());
                                 std::string crdLon = ConvertToDms(entryPoint->GetLon());
                                 pointString        = pointString + "   // (" + crdLat + ", " + crdLon + ")";
                              }
                              else
                              {
                                 pointString = pointString + "   //  (" + std::to_string(entryPoint->GetLat()) + "," +
                                               std::to_string(entryPoint->GetLon()) + ")";
                              }
                           }
                           SimulateOrbitIfOrbitIntent(pointString, currIntent, *entryPoint, speed);
                           aOutputRouteFile << pointString << std::endl;
                           prevPointRef = entryPointRef;
                        }
                        // NLD TBD
                        // for exit point use the speed string for the next intent
                        // Calculate here
                        // if (nextIntentSpeed != doubleMaxValue) {
                        //   speed = nextIntentSpeed;
                        //   speedString = " speed " + std::to_string(speed) + " kts";
                        //}

                        if (exitPointAltValue != doubleMaxValue)
                        {
                           altString = " altitude " + std::to_string(exitPointAltValue) + " ft " + altInputType;
                        }
                        else
                        {
                           altString.clear();
                        }

                        pointString = indentString + "position ";
                        pointString += exitPoint->GetLatString() + " ";
                        pointString += exitPoint->GetLonString();
                        pointString += altString + speedString;

                        if (mPrintAltCoordinateFormat)
                        {
                           if (mPointFormat == cDECIMAL_FORMAT)
                           {
                              // slap the lat/lon on the ent of the string in DMS so the user/analyst can check against
                              // the original CRD points
                              std::string crdLat = ConvertToDms(exitPoint->GetLat());
                              std::string crdLon = ConvertToDms(exitPoint->GetLon());
                              pointString        = pointString + "   // (" + crdLat + ", " + crdLon + ")";
                           }
                           else
                           {
                              pointString = pointString + "   //  (" + std::to_string(exitPoint->GetLat()) + "," +
                                            std::to_string(exitPoint->GetLon()) + ")";
                           }
                        }
                        SimulateOrbitIfOrbitIntent(pointString, currIntent, *entryPoint, speed);
                        aOutputRouteFile << pointString;
                        prevPointRef     = exitPointRef;
                        Point* prevPoint = nullptr;
                        if (points.find(prevPointRef) != points.end())
                        {
                           prevPoint = points.at(prevPointRef).get();
                        }
                        else
                        {
                           prevPoint = aMissionPoints.at(prevPointRef).get();
                        }
                        if (!PointsEqualWithinTolerance(*entryPoint, *prevPoint))
                        {
                           if (mDebugFlag)
                           {
                              std::string latString = ConvertToDms(entryPoint->GetLat());
                              std::string lonString = ConvertToDms(entryPoint->GetLon());
                              aOutputRouteFile << std::endl
                                               << "//Entry Point " << entryPointRef << " lat =  " << latString
                                               << " lon = " << lonString << std::endl;
                              aOutputRouteFile << "// merged two transition set of two points";
                           }
                           aOutputRouteFile << std::endl;
                        }
                        if (nextIntentRef != maxLongValue)
                        {
                           if (mDebugFlag)
                           {
                              aOutputRouteFile << std::endl
                                               << "//Processing Intent " << nextIntentRef << " from Transition "
                                               << nextTransitionRef << std::endl;
                           }
                        }
                        index += 1; // we've processed the next transition at this point, by combining it with the
                                    // current transition.
                     }
                     else
                     { // entry and exit points are NOT the same for current and next transition/intent
                        pointString += altString + speedString; // entry point of intent

                        // if (prevPointRef != entryPointRef && !ascendingTransitionHierarchy) { // otherwise it was
                        // already written (like the (1,1) example intent)

                        if (prevPoint == nullptr ||
                            (!PointsEqualWithinTolerance(*entryPoint, *prevPoint) && !ascendingTransitionHierarchy))
                        {
                           if (mDebugFlag)
                           {
                              std::string latString = ConvertToDms(entryPoint->GetLat());
                              std::string lonString = ConvertToDms(entryPoint->GetLon());
                              aOutputRouteFile << std::endl
                                               << "//Entry Point " << entryPointRef << " lat =  " << latString
                                               << " lon = " << lonString << std::endl;
                           }
                           if (mPrintAltCoordinateFormat)
                           {
                              if (mPointFormat == cDECIMAL_FORMAT)
                              {
                                 // slap the lat/lon on the ent of the string in DMS so the user/analyst can check
                                 // against the original CRD points
                                 std::string crdLat = ConvertToDms(entryPoint->GetLat());
                                 std::string crdLon = ConvertToDms(entryPoint->GetLon());
                                 pointString        = pointString + "   // (" + crdLat + ", " + crdLon + ")";
                              }
                              else
                              {
                                 pointString = pointString + "   //  (" + std::to_string(entryPoint->GetLat()) + "," +
                                               std::to_string(entryPoint->GetLon()) + ")";
                              }
                           }
                           SimulateOrbitIfOrbitIntent(pointString, currIntent, *entryPoint, speed);
                           aOutputRouteFile << pointString << std::endl;
                           prevPointRef = entryPointRef;
                        }
                        exitPointAltValue = currIntent->GetCommandedAltitudeValue();
                        altInputType      = currIntent->GetAltitudeInputType();
                        transform(altInputType.begin(),
                                  altInputType.end(),
                                  altInputType.begin(),
                                  ::tolower); // applies tolower to every char (MSG or AGL)
                        if (exitPointAltValue != doubleMaxValue)
                        {
                           altString = " altitude " + std::to_string(exitPointAltValue) + " ft " + altInputType;
                        }
                        else
                        {
                           altString.clear();
                        }

                        if (!(exitPointAltValue == doubleMaxValue && topElement != aCurrPath &&
                              index == transitions->size() - 1))
                        {
                           // if clause prevents the exit point of the last transition in a sub-transition_list from
                           // printing here. The last transition typically doesn't have the speed or altitude attached,
                           // this is in the exit point of the parent transition.  So instead, we print out the exit
                           // point of the parent on ascending out of the sub list.

                           if (!PointsEqualWithinTolerance(*entryPoint, *exitPoint))
                           {
                              pointString = indentString + "position ";
                              pointString += exitPoint->GetLatString() + " ";
                              pointString += exitPoint->GetLonString();

                              if (mDebugFlag)
                              {
                                 std::string latString = ConvertToDms(exitPoint->GetLat());
                                 std::string lonString = ConvertToDms(exitPoint->GetLon());
                                 aOutputRouteFile << std::endl
                                                  << "//Exit Point " << exitPointRef << " lat =  " << latString
                                                  << " lon = " << lonString << std::endl;
                              }
                              // NLD TBD
                              // for exit point use the speed string for the next intent
                              // Calculate here
                              // if (nextIntentSpeed != doubleMaxValue) {
                              //   speed = nextIntentSpeed;
                              //   speedString = " speed " + std::to_string(speed) + " kts";
                              //}
                              pointString += altString + speedString;
                              if (mPrintAltCoordinateFormat)
                              {
                                 if (mPointFormat == cDECIMAL_FORMAT)
                                 {
                                    // slap the lat/lon on the ent of the string in DMS so the user/analyst can check
                                    // against the original CRD points
                                    std::string crdLat = ConvertToDms(exitPoint->GetLat());
                                    std::string crdLon = ConvertToDms(exitPoint->GetLon());
                                    pointString        = pointString + "   // (" + crdLat + ", " + crdLon + ")";
                                 }
                                 else
                                 {
                                    pointString = pointString + "   //  (" + std::to_string(exitPoint->GetLat()) + "," +
                                                  std::to_string(exitPoint->GetLon()) + ")";
                                 }
                              }
                              SimulateOrbitIfOrbitIntent(pointString, currIntent, *entryPoint, speed);
                              aOutputRouteFile << pointString;
                              prevPointRef = exitPointRef;
                              if (mDebugFlag)
                              {
                                 aOutputRouteFile << "// non-merged two point set";
                              }
                              aOutputRouteFile << std::endl; // exit point of intent
                           }
                        }
                     }
                  }
                  else
                  { // entry point ref equals exit point ref
                     exitPointAltValue = entryPointAlt;
                     // NLD TBD
                     //  for exit point use the speed string for the next intent
                     //  Calculate here
                     // if (nextIntentSpeed != doubleMaxValue) {
                     //    speed = nextIntentSpeed;
                     //    speedString = " speed " + std::to_string(speed) + " kts";
                     // }

                     pointString += altString + speedString; // entry point of intent

                     // if (prevPointRef != entryPointRef) {
                     if (prevPoint == nullptr || !PointsEqualWithinTolerance(*entryPoint, *prevPoint))
                     {
                        if (mDebugFlag)
                        {
                           std::string latString = ConvertToDms(entryPoint->GetLat());
                           std::string lonString = ConvertToDms(entryPoint->GetLon());
                           aOutputRouteFile << std::endl
                                            << "//Entry Point " << entryPointRef << " lat =  " << latString
                                            << " lon = " << lonString << std::endl;
                        }

                        if (mPrintAltCoordinateFormat)
                        {
                           if (mPointFormat == cDECIMAL_FORMAT)
                           {
                              // slap the lat/lon on the ent of the string in DMS so the user/analyst can check against
                              // the original CRD points
                              std::string crdLat = ConvertToDms(entryPoint->GetLat());
                              std::string crdLon = ConvertToDms(entryPoint->GetLon());
                              pointString        = pointString + "   // (" + crdLat + ", " + crdLon + ")";
                           }
                           else
                           {
                              pointString = pointString + "   //  (" + std::to_string(entryPoint->GetLat()) + "," +
                                            std::to_string(entryPoint->GetLon()) + ")";
                           }
                        }
                        SimulateOrbitIfOrbitIntent(pointString, currIntent, *entryPoint, speed);
                        aOutputRouteFile << pointString << std::endl;
                        prevPointRef = entryPointRef;
                     }
                     else // previous point already written, this one will be ignore but we must adjust the previous one
                          // to account for orbit by appending the pause time any way.
                     {
                        std::string appendAtttribute = "";
                        SimulateOrbitIfOrbitIntent(appendAtttribute, currIntent, *entryPoint, speed);
                        aOutputRouteFile << appendAtttribute << std::endl;
                     }
                     if (mDebugFlag)
                     {
                        aOutputRouteFile << "// Duplicate point detected, point reference " << entryPointRef << std::endl;
                     }
                  }
                  index++;
               }
               else if (descendingTransitionHierarchy)
               {
                  // we're going into a transition list of a transition.  From the (very) few examples I've seen,
                  // the entry point of the parent transition should match the entry point of the first transition in
                  // the transition_list. most all of the info we need for the entry point is in the parent node (again,
                  // in the few examples I've seen). At this point, we definitely don't want to use the exit point of
                  // the parent node, as that point may be way out of sequence of the real path (there could be a bunch
                  // of points in this sub-transition list that need to go first).
                  if (mDebugFlag)
                  {
                     aOutputRouteFile << std::endl
                                      << "//Processing Intent (sub-transition descent)" << intentRef
                                      << " from Transition " << transitionRef << std::endl;
                  }
                  if (entryPointRef == nextEntryPointRef &&
                      (prevPoint == nullptr || !PointsEqualWithinTolerance(*entryPoint, *prevPoint)))
                  {
                     // this is our assumption based on files seen so far!
                     if (mDebugFlag)
                     {
                        std::string latString = ConvertToDms(entryPoint->GetLat());
                        std::string lonString = ConvertToDms(entryPoint->GetLon());
                        aOutputRouteFile << std::endl
                                         << "//Entry Point " << entryPointRef << " lat =  " << latString
                                         << " lon = " << lonString << std::endl;
                     }

                     OutputPointString(aOutputRouteFile, entryPoint, entryPointAlt, aMsgOrAgl, currIntent, indentString);
                     // NLD TBD
                     //  for exit point use the speed string for the next intent
                     //  Calculate here
                     //  Think if we pass in the next intent instead of the currIntent, it might get the correct leg
                     //  speed for our AFSIM position point
                     // if (nextIntentSpeed != doubleMaxValue) {
                     //    OutputPointString(aOutputRouteFile, entryPoint, entryPointAlt, aMsgOrAgl, nextIntent,
                     //    indentString);
                     // }
                     // else
                     //{
                     //    OutputPointString(aOutputRouteFile, entryPoint, entryPointAlt, aMsgOrAgl, currIntent,
                     //    indentString);
                     // }
                  }
                  else
                  {
                     if (entryPointRef != nextEntryPointRef)
                     {
                        mLogfileStream << "Assumed file format error:  first point in transition_list has a different "
                                          "entry point than parent transition entry point."
                                       << std::endl;
                     }
                  }
                  prevPointRef = entryPointRef;
               }
               // else if (ascendingTransitionHierarchy) {
               //	// leaving a sub transition list.  The parent transition is the current transition. This needs to write
               // the exit point
               //	// of the parent transition..  Most of the information we need seems to be in the parent transition for
               //	//  the parent's entry and exit points.
               //	exitPointAltValue = currIntent->GetCommandedAltitudeValue();
               //	altInputType = currIntent->GetAltitudeInputType();
               //	if (exitPointRef == nextExitPointRef){
               //		OutputPointString(outputRouteFile, exitPoint, exitPointAltValue, altInputType, currIntent, indentString);
               //	}
               //	else {
               //		mLogfileStream << "Assumed file error:  first point in transition_list has a different entry point
               // than parent transition entry point." << std::endl;
               //	}
               //	prevPointRef = exitPointRef;
               // }
            }
            else
            {
               searchError = true;
               mLogfileStream << "Could not find points for Transition ID=" << transitionRef << std::endl;
            }
         }
         else
         {
            searchError = true;
            mLogfileStream << "Could not find route points for Transition ID=" << transitionRef << std::endl;
         }
      }
      else
      {
         searchError = true;
         mLogfileStream << "Could not find intent ID=" << intentRef << "for Transition ID=" << transitionRef << std::endl;
      }

      if (searchError)
      {
         retVal = false;
         return retVal;
      }
      // Mach change
      if (exitPointAltValue != doubleMaxValue)
      { // otherwise leave it as is - some intents won't have altitudes
         entryPointAlt = exitPointAltValue;
      }

      if (index > transitions->size() - 1)
      {
         // either done if this is the top list of transitions from the Path element, OR
         // need to pop up to the next level of transition_list
         if (transitionListOwnerStack.empty())
         {
            finishedTransitionTraversal = true;
            currTransition              = nullptr;
         }
         else
         {
            topElement  = transitionListOwnerStack.back();
            index       = topElement->GetNextTraversalIndex();
            transitions = topElement->GetTransitionList();

            if (index < transitions->size())
            {
               currTransition = transitions->at(index).get();
            }
            else
            {
               currTransition = nullptr;
            }

            transitionListOwnerStack.pop_back();
            descendingTransitionHierarchy = false;
            ascendingTransitionHierarchy  = true;
         }
      }
      else
      {
         // get the next transition in this list (or sub-list)
         currTransition                = transitions->at(index).get();
         descendingTransitionHierarchy = false;
         ascendingTransitionHierarchy  = false;
      }
   }
   return retVal;
}

bool CrdFileImporter::WritePathsForRoute(const Route*       aRoute,
                                         const PointList&   aMissionPoints,
                                         std::ofstream&     aOutputMissionFile,
                                         const std::string& aRouteNameString)
{
   // need to iterate through the paths
   bool            retVal              = true; // assume successful write until proven otherwise
   const PathList& paths               = aRoute->GetPathList();
   auto            it                  = paths->begin();
   double          pathInitialAltitude = 0.0;
   std::string     pathAltType; // MSG OR AGL
   while (it != paths->end())
   {
      Path*              currPath = (*it).second.get();
      const std::string& pathType = currPath->GetPathType();
      pathInitialAltitude         = currPath->GetInitialPathAltitude();
      pathAltType                 = currPath->GetAltType();
      mLogfileStream << "Writing CRD Path: " << std::to_string(currPath->GetId())
                     << " type: " << currPath->GetPathType() << std::endl;

      UtCalendar creationTime;
      creationTime.SetDate(cINVALID_YEAR, 1, 1);
      const std::vector<std::unique_ptr<CrdTransition>>* transitionList = currPath->GetTransitionList();
      if (!transitionList->empty())
      {
         long             startStateReference = transitionList->at(0)->GetStartStateReference();
         const StateList& stateList           = aRoute->GetStateList();
         auto             iter                = stateList->find(startStateReference);
         if (iter != stateList->end())
         {
            creationTime = iter->second->GetClockDate();
         }
      }

      // write information about this Path
      aOutputMissionFile << std::endl << "//  Path ID: " << std::to_string(currPath->GetId()) << std::endl;
      aOutputMissionFile << "//     Type: " << currPath->GetPathType() << std::endl;
      aOutputMissionFile << "//     Path Start time:  " << currPath->GetDate() << " " << currPath->GetTime() << std::endl;

      std::string routeName = aRouteNameString + "_Path" + pathType + std::to_string(currPath->GetId());
      aOutputMissionFile << "route " << routeName << std::endl;
      int indent = 3;
      // CrdTransition* topTransition = nullptr;
      bool success =
         OutputRoutePoints(aOutputMissionFile, aRoute, currPath, aMissionPoints, pathInitialAltitude, pathAltType, indent);
      if (success)
      {
         aOutputMissionFile << "end_route " << std::endl << std::endl;
         if (mGeneratePlatformStubs)
         {
            WritePlatformStub(aOutputMissionFile, routeName, creationTime);
            aOutputMissionFile << std::endl;
         }
         mLogfileStream << "Completed writing CRD Path: " << std::to_string(currPath->GetId())
                        << " type: " << currPath->GetPathType() << std::endl;
      }
      else
      {
         aOutputMissionFile
            << ">>>>ERROR in CRD XML content.  Check for missing points, route points, or transition elements. <<<<"
            << std::endl;
         mLogfileStream << "Error writing CRD Path " << std::to_string(currPath->GetId())
                        << " type: " << currPath->GetPathType() << std::endl;
         retVal = false;
         break;
      }
      ++it;
   }
   return retVal;
}

std::vector<std::string> CrdFileImporter::WriteOutputRoutes(const std::string& aInputCrdFilename)
{
   // our route files will actually each be a path
   std::vector<std::string> retVal; // list of filenames of mission routes generated from a single crd file

   for (const auto& mission : mMissionList)
   {
      const std::string& missionName       = mission->GetMissionName();
      std::string        routeBaseFilename = missionName;
      const PointList&   missionPointList  = mission->GetPointList();
      std::string        routeNameString;
      std::string        filenameOnly;

      UtPath outputFolderPath(mOutputFolder);
      if (!outputFolderPath.Exists())
      {
         mLogfileStream << "Output directory is " << outputFolderPath.GetRealPath().GetSystemPath().c_str() << std::endl;
         // outputFolderPath.Mkdir();  //had to use below realPath to get the directory created.  Strange thing is it
         // didn't crash trying to write files
         outputFolderPath.GetRealPath().Mkdir();
      }

      if (!mOutputFileBaseString.empty())
      {
         routeBaseFilename = mOutputFileBaseString + "_" + routeBaseFilename;
      }
      filenameOnly           = routeBaseFilename;
      std::string folderPart = "";
      if (!mOutputFolder.empty())
      {
         routeBaseFilename = mOutputFolder + "/" + routeBaseFilename;
         folderPart        = mOutputFolder + "/";
      }

      if (mProcessedMissionNamesCurrentSequenceNumbers.find(missionName) !=
          mProcessedMissionNamesCurrentSequenceNumbers.end())
      {
         // already have a sequence number stored for this mission, need to increment it for this instance of the
         // mission name and store it back
         int newSequenceNumber = mProcessedMissionNamesCurrentSequenceNumbers.at(missionName);
         newSequenceNumber++;
         routeBaseFilename = routeBaseFilename + "_" + std::to_string(newSequenceNumber);
         mProcessedMissionNamesCurrentSequenceNumbers[missionName] = newSequenceNumber;
      }
      else
      {
         // we haven't created the first of potentially several files for this mission name for this import click yet
         // if there are existing files that start with this routeBaseFilename in this folder, we need to move them from
         // this folder to a folder named "old". Was going to delete them, but this lib doesn't know about Qt. ParseCrdFiles()
         // is called from the Wizard dialog to parse all files at once.  so if one file is found that is duplicated, we
         // can't send a message to the dialog till after they're all done asking if it's ok to delete.
         int newSequenceNumber                                     = 1;
         mProcessedMissionNamesCurrentSequenceNumbers[missionName] = newSequenceNumber;
         routeBaseFilename         = routeBaseFilename + "_" + std::to_string(newSequenceNumber);
         std::string firstFilename = filenameOnly + "_" + std::to_string(newSequenceNumber) + ".txt";
         UtPath      firstFilePath(folderPart + firstFilename);
         if (firstFilePath.Exists())
         {
            UtPath outputFolderOldPath(folderPart + "old");
            if (!outputFolderOldPath.Exists())
            {
               outputFolderOldPath.GetRealPath().Mkdir();
            }
            // for each file, copy it to old, then delete it
            std::vector<std::string> files;
            std::vector<std::string> directories;
            outputFolderPath.ListDir(files, directories);
            if (!files.empty())
            {
               std::string suffix = ".crd";
               // we are going to match files with ignore case for now, assuming they won't just change case for a
               // different set of crd missions
               std::string lowerCaseFilenameOnly = filenameOnly;
               transform(lowerCaseFilenameOnly.begin(), lowerCaseFilenameOnly.end(), lowerCaseFilenameOnly.begin(), ::tolower);
               for (const std::string& fileInDir : files)
               {
                  std::string lowerCaseCurrentFilename = fileInDir;
                  transform(lowerCaseCurrentFilename.begin(),
                            lowerCaseCurrentFilename.end(),
                            lowerCaseCurrentFilename.begin(),
                            ::tolower);
                  auto   filenameSize      = fileInDir.size();
                  size_t filenameOnlyFound = lowerCaseCurrentFilename.find(lowerCaseFilenameOnly + "_");
                  if (filenameOnlyFound == 0)
                  {
                     size_t suffixFound = lowerCaseCurrentFilename.find(".txt");
                     if (suffixFound == filenameSize - 4)
                     { // 4 is the size of .txt, find_last_of returns last character that matches
                       //  which should be the last t in .txt, which is the last char in the string
                        if (filenameSize - filenameOnly.size() - 4 < 5)
                        {
                           // no more than 999 sequence numbers allowed 4 = size of 999 + an underscore which is < 5
                           // at this point we've established that this file should be moved to the old folder
                           UtPath fileToMovePath(folderPart + fileInDir);
                           // std::string fileToMove = fileToMovePath.GetFileName();
                           std::string newPathAndName = folderPart + "old/" + fileInDir;
                           bool        successfulMove = fileToMovePath.Rename(newPathAndName);
                           if (!successfulMove)
                           {
                              // Rename doesn't work if the new path and name for the file already exists.
                              // In this case, delete the file in old, and try the rename again.
                              UtPath newPath(newPathAndName);
                              if (newPath.Exists())
                              {
                                 bool successfulDelete = newPath.RemoveFile();
                                 if (!successfulDelete)
                                 {
                                    std::cout
                                       << "Unable to delete an old file from the backup folder, check file permissions."
                                       << std::endl;
                                 }
                                 else
                                 {
                                    successfulMove = fileToMovePath.Rename(newPathAndName);
                                 }
                                 if (!successfulMove)
                                 {
                                    std::cout
                                       << "Unable to move an old file to the backup folder, check file permissions."
                                       << std::endl;
                                 }
                              }
                           }
                        }
                     }
                  }
                  std::cout << "existing file found: " << fileInDir << std::endl;
               }
            }
         }
      }
      std::string   fullFilename = routeBaseFilename + ".txt";
      std::ofstream outputMissionFile;
      outputMissionFile.open(fullFilename, std::ofstream::out);
      mLogfileStream << "Writing mission file: " << missionName << std::endl;
      WriteMissionFileHeader(aInputCrdFilename, mission.get(), outputMissionFile);

      bool successfulWrite = true;
      for (const auto& route : mission->GetRouteList())
      {
         const Route*       currRoute     = route.get(); // routePair.second.get();
         const std::string& routeName     = currRoute->GetName();
         std::string        vehicleString = std::to_string(currRoute->GetVehicleReference());

         routeNameString = routeName + "_" + vehicleString;
         mLogfileStream << "Writing CRD Route: " << routeName << std::endl;
         WriteRouteHeader(routeName, vehicleString, outputMissionFile);
         successfulWrite = WritePathsForRoute(currRoute, missionPointList, outputMissionFile, routeNameString);

         if (!successfulWrite)
         {
            mLogfileStream << "<<<ERROR>>>. CRD XML content was missing point, route point, or transition elements."
                           << std::endl;
            break;
         }
         mLogfileStream << "Completed writing CRD Route: " << routeName << std::endl;
      }

      if (successfulWrite)
      {
         outputMissionFile.close();
         mLogfileStream << "Completed mission file: " << missionName << std::endl;
         retVal.push_back(fullFilename);
      }
      else
      {
         outputMissionFile.clear();
         outputMissionFile.close();
         mLogfileStream << "Aborted processing of mission file: " << missionName
                        << ".  Mission point, route point, or transition elements." << std::endl;
      }
   }
   return retVal;
}

void CrdFileImporter::CancelCurrentImport()
{
   mCancelFlag = true;
}

void CrdFileImporter::WriteRouteHeader(const std::string& aRouteName,
                                       const std::string& aVehicleString,
                                       std::ofstream&     aOutputMissionFile)
{
   aOutputMissionFile << "//  Route Name: " << aRouteName << std::endl;
   aOutputMissionFile << "//  Vehicle ID:  " << aVehicleString << std::endl;
}

void CrdFileImporter::WriteMissionFileHeader(const std::string& aSourceCrdFilename,
                                             const CrdMission*  aMission,
                                             std::ofstream&     aOutputMissionFile)
{
   aOutputMissionFile << std::endl;
   aOutputMissionFile << "//  Mission ID        :  " << std::to_string(aMission->GetId()) << std::endl;
   aOutputMissionFile << "//  Mission Name      :  " << aMission->GetName() << std::endl;
   aOutputMissionFile << "//  DAFIF_DATE_TIME   :  " << aMission->GetDafifTime() << std::endl;
   aOutputMissionFile << "//  CREATION_DATE_TIME:  " << aMission->GetCreationTime() << std::endl;
   aOutputMissionFile << "//  Input CRD filename:  " << aSourceCrdFilename << std::endl;

   aOutputMissionFile << std::endl;
}

void CrdFileImporter::ParseCrdFiles()
{
   mCancelFlag = false;
   mProcessedMissionNamesCurrentSequenceNumbers.clear(); // reset all known missions for each import sequence.
   std::vector<std::string>::const_iterator it = mSourceFilenames.begin();
   while (it != mSourceFilenames.end() && !mCancelFlag)
   {
      const std::string& currFilename   = *it;
      auto               filesProcessed = ParseCrdFile(currFilename);
      if (filesProcessed.size() < mMissionList.size())
      {
         // indicator that one or more missions failed to parse, or there were missing points, route points, or transitions
         filesProcessed.clear();
         mErrorCount++;
         filesProcessed.emplace_back(
            "ERROR in processing file.  Incorrect XML, or missing points, route points, or transition elements");
      }
      mInputOutputFilenamesMap[currFilename] = std::move(filesProcessed);
      ++it;
      Reset();
   }

   WriteIncludeAllFile();

   if (mCancelFlag)
   {
      Reset();
   }
}

std::vector<std::string> CrdFileImporter::ParseCrdFile(const std::string& aInputCrdFilename)
{
   double                   doubleMaxValue = std::numeric_limits<double>::max();
   std::vector<std::string> retVal;
   // locals needed for a CRD file parse
   int         transitionNestingLevel    = 0;
   bool        popEnclosingElementNeeded = false;
   std::string characters;
   std::string airspeedType;
   std::string name;        // MISSION, POINT, and ROUTE have NAME as a sub-element
   std::string missionName; // for some reason Mission has its own name tag, instead of using name as above
   std::string pathType;
   double      airspeedValue = doubleMaxValue;
   double      altitudeValue = doubleMaxValue;
   std::string altitudeInputType;
   Intent*     intent = nullptr;
   std::unique_ptr<OrbitIntent>                           orbitIntent;
   std::string                                            pointLatitudeString;
   std::string                                            pointLongitudeString;
   std::string                                            dafifDateTime;
   std::string                                            creationDateTime;
   std::string                                            pathDate;
   std::string                                            pathTime;
   std::string                                            inputType;
   std::unique_ptr<Point>                                 point      = nullptr;
   std::unique_ptr<RoutePoint>                            routePoint = nullptr;
   std::vector<std::unique_ptr<Point>>                    pointList;
   RoutePointList                                         routePointList;
   std::vector<std::unique_ptr<Route>>                    routeList;
   IntentList                                             intentListPtr;
   std::unique_ptr<std::map<long, std::unique_ptr<Path>>> pathList;
   VehicleList vehicleList; // = ut::make_unique< std::map<long, std::unique_ptr<Vehicle>> >();
   StateList   stateList;
   std::vector<std::unique_ptr<CrdTransition>>* transitionList = nullptr;

   Route*                      route      = nullptr;
   Path*                       path       = nullptr;
   std::unique_ptr<CrdMission> mission    = nullptr;
   CrdTransition*              transition = nullptr;
   CrdState*                   state;
   std::unique_ptr<Vehicle>    vehicle;
   long                        vehicleReference                  = 0;
   long                        id                                = 0;
   long                        commandedEntryRoutePointReference = 0;
   long                        commandedExitRoutePointReference  = 0;
   double                      value                             = doubleMaxValue;
   long                        intentReference                   = 0;
   long                        startStateReference               = 0;
   long                        endStateReference                 = 0;
   long                        pointReference                    = 0;
   auto                        unNeededParentElement             = ut::make_unique<ElementBase>();


   QString xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                 "<name>" //\n"
                 "  New York"
                 " <id>1</id>\n"
                 " Dayton"
                 "</name>\n";
   // FILE* f1 = fopen(aInputCrdFilename.c_str(), "r");
   std::ifstream inputFile(aInputCrdFilename.c_str());

   ;
   // char buffer[2000];
   // size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, f1);
   // mLogfileStream << bytesRead << " bytes read." << std::endl;
   // buffer[1999] = 0;
   if (!inputFile.good()) //(f1 == nullptr)
   {
      mLogfileStream << "File not opened successfully." << std::endl;
   }
   else
   {
      inputFile.close(); // fclose(f1);
   }
   QFile xmlFile(aInputCrdFilename.c_str());
   mLogfileStream << "Processing " << aInputCrdFilename.c_str() << std::endl;
   bool fileOpenSuccess = xmlFile.open(QIODevice::ReadOnly);
   if (fileOpenSuccess)
   {
      QXmlStreamReader xmlReader(&xmlFile);
      std::string      su = xml.toStdString();
      QXmlStreamReader xmlReader2(xml);
      mNewParentElement       = nullptr;
      mEnclosingParentElement = nullptr;
      bool endTagFlag         = false;
      bool startTagFlag       = false;
      bool contentFlag        = false;
      while (!xmlReader.atEnd() && !xmlReader.hasError() && !mCancelFlag)
      {
         endTagFlag   = false;
         startTagFlag = false;
         contentFlag  = false;
         xmlReader.readNext();
         std::string tagName = xmlReader.name().toString().toStdString();
         if (mDebugFlag)
         {
            mLogfileStream << "Line: " << xmlReader.lineNumber()
                           << ": tokenString = " << xmlReader.tokenString().toStdString().c_str() << " ";
            mLogfileStream << "tag = " << tagName.c_str() << " " << std::endl;
         }
         QString     qstr    = xmlReader.text().toString().trimmed();
         std::string content = qstr.toStdString();

         // looking at the schema for CRD, it looks like that elements can contain elements OR text, not both.  I did
         // not verify this for all tags, there are 50 pages of schema. Schema distinguishes between simple types and
         // complex types the former being text, and the latter being elements. Latitude and Longitude are examples of
         // simple types, Point and Point_list are examples of complex types.

         //////////////////   Beginning of Start Tag Processing

         if (xmlReader.isStartElement())
         {
            startTagFlag = true;
            characters.clear();
            mNewParentElement = nullptr; // this will be set if we enter a complex tag which needs subelements assigned
                                         // as they are completed
            if (tagName == "POINT")
            {
               point             = ut::make_unique<Point>();
               mNewParentElement = point.get();
            }
            else if (tagName == "POINT_REFERENCE")
            {
            }
            else if (tagName == "ID")
            {
               id = INT_MAX;
            }
            else if (tagName == "NAME")
            {
            }
            else if (tagName == "POINT_LIST")
            {
               pointList.clear(); // old points have been copied to their route in end tag handler, deletion is handled
                                  // when the route is deleted
            }
            else if (tagName == "ROUTE_POINT")
            {
               // routePoint.reset(new RoutePoint());
               routePoint        = ut::make_unique<RoutePoint>();
               mNewParentElement = routePoint.get();
            }
            else if (tagName == "ROUTE_POINT_LIST")
            {
               if (routePointList == nullptr)
               {
                  // routePointList.reset(new std::map<long, std::unique_ptr<RoutePoint>>());
                  routePointList = ut::make_unique<std::map<long, std::unique_ptr<RoutePoint>>>();
               }
               else
               {
                  routePointList->clear();
               }
            }
            else if (tagName == "ROUTE")
            {
               route             = new Route();
               mNewParentElement = route;
            }
            else if (tagName == "INTENT")
            {
               intent            = new Intent();
               mNewParentElement = intent;
            }
            else if (tagName == "ORBIT_INTENT")
            {
               orbitIntent       = ut::make_unique<OrbitIntent>();
               mNewParentElement = orbitIntent.get();
            }
            else if (tagName == "AUTO_TIME_FLAG")
            {
            }
            else if (tagName == "BANK_ANGLE")
            {
            }
            else if (tagName == "CLOCK_TYPE")
            {
            }
            else if (tagName == "CLOCK_DATE")
            {
            }
            else if (tagName == "CLOCK_TIME")
            {
            }
            else if (tagName == "COMMANDED_COMPLETION_TIME")
            {
            }
            else if (tagName == "INBOUND_TRUE_COURSE")
            {
            }
            else if (tagName == "LEG_DISTANCE")
            {
            }
            else if (tagName == "REFERENCE_POINT")
            {
            }
            else if (tagName == "TURN_RIGHT")
            {
            }
            else if (tagName == "PATH")
            {
               path              = new Path();
               mNewParentElement = path;
               transitionList =
                  nullptr; // need to reinitialize this so the last list for previous path is not used on the next one
            }
            else if (tagName == "PATH_TYPE")
            {
            }
            else if (tagName == "DATE")
            {
            }
            else if (tagName == "TIME")
            {
            }
            else if (tagName == "VEHICLE")
            {
               vehicle           = ut::make_unique<Vehicle>();
               mNewParentElement = vehicle.get();
            }
            else if (tagName == "STATE")
            {
               state             = new CrdState();
               mNewParentElement = state;
            }
            else if (tagName == "TRANSITION")
            {
               transition        = new CrdTransition();
               mNewParentElement = transition;
            }
            else if (tagName == "INTENT_REFERENCE")
            {
            }
            else if (tagName == "START_STATE_REFERENCE")
            {
            }
            else if (tagName == "END_STATE_REFERENCE")
            {
            }
            else if (tagName == "CLIMB_STYLE")
            {
            }
            else if (tagName == "CLIMB_DIFFERENTIAL")
            {
            }
            else if (tagName == "DESCENT_STYLE")
            {
            }
            else if (tagName == "DESCENT_DIFFERENTIAL")
            {
            }
            else if (tagName == "TURN_TYPE")
            {
            }
            else if (tagName == "MISSION")
            {
               mission           = ut::make_unique<CrdMission>();
               mNewParentElement = mission.get();
            }
            else if (tagName == "DAFIF_DATE_TIME")
            {
            }
            else if (tagName == "CREATION_DATE_TIME")
            {
            }
            else if (tagName == "MISSION_NAME")
            {
            }
            else if (tagName == "CRD")
            {
               // top level tag, don't need to do anything here
            }
            else if (tagName == "COMMANDED_ENTRY_ROUTE_POINT_REFERENCE")
            {
            }
            else if (tagName == "COMMANDED_EXIT_ROUTE_POINT_REFERENCE")
            {
            }
            else if (tagName == "LATITUDE")
            {
               pointLatitudeString.clear();
            }
            else if (tagName == "LONGITUDE")
            {
               pointLongitudeString.clear();
            }
            else if (tagName == "COMMANDED_ALTITUDE")
            {
            }
            else if (tagName == "ALTITUDE")
            {
            }
            else if (tagName == "VALUE")
            {
            }
            else if (tagName == "ALTITUDE_TYPE")
            {
            }
            else if (tagName == "INITIAL_STATE_INTENT")
            {
            }
            else if (tagName == "COMMANDED_LEG_AIRSPEED")
            {
               // set the airspeed from the value, then reset the flag to false
            }
            else if (tagName == "AIRSPEED")
            {
               // other tags use this as a sub-element, we want to use COMMANDED_LEG_AIRSPEED (above), since it is
               // a sub-element of INTENT
            }
            else if (tagName == "INPUT_TYPE")
            {
            }
            else if (tagName == "INTENT_LIST")
            {
               intentListPtr = ut::make_unique<std::map<long, std::unique_ptr<Intent>>>();
            }
            else if (tagName == "TRANSITION_LIST")
            {
               transitionNestingLevel++; // if we're in the Path's list, bump it up 1, so we know which transition list
                                         // to add to. need to make a stack of transition lists to handle the recursive
                                         // case
               // if we have a currently active transition list, put it on the stack, making way for the nested one defined below
               if (transitionList != nullptr)
               {
                  mTransitionListStack.push_back(transitionList);
               }
               transitionList = new std::vector<std::unique_ptr<CrdTransition>>();
               if (transitionNestingLevel > 2)
               {
                  // mLogfileStream << "TRANSITION NESTING LEVEL > 2, ALL STOP!" << std::endl; // TODO, implement stack
                  // of transition lists to handle this exit(1);
                  //  Think these are implemented, we have transitions with transition_lists working (with 1 input
                  //  file!) Needs testing to validate that the general n-level hierarchy is going to work correctly.
                  mLogfileStream << "TRANSITION NESTING LEVEL > 2, SLOW, DANGER AHEAD!" << std::endl;
               }
            }
            else if (tagName == "STATE_LIST")
            {
               stateList = ut::make_unique<std::map<long, std::unique_ptr<CrdState>>>();
            }
            else if (tagName == "PATH_LIST")
            {
               pathList = ut::make_unique<std::map<long, std::unique_ptr<Path>>>();
            }
            else if (tagName == "ROUTE_LIST")
            {
               routeList.clear();
            }
            else if (tagName == "MISSION_LIST")
            {
               mMissionList.clear();
            }
            else if (tagName == "VEHICLE_LIST")
            {
               vehicleList = ut::make_unique<std::map<long, std::unique_ptr<Vehicle>>>();
            }
            else if (tagName == "VEHICLE_REFERENCE")
            {
            }
            else
            {
               //  the tag we processed was an open tag for an unneeded (as of now) element
               mNewParentElement = unNeededParentElement.get();
            }

            // at this point we've either saved a simple value, created a new object, or started a list of objects soon
            // to come. newParentElement was set to nullptr before the cascading if statement.  If it is still null at
            // this point, we didn't create an object, and can keep the current parent - which was assigned to
            // enclosingParentElement.
            // Lists (at least the ones currently being processed, don't have any simple attributes (like id, name, etc)
            // to collect.
            if (mNewParentElement != nullptr)
            {
               mParentElementStack.push(mNewParentElement);
               mEnclosingParentElement = mNewParentElement;
               mNewParentElement = nullptr; // duplicated at the beginning of the loop, for no good reason at this point
            }
         }
         ////////////////////  END of Start Tag Processing
         else if (xmlReader.isCharacters())
         {
            contentFlag = true;
            if (!content.empty())
            {
               characters += content;
            }
         }
         ///////////////////  Beginning of End Tag Processing
         else if (xmlReader.isEndElement())
         {
            popEnclosingElementNeeded = false;
            endTagFlag                = true;
            if (!characters.empty() && mDebugFlag)
            {
               mLogfileStream << "Line: " << xmlReader.lineNumber() << " value: " << characters
                              << "|collected for tag:" << tagName << std::endl;
            }
            if (tagName == "POINT")
            {
               popEnclosingElementNeeded = true;

               PopulatePoint(ConvertToDecimalLat(pointLatitudeString), ConvertToDecimalLon(pointLongitudeString), *point);

               point->SetId(id);
               point->SetName(name);
               pointList.push_back(std::move(point));
            }
            else if (tagName == "POINT_REFERENCE")
            {
               pointReference = stol(characters);
            }
            else if (tagName == "ID")
            {
               id = stol(characters);
               if (mEnclosingParentElement != nullptr)
               {
                  mEnclosingParentElement->SetId(id);
               }
            }
            else if (tagName == "NAME")
            {
               name          = characters;
               auto theRoute = dynamic_cast<Route*>(mEnclosingParentElement);
               if (theRoute != nullptr)
               {
                  ReplaceIllegalCharacters(name);
                  theRoute->SetName(name);
               }
               else if (mEnclosingParentElement == point.get())
               {
                  point->SetName(name);
               }
            }
            else if (tagName == "POINT_LIST")
            {
               // shouldn't need to do anything, enclosing tag should grab the list and create the map
               auto missionTemp = dynamic_cast<CrdMission*>(mEnclosingParentElement);
               if (missionTemp != nullptr)
               {
                  missionTemp->SetPointList(pointList);
               }
               else
               {
                  // must be route's point list
                  route->AddToPointList(pointList);
               }
               // at this stage, the pointList has a list of empty points, all points moved off from unique pointers to
               // either Mission or Route
               pointList.clear(); // this is also done in the begin tag if clause, probably isn't needed in both
               if (mDebugFlag)
               {
                  mLogfileStream << "debug stop point to check points allocated on this list" << std::endl;
               }
            }
            else if (tagName == "ROUTE_POINT")
            {
               popEnclosingElementNeeded = true;
               routePoint->SetReferencePointId(pointReference);
               if (routePointList->find(routePoint->GetId()) == routePointList->end())
               {
                  routePointList->emplace(routePoint->GetId(), std::move(routePoint));
               }
            }
            else if (tagName == "ROUTE_POINT_LIST")
            {
               // shouldn't need to do anything, enclosing tag should grab the list and create the map
               mLogfileStream << "End of route point list" << std::endl;
            }
            else if (tagName == "CLOCK_DATE")
            {
               auto crdStateParent = dynamic_cast<CrdState*>(mEnclosingParentElement);
               if (crdStateParent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  if (characters.length() == 8)
                  {
                     int year  = stoi(characters.substr(0, 4));
                     int month = stoi(characters.substr(4, 2));
                     int day   = stoi(characters.substr(6, 2));
                     crdStateParent->SetClockDate(year, month, day);

                     // Save off the earliest date we encounter;
                     UtCalendar temp;
                     temp.SetDate(year, month, day);
                     if (temp < mStartDate)
                     {
                        temp.SetTime(0); // Default constructed UtCalendar sets time to noon (we want midnight)
                        mStartDate = temp;
                     }
                  }
               }
            }
            else if (tagName == "CLOCK_TIME")
            {
               auto crdStateParent = dynamic_cast<CrdState*>(mEnclosingParentElement);
               if (crdStateParent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  long time = stol(characters);
                  crdStateParent->SetClockTime(time);
               }
            }
            else if (tagName == "ROUTE")
            {
               popEnclosingElementNeeded = true;
               //  set at end of correct name element - route->SetName(name);
               route->SetIntentList(std::move(intentListPtr));
               route->SetRoutePointList(std::move(routePointList));
               route->SetPathList(std::move(pathList));
               route->SetStateList(std::move(stateList));
               route->SetVehicleReference(vehicleReference);
               std::unique_ptr<Route> rteUptr(route); // = ut::make_unique<Route>();
               routeList.push_back(std::move(rteUptr));
            }
            else if (tagName == "INTENT")
            {
               popEnclosingElementNeeded = true;
               // set this when the id tag closes, else nested id's will interfere					intent->SetId(id);
               intent->SetEntryPointRef(commandedEntryRoutePointReference);
               intent->SetExitPointRef(commandedExitRoutePointReference);
               intent->SetCommandedAirspeedValue(airspeedValue);
               intent->SetAirspeedType(airspeedType);
               intent->SetCommandedAltitudeValue(altitudeValue);
               intent->SetAltitudeInputType(altitudeInputType);
               airspeedValue =
                  doubleMaxValue; // reset so next intent won't get the same value if it's not tagged within that element
               altitudeValue =
                  doubleMaxValue; // reset so next intent won't get the same value if it's not tagged within that element
               if (intentListPtr)
               {
                  if (intentListPtr->find(intent->GetId()) == intentListPtr->end())
                  {
                     intentListPtr->emplace(intent->GetId(), std::unique_ptr<Intent>(intent));
                  }
                  else
                  {
                     std::cout << "ERROR: Duplicate intent ID " << intent->GetId() << std::endl;
                  }
               }
               else
               {
                  std::cout << "ERROR: Intent list error when processing intent ID " << intent->GetId() << std::endl;
               }
            }
            else if (tagName == "ORBIT_INTENT")
            {
               // HACK! HACK! I hate this code, but how else with this stack tracking?!?
               auto me = mParentElementStack.top();
               // pop myself off the stack, should always be able to do
               //  this safely as i added myself on during open tag.
               mParentElementStack.pop();
               if (!mParentElementStack.empty()) // no parent intent to add me to if empty, bad schema.
               {
                  mEnclosingParentElement = mParentElementStack.top(); // intent i belong to.
                  auto parentIntent       = dynamic_cast<Intent*>(mEnclosingParentElement);
                  if (parentIntent != nullptr) // Inside of wrong parent type if fails, bad schema
                  {
                     parentIntent->SetOrbitIntent(std::move(orbitIntent));
                  }
               }
               // Return stack to original structure.
               mEnclosingParentElement = me;
               mParentElementStack.push(mEnclosingParentElement);
               popEnclosingElementNeeded = true; // ensure normal pop operations later.
            }
            else if (tagName == "AUTO_TIME_FLAG")
            {
               auto parentOrbitIntent = dynamic_cast<OrbitIntent*>(mEnclosingParentElement);
               if (parentOrbitIntent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  if (characters == "TRUE")
                  {
                     parentOrbitIntent->SetAutoTimeFlag(true);
                  }
               }
            }
            else if (tagName == "BANK_ANGLE")
            {
               auto parentOrbitIntent = dynamic_cast<OrbitIntent*>(mEnclosingParentElement);
               if (parentOrbitIntent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  parentOrbitIntent->SetBankAngle(std::stod(characters));
               }
            }
            else if (tagName == "CLOCK_TYPE")
            {
               auto parentOrbitIntent = dynamic_cast<OrbitIntent*>(mEnclosingParentElement);
               if (parentOrbitIntent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  parentOrbitIntent->SetClockType(characters);
               }
            }
            else if (tagName == "COMMANDED_COMPLETION_TIME")
            {
               auto parentOrbitIntent = dynamic_cast<OrbitIntent*>(mEnclosingParentElement);
               if (parentOrbitIntent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  parentOrbitIntent->SetCommandCommandCompletionTime(std::stod(characters));
               }
            }
            else if (tagName == "INBOUND_TRUE_COURSE")
            {
               auto parentOrbitIntent = dynamic_cast<OrbitIntent*>(mEnclosingParentElement);
               if (parentOrbitIntent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  parentOrbitIntent->SetInboundTrueCourse(std::stof(characters));
               }
            }
            else if (tagName == "LEG_DISTANCE")
            {
               auto parentOrbitIntent = dynamic_cast<OrbitIntent*>(mEnclosingParentElement);
               if (parentOrbitIntent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  parentOrbitIntent->SetLegDistance(std::stod(characters));
               }
            }
            else if (tagName == "REFERENCE_POINT")
            {
               auto parentOrbitIntent = dynamic_cast<OrbitIntent*>(mEnclosingParentElement);
               if (parentOrbitIntent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  parentOrbitIntent->SetReferencePoint(characters);
               }
            }
            else if (tagName == "TURN_RIGHT")
            {
               auto parentOrbitIntent = dynamic_cast<OrbitIntent*>(mEnclosingParentElement);
               if (parentOrbitIntent != nullptr) // Inside of wrong parent type if fails, bad schema
               {
                  if (characters == "TRUE")
                  {
                     parentOrbitIntent->SetTurnRight(true);
                  }
               }
            }
            else if (tagName == "PATH")
            {
               // need to collect the parts of the path here
               popEnclosingElementNeeded = true;
               path->SetPathType(pathType);
               if (pathList->find(path->GetId()) == pathList->end())
               {
                  pathList->emplace(path->GetId(), std::unique_ptr<Path>(path));
               }
               else
               {
                  std::cout << "ERROR in CRD Input file: Duplicate Path ID " << path->GetId() << std::endl;
               }
            }
            else if (tagName == "PATH_TYPE")
            {
               pathType = characters;
            }
            else if (tagName == "DATE")
            {
               pathDate = characters;
            }
            else if (tagName == "TIME")
            {
               pathTime = characters;
            }
            else if (tagName == "VEHICLE")
            {
               // need to collect vehicle parts here
               popEnclosingElementNeeded = true;
               if (vehicle->GetId() > -1)
               {
                  (*vehicleList)[vehicle->GetId()] = (std::move(vehicle));
               }
            }
            else if (tagName == "STATE")
            {
               // need to collect state parts
               popEnclosingElementNeeded = true;
               if (stateList->find(state->GetId()) == stateList->end())
               {
                  stateList->emplace(state->GetId(), std::unique_ptr<CrdState>(state));
               }
               else
               {
                  std::cout << "ERROR in CRD Input file: Duplicate State ID " << state->GetId() << std::endl;
               }
            }
            else if (tagName == "TRANSITION")
            {
               // need to collect transition parts

               // set the transition list for this transition
               // transition->SetTransitionList();
               // pop the stack of transition lists to go to the next level up - TODO
               popEnclosingElementNeeded = true;

               // transition->SetIntentReference(intentReference);
               transition->SetStartStateReference(startStateReference);
               transition->SetEndStateReference(endStateReference);
               transitionList->push_back(std::unique_ptr<CrdTransition>(transition));
            }
            else if (tagName == "INTENT_REFERENCE")
            {
               intentReference = stol(characters);
               auto t          = dynamic_cast<CrdTransition*>(mEnclosingParentElement);
               if (t != nullptr)
               {
                  t->SetIntentReference(intentReference);
               }
            }
            else if (tagName == "START_STATE_REFERENCE")
            {
               startStateReference = stol(characters);
            }
            else if (tagName == "END_STATE_REFERENCE")
            {
               endStateReference = stol(characters);
            }
            else if (tagName == "CLIMB_STYLE")
            {
               intent->SetClimbStyle(characters);
            }
            else if (tagName == "CLIMB_DIFFERENTIAL")
            {
               if (mDebugFlag)
               {
                  mLogfileStream << "end tag for CLIMB_DIFFERENTIAL" << std::endl;
               }
               intent->SetClimbDifferential(characters);
            }
            else if (tagName == "DESCENT_STYLE")
            {
               intent->SetDescentStyle(characters);
            }
            else if (tagName == "DESCENT_DIFFERENTIAL")
            {
               intent->SetDescentDifferential(characters);
            }
            else if (tagName == "TURN_TYPE")
            {
            }
            else if (tagName == "MISSION")
            {
               // need to collect parts of mission
               popEnclosingElementNeeded = true;
               // mission->SetName(missionName);
               mission->TakeOwnershipOfRouteList(std::move(routeList));
               // mission->SetPointList(pointList); set in POINT_LIST handler, since ROUTE will typically have a POINT_LIST also
               mMissionList.push_back(std::move(mission));
            }
            else if (tagName == "DAFIF_DATE_TIME")
            {
               dafifDateTime = characters;
               mission->SetDafifTime(characters);
            }
            else if (tagName == "CREATION_DATE_TIME")
            {
               creationDateTime = characters;
               mission->SetCreationTime(characters);
            }
            else if (tagName == "MISSION_NAME")
            {
               missionName = characters;
               mission->SetMissionName(characters);
            }
            else if (tagName == "CRD")
            {
               // pretty much done parsing when we hit this end tag.  After exiting the loop, write out the routes using
               // the structure built
               mLogfileStream << "Processed CRD element end tag." << std::endl;
            }
            else if (tagName == "COMMANDED_ENTRY_ROUTE_POINT_REFERENCE")
            {
               commandedEntryRoutePointReference = stol(characters);
            }
            else if (tagName == "COMMANDED_EXIT_ROUTE_POINT_REFERENCE")
            {
               commandedExitRoutePointReference = stol(characters);
            }
            // lat and long are inside the WGS84_POSITION tag, but nowhere else, so I think WGS84_POSITION doesn't need parsing
            else if (tagName == "LATITUDE")
            {
               pointLatitudeString = characters;
               if (mDebugFlag)
               {
                  mLogfileStream << "collected latitude string:" << pointLatitudeString << "x" << std::endl;
               }
            }
            else if (tagName == "LONGITUDE")
            {
               pointLongitudeString = characters;
               if (mDebugFlag)
               {
                  mLogfileStream << "collected longitude string:" << pointLongitudeString << "x" << std::endl;
               }
            }
            else if (tagName == "COMMANDED_ALTITUDE")
            {
               altitudeValue = value;
            }
            else if (tagName == "ALTITUDE")
            {
               altitudeValue = value;
            }
            else if (tagName == "INITIAL_STATE_INTENT")
            {
               path->SetInitialPathAltitude(altitudeValue);
               path->SetAltType(altitudeInputType);
               path->SetTime(pathTime);
               path->SetDate(pathDate);
            }
            else if (tagName == "VALUE")
            {
               value = stod(characters);
            }
            else if (tagName == "ALTITUDE_TYPE")
            {
               altitudeInputType = characters;
            }
            else if (tagName == "COMMANDED_LEG_AIRSPEED")
            {
               // set the airspeed and type from the value, then reset the flag to false
               airspeedValue = value;
               airspeedType  = inputType;
            }
            else if (tagName == "AIRSPEED")
            {
            }
            else if (tagName == "INPUT_TYPE")
            {
               inputType = characters;
            }
            else if (tagName == "INTENT_LIST")
            {
               mLogfileStream << "End of intent list" << std::endl;
            }
            else if (tagName == "TRANSITION_LIST")
            {
               transitionNestingLevel--;
               std::vector<std::unique_ptr<CrdTransition>>* tList = nullptr;
               if (!mTransitionListStack.empty())
               {
                  tList = mTransitionListStack.back();
                  mTransitionListStack.pop_back();
               }
               auto t =
                  dynamic_cast<CrdTransition*>(mEnclosingParentElement); // only set this as the current transition if
                                                                         // it's not the enclosing path

               if (t == nullptr || mFullTransitionDepth)
               { // first condition means it's the Path's transition list, second means we want them all
                  mEnclosingParentElement->SetTransitionList(transitionList);
               }
               else
               {
                  // TODO - transitionList is not going to be attached to its transition, so it should be deleted here.
                  // Otherwise the memory will be orphaned.
                  for (auto& currTransition : *transitionList)
                  {
                     currTransition.reset();
                  }
               }
               transitionList = tList;

               if (t != nullptr)
               {
                  transition = t; // this was the owner of the current transition list.  His /Transition tag will come
                                  // up next, and he'll have to be saved onto the next higher list on the stack
               }
            }
            else if (tagName == "STATE_LIST")
            {
            }
            else if (tagName == "PATH_LIST")
            {
            }
            else if (tagName == "ROUTE_LIST")
            {
            }
            else if (tagName == "MISSION_LIST")
            {
            }
            else if (tagName == "VEHICLE_LIST")
            {
               if (vehicleList != nullptr)
               {
                  mission->SetVehicleList(std::move(vehicleList));
               }
            }
            else if (tagName == "VEHICLE_REFERENCE")
            {
               vehicleReference = stol(characters);
            }
            else
            {
               popEnclosingElementNeeded =
                  true; // it's a currently unneeded tag, but we need to pop the unNeededParentElement off the
                        // parent stack to keep nested elements (like id's ) from being assigned to the wrong objects
            }

            // after processing the end tag, we need to pop the the next parent element off the parentElementStack, and
            // point the enclosingParentElement at it.
            if (popEnclosingElementNeeded && !mParentElementStack.empty())
            {
               mParentElementStack.pop();
               if (!mParentElementStack.empty())
               {
                  mEnclosingParentElement = mParentElementStack.top();
               }
               else
               {
                  mEnclosingParentElement = nullptr;
               }
            }
         }
      } // end loop

      if (xmlReader.hasError())
      {
         mLogfileStream << "Input file problem with file: " << aInputCrdFilename << std::endl;
         mLogfileStream << "xml Reader error: " << xmlReader.errorString().toStdString().c_str() << std::endl;
         mLogfileStream << "the error occurred at line number " << xmlReader.lineNumber() << ", column "
                        << xmlReader.columnNumber() << ".  The character offset is " << xmlReader.characterOffset()
                        << std::endl;

         if (startTagFlag || endTagFlag || contentFlag)
         {
            mLogfileStream << "The last token attempted was a ";
            if (startTagFlag)
            {
               mLogfileStream << "start tag.";
            }
            else if (endTagFlag)
            {
               mLogfileStream << "end tag.";
            }
            else if (contentFlag)
            {
               mLogfileStream << "content field.";
            }
            mLogfileStream << std::endl;
         }
         // exit(1);  - this closes the dialog version, which is a little too abrupt.
         //  try canceling the processing of this file and move on to the next.
      }
      else if (!mCancelFlag)
      {
         retVal = WriteOutputRoutes(aInputCrdFilename);
      }

      xmlReader.clear();
   }
   return retVal;
}

void CrdFileImporter::ClearFilenamesMap()
{
   // for (auto& vec : mInputOutputFilenamesMap)
   //{
   //    auto theVector = std::move(vec.second);
   // }
   mInputOutputFilenamesMap.clear();
}

void CrdFileImporter::SimulateOrbitIfOrbitIntent(std::string&  aPointString,
                                                 const Intent* aCurrIntent,
                                                 const Point&  aEntryPoint,
                                                 double        aSpeed_ms) const
{
   // std::string needsNewline = aPointString.empty() ? "" : "\n"; // This is so orbit additions stay uniform in
   // formatting regardless if its appended to previous point.
   const OrbitIntent* orbit = aCurrIntent->GetOrbitIntent();
   if (orbit)
   {
      if (mUseSimpleOrbits) // point is an orbit intent and will be simulated with pause time
      {
         aPointString = aPointString + NeedsNewLine(aPointString) + "\tpause_time " +
                        std::to_string(orbit->GetCommandCompletionTime()) + " sec";
      }
      else
      {
         // Total Duration
         double totalDuration_s = orbit->GetCommandCompletionTime(); // This is in seconds.
         // Leg Length
         double legDistance_m = orbit->GetLegDistance(); // I assume this is in meters. If this is not provided, can we
                                                         // make up a default? Should we?
         // Orbit Heading
         double orbitHeading_deg = orbit->GetInboundTrueCourse();
         // Second Location (Lat/Lon)
         // Compute the second position;
         double latitude2, longitude2;
         UtSphericalEarth::ExtrapolateGreatCirclePosition(aEntryPoint.GetLat(),
                                                          aEntryPoint.GetLon(),
                                                          orbitHeading_deg,
                                                          legDistance_m,
                                                          latitude2,
                                                          longitude2);
         // Bank Angle
         double bankAngle = orbit->GetBankAngle();
         // Travel time per leg
         double timePerLeg_s = 0;
         // One revolution is the complete transit of the orbit (Start and End point are the same)
         double turnTime_s          = 0;
         double timePerRevolution_s = 0;
         double totalRevolutions    = 0;
         if (aSpeed_ms > 0 && bankAngle > 0) // make sure speed_ms is valid
         {
            timePerLeg_s = legDistance_m / aSpeed_ms;
            // Turn Radius is       radius = speed^2 / ( g * tan(bankAngle)
            double turnRadius =
               (aSpeed_ms * aSpeed_ms) / (UtEarth::cACCEL_OF_GRAVITY * tan(bankAngle * UtMath::cRAD_PER_DEG));
            // Time for a 180 degree turn is     time = PI * radius / Speed
            turnTime_s = (UtMath::cPI * turnRadius) / aSpeed_ms;
            timePerRevolution_s =
               (timePerLeg_s + turnTime_s) * 2.0; // times 2.0 for the whole orbit (2 legs, and 2 180 degree turns)
            if (timePerRevolution_s > 0)          // make sure timePerRevolution_s is valid
            {
               totalRevolutions = totalDuration_s / timePerRevolution_s;
            }
         }

         Point secondPoint;
         PopulatePoint(latitude2, longitude2, secondPoint);

         // This conditions must be met for a valid orbit to be generated
         if (totalRevolutions > 0 && !PointsEqualWithinTolerance(aEntryPoint, secondPoint))
         {
            // Ignore Visual Assist here, this can not be change to const &, because aPointString is
            // modified in the loop and initialPointString should not be changed over the course of the loop
            std::string initialPointString = aPointString;
            RemoveLeadingIndent(initialPointString);
            std::string secondPointString = "position " + secondPoint.GetLatString() + " " + secondPoint.GetLonString();

            double remainingRevolutions = totalRevolutions;
            // Add the second point followed by the entry point once for each full Revolution
            while (remainingRevolutions > 1)
            {
               aPointString += NeedsNewLine(aPointString) + "\t" + secondPointString +
                               NeedsNewLine(initialPointString) + "\t" + initialPointString;
               remainingRevolutions = remainingRevolutions - 1.0;
            }
            // Handle a partial Revolution
            if (remainingRevolutions > 0)
            {
               double timeRemaining_s = remainingRevolutions * timePerRevolution_s;
               if (timeRemaining_s < (2 * turnTime_s))
               {
                  // We don't have enough time to do a 360 degree turn.
                  // TODO figure out how to handle this.
                  // For now, just pause the remaining amount of time.
                  aPointString = aPointString + "\tpause_time " + std::to_string(timeRemaining_s) + " sec";
               }
               else
               {
                  // Get the time remaining to travel the partial leg of the orbit
                  timeRemaining_s = timeRemaining_s - (2 * turnTime_s);
                  // Compute the distance we are able to travel on the partial leg of the orbit
                  // This distance is travel twice (once both ways)
                  double distanceOffset_m = (aSpeed_ms * timeRemaining_s) / 2.0;
                  double latitude3, longitude3;
                  // Compute the 3rd point we need (location at the end of the partial leg)
                  UtSphericalEarth::ExtrapolateGreatCirclePosition(aEntryPoint.GetLat(),
                                                                   aEntryPoint.GetLon(),
                                                                   orbitHeading_deg,
                                                                   distanceOffset_m,
                                                                   latitude3,
                                                                   longitude3);
                  Point point3;
                  // Populate the point to get the Lat/Lon strings
                  PopulatePoint(latitude3, longitude3, point3);
                  std::string thirdPointString = "position " + point3.GetLatString() + " " + point3.GetLonString();
                  // Write the positions to aPointString
                  aPointString += NeedsNewLine(aPointString) + "\t" + thirdPointString +
                                  NeedsNewLine(initialPointString) + "\t" + initialPointString;
               }
            }
         }
         else // Fall back the the "Simple Orbits" implementation with a warning
         {
            mLogfileStream
               << "WARNING Insufficient information to generate detailed orbit, using \"pause\" command instead."
               << std::endl;
            aPointString = aPointString + NeedsNewLine(aPointString) + "\tpause_time " +
                           std::to_string(orbit->GetCommandCompletionTime()) + " sec";
         }
      }
   }
}

void CrdFileImporter::PopulatePoint(double aLat, double aLon, Point& aPoint) const
{
   if (mPointFormat == cDECIMAL_FORMAT)
   {
      std::string outputString;
      char        direction = (aLat < 0 ? 's' : 'n');

      // to_string apparently doesn't allow customizing # of decimal points, so use stringstream

      std::ostringstream oss;
      oss << std::fixed;
      oss << std::setprecision(cCOORDINATE_PRECISION);
      oss << std::abs(aLat); // do not stream negative latitude for southern hemisphere, use 's' instead.
      outputString = oss.str() + direction;

      aPoint.SetLatString(outputString);
      aPoint.SetLat(aLat);

      oss.str("");
      oss.clear();

      direction = (aLon < 0 ? 'w' : 'e');
      oss << std::abs(aLon); // do not stream negative longitude for western hemisphere, use 'w' instead.
      outputString = oss.str() + direction;
      aPoint.SetLonString(outputString);
      aPoint.SetLon(aLon);
   }
   else
   {
      // convert from CRD format, which means changing the hemisphere character to the end, adding colons, and latitude
      // space removal. OR, convert from decimal to AFSIM format.
      std::string outputString = ConvertToUnsignedDms(aLat);
      if (aLat < 0)
      {
         outputString += "s";
      }
      else
      {
         outputString += "n";
      }
      aPoint.SetLatString(outputString);
      aPoint.SetLat(aLat);

      outputString = ConvertToUnsignedDms(aLon);
      if (aLon < 0)
      {
         outputString += "w";
      }
      else
      {
         outputString += "e";
      }
      aPoint.SetLonString(outputString);
      aPoint.SetLon(aLon);
   }
}

std::string CrdFileImporter::NeedsNewLine(const std::string& aPointString) const
{
   return aPointString.empty() ? "" : "\n";
}

void CrdFileImporter::RemoveLeadingIndent(std::string& aPointString) const
{
   // remove leading indention, we handle whitespace.
   aPointString.erase(aPointString.begin(),
                      std::find_if(aPointString.begin(), aPointString.end(), [](int ch) { return !std::isspace(ch); }));
}

void CrdFileImporter::WriteIncludeAllFile() const
{
   // If there are multiple output files:
   //  This can be done with multiple input files, each creating an output file,
   //  Or from one input files creating multiple output files.
   if (mInputOutputFilenamesMap.size() > 1 ||
       (mInputOutputFilenamesMap.size() == 1 && mInputOutputFilenamesMap.begin()->second.size() > 1))
   {
      std::string   fileName = mOutputFolder + "/" + mOutputFileBaseString + "_IncludeAll.txt";
      std::ofstream outputStream;
      outputStream.open(fileName, std::ofstream::out);

      for (const auto& filePair : mInputOutputFilenamesMap)
      {
         outputStream << "// Input File : " << filePair.first << std::endl;
         for (const auto& outputPath : filePair.second)
         {
            // Strip off the directory, only include filename
            size_t last_slash_idx = outputPath.find_last_of("/");
            if (last_slash_idx != std::string::npos)
            {
               last_slash_idx++; // Increment once to remove the slash
            }
            outputStream << "include_once " << outputPath.substr(last_slash_idx) << std::endl;
         }
         outputStream << std::endl;
      }
      outputStream.close();
   }
}

void CrdFileImporter::WritePlatformStub(std::ofstream&     aOutputMissionFile,
                                        const std::string& aRouteName,
                                        const UtCalendar&  aCreationTime) const
{
   aOutputMissionFile << std::endl;
   aOutputMissionFile << "platform " << aRouteName << "_platform UNKNOWN_PLATFORM_TYPE" << std::endl;
   // Do not output the creation time if the creation time or Start Date was not set (still year cINVALID_YEAR) or
   // the creation time is prior to the start date.
   if (mStartDate.GetYear() != cINVALID_YEAR && aCreationTime.GetYear() != cINVALID_YEAR && !(aCreationTime < mStartDate))
   {
      aOutputMissionFile << "   // Assumed scenario start_date of ";
      aOutputMissionFile << mStartDate.GetYear() << "/" << mStartDate.GetMonth() << "/" << mStartDate.GetDay()
                         << " at midnight." << std::endl;
      aOutputMissionFile << "   // Creation Date & Time ";
      aOutputMissionFile << aCreationTime.GetYear() << "/" << aCreationTime.GetMonth() << "/" << aCreationTime.GetDay();
      aOutputMissionFile << " and " << aCreationTime.GetTime() << " seconds since midnight." << std::endl;
      aOutputMissionFile << "   creation_time " << aCreationTime.GetTimeSince(mStartDate) << " sec" << std::endl;
   }
   aOutputMissionFile << "   edit mover" << std::endl;
   aOutputMissionFile << "      use_route " << aRouteName << std::endl;
   aOutputMissionFile << "   end_mover" << std::endl;
   aOutputMissionFile << "end_platform" << std::endl;
}

} // end namespace CrdImporter
