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

// This might get templated, depends on needs.

#ifndef INPUTBLOCKPROPERTY_HPP
#define INPUTBLOCKPROPERTY_HPP

#include "usmtf_export.h"

#include <string>

namespace usmtf
{
/*!
An Input Block Property is a class used to add delimited data to an Input Block.

          ______     header
          |              properties <---- n Properties
          |              blocks                 __________
Input    |                 -- properties                 |
Block     |                 -- blocks                     | Nested Input block
          |                   -- etc                      |
          |                    -- etc.. n times __________|
          |_____     end_header
*/
class USMTF_EXPORT InputBlockProperty
{
public:
   InputBlockProperty(const std::string& aName, const std::string& aValue, const std::string& aDelimeter = " = ") noexcept;
   const std::string& GetProperty() const noexcept;

private:
   const std::string mProp;
};
} // namespace usmtf

#endif
