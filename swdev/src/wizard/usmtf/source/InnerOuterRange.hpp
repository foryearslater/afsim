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

#ifndef INNEROUTERRANGE_HPP
#define INNEROUTERRANGE_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "Radius.hpp"
#include "UtCast.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of usmtf InnerOuterRange.
//! An InnerOuterRange is essentially two radius (as I use Radius class often to show distance,
//! one inner and one outer. Its represented in the usmtf as the following:
//!                         |This part to the end.
//! EXAMPLE: /SECTIO:058190T45-60NM
//! Given SectorGeometry covers start stop angle, this is used by it to cover
//! parsing and validation of the inner outer Radii.
class USMTF_EXPORT InnerOuterRange : public Radius
{
public:
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetInnerRadiusValue() const noexcept;
   double             GetInnerRadius() const noexcept;
   double             GetInnerRadiusInMeters() const noexcept;
   // All Radius Members represent outer radius.
protected:
   std::string mInnerRadius;
   double      mInnerRadiusDouble;
   // outer will be cover by base class members
};
} // namespace usmtf
#endif
