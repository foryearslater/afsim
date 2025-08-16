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

#ifndef FREETEXT_HPP
#define FREETEXT_HPP

#include "usmtf_export.h"

#include <string>

#include "FFIRN_Validator.hpp"
#include "RegexValidator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of FreeText.
//! FreeText is simply unvalidated text. Anything can go here.
//! USMTF Excerpt:
//! /anything goes here
class USMTF_EXPORT FreeText : public RegexValidator
{
public:
   FreeText();
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetText() const noexcept;


protected:
   std::string mFreeText;
};
} // namespace usmtf
#endif
