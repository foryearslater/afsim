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

#ifndef AdvancedWeaponBrowserSimEvents_HPP
#define AdvancedWeaponBrowserSimEvents_HPP

#include "WeaponBrowserDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkWeaponBrowser
{
class WeaponEvent : public warlock::SimEvent
{
public:
   virtual void Process(DataContainer& aData) = 0;
};

class SimulationCompleteEvent : public WeaponEvent
{
public:
   SimulationCompleteEvent()
      : WeaponEvent()
   {
   }

   void Process(DataContainer& aData) override { aData.ResetData(); }
};

class PlatformChangedEvent : public WeaponEvent
{
public:
   PlatformChangedEvent(const std::string& aPlatformName, std::vector<WeaponMapData>& aWeaponData)
      : WeaponEvent()
      , mPlatformName(aPlatformName)
      , mWeaponData(aWeaponData)
   {
   }

   void Process(DataContainer& aData) override { aData.PlatformChangedUpdate(mPlatformName, mWeaponData); }

private:
   std::string                mPlatformName;
   std::vector<WeaponMapData> mWeaponData;
};

class PlatformDeletedEvent : public WeaponEvent
{
public:
   PlatformDeletedEvent(const std::string& aPlatformName)
      : WeaponEvent()
      , mPlatformName(aPlatformName)
   {
   }

   void Process(DataContainer& aData) override { aData.PlatformDeletedUpdate(mPlatformName); }

private:
   std::string mPlatformName;
};

class WeaponQuantityEvent : public WeaponEvent
{
public:
   WeaponQuantityEvent(const std::string& aPlatformName, const std::string& aWeaponName, int aQuantity)
      : WeaponEvent()
      , mPlatformName(aPlatformName)
      , mWeaponName(aWeaponName)
      , mQuantity(aQuantity)
   {
   }

   void Process(DataContainer& aData) override { aData.WeaponQuantityUpdate(mPlatformName, mWeaponName, mQuantity); }

private:
   std::string mPlatformName;
   std::string mWeaponName;
   int         mQuantity;
};
} // namespace WkWeaponBrowser

#endif