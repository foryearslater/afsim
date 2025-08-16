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

#ifndef PATH_HPP
#define PATH_HPP

#include <map>
#include <string>
#include <vector>

#include "ElementBase.hpp"

namespace CrdImporter
{
class CrdTransition;

class Path : public ElementBase
{
public:
   Path()           = default;
   ~Path() override = default;

   const std::string& GetPathType() const { return mPathType; }
   void               SetPathType(const std::string& aKind) { mPathType = aKind; }

   const std::string& GetAltType() const { return mAltType; }
   void               SetAltType(const std::string& aType) { mAltType = aType; }

   double GetInitialPathAltitude() const { return mInitialPathAltitude; }
   void   SetInitialPathAltitude(double aAlt) { mInitialPathAltitude = aAlt; }

   void               SetTime(const std::string& aTime) { mTime = aTime; }
   const std::string& GetTime() const { return mTime; }

   void               SetDate(const std::string& aDate) { mDate = aDate; }
   const std::string& GetDate() const { return mDate; }

private:
   std::string mPathType;                 // PRIMARY, DIVERT, OR ALTERNATE (Only one can be a PRIMARY in a pathlist)
   int         mHeadingCalculationOption; // GREAT_CIRCLE or RHUMB_LINE
   double      mEmergencySafeMslAlt;
   std::string mAltitudeUnits;

   double      mInitialPathAltitude; // from INITIAL_STATE_INTENT
   std::string mAltType;             // MSG or AGL
   std::string mDate;
   std::string mTime;
};
} // namespace CrdImporter
#endif
