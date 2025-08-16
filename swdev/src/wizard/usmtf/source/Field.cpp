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

#include "Field.hpp"

#include <sstream>
#include <vector>

namespace usmtf
{
Field::Field(const std::string& aContent) noexcept
{
   SetOptionalDescriptor(aContent);
}

const std::string& Field::GetContent() const noexcept
{
   return mContent;
}

const std::string& Field::GetDescriptor() const noexcept
{
   return mDescriptor;
}

bool Field::HasDescriptor() const noexcept
{
   return !mDescriptor.empty();
}

void Field::SetOptionalDescriptor(const std::string& aFieldValue) noexcept
{
   if (aFieldValue.empty())
   {
      mContent = aFieldValue;
      mDescriptor.clear();
      return;
   }
   std::stringstream        ss{aFieldValue};
   std::vector<std::string> results;
   std::string              token;
   while (std::getline(ss, token, ':'))
   {
      results.push_back(token);
   }
   // designator found
   if (results.size() > 1)
   {
      mContent    = results[1];
      mDescriptor = results[0];
   }
   else // no designator
   {
      mContent = results[0];
      mDescriptor.clear();
   }
}
} // namespace usmtf
