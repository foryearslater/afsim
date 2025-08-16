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

#include "InputBlockProperty.hpp"

namespace usmtf
{
InputBlockProperty::InputBlockProperty(const std::string& aName, const std::string& aValue, const std::string& aDelimeter) noexcept
   : mProp(aName + aDelimeter + aValue)
{
}

const std::string& InputBlockProperty::GetProperty() const noexcept
{
   return mProp;
}
} // namespace usmtf
