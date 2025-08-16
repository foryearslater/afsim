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


#ifndef VERTICALDIMENSION_HPP
#define VERTICALDIMENSION_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a VerticalDimension.
//! VerticalDimensions define the altitude lower and upper bound. These are used in zones for Z axis volume.

//! USMTF Example:
//!                                                                VerticalDimension
//!                                                                   ||||||||||
//! 1TRACK                                                             vvvvvvvvvv
//!   / LEG / LEG - BEGIN     / LEG - END      / LEG - WIDTH       / MINALT - MAXALT
//!   / 01 / 152345N0505657E / 192646N0531226E / 30.5NML - 60.9NMR / 050AMSL - 100AMSL
//!   / 02 / 192646N0531226E / 231031N0545323E / 60.5NML - 60.5NMR / 080AMSL - 120AMSL
//!   / 03 / 231031N0545323E / 280628N0562901E / 60NML - 80NMR     / 100AMSL - 150AMSL
//!   //
class USMTF_EXPORT VerticalDimension : public FFIRN_Validator
{
public:
   VerticalDimension() = default;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetBaseReference() const noexcept;
   const std::string& GetMinAltitude() const noexcept;
   const std::string& GetAltitudeUnitOfMeasure() const noexcept;
   const std::string& GetFlightIndicator() const noexcept;
   const std::string& GetMaxAltitude() const noexcept;
   /// Int because its expressed as 000-999 in hundreds of feet.
   int GetNumericalMinAltitude() const noexcept;
   int GetNumericalMaxAltitude() const noexcept;

protected:
   bool IsValidVerticalDimensionBaseToFlightLevel() noexcept;
   bool IsValidVerticalDimensionBaseToRelativeAlt() noexcept;
   /// /RAFL:100AGL-FL230
   bool IsValidVerticalDimensionRelativeAltToFlightLevel() noexcept;
   /// /RARA:000AGL-020AGL
   bool IsValidVerticalDimensionRelativeAltToRelativeAlt() noexcept;
   /// /FLFL:FL250-FL290
   bool IsValidVerticalDimensionFlightLevelToFlightLevel() noexcept;
   bool IsValidAltitudeBaseReferencePoint(const std::string& aValue) noexcept;
   bool IsValidRelativeAltitude(const std::string& aValue) noexcept;
   bool IsValidAltitudeUOM(const std::string& aValue) noexcept;
   bool IsValidFlightLevelIndicator(const std::string& aValue) noexcept;

   std::string mBaseReference;
   std::string mMinAltitude;
   std::string mAltitudeMeasurement;
   std::string mFlightIndicator;
   std::string mMaxAltitude;
};
} // namespace usmtf
#endif
