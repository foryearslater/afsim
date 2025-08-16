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

#include "USMTF_Transformer.hpp"

#include <cctype>

namespace usmtf
{
const std::string USMTF_Transformer::ReplaceSpaces(const std::string& aTarget, char aDesiredChar) noexcept
{
   // Brute force, can be improved if needed.
   std::string copy(aTarget);
   for (auto& character : copy)
   {
      if (std::isspace(character))
      {
         character = aDesiredChar;
      }
   }
   return copy;
}
} // namespace usmtf
