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

#ifndef CYBERENGAGEMENTBROWSERDATACONTAINER_HPP
#define CYBERENGAGEMENTBROWSERDATACONTAINER_HPP

#include <string>
#include <vector>

#include <QObject>

namespace WkCyberEngagementBrowser
{
class DataContainer : public QObject
{
   Q_OBJECT

public:
   struct CyberEvent
   {
      CyberEvent(const std::string& aAttackType,
                 const std::string& aAttacker,
                 const std::string& aAttackTeam,
                 const std::string& aVictim,
                 const std::string& aVictimTeam,
                 const std::string& aResult)
         : mAttackType(aAttackType)
         , mAttacker(aAttacker)
         , mAttackTeam(aAttackTeam)
         , mVictim(aVictim)
         , mVictimTeam(aVictimTeam)
         , mResult(aResult)
      {
      }

      bool operator==(const CyberEvent& aRHS) const
      {
         // Only care about the attack type, attacker, and victim since these make up the "key" for each event.
         // Only one event with this key will be shown at a time.
         return (mAttackType == aRHS.mAttackType && mAttacker == aRHS.mAttacker && mVictim == aRHS.mVictim);
      }

      std::string mAttackType;
      std::string mAttacker;
      std::string mAttackTeam;
      std::string mVictim;
      std::string mVictimTeam;
      std::string mResult;
   };

   DataContainer() = default;

   void ClearData();

   void AddCyberEvent(const std::string& aAttackType,
                      const std::string& aAttacker,
                      const std::string& aAttackTeam,
                      const std::string& aVictim,
                      const std::string& aVictimTeam,
                      const std::string& aResult);

   const std::vector<CyberEvent>& GetEvents() const { return mCyberEvents; }

signals:
   void EventAdded(const CyberEvent& aCyberEvent);
   void EventRemoved(const CyberEvent& aCyberEvent);

private:
   void RemoveCyberEvent(const CyberEvent& aEvent);

   std::vector<CyberEvent> mCyberEvents;
};
} // namespace WkCyberEngagementBrowser

#endif