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


#include "USMTF_Exceptions.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

// These are  obviously identical, this is common. The Taxonomy is the distinction. If need for more
// complex hierarchies arise, please feel free to expand.
namespace usmtf
{
ImportError::ImportError(const std::string& aWhatHappened) noexcept
   : std::runtime_error(aWhatHappened)
{
}

ExportError::ExportError(const std::string& aWhatHappened) noexcept
   : std::runtime_error(aWhatHappened)
{
}

MessageError::MessageError(const std::string& aWhatHappened) noexcept
   : std::runtime_error(aWhatHappened)
{
}

SetError::SetError(const std::string& aWhatHappened, size_t aOptionalPosition) noexcept
   : std::runtime_error(aWhatHappened)
   , mOptionalPosition{aOptionalPosition}
{
}

size_t SetError::GetPosition() const noexcept
{
   return mOptionalPosition;
}

ValidationError::ValidationError(const std::string& aFailureReason,
                                 const std::string& aValueValidated,
                                 const std::string& aOptionalSuggestion) noexcept
   : std::runtime_error(ConstructError(aFailureReason, aValueValidated, aOptionalSuggestion))
{
}

ValidationError::ValidationError(const std::string& aReason) noexcept
   : std::runtime_error(aReason)
{
} // This bypasses formatting. This can DEF be done better.

const std::string ValidationError::ConstructError(const std::string& aFailureReason,
                                                  const std::string& aValueValidated,
                                                  const std::string& aOptionalSuggestion) noexcept
{
   std::stringstream ss{};
   ss << "Value: " << aValueValidated << " is invalid. \n";
   ss << "Reason: " << aFailureReason << "\n";
   ss << "Suggestion: " << aOptionalSuggestion << "\n\n";
   return ss.str();
}
} // namespace usmtf
