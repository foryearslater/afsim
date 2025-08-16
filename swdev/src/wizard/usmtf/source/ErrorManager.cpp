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

#include "ErrorManager.hpp"

#include <iostream>

namespace usmtf
{
bool ErrorManager::HasErrors() const noexcept
{
   return !mErrors.empty();
}

void ErrorManager::LogErrors(std::ostream& aOut) const noexcept
{
   for (const auto& err : mErrors)
   {
      aOut << err.what();
   }
}

const std::vector<ValidationError>& ErrorManager::GetErrors() const
{
   return mErrors;
}

void ErrorManager::AddError(const ValidationError& aError) noexcept
{
   mErrors.push_back(aError);
}

void ErrorManager::AddError(const std::string& aFailureReason,
                            const std::string& aValueValidatied,
                            const std::string& aOptionalSuggestion) noexcept
{
   mErrors.emplace_back(aFailureReason, aValueValidatied, aOptionalSuggestion);
}

void ErrorManager::AddError(const std::string& aErrorMessage) noexcept
{
   mErrors.emplace_back(aErrorMessage);
}

void ErrorManager::AddErrors(const std::vector<ValidationError>& aErrors) noexcept
{
   mErrors.insert(mErrors.end(), aErrors.begin(), aErrors.end());
}

void ErrorManager::ClearErrors() noexcept
{
   mErrors.clear();
}
} // namespace usmtf
