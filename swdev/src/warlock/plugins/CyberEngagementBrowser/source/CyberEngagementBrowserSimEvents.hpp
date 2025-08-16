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

#ifndef CYBERENGAGEMENTBROWSERSIMEVENTS_HPP
#define CYBERENGAGEMENTBROWSERSIMEVENTS_HPP

#include <string>
#include <vector>

#include "CyberEngagementBrowserDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkCyberEngagementBrowser
{
class CyberEvent : public warlock::SimEvent
{
public:
   virtual void Process(DataContainer& aDataContainer) = 0;
};

class SimCompleteEvent : public CyberEvent
{
public:
   SimCompleteEvent()
      : CyberEvent()
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.ClearData(); }
};

class CyberEngagementEvent : public CyberEvent
{
public:
   CyberEngagementEvent(const std::string& aAttackType,
                        const std::string& aAttacker,
                        const std::string& aAttackTeam,
                        const std::string& aVictim,
                        const std::string& aVictimTeam,
                        const std::string& aResult)
      : CyberEvent()
      , mAttackType(aAttackType)
      , mAttacker(aAttacker)
      , mAttackTeam(aAttackTeam)
      , mVictim(aVictim)
      , mVictimTeam(aVictimTeam)
      , mResult(aResult)
   {
   }

   void Process(DataContainer& aDataContainer) override
   {
      aDataContainer.AddCyberEvent(mAttackType, mAttacker, mAttackTeam, mVictim, mVictimTeam, mResult);
   }

private:
   std::string mAttackType;
   std::string mAttacker;
   std::string mAttackTeam;
   std::string mVictim;
   std::string mVictimTeam;
   std::string mResult;
};

} // namespace WkCyberEngagementBrowser

#endif
