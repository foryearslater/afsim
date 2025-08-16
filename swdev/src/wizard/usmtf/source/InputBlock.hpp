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

#ifndef INPUTBLOCK_HPP
#define INPUTBLOCK_HPP

#include "usmtf_export.h"

#include <vector>

#include "InputBlockProperty.hpp"

namespace usmtf
{
/*!
An Input Block is a class used to abstract away the export of entities and attributes describing those entities
in the shapes that AFSIM Scenarios files need.

A Block can be flat or contain nested blocks. They always construct properties first then nested blocks second, recursively.

          ______     header
          |              properties <---- n Properties
          |              blocks                 __________
Input     |                 -- properties                 |
Block     |                 -- blocks                     | Nested Input block
          |                   -- etc                      |
          |                    -- etc.. n times __________|
          |_____     end_header
*/
class USMTF_EXPORT InputBlock
{
public:
   InputBlock(const std::string& aHeader,
              const std::string& aName,
              const bool         aExcludeFooter = false,
              const bool         aIgnoreNesting = false);
   //! Add an attribute to a block.
   void AddProperty(const InputBlockProperty& aProp) noexcept;
   //! Add a nested block
   void AddBlock(const InputBlock& aBlock) noexcept;
   //! Return the constructed string complete with nesting space
   const std::string GetBlock(unsigned nesting = 0) const noexcept;
   //! Grab the vector of input properties.
   const std::vector<InputBlockProperty>& GetProperties() const noexcept;

private:
   const std::string               BuildIndention(unsigned nesting) const noexcept;
   std::vector<InputBlockProperty> mProperties;
   std::vector<InputBlock>         mNestedBlocks;
   const std::string               mHeader;
   const std::string               mName;
   const std::string               mFooter;
   bool                            mIgnoreNesting;
};
} // namespace usmtf

#endif
