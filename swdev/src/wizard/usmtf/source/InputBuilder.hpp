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

#ifndef INPUTBUILDER_HPP
#define INPUTBUILDER_HPP

#include "usmtf_export.h"

#include "InputBlock.hpp"
#include "USMTF_Transformer.hpp"

namespace usmtf
{
/*!
An Input Builder is a class used to abstract away the construction
and export of an AFSIM Scenario file. Its done through composing
input blocks. See InputBlock.hpp and InputBlockProperty.cpp

          ______     header
          |              properties <---- n Properties
          |              blocks                 __________
Input     |                 -- properties                 |
Block     |                 -- blocks                     | Nested Input block
          |                   -- etc                      |
          |                    -- etc.. n times __________|
          |_____     end_header
*/
class USMTF_EXPORT InputBuilder
{
public:
   explicit InputBuilder(const std::string& aFileName);
   void Export(bool aIgnoreBlockSeperation = false) const;
   void AddBlock(const InputBlock& aBlock) noexcept;
   void AddBlock(const Exportable& aExportbableEntity) noexcept;
   void AddBlock(const Exportable* aExportbableEntity) noexcept;
   void AddBlocks(const std::vector<InputBlock>& aBlocks) noexcept;
   void AddBlocks(const std::vector<const Exportable*>& aExportbableEntities) noexcept;

private:
   const std::string       mFileName;
   std::vector<InputBlock> mBlocks;
};
} // namespace usmtf

#endif
