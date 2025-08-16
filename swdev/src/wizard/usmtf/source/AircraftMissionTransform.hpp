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

#ifndef AIRCRAFTMISSIONTRANSFORM_HPP
#define AIRCRAFTMISSIONTRANSFORM_HPP

#include "AerialRefueling.hpp"
#include "AerialRefuelingInfo.hpp"
#include "AirAssetControl.hpp"
#include "AircraftMission.hpp"
#include "AircraftMissionData.hpp"
#include "AircraftMissionLocation.hpp"
#include "PackageCommander.hpp"
#include "PackageData.hpp"
#include "Platform.hpp"
#include "RefuelingTasking.hpp"
#include "TaskUnit.hpp"
#include "USMTF_Transformer.hpp"

namespace usmtf
{

//! Transforms a Segment identifying an Aircraft Mission into one or more AFSIM script blocks containing platforms.

/*!
Example Segment:
   TASKUNIT/CVN68 VAQ-138/ICAO:NMTZ/00301-00450//
   POC/ZAPOLSKI/CDR/N61/LOC:NIMITZ/FRQ:243.0GHZ//
   AMSNDAT/N/0171S/-/AN/-/EW/SEAD//
   MSNACFT/1/ACTYP:EA6B/CLAW 71/HARM/PODS/-/-/-/B:20171/-/B:30171//
   ARINFO/APPLE 20/4010A/B:34010/NAME:BLUE TRACK/200/ARCT:010815Z
   /NDAR:010845ZOCT/KLBS:20.0/PFREQ:343.3/SFREQ:277.8/AE20/ACTYP:KC10
   /CDT/2/TNKR:2/18-81/2-2-3//
   PKGCMD/AN/CVN68 VA-165/0111I/TALON 11/TN01//
   AMSNLOC/010950ZOCT/011010ZOCT/SEIRAQ/270/1//
   CONTROLA/AWAC/DARKSTAR/PDESIG:GOLD/SDESIG:BLUE/DR01/NAME:GINGER//
   6EWDATA
   /EMITTYP        /ELNOT  /FC/LOWFRQ       /UPFRQ        /EA-TECQ
   /TYPE:GCIRDR    /-      /ET/ F:365.798GHZ/ F:650.477GHZ/INCDRGPO//
 */
class AircraftMissionTransform final : public USMTF_Transformer
{
public:
   explicit AircraftMissionTransform(const Segment& aSegment) noexcept;
   //! While this does also export the collection of blocks. It
   //! is advised to use GetPlatforms as that allows more flexibility in
   //! the layout of the export.
   InputBlock                   GetInputBlock() const override;
   const std::vector<Platform>& GetPlatforms() const noexcept;

private:
   void BuildPlatforms();
   //! Responsible for setting the position information of a platform based on priority
   //! sets that may or may not be within the transform.
   /*
     Priority for position selection:
         Taskunit
         AirCraftMissionData
         AerialRefulingInfo
         AirAssetControl
   */
   void SetPriorityPosition(Platform& platform);
   //! Macros are used for text replacement inside exported AFSIM Input Block.
   const std::string              UseMacro(const std::string& aMacroName);
   const TaskUnit*                mTaskUnit;
   const AircraftMissionData*     mMissionData;
   const AircraftMission*         mMission;
   const AircraftMissionLocation* mMissionLocaton;
   const AerialRefueling*         mAerialRefueling;
   const AirAssetControl*         mAirAssetcontrol;
   const PackageData*             mPackageData;
   const AerialRefuelingInfo*     mAerialRefuelingInfo;
   std::vector<Platform>          mPlatforms;
};
} // namespace usmtf

#endif
