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

#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "usmtf_export.h"

#include "AerialRefuelingInfo.hpp"
#include "AircraftMission.hpp"
#include "Exportable.hpp"
#include "MacroEnabled.hpp"
#include "Message.hpp"
#include "PlatformTypeDefinition.hpp"
#include "TaskUnit.hpp"

namespace usmtf
{
/*!
   A platform is the representation of an AFSIM platform script block, including the position attribute.
   This will export:
   platform name type
      position lat lon
   end_platform
*/
class USMTF_EXPORT Platform : public Exportable
{
public:
   explicit Platform() noexcept;
   InputBlock                    GetInputBlock() const override;
   const PlatformTypeDefinition& GetTypeDefinition() const noexcept;
   void                          SetTypeDefinition(const PlatformTypeDefinition& aDefinition);
   void                          SetCategory(const std::string& aCategory);
   void                          SetPosition(const std::string& aPosition);
   void                          SetName(const std::string& aName);

private:
   PlatformTypeDefinition mDefinition;
   std::string            mCategory;
   std::string            mPosition;
   std::string            mName;
};
} // namespace usmtf
#endif
