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

#include "InputBlock.hpp"

#include <sstream>

namespace usmtf
{
InputBlock::InputBlock(const std::string& aHeader, const std::string& aName, const bool aExcludeFooter, const bool aIgnoreNesting)
   : mHeader(aHeader)
   , mName(aName)
   , mFooter(aExcludeFooter ? "" : "end_" + aHeader)
   , mIgnoreNesting(aIgnoreNesting)
{
}

const std::string InputBlock::GetBlock(unsigned nesting) const noexcept
{
   std::stringstream ss{};
   ss << BuildIndention(nesting) << mHeader << " " << mName << "\n";
   for (const auto& prop : mProperties)
   {
      ss << BuildIndention(nesting + 1) << prop.GetProperty() << "\n";
   }
   for (const auto& block : mNestedBlocks)
   {
      ss << "\n";
      if (mIgnoreNesting)
      {
         ss << block.GetBlock(nesting) << "\n";
      }
      else
      {
         ss << block.GetBlock(nesting + 1) << "\n";
      }
   }

   ss << BuildIndention(nesting) << mFooter << "\n";
   return ss.str();
}

void InputBlock::AddProperty(const InputBlockProperty& aProp) noexcept
{
   mProperties.push_back(aProp);
}

void InputBlock::AddBlock(const InputBlock& aBlock) noexcept
{
   mNestedBlocks.push_back(aBlock);
}

const std::string InputBlock::BuildIndention(unsigned nesting) const noexcept
{
   if (mIgnoreNesting)
   {
      return "";
   }
   std::stringstream ss{};
   for (unsigned i = 0; i < nesting; i++)
   {
      // three spaces
      for (int j = 0; j < 3; j++)
      {
         ss << " ";
      }
   }
   return ss.str();
}

const std::vector<InputBlockProperty>& InputBlock::GetProperties() const noexcept
{
   return mProperties;
}
} // namespace usmtf
