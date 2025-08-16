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

#ifndef SECTORGEOMETRY_HPP
#define SECTORGEOMETRY_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "InnerOuterRange.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a SectorGeometry.
//! SectorGeometry is a semi circular shape that might represent a wedge with potentially
//! inner and out radius.
//! usmtf Example:
//! /SECTIO:058190T45-60NM
//!
class USMTF_EXPORT SectorGeometry : public RangeValidator<int>
{
public:
   SectorGeometry();
   bool                   ParseAndValidate(const Field& aField) override;
   const std::string&     GetSectorStartAngle() const noexcept;
   const std::string&     GetSectorStopAngle() const noexcept;
   const std::string&     GetAngleOrientation() const noexcept;
   const InnerOuterRange& GetRange() const noexcept;

protected:
   std::string     mSectorStartAngle;
   std::string     mSectorStopAngle;
   std::string     mAngularOrientation;
   InnerOuterRange mRange;
};
} // namespace usmtf
#endif
