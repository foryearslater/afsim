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

#include "Zone.hpp"

#include "USMTF_Transformer.hpp"
#include "UtStringUtil.hpp"

namespace usmtf
{
Zone::Zone(const Segment& aSegment) noexcept
   : USMTF_Transformer(aSegment)
{
   mOptionalAmpn = ExtractSet<Ampn>("AMPN", "Zone", false, true);
}

InputBlock Zone::GetInputBlock() const
{
   InputBlock block{"zone", "Invalid_Or_Base_Zone"};
   return block;
}

void Zone::AddAmplificationBlockIfPresent(InputBlock& aInputBlock) const
{
   if (!mOptionalAmpn)
   {
      return;
   }
   InputBlock         ampn{"aux_data", ""};
   InputBlockProperty ampnProp{"string AMPN_TEXT", "\"" + mOptionalAmpn->GetAmplificationText() + "\""};
   ampn.AddProperty(ampnProp);
   aInputBlock.AddBlock(ampn);
}
} // namespace usmtf
