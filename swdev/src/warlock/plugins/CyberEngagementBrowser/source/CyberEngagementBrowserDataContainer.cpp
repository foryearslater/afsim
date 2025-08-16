// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CyberEngagementBrowserDataContainer.hpp"

#include "WkfEnvironment.hpp"

void WkCyberEngagementBrowser::DataContainer::ClearData()
{
   for (const auto& cyberEvent : mCyberEvents)
   {
      emit EventRemoved(cyberEvent);
   }
   mCyberEvents.clear();
}

void WkCyberEngagementBrowser::DataContainer::AddCyberEvent(const std::string& aAttackType,
                                                            const std::string& aAttacker,
                                                            const std::string& aAttackTeam,
                                                            const std::string& aVictim,
                                                            const std::string& aVictimTeam,
                                                            const std::string& aResult)
{
   // Find and remove any pre-existing events with the same attack type, attacker, and victim before
   // adding the new event. This ensures uniqueness of all events.
   CyberEvent evt(aAttackType, aAttacker, aAttackTeam, aVictim, aVictimTeam, aResult);
   RemoveCyberEvent(evt);
   mCyberEvents.push_back(evt);

   emit EventAdded(evt);
}

void WkCyberEngagementBrowser::DataContainer::RemoveCyberEvent(const CyberEvent& aEvent)
{
   for (auto evtIt = mCyberEvents.begin(); evtIt != mCyberEvents.end(); ++evtIt)
   {
      if (aEvent == *evtIt)
      {
         mCyberEvents.erase(evtIt);
         emit EventRemoved(aEvent);
         break;
      }
   }
}
