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

#include "RadioEnabled.hpp"

namespace usmtf
{
const VariantValidator<Frequency, FreeText>& RadioEnabled::GetPrimaryFrequency() const noexcept
{
   return mPrimaryFrequency;
}

const VariantValidator<Frequency, FreeText>& RadioEnabled::GetSecondaryFrequency() const noexcept
{
   return mSecondaryFrequency;
}
} // namespace usmtf
