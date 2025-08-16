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

#ifndef FUELLOAD_HPP
#define FUELLOAD_HPP

#include "usmtf_export.h"

#include <string>

#include "RangeValidator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of FeulLoad.
//! USMTF Excerpt:
/*!
   THE TOTAL AMOUNT OF FUEL IN THOUSANDS OF POUNDS SCHEDULED TO BE TRANSFERRED DURING AN ENTIRE REFUELING MISSION.
   ALLOWABLE ENTRIES: (0.1 THROUGH 999.9) [1 DECIMAL PLACE].
   EXAMPLE: /KLBS:25.5
*/
class USMTF_EXPORT FuelLoad : public RangeValidator<float>
{
public:
   FuelLoad();
   bool               ParseAndValidate(const Field& aField) override;
   float              GetNumericalFuelAmount() const noexcept;
   const std::string& GetFuelAmount() const noexcept;

protected:
   std::string mFuelAmount;
};
} // namespace usmtf
#endif
