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

#ifndef AIRCRAFTMISSIONDATA_HPP
#define AIRCRAFTMISSIONDATA_HPP

#include "usmtf_export.h"

#include <string>
#include <vector>

#include "Field.hpp"
#include "FreeText.hpp"
#include "NamedLocation.hpp"
#include "RangeValidator.hpp"
#include "Set.hpp"
#include "UTM_Location.hpp"
#include "UtMemory.hpp"
#include "VariantValidator.hpp"

namespace usmtf
{
/*!
|||           THE AMSNDAT SET PROVIDES TASKING INFORMATION FOR AIR MISSIONS.
|||
|||C    M        AMSNDAT/                             /                 /.....................................
||||                    1 RESIDUAL MISSION INDICATOR  2 MISSION NUMBER  3 AMC MISSION NUMBER OR EVENT NUMBER
||||                    M   [1 A]                     M   [1-8 ABNS]    O   [1/12 AN]
||||
||||                    /                         /..................../ /.........................
||||                    4 PACKAGE IDENTIFICATION  5 MISSION COMMANDER  6 PRIMARY MISSION TYPE  7 SECONDARY MISSION TYPE
||||                    M   [1-5 ANS]             O   [2 A]            M   [2-6 AN]            O   [2-6 AN]
||||
|||
||||                    /.............../...................../.................................../..........R..........
||||                    8 ALERT STATUS  9 DEPARTURE LOCATION  10 DAY-TIME AND MONTH OF DEPARTURE  11 RECOVERY LOCATION
||||                    C   [2/3 AN]    O   [1/20 ABNS]       O.   [10 AN]                        C.   [1/20 ABNS]
||||
||||
||||                    /................R.................//
||||                    12 DAY-TIME AND MONTH OF RECOVERY
||||                    O.   [10 AN]
|||||                  O.   [3-7 NS]                          O.   [5 NS]

Detailed Document:
AMSNDAT.UF.html
*/
class USMTF_EXPORT AircraftMissionData : public Set
{
public:
   using Location  = VariantValidator<UTM_Location, NamedLocation, FreeText>;
   using Locations = std::vector<std::unique_ptr<Location>>;
   AircraftMissionData(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   AircraftMissionData(const AircraftMissionData&) = delete;
   const FreeText&                                        GetResidualMissionIndicator() const noexcept;
   const FreeText&                                        GetMissionNumber() const noexcept;
   const VariantValidator<RangeValidator<int>, FreeText>& GetEventNumber() const noexcept;
   const FreeText&                                        GetPackageIdentification() const noexcept;
   const FreeText&                                        GetMissionCommander() const noexcept;
   const FreeText&                                        GetPrimaryMissionType() const noexcept;
   const FreeText&                                        GetSecondaryMissionType() const noexcept;
   const FreeText&                                        GetAlertStatus() const noexcept;
   const Location&                                        GetDepartureLocation() const noexcept;
   const FreeText&                                        GetTimeOfDeparture() const noexcept;
   const Locations&                                       GetRecoveryLocations() const noexcept;
   const std::vector<FreeText>&                           GetRecoverTimes() const noexcept;

private:
   FreeText                                        mRMI;
   FreeText                                        mMissionNumber;
   VariantValidator<RangeValidator<int>, FreeText> mAmc;
   FreeText                                        mPackageId;
   FreeText                                        mMissionCommander;
   FreeText                                        mPrimaryMissionType;
   FreeText                                        mSecondaryMissionType;
   FreeText                                        mAlertStatus;
   Location                                        mDepartureLocation;
   FreeText                                        mTimeOfDeparture;
   Locations                                       mRecoveryLocations;
   std::vector<FreeText>                           mTimeOfRecovery;
};
} // namespace usmtf

#endif
