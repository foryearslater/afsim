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

#ifndef ERRORMANAGER_HPP
#define ERRORMANAGER_HPP

#include "usmtf_export.h"

#include <iostream>

#include "USMTF_Exceptions.hpp"

namespace usmtf
{
//! A Class with responsibility of aggregating and reporting errors.
//! Given so many items are validated upon construction, its useful to have this class
//! help them out.
class USMTF_EXPORT ErrorManager
{
public:
   virtual ~ErrorManager() = default;
   bool                                HasErrors() const noexcept;
   void                                LogErrors(std::ostream& aOut) const noexcept;
   const std::vector<ValidationError>& GetErrors() const;

protected:
   void AddError(const ValidationError& aError) noexcept;
   void AddError(const std::string& aFailureReason,
                 const std::string& aValueValidatied,
                 const std::string& aOptionalSuggestion = "") noexcept;
   void AddError(const std::string& aErrorMessage) noexcept;
   void AddErrors(const std::vector<ValidationError>& aErrors) noexcept;
   void ClearErrors() noexcept;

private:
   std::vector<ValidationError> mErrors;
};
} // namespace usmtf
#endif
