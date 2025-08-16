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

#include "WeaponBrowserDataContainer.hpp"

#include <algorithm>

void WkWeaponBrowser::DataContainer::ResetData()
{
   mWeaponDataMap.clear();
   emit RelevantDataChanged();
}

// if the platform of interest is already being tracked, no change...otherwise begin tracking it
// removal and addition of weapons are handled in other cases
// Note: It is assumed that it isn't possible to add new weapon types to an existing platform mid-simulation (not even
// via script). Logic will need to change if this assumption is wrong.
void WkWeaponBrowser::DataContainer::PlatformChangedUpdate(const std::string&                aPlatformName,
                                                           const std::vector<WeaponMapData>& aData)
{
   if (!aData.empty())
   {
      auto mapIt = mWeaponDataMap.find(aPlatformName);
      if (mapIt == mWeaponDataMap.end())
      {
         mWeaponDataMap[aPlatformName] = aData;
      }
   }
   emit RelevantDataChanged();
}

void WkWeaponBrowser::DataContainer::PlatformDeletedUpdate(const std::string& aPlatformName)
{
   auto mapIt = mWeaponDataMap.find(aPlatformName);
   // don't keep data for deleted platforms
   if (mapIt != mWeaponDataMap.end())
   {
      mWeaponDataMap.erase(mapIt);
   }
   // display an empty table when the platform of interest is deleted
   if (aPlatformName == mPlatformOfInterest || mPlatformOfInterest.empty())
   {
      emit RelevantDataChanged();
   }
}

void WkWeaponBrowser::DataContainer::WeaponQuantityUpdate(const std::string& aPlatformName,
                                                          const std::string& aWeaponName,
                                                          int                aQuantity)
{
   auto mapIt = mWeaponDataMap.find(aPlatformName);
   if (mapIt != mWeaponDataMap.end())
   {
      auto mapDataIt = std::find_if(mapIt->second.begin(),
                                    mapIt->second.end(),
                                    [aWeaponName](const WeaponMapData& arg) { return aWeaponName == arg.mName; });
      if (mapDataIt != mapIt->second.end())
      {
         mapDataIt->mQuantityRemaining = aQuantity;
         // Design Decision: instead of editing the quantity to reflect some new lower maximum if amountChanged < 0, the
         // quantity will have red text and greyed out Fire button when invalid only update the dialog if the weapon is
         // fired from the platform of interest
         if (aPlatformName == mPlatformOfInterest)
         {
            emit RelevantDataChanged();
         }
      }
   }
}

void WkWeaponBrowser::DataContainer::MenuExpansionUpdate(const std::string& aPlatformName,
                                                         const std::string& aWeaponName,
                                                         bool               aShow)
{
   auto mapIt = mWeaponDataMap.find(aPlatformName);
   if (mapIt != mWeaponDataMap.end())
   {
      auto mapDataIt = std::find_if(mapIt->second.begin(),
                                    mapIt->second.end(),
                                    [aWeaponName](const WeaponMapData& arg) { return aWeaponName == arg.mName; });
      if (mapDataIt != mapIt->second.end())
      {
         mapDataIt->mExpand = aShow;
      }
   }
}

void WkWeaponBrowser::DataContainer::SetPlatformOfInterest(const std::string& aPlatformName)
{
   mPlatformOfInterest = aPlatformName;
}

std::vector<WkWeaponBrowser::WeaponMapData> WkWeaponBrowser::DataContainer::GetWidgetData() const
{
   auto it = mWeaponDataMap.find(mPlatformOfInterest);
   if (it != mWeaponDataMap.end())
   {
      return it->second;
   }
   // return an empty vector if actual data is unable to be found (some platforms have no weapons)
   return std::vector<WkWeaponBrowser::WeaponMapData>();
}

void WkWeaponBrowser::DataContainer::UpdateWeaponTrack(const QString& aText, const std::string& aWeaponName, int aIndex)
{
   auto mapIt = mWeaponDataMap.find(mPlatformOfInterest);
   if (mapIt != mWeaponDataMap.end())
   {
      auto mapDataIt = std::find_if(mapIt->second.begin(),
                                    mapIt->second.end(),
                                    [aWeaponName](const WeaponMapData& arg) { return aWeaponName == arg.mName; });
      if (mapDataIt != mapIt->second.end())
      {
         if (aIndex < static_cast<int>(mapDataIt->mWeaponCommands.size()))
         {
            // if not a number, sets the track to 0
            mapDataIt->mWeaponCommands[aIndex].trackIndex = aText.toInt();
         }
      }
   }
}

void WkWeaponBrowser::DataContainer::UpdateWeaponQueuedQuantity(const QString& aText, const std::string& aWeaponName, int aIndex)
{
   auto mapIt = mWeaponDataMap.find(mPlatformOfInterest);
   if (mapIt != mWeaponDataMap.end())
   {
      auto mapDataIt = std::find_if(mapIt->second.begin(),
                                    mapIt->second.end(),
                                    [aWeaponName](const WeaponMapData& arg) { return aWeaponName == arg.mName; });
      if (mapDataIt != mapIt->second.end())
      {
         if (aIndex < static_cast<int>(mapDataIt->mWeaponCommands.size()))
         {
            // if not a number, sets the track to 0
            mapDataIt->mWeaponCommands[aIndex].quantity = aText.toInt();
         }
      }
   }
}

void WkWeaponBrowser::DataContainer::AddWeaponCommand(const std::string& aWeaponName)
{
   auto mapIt = mWeaponDataMap.find(mPlatformOfInterest);
   if (mapIt != mWeaponDataMap.end())
   {
      auto mapDataIt = std::find_if(mapIt->second.begin(),
                                    mapIt->second.end(),
                                    [aWeaponName](const WeaponMapData& arg) { return aWeaponName == arg.mName; });
      if (mapDataIt != mapIt->second.end())
      {
         mapDataIt->mWeaponCommands.emplace_back(FiringData());
         mapDataIt->mExpand = true;
      }
   }
}

void WkWeaponBrowser::DataContainer::RemoveWeaponCommand(const std::string& aWeaponName, int aIndex)
{
   auto mapIt = mWeaponDataMap.find(mPlatformOfInterest);
   if (mapIt != mWeaponDataMap.end())
   {
      auto mapDataIt = std::find_if(mapIt->second.begin(),
                                    mapIt->second.end(),
                                    [aWeaponName](const WeaponMapData& arg) { return aWeaponName == arg.mName; });
      if (mapDataIt != mapIt->second.end())
      {
         mapDataIt->mWeaponCommands.erase(std::next(mapDataIt->mWeaponCommands.begin(), aIndex));
      }
   }
}
