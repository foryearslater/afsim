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
#ifndef FIELD_HPP
#define FIELD_HPP

#include "usmtf_export.h"

#include <string>

namespace usmtf
{
//! Formal USMTF Excerpt:
//!   Field Formats.A field format establishes and is derived
//! from the structure of allowed field content, i.e., the words
//! (literals) or codes used in completing a set.A field format
//! may support either elemental concepts, such as the name of a
//! location, or a combination of elemental concepts, such as date time
//! group, that are linked by common usage or formatting
//! requirements.The field format, therefore, is the fundamental
//! building block in the overall structure of formatted messages
//! and provides the bridge between that structure and the
//! vocabulary of allowed literals and codes.The USMTF categorizes
//! the vocabulary used in formatted messages by use of the Field
//! Format Index Reference Number[FFIRN] and Field Use Designator
//! Number[FUDN].The FFIRNs / FUDNs to be used in a particular set
//! format are identified in Paragraph 5.3.1.The USMTF Catalog of
//! Field Formats provides, by FFIRN / FUDN, the specific data items
//! and corresponding data codes to be used as field content.A
//! detailed discussion of rules concerning fields is provided in
//! Paragraph 4.3.6 below.

//! Layman's Explanation:
//! A Field is simply some data in a set governed by some rules.
//! It can have an optional descriptor that identifies it as well as the data
//! itself, to which i call content.

//! The rules are up to the Set and enforced by the FIRN/FUDN applied.
//! Programmatically we use FFIRN_Validators for parsing and enforcement.
class USMTF_EXPORT Field final
{
public:
   explicit Field(const std::string& aContent = std::string()) noexcept;
   const std::string& GetContent() const noexcept;
   //! Descriptors are optional data elements meant to describe
   //! the data is prefixing, we use them to optimize parsing and validation
   const std::string& GetDescriptor() const noexcept;
   bool               HasDescriptor() const noexcept;

private:
   //! Sets up Field with Descriptor or Defaults it to "".
   void        SetOptionalDescriptor(const std::string& aFieldValue) noexcept;
   std::string mContent;
   std::string mDescriptor;
};
} // namespace usmtf
#endif
