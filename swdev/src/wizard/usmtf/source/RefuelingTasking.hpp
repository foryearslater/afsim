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

#ifndef REFUELINTASKING_HPP
#define REFUELINTASKING_HPP

#include "usmtf_export.h"

#include <string>

#include "AerialRefuelingSystem.hpp"
#include "Beacon.hpp"
#include "Field.hpp"
#include "FuelLoad.hpp"
#include "RadioEnabled.hpp"
#include "Set.hpp"
#include "TacanChannel.hpp"

namespace usmtf
{
/*!
USMTF Example:
      REFTSK/BOM/KLBS:25.5/KLBS:5.1/PDESIG:TAD01/SDESIG:TAD02/16-72/4-3-1//

Standard Excerpt:
|||||        THE REFTSK SET PROVIDES THE REFUELING TASKING FOR THE REFUELING TANKER AIRCRAFT.
|||||
|||||          REFTSK/                          /                     /.................................
|||||                1 AERIAL REFUELING SYSTEM  2 TOTAL OFFLOAD FUEL  3 ALERT/CONTINGENCY OFFLOAD FUEL
|||||                M   [3 A]                  M   [3-5 NS]          O   [3-5 NS]
|||||
|||||                /                    /....................../...................................../..........//
|||||                4 PRIMARY FREQUENCY  5 SECONDARY FREQUENCY  6 TACAN CHANNEL, RECEIVER AND TANKER  7 BEACON

Detailed Document:
REFTSK.UF.html
*/
class USMTF_EXPORT RefuelingTasking : public Set, public RadioEnabled
{
public:
   RefuelingTasking(const std::string& aSTF, const std::vector<Field>& aFields) noexcept;
   RefuelingTasking(const RefuelingTasking&) = delete;
   const AerialRefuelingSystem& GetRefuelingSystem() const noexcept;
   const FuelLoad&              GetOffloadFuel() const noexcept;
   const FuelLoad&              GetContingencyOffloadFuel() const noexcept;
   const TacanChannel&          GetTacanChannel() const noexcept;
   const Beacon&                GetBeacon() const noexcept;

private:
   AerialRefuelingSystem mAerialRefuelingSystem;
   FuelLoad              mOffloadFuel;
   FuelLoad              mContingencyOffloadFuel;
   TacanChannel          mTacanChannel;
   Beacon                mBeacon;
};
} // namespace usmtf

#endif
