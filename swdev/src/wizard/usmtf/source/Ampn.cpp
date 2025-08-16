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

#include "Ampn.hpp"

#include "USMTF_Exceptions.hpp"
#include "UtMemory.hpp"

namespace usmtf
{
Ampn::Ampn(const std::string& aSTF, const std::vector<Field>& aFields) noexcept
   : Set(aSTF, aFields)
{
   // Test Field presence
   try
   {
      GetField(1);
   }
   catch (SetError&)
   {
      AddError("AMPN Is Missing Required Fields", "AMPN", "Please make sure AMPN has fields in positions 1");
   }
}

const std::string& Ampn::GetAmplificationText() const
{
   return GetFieldContent(1);
}
} // namespace usmtf
