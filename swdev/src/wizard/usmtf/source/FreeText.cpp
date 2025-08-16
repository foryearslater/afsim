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


#include "FreeText.hpp"

#include <sstream>

namespace usmtf
{
FreeText::FreeText()
{
   std::string expr = cStart + cAny + cEnd;
   SetRegexExpression(expr);
}

bool FreeText::ParseAndValidate(const Field& aField)
{
   if (!RegexValidator::ParseAndValidate(aField))
   {
      return false; // Adds Errors
   }

   mFreeText = mMatchResults.captured(1).toStdString();
   return true;
}

const std::string& FreeText::GetText() const noexcept
{
   return mFreeText;
}

} // namespace usmtf
