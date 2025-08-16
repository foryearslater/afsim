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

#ifndef AERIALREFUELING_HPP
#define AERIALREFUELING_HPP

#include "usmtf_export.h"

#include <string>
#include <tuple>
#include <vector>

#include "AerialRefuelingSystem.hpp"
#include "Field.hpp"
#include "FreeText.hpp"
#include "FuelLoad.hpp"
#include "FuelType.hpp"
#include "TabularSet.hpp"

namespace usmtf
{
/*!
||||        SET 44: THE 5REFUEL SET PROVIDES THE RECEIVER INFORMATION TO THE TANKER.
|||||
|||||   O        5REFUEL
|||||            /MSNNO            /RECCS                /NO                                /ACTYPE
|||||            /                 /                     /                                  /
|||||            1 MISSION NUMBER  2 RECEIVER CALL SIGN  3 AIRCRAFT ALLOCATED TO A MISSION  4 TYPE OF AIRCRAFT
|||||            M   [1-8 ABNS]    M   [1-12 ABNS]       M   [1-2 N]                        M   [2/6 AN]
|||||
|||||            /OFLD           /ARCT                             /SEQ                       /TYP
|||||            /               /                                 /........................../......................
|||||            5 OFFLOAD FUEL  6 AIR REFUELING CONTROL DAY-TIME  7 CELL SEQUENCE NUMBER(S)  8 REFUELING FUEL TYPE
|||||            M   [3-5 NS]    M   [7 AN]                        O   [1/3 NS]               O   [3 AN]
|||||
|||||            /ARS
|||||            /..........................//
|||||            9 AERIAL REFUELING SYSTEM
|||||            O   [3 A]
*/
/*!
Example:
5REFUEL
/MSNNO   /RECCS       /NO/ACTYPE   /OFLD     /ARCT   /SEQ  /TYP  /ARS
/0151H   /HAMMER 51   / 2/AC:FA18  / KLB:15.0/141325Z/  A:3/A:JP8/CLD
/AMC123  /LIFTER 33   / 1/AC:C5A   / KLB:95.0/141500Z/B:1,2/A:JP4/BOM
//
*/
//! Detailed Document:
//! 5REFUEL.UF.html
class USMTF_EXPORT AerialRefueling : public TabularSet
{
public:
   // For code clarity
   struct RefuelInstruction
   {
      FreeText              mMissionNumber;
      FreeText              mRecieverCallsign;
      int                   mAllocatedAircraft;
      FreeText              mTypeOfAircraft;
      FuelLoad              mFuelLoad;
      FreeText              mControlTime;
      FreeText              mCellSequence;
      FuelType              mRefuelType;
      AerialRefuelingSystem mRefuelSystem;
   };
   // using RefuelInstruction= std::tuple<FreeText, FreeText, int, FreeText, FuelLoad, FreeText, FreeText, FuelType,
   // AerialRefuelingSystem>;
   using RefuelInstructions = std::vector<RefuelInstruction>;
   AerialRefueling(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   const RefuelInstructions& GetRefuelInstructions() const noexcept;

private:
   RefuelInstructions mInstructions;
};
} // namespace usmtf

#endif
