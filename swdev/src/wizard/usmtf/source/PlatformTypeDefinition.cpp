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

#include "PlatformTypeDefinition.hpp"

namespace usmtf
{
PlatformTypeDefinition::PlatformTypeDefinition(const std::string& aDefinitionName, const std::string& aAFSIMBaseType) noexcept
   : mDefinitionName{aDefinitionName}
   , mAFSIMBaseType{aAFSIMBaseType}
{
}

void PlatformTypeDefinition::SetDefinitionName(const std::string& aDefinitionName)
{
   mDefinitionName = aDefinitionName;
}

void PlatformTypeDefinition::SetAfsimBase(const std::string& aAFSIMBaseType)
{
   mAFSIMBaseType = aAFSIMBaseType;
}

const std::string& PlatformTypeDefinition::GetDefinition() const noexcept
{
   return mDefinitionName;
}

InputBlock PlatformTypeDefinition::GetInputBlock() const
{
   InputBlock platformDefinition{"platform_type", mDefinitionName + " " + mAFSIMBaseType};
   return platformDefinition;
}
} // namespace usmtf
