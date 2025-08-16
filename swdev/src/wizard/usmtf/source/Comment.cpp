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

#include "Comment.hpp"

#include <sstream>
#include <string>

namespace usmtf
{
Comment::Comment(const Segment& aSegment) noexcept
   : USMTF_Transformer(aSegment)
{
}

InputBlock Comment::GetInputBlock() const
{
   InputBlock block{"", "", true, true};
   for (const auto& loc : mSegment)
   {
      // data split by new lines become commented properties
      std::string              originalSet = loc.first.GetReconstructedSet();
      std::vector<std::string> parts;
      std::string              split;
      std::istringstream       stream(originalSet);
      while (std::getline(stream, split, '\n'))
      {
         parts.push_back(split);
      }
      for (const auto& str : parts)
      {
         InputBlockProperty prop{"#", str, ""};
         block.AddProperty(prop);
      }
   }
   return block;
}
} // namespace usmtf
