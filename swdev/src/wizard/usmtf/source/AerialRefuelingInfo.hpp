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

#ifndef AERIALREFUELINGINFO_HPP
#define AERIALREFUELINGINFO_HPP

#include "usmtf_export.h"

#include <string>

#include "AerialRefuelingSystem.hpp"
#include "Altitude.hpp"
#include "Beacon.hpp"
#include "Field.hpp"
#include "FreeText.hpp"
#include "FuelLoad.hpp"
#include "LatLon.hpp"
#include "RadioEnabled.hpp"
#include "RelativeBearingRange.hpp"
#include "Set.hpp"
#include "TacanChannel.hpp"
#include "VariantValidator.hpp"


namespace usmtf
{
/*!
    THE ARINFO SET PROVIDES AERIAL REFUELING INFORMATION TO RECEIVER AIRCRAFT.


USMTF Example:
      ARINFO/EXXON 11/AR123HA/B:37700/NAME:BUICK/170/ARCT:141325Z
      /NDAR:141400ZFEB/KLBS:15.0/PFREQ:243.0/SFREQ:121.5/EN11/ACTYP:KC135R
      /BOM/4/TNKRS:1,3/16-72/4-3-1//

Standard Excerpt:
|||||
|||||   O   R    ARINFO/                     /                 /                        /
|||||                  1 AIRCRAFT CALL SIGN  2 MISSION NUMBER  3 IFF/SIF MODE AND CODE  4 AIR REFUELING CONTROL POINT
|||||                  M   [1-12 ABNS]       M   [1-8 ABNS]    M   [3/5 N]              M   [1/20 ABNS]
|||||
|||||                  /                               /
|||||                  5 ALTITUDE IN HUNDREDS OF FEET  6 AIR REFUELING CONTROL DAY-TIME
|||||                  M   [1-3 N]                     M   [7 AN]
|||||
|||||                  /........................................../               /
|||||                  7 DAY-TIME AND MONTH OF END AIR REFUELING  8 OFFLOAD FUEL  9 PRIMARY FREQUENCY
|||||                  O   [10 AN]                                M   [3-5 NS]    M   [1/8 ANS]
|||||
|||||                  /......................./................................./..............................
|||||                  10 SECONDARY FREQUENCY  11 LINK 16 ABBREVIATED CALL SIGN  12 TYPE OF REFUELING AIRCRAFT
|||||                  O.   [1/8 ANS]          O.   [4 AN]                       O.   [2/6 AN]
|||||
|||||                  /.........................../............................../...........................
|||||                  13 AERIAL REFUELING SYSTEM  14 NUMBER OF AIRCRAFT IN CELL  15 CELL SEQUENCE NUMBER(S)
|||||                  O.   [3 A]                  O.   [1-2 N]                   O.   [1/3 NS]
|||||
|||||                  /....................................../...........//
|||||                  16 TACAN CHANNEL, RECEIVER AND TANKER  17 BEACON
|||||                  O.   [3-7 NS]                          O.   [5 NS]

Detailed Document:
ARINFO.UF.html
*/
class USMTF_EXPORT AerialRefuelingInfo : public Set, public RadioEnabled
{
public:
   AerialRefuelingInfo(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   AerialRefuelingInfo(const AerialRefuelingInfo&) = delete;
   const FreeText&                                                 GetCallsign() const noexcept;
   const FreeText&                                                 GetMissionNumber() const noexcept;
   const FreeText&                                                 GetIffSifMode() const noexcept;
   const VariantValidator<LatLon, RelativeBearingRange, FreeText>& GetControlPoint() const noexcept;
   const Altitude&                                                 GetAltitude() const noexcept;
   const FreeText&                                                 GetRefuelTime() const noexcept;
   const FreeText&                                                 GetRefuelEndTime() const noexcept;
   const FuelLoad&                                                 GetOffloadFuel() const noexcept;
   const FreeText&                                                 GetLink16Callsign() const noexcept;
   const FreeText&                                                 GetAircraftType() const noexcept;
   const AerialRefuelingSystem&                                    GetRefuelSystem() const noexcept;
   const std::string&                                              GetNumberOfAircraftInCell() const noexcept;
   const FreeText&                                                 GetCellSequence() const noexcept;
   const TacanChannel&                                             GetTacanChannel() const noexcept;
   const Beacon&                                                   GetBeacon() const noexcept;

private:
   FreeText mAirCallSign;
   FreeText mMissionNumber;
   FreeText mIffSifMode;
   // FreeText must go at the end because everything passes this check. Unfortunately a string literal is a valid
   // control point. User must supplement definition.
   VariantValidator<LatLon, RelativeBearingRange, FreeText> mControlPoint;
   Altitude                                                 mAltitude;
   FreeText                                                 mRefuelTime;
   FreeText                                                 mRefuelTimeEnd;
   FuelLoad                                                 mOffloadFuel;
   FreeText                                                 mLink16Callsign;
   FreeText                                                 mAircraftType;
   AerialRefuelingSystem                                    mRefuelSystem;
   std::string                                              mAircraftInCell;
   FreeText                                                 mCellSequence;
   TacanChannel                                             mTacanChannel;
   Beacon                                                   mBeacon;
};
} // namespace usmtf

#endif
