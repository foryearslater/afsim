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

#ifndef AIRSPACETIMEMODE_HPP
#define AIRSPACETIMEMODE_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of an AirspaceTimeMode
//! An AirspaceTimeMode is a field that indicates whether a time frame is defined as an interval or
//! if it is discrete.

//!     AirspaceTimeMode
//!         | | | |
//!         v v v v
//! APERIOD/DISCRETE/141325ZFEB/142359ZFEB//

//! USMTF Excerpt
//! TABLE A
//! AIRSPACE TIME MODE
//!   CODE
//!   TIME BLOCK FIXED(U)	   DISCRETE
//!   TIME BLOCK REPEATING(U)	INTERVAL
class USMTF_EXPORT AirspaceTimeMode : public FFIRN_Validator
{
public:
   AirspaceTimeMode() = default;
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetTimeMode() const noexcept;

protected:
   bool        IsValidAirspaceTimeMode() noexcept;
   std::string mTimeMode;
};
} // namespace usmtf
#endif
