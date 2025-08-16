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

#ifndef PLATFORMTYPEDEFINITION_HPP
#define PLATFORMTYPEDEFINITION_HPP

#include "usmtf_export.h"

#include "Exportable.hpp"
#include "Message.hpp"

namespace usmtf
{
//! Exportable entity that exports an input block for a type definition.
/*!
   A PlatformTypeDefinition is the representation of an AFSIM platform type script block.
   This will export:
   platform_type name base type
   end_platform_type
*/
class USMTF_EXPORT PlatformTypeDefinition : public Exportable
{
public:
   PlatformTypeDefinition() = default;
   explicit PlatformTypeDefinition(const std::string& aDefinitionName, const std::string& aAFSIMBaseType) noexcept;
   void               SetDefinitionName(const std::string& aDefinitionName);
   void               SetAfsimBase(const std::string& aAFSIMBaseType);
   InputBlock         GetInputBlock() const override;
   const std::string& GetDefinition() const noexcept;

private:
   std::string mDefinitionName;
   std::string mAFSIMBaseType;
};
} // namespace usmtf
#endif
