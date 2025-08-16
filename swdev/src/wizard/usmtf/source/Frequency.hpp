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

#ifndef FREQUNECY_HPP
#define FREQUNECY_HPP

#include "usmtf_export.h"

#include <string>

#include "RangeValidator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of Frequency.
//! USMTF Excerpt:
/*!
   A. PRIMARY FREQUENCY IN MEGAHERTZ

   ALLOWABLE ENTRIES: (.0000001 THROUGH 99999999) [0 TO 7 DECIMAL PLACES]. DATA ITEM AND DATA CODE VALUES
   CONSIST OF THE INTEGERS 1 THROUGH 99999999, AND DECIMAL POINT VALUES. DECIMAL POINT VALUES RANGE FROM .0000001
   TO 999999.9, CONSISTING OF ALL POSSIBLE COMBINATIONS OF SEVEN OR FEWER SIGNIFICANT DIGITS AND A
   SIGNIFICANT DECIMAL POINT.

   B. PRIMARY FREQUENCY DESIGNATOR
   ALLOWABLE ENTRIES: [LITERAL]. THE DESIGNATORS ARE ASSIGNED BY OPERATIONS ORDERS, HANDBOOKS, ETC.
*/
//! /EXAMPLE: /PFREQ:342.5
//! EXAMPLE: /PDESIG:TAD01
class USMTF_EXPORT Frequency : public RangeValidator<float>
{
public:
   Frequency();
   bool               ParseAndValidate(const Field& aField) override;
   float              GetNumericalFrequency() const noexcept;
   const std::string& GetFrequency() const noexcept;

protected:
   std::string mFrequency;
};
} // namespace usmtf
#endif
