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

#ifndef TACANCHANNEL_HPP
#define TACANCHANNEL_HPP

#include "usmtf_export.h"

#include <string>

#include "RegexValidator.hpp"

namespace usmtf
{
//! FFIRN_Validator responsible for the validation and property extraction of TacanChannel.
//! USMTF Excerpt:
/*!
   THE TACAN CHANNEL OF THE RECEIVER AND TANKER AIRCRAFT INVOLVED IN AIR-TO-AIR REFUELING.

   EXPRESSED AS FOLLOWS:

   RECEIVER TACAN CHANNEL
   ALLOWABLE ENTRIES: (1 THROUGH 999).

   HYPHEN
   ALLOWABLE ENTRIES: -. A CHARACTER THAT SEPARATES DATA ELEMENTS.

   TANKER TACAN CHANNEL
   ALLOWABLE ENTRIES: (1 THROUGH 999).

   EXAMPLE: /16-72
*/
class USMTF_EXPORT TacanChannel : public RegexValidator
{
public:
   TacanChannel();
   bool               ParseAndValidate(const Field& aField) override;
   const std::string& GetRecieverChannel() const noexcept;
   const std::string& GetTankerChannel() const noexcept;

protected:
   std::string mRecieverChannel;
   std::string mTankerChannel;
};
} // namespace usmtf
#endif
