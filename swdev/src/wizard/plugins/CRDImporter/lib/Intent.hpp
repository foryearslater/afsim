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

#ifndef INTENT_HPP
#define INTENT_HPP

#include <memory>
#include <string>

#include "ElementBase.hpp"
#include "OrbitIntent.hpp"

namespace CrdImporter
{
class Intent : public ElementBase
{
public:
   Intent()           = default;
   ~Intent() override = default;

   long               GetEntryPointRef() const { return mCommandedEntryRoutePointReference; }
   long               GetExitPointRef() const { return mCommandedExitRoutePointReference; }
   double             GetCommandedAirspeedValue() const { return mCommandedLegAirspeed; }
   const std::string& GetAirspeedType() const { return mAirspeedInputType; }
   double             GetCommandedAltitudeValue() const { return mCommandedAltitude; }
   const std::string& GetAltitudeInputType() const { return mAltitudeType; }

   void SetEntryPointRef(long aId) { mCommandedEntryRoutePointReference = aId; }
   void SetExitPointRef(long aId) { mCommandedExitRoutePointReference = aId; }
   void SetCommandedAirspeedValue(double aValue) { mCommandedLegAirspeed = aValue; }
   void SetAirspeedType(const std::string& aType) { mAirspeedInputType = aType; }
   void SetCommandedAltitudeValue(double aValue) { mCommandedAltitude = aValue; }
   void SetAltitudeInputType(const std::string& aType) { mAltitudeType = aType; }

   void SetDescentStyle(const std::string& aValue) { mDescentStyle = aValue; }
   void SetDescentDifferential(const std::string& aValue) { mDescentDifferential = aValue; }
   void SetClimbStyle(const std::string& aValue) { mClimbStyle = aValue; }
   void SetClimbDifferential(const std::string& aValue) { mClimbDifferential = aValue; }

   const std::string& GetDescentStyle() const { return mDescentStyle; }
   const std::string& GetClimbStyle() const { return mClimbStyle; }
   const std::string& GetClimbDifferential() const { return mClimbDifferential; }
   const std::string& GetDescentDifferential() const { return mDescentDifferential; }

   void SetOrbitIntent(std::unique_ptr<OrbitIntent> aOrbitIntent) { mOrbitIntent = std::move(aOrbitIntent); }
   const OrbitIntent* GetOrbitIntent() const { return mOrbitIntent.get(); }

private:
   long mCommandedEntryRoutePointReference;
   long mCommandedExitRoutePointReference;

   double mCommandedAltitude;

   /*
                   <COMMANDED_ALTITUDE>
                        <ALTITUDE>
                                <VALUE>15000</VALUE>
                                <ALTITUDE_TYPE>MSL</ALTITUDE_TYPE>
                        </ALTITUDE>
                </COMMANDED_ALTITUDE>
   */
   double mCommandedLegAirspeed;

   /*
                   <COMMANDED_LEG_AIRSPEED>
                        <AIRSPEED>
                                <VALUE>250</VALUE>
                                <INPUT_TYPE>KTAS</INPUT_TYPE>
                        </AIRSPEED>
                </COMMANDED_LEG_AIRSPEED>
            */
   // unused currently
   //  weaponsReleaseIntentReference;
   bool        mClimbDescentTypeStartAtStart;
   double      mDeltaCargoWeight;
   double      mDeltaStoreWeight;
   std::string mAltitudeType;          // think this will end up being AGL, MSL, etc.
   std::string mAltitudeValueBaseType; // maybe "feet" - see ALTITUDE_VALUE_BASE_TYPE
   std::string mAirspeedInputType;     // KTAS
   // in elementBase - missed removing this so it was masking the right id?  - long id;

   // not currently used.  Mapping to AFSIM is not 1:1 for type and style.  Differential can be calculated using exit
   // point minus entry point altitude
   std::string                  mTurnType;
   std::string                  mClimbStyle;
   std::string                  mDescentStyle;
   std::string                  mClimbDifferential;
   std::string                  mDescentDifferential;
   std::unique_ptr<OrbitIntent> mOrbitIntent;
};
} // namespace CrdImporter
#endif
