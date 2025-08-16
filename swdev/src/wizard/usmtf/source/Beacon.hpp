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

#ifndef BEACON_HPP
#define BEACON_HPP

#include "usmtf_export.h"

#include <string>

#include "RegexValidator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of Beacon.
//! USMTF Excerpt:
/*!
   THE CODED SIGNAL TRANSMITTED AIR TO AIR FOR THE PURPOSE OF TANKER IDENTIFICATION DURING RENDEZVOUS.

   EXPRESSED AS FOLLOWS:

   CONTEXT QUANTITY, 0-6
   ALLOWABLE ENTRIES: (0 THROUGH 6).

   HYPHEN
   ALLOWABLE ENTRIES: -. A CHARACTER THAT SEPARATES DATA ELEMENTS.

   CONTEXT QUANTITY, 0-6
   ALLOWABLE ENTRIES: (0 THROUGH 6).

   HYPHEN
   ALLOWABLE ENTRIES: -. A CHARACTER THAT SEPARATES DATA ELEMENTS.

   CONTEXT QUANTITY, 0-6
   ALLOWABLE ENTRIES: (0 THROUGH 6).

   EXAMPLE: /4-3-1
*/
class USMTF_EXPORT Beacon : public RegexValidator
{
public:
   Beacon();
   bool ParseAndValidate(const Field& aField) override;
   //! The USMTF Specification fails to elaborate on the purpose of the beacon
   //! portions, so they are named by position.
   const std::string& GetFirst() const noexcept;
   const std::string& GetSecond() const noexcept;
   const std::string& GetThird() const noexcept;

protected:
   std::string mFirst;
   std::string mSecond;
   std::string mThird;
};
} // namespace usmtf
#endif
