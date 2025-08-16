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

#include "MacroEnabled.hpp"

#include <algorithm>
#include <iterator>

namespace usmtf
{
bool MacroEnabled::HasMacros() const noexcept
{
   return !mMacroCollection.empty();
}

void MacroEnabled::AddMacro(const Macro& aMacro)
{
   mMacroCollection.insert({aMacro.GetKey(), aMacro});
}

void MacroEnabled::AddMacros(const std::vector<Macro>& aMacros)
{
   for (auto& mac : aMacros)
   {
      mMacroCollection.insert({mac.GetKey(), mac});
   }
}

const Macro* MacroEnabled::GetMacro(const std::string& aKey) const noexcept
{
   auto location = mMacroCollection.find(aKey);
   if (location != mMacroCollection.end())
   {
      return &location->second;
   }
   return nullptr;
}

const std::vector<Macro> MacroEnabled::GetMacros() const noexcept
{
   std::vector<Macro> macros;
   std::transform(mMacroCollection.cbegin(),
                  mMacroCollection.cend(),
                  std::back_inserter(macros),
                  [](const std::pair<std::string, Macro>& macro) -> Macro { return macro.second; });
   return macros;
}


} // namespace usmtf
