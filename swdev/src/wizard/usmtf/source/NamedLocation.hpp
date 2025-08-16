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

#ifndef NAMEDLOCATION_HPP
#define NAMEDLOCATION_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "ICAO_Resolver.hpp"
#include "LatLon.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a NamedLocation.
//! It comes in several forms.

//! USMTF Excerpt:
//! A.Geographic Location, LAT / LONG, MINUTES/Seconds // this is delegated to base LatLon
//! B. Place Name, a literal string where meaning must be provided by user
//! C. ICAO BASE CODE, A known value with a known spherical location.
class USMTF_EXPORT NamedLocation : public LatLon
{
public:
   NamedLocation() = default;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetLocation() const noexcept;
   bool               IsLocationFreeForm() const noexcept;

protected:
   ICAO_Resolver mICAOService; // Uses ICAO table to associate user provided name to spherical position.
   std::string   mLocation;
   bool          mIsFreeForm;
};
} // namespace usmtf
#endif
