// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WKVASCENARIO_HPP
#define WKVASCENARIO_HPP

#include "warlock_core_export.h"

#include "WkfScenario.hpp"

class DisEntityId;

namespace warlock
{
class Platform;

class WARLOCK_CORE_EXPORT Scenario : public wkf::Scenario
{
public:
   Scenario() = default;

   Platform* FindPlatformByDisId(DisEntityId aId) const;
};
} // namespace warlock
#endif
