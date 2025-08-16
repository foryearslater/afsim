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
#include "WkScenario.hpp"

#include "DisEntityId.hpp"
#include "WkPlatform.hpp"

warlock::Platform* warlock::Scenario::FindPlatformByDisId(DisEntityId aId) const
{
   if (aId.IsDefined())
   {
      for (const auto& it : GetIndexPlatformMap())
      {
         auto plat = dynamic_cast<warlock::Platform*>(it.second);
         if (plat && plat->GetDisEntityId() == aId)
         {
            return plat;
         }
      }
   }
   return nullptr;
}
