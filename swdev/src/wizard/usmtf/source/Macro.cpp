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

#include "Macro.hpp"

#include <sstream>
#include <string>

namespace usmtf
{
Macro::Macro(const std::string& aKey, const std::string& aInputScaffold) noexcept
   : mKey{aKey}
   , mInputScaffold{aInputScaffold}
{
}

InputBlock Macro::GetInputBlock() const
{
   InputBlock         block{"", "", true, true};
   InputBlockProperty macro{"$define " + mKey, mInputScaffold, " "};
   block.AddProperty(macro);
   return block;
}

const std::string& Macro::GetKey() const noexcept
{
   return mKey;
}

const std::string Macro::GetPlaceholder() const noexcept
{
   return "$<" + mKey + ">$";
}
} // namespace usmtf
