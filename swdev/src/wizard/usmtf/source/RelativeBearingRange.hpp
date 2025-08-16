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

#ifndef RELATIVEBEARINGRANGE_HPP
#define RELATIVEBEARINGRANGE_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "RegexValidator.hpp"
#include "UtLLPos.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of a RelativeBearingRange.
//! USMTF Excerpt:
//! /330-PT ALFA-50
class USMTF_EXPORT RelativeBearingRange : public RegexValidator
{
public:
   RelativeBearingRange();
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetDegrees() const noexcept;
   const std::string& GetFreeTextOne() const noexcept;
   const std::string& GetFreeTextTwo() const noexcept;
   const std::string& GetDistance() const noexcept;

protected:
   std::string mDegree;
   std::string mFreeOne;
   std::string mFreeTwo;
   std::string mDistance;
};
} // namespace usmtf
#endif
