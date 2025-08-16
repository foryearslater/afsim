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

#ifndef RADIOENABLED_HPP
#define RADIOENABLED_HPP

#include "usmtf_export.h"

#include <string>

#include "Field.hpp"
#include "FreeText.hpp"
#include "Frequency.hpp"
#include "VariantValidator.hpp"

namespace usmtf
{
//! A class to dry up code thats adding frequency validation support to Sets.
class USMTF_EXPORT RadioEnabled
{
public:
   RadioEnabled() = default;
   const VariantValidator<Frequency, FreeText>& GetPrimaryFrequency() const noexcept;
   const VariantValidator<Frequency, FreeText>& GetSecondaryFrequency() const noexcept;

protected:
   VariantValidator<Frequency, FreeText> mPrimaryFrequency;
   VariantValidator<Frequency, FreeText> mSecondaryFrequency;
};
} // namespace usmtf
#endif
