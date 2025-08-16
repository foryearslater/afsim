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

#ifndef CrdFileImporter_HPP
#define CrdFileImporter_HPP

#include "crd_importer_lib_export.h"

#include <fstream>
#include <map>
#include <memory>
#include <stack>
#include <vector>

#include "CrdMission.hpp"
#include "CrdTransition.hpp"
#include "Route.hpp"

namespace CrdImporter
{
class CrdMission;
class CrdTransition;
class ElementBase;
class Intent;
class Mission;
class Path;
class Point;

class CRD_IMPORTER_LIB_EXPORT CrdFileImporter
{
public:
   CrdFileImporter();
   ~CrdFileImporter() = default;

   CrdFileImporter(const CrdFileImporter& aRhs) = delete;
   CrdFileImporter& operator=(const CrdFileImporter& aRhs) = delete;

   void CancelCurrentImport();
   void WriteRouteHeader(const std::string& aRouteName, const std::string& aVString, std::ofstream& aOutputMissionFile);
   void WriteMissionFileHeader(const std::string& aSourceCrdFilename,
                               const CrdMission*  aMission,
                               std::ofstream&     aOutputMissionFile);
   void ParseCrdFiles();
   std::vector<std::string> ParseCrdFile(const std::string& aFilename);
   void                     ReplaceIllegalCharacters(std::string& aString);
   void                     OutputPointString(std::ofstream&     aOutputRouteFile,
                                              const Point*       aEntryPoint,
                                              double             aEntryPointAlt,
                                              const std::string& aEntryAltType,
                                              const Intent*      aCurrIntent,
                                              const std::string& aIndentString);
   bool                     OutputRoutePoints(std::ofstream&     aOutputRouteFile,
                                              const Route*       aRoute,
                                              Path*              aCurrPath,
                                              const PointList&   aMissionPoints,
                                              double             aPathInitialAltitude,
                                              const std::string& aMsgOrAgl,
                                              int                aIndent);
   bool                     WritePathsForRoute(const Route*       aRoute,
                                               const PointList&   aMissionPoints,
                                               std::ofstream&     aOutputMissionFile,
                                               const std::string& aRouteNameString);
   std::vector<std::string> WriteOutputRoutes(const std::string& aInputCrdFilename);

   double             CalculateSpeedOfSound(double aAltitude);
   static std::string ConvertToDms(double aValue);
   static std::string ConvertToUnsignedDms(double aValue);
   static double      ConvertToDecimalLat(const std::string& aLatitudeString);
   static double      ConvertToDecimalLon(const std::string& aLongitudeString);

   bool PointsEqualWithinTolerance(const Point& aPoint1, const Point& aPoint2) const;

   void Reset();

   enum PointFormatType
   {
      cDECIMAL_FORMAT,
      cDMS_FORMAT
   };

   void SetPointFormat(PointFormatType aFormat) { mPointFormat = aFormat; }
   void SetDebug(bool aEnabled) { mDebugFlag = aEnabled; }
   void SetRecurseTransitions(bool aRecurse) { mFullTransitionDepth = aRecurse; }
   void SetOutputFilenameBase(const std::string& aOutputFilenameBase) { mOutputFileBaseString = aOutputFilenameBase; }
   void SetOutputFolder(const std::string& aOutputFilename) { mOutputFolder = aOutputFilename; }
   void SetGeneratePlatformStubs(bool aEnable) { mGeneratePlatformStubs = aEnable; }
   void SetGeoEqualityTolerance(double aTolerance) { mEqualityTolerance = aTolerance; }
   void SetSourceCrdInputFiles(const std::vector<std::string>& aSources) { mSourceFilenames = aSources; }
   void SetPrintAltCoordinateFormat(bool aPrint) { mPrintAltCoordinateFormat = aPrint; }
   void SetUseSimpleOrbits(bool aUseSimpleOrbits) { mUseSimpleOrbits = aUseSimpleOrbits; }
   const std::map<std::string, std::vector<std::string>>& GetFilenamesMap() const { return mInputOutputFilenamesMap; }
   void                                                   ClearFilenamesMap();
   int                                                    GetErrorCount() const { return mErrorCount; }
   bool                                                   GetCancelFlag() const { return mCancelFlag; }

private:
   void SimulateOrbitIfOrbitIntent(std::string&  aPointString,
                                   const Intent* aCurrIntent,
                                   const Point&  aEntryPoint,
                                   double        aSpeed_ms) const;

   void        PopulatePoint(double aLat, double aLon, Point& aPoint) const;
   std::string NeedsNewLine(const std::string& aPointString) const;
   void        RemoveLeadingIndent(std::string& aPointString) const;

   void WriteIncludeAllFile() const;
   void WritePlatformStub(std::ofstream&     aOutputMissionFile,
                          const std::string& aRouteName,
                          const UtCalendar&  aCreationTime) const;

   std::vector<std::unique_ptr<CrdMission>> mMissionList;

   const int  cINVALID_YEAR = 9000;
   UtCalendar mStartDate; // This is the earliest date encountered;

   const static int                                          cCOORDINATE_PRECISION = 8;
   PointFormatType                                           mPointFormat;
   std::stack<ElementBase*>                                  mParentElementStack;  // Non-owning
   std::vector<std::vector<std::unique_ptr<CrdTransition>>*> mTransitionListStack; // nested stack of transition lists
   ElementBase*                                              mNewParentElement{
      nullptr}; // this will push the current parent element, and set the current element to be the new parent
                // this is done in the start element processing.  When an end element is processed, if it is a complex
                // element of one of the types we are handling, it will be popped off the stack, and the next enclosing
                // element will become the "parent" to which sub-element tag values are assigned.
   ElementBase* mEnclosingParentElement{nullptr};
   bool         mDebugFlag{true};
   bool         mGeneratePlatformStubs{true};
   bool         mPrintAltCoordinateFormat{true};
   bool         mUseSimpleOrbits{true};
   bool         mFullTransitionDepth{true};
   double mEqualityTolerance{0.000001}; // this is in degrees longitude or latitude.  Note that this means it is not a
                                        // constant distance (longitude degree is more at equator).
   std::string                                     mOutputFileBaseString;
   std::vector<std::string>                        mSourceFilenames;
   std::string                                     mOutputFolder;
   bool                                            mCancelFlag{false};
   std::map<std::string, std::vector<std::string>> mInputOutputFilenamesMap;
   mutable std::ofstream mLogfileStream; // This is marked mutable so messages can be logged from "const" functions.
   int                   mErrorCount{0};
   std::map<std::string, int> mProcessedMissionNamesCurrentSequenceNumbers; // used for sequencing mission filenames for
                                                                            // current import only
};
} // namespace CrdImporter
#endif
