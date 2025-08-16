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

#include "Platform.hpp"

#include "USMTF_Transformer.hpp"


namespace usmtf
{
Platform::Platform() noexcept {}

InputBlock Platform::GetInputBlock() const
{
   InputBlock         platform{"platform", mName + " " + GetTypeDefinition().GetDefinition()};
   InputBlockProperty category{"category", mCategory, " "};
   InputBlockProperty position{"position", mPosition, " "};
   platform.AddProperty(category);
   platform.AddProperty(position);

   return platform;
}

const PlatformTypeDefinition& Platform::GetTypeDefinition() const noexcept
{
   return mDefinition;
}

void Platform::SetTypeDefinition(const PlatformTypeDefinition& aDefinition)
{
   mDefinition = aDefinition;
}

void Platform::SetCategory(const std::string& aCategory)
{
   mCategory = aCategory;
}

void Platform::SetPosition(const std::string& aPosition)
{
   mPosition = aPosition;
}

void Platform::SetName(const std::string& aName)
{
   mName = aName;
}
} // namespace usmtf
