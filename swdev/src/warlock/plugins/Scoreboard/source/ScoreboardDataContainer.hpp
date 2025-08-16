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

#ifndef SCOREBOARDDATACONTAINER_HPP
#define SCOREBOARDDATACONTAINER_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <QObject>

#include "WsfPlatform.hpp"

namespace WkScoreboard
{
enum class WeaponEventType
{
   Fire,
   Hit,
   Kill,
   Miss
};

struct WeaponEvent
{
   // Not saving a WsfPlatform* b/c contents of ptr can get deleted during sim
   WeaponEvent(const std::string&     aFiringPlatformName,
               const std::string&     aFiringPlatformTeam,
               const std::string&     aFiringPlatformType,
               const std::string&     aTargetPlatformName,
               const std::string&     aTargetPlatformTeam,
               const std::string&     aTargetPlatformType,
               const std::string&     aWeaponType,
               int                    aWeaponQuantity,
               double                 aSimTime,
               const WeaponEventType& aEventType)
      : firingPlatformName(aFiringPlatformName)
      , firingPlatformTeam(aFiringPlatformTeam)
      , firingPlatformType(aFiringPlatformType)
      , targetPlatformName(aTargetPlatformName)
      , targetPlatformTeam(aTargetPlatformTeam)
      , targetPlatformType(aTargetPlatformType)
      , weaponType(aWeaponType)
      , weaponQuantity(aWeaponQuantity)
      , simTime(aSimTime)
      , eventType(aEventType)
   {
   }

   WeaponEvent(const WsfPlatform*     aFiringPlatformPtr,
               const WsfPlatform*     aTargetPlatformPtr,
               const WsfPlatform*     aWeaponPlatformPtr,
               int                    aWeaponQuantity,
               double                 aSimTime,
               const WeaponEventType& aEventType)
      : firingPlatformName("")
      , firingPlatformTeam("")
      , firingPlatformType("")
      , targetPlatformName("")
      , targetPlatformTeam("")
      , targetPlatformType("")
      , weaponType("")
      , weaponQuantity(aWeaponQuantity)
      , simTime(aSimTime)
      , eventType(aEventType)
   {
      if (aFiringPlatformPtr)
      {
         firingPlatformName = aFiringPlatformPtr->GetName();
         firingPlatformTeam = aFiringPlatformPtr->GetSide();
         firingPlatformType = aFiringPlatformPtr->GetType();
      }
      if (aTargetPlatformPtr)
      {
         targetPlatformName = aTargetPlatformPtr->GetName();
         targetPlatformTeam = aTargetPlatformPtr->GetSide();
         targetPlatformType = aTargetPlatformPtr->GetType();
      }
      if (aWeaponPlatformPtr)
      {
         weaponType = aWeaponPlatformPtr->GetType();
      }
   }

   std::string     firingPlatformName;
   std::string     firingPlatformTeam;
   std::string     firingPlatformType;
   std::string     targetPlatformName;
   std::string     targetPlatformTeam;
   std::string     targetPlatformType;
   std::string     weaponType;
   int             weaponQuantity;
   double          simTime;
   WeaponEventType eventType;
};

class DataContainer : public QObject
{
public:
   DataContainer() {}

   std::vector<WeaponEvent> GetScoreboardData() { return mScoreboardData; }

   void Clear() { mScoreboardData.clear(); }

   void AddScoreboardData(const WeaponEvent& aWeaponEvent) { mScoreboardData.push_back(aWeaponEvent); }

private:
   std::vector<WeaponEvent> mScoreboardData;
};
} // namespace WkScoreboard

#endif
