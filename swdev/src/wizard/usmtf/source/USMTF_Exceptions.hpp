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


#ifndef USMTFEXCPEPTIONS_HPP
#define USMTFEXCPEPTIONS_HPP

#include "usmtf_export.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace usmtf
{
/*!
   Collection of Exceptions used to distinguish errors from points
   in the parsing and validation process.
*/

//! Error with import file streams or Parsing with the Standard in mind.
class USMTF_EXPORT ImportError : public std::runtime_error
{
public:
   explicit ImportError(const std::string& aWhatHappened) noexcept;
};

//! Error with Export file streams.
class USMTF_EXPORT ExportError : public std::runtime_error
{
public:
   explicit ExportError(const std::string& aWhatHappened) noexcept;
};

//! Error in accessing a Set that isn't present in the Message.
class USMTF_EXPORT MessageError : public std::runtime_error
{
public:
   explicit MessageError(const std::string& aWhatHappened) noexcept;
};

//! Error in accessing a field that isn't present in the set.
class USMTF_EXPORT SetError : public std::runtime_error
{
public:
   explicit SetError(const std::string& aWhatHappened, size_t aOptionalPosition = 0) noexcept;
   size_t GetPosition() const noexcept;

private:
   size_t mOptionalPosition;
};

/// Validation Errors, used heavily in anything thats Validatable
class USMTF_EXPORT ValidationError : public std::runtime_error
{
public:
   ValidationError(const std::string& aFailureReason,
                   const std::string& aValueValidatied,
                   const std::string& aOptionalSuggestion = "") noexcept;
   explicit ValidationError(const std::string& aSeperator) noexcept;

private:
   const std::string ConstructError(const std::string& aFailureReason,
                                    const std::string& aValueValidatied,
                                    const std::string& aSuggestion) noexcept;
};
} // namespace usmtf

#endif
