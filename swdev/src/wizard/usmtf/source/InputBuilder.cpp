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

#include "InputBuilder.hpp"

#include <algorithm>
#include <fstream>
#include <iterator>

#include "USMTF_Exceptions.hpp"

namespace usmtf
{
InputBuilder::InputBuilder(const std::string& aFileName)
   : mFileName(aFileName)
{
}

void InputBuilder::Export(bool aIgnoreBlockSeperation) const
{
   std::ofstream file(mFileName);
   if (!file)
   {
      throw ExportError("Failed to open file: " + mFileName);
   }
   file << "#Written by AFSIM USMTF Script builder\n\n";
   for (const auto& block : mBlocks)
   {
      if (aIgnoreBlockSeperation)
      {
         file << block.GetBlock();
      }
      else
      {
         file << block.GetBlock() << "\n";
      }
   }
   file << "#End Script Builder Write\n\n";
   file.close();
}

void InputBuilder::AddBlock(const InputBlock& aBlock) noexcept
{
   mBlocks.push_back(aBlock);
}

void InputBuilder::AddBlock(const Exportable& aExportbableEntity) noexcept
{
   mBlocks.push_back(aExportbableEntity.GetInputBlock());
}

void InputBuilder::AddBlock(const Exportable* aExportbableEntity) noexcept
{
   if (aExportbableEntity)
   {
      mBlocks.push_back(aExportbableEntity->GetInputBlock());
   }
}

void InputBuilder::AddBlocks(const std::vector<InputBlock>& aBlocks) noexcept
{
   std::copy(aBlocks.cbegin(), aBlocks.cend(), std::back_inserter(mBlocks));
}

void InputBuilder::AddBlocks(const std::vector<const Exportable*>& aTransforms) noexcept
{
   for (const auto& transform : aTransforms)
   {
      if (transform)
      {
         mBlocks.push_back(transform->GetInputBlock());
      }
   }
}

} // namespace usmtf
