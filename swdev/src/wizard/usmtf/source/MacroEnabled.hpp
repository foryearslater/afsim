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

#ifndef MACROENABLED_HPP
#define MACROENABLED_HPP

#include "usmtf_export.h"

#include <map>
#include <vector>

#include "Macro.hpp"

namespace usmtf
{
//! Classes that use this can potentially contain macro definitions that need written out.
//! Macro definitions cannot be duplicates.
class MacroEnabled
{
public:
   bool                     HasMacros() const noexcept;
   void                     AddMacro(const Macro& aMacro);
   void                     AddMacros(const std::vector<Macro>& aMacros);
   const Macro*             GetMacro(const std::string& aKey) const noexcept;
   const std::vector<Macro> GetMacros() const noexcept;

private:
   // sorted for uniform export.
   std::map<std::string, Macro> mMacroCollection;
};
} // namespace usmtf

#endif
