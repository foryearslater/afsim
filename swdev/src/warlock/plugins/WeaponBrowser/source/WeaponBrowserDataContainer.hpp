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

#ifndef AdvancedWeaponBrowserDataContainer_HPP
#define AdvancedWeaponBrowserDataContainer_HPP

#include <unordered_map>

#include <QObject>

#include "UtOptional.hpp"

// Note: Weapons do not get a unique Id until they have already been fired (platform is created when fired), so need to
// manually track it. The Id associated with the weapon is for its name/type, which means a quantity of a weapon sharing
// name/type will have the same id.

namespace WkWeaponBrowser
{
struct FiringData
{
   // must maintain this data even if platform of interest changes
   FiringData()
      : quantity(1)
      , trackIndex(0)
   {
   }

   double quantity;
   size_t trackIndex;
};

struct WeaponMapData
{
   WeaponMapData(const std::string&   aName,
                 const std::string&   aType,
                 double               aQuantityRemaining,
                 ut::optional<double> aNoEscapeDistance,
                 ut::optional<double> aNoManeuverDistance)
      : mName(aName)
      , mType(aType)
      , mQuantityRemaining(aQuantityRemaining)
      , mExpand(false)
      , mNoEscapeDistance(aNoEscapeDistance)
      , mNoManeuverDistance(aNoManeuverDistance)
   {
   }

   std::string             mName;
   std::string             mType;
   double                  mQuantityRemaining;
   bool                    mExpand;
   std::vector<FiringData> mWeaponCommands;
   ut::optional<double>    mNoEscapeDistance;
   ut::optional<double>    mNoManeuverDistance;
};

class DataContainer : public QObject
{
   Q_OBJECT
public:
   DataContainer()
      : QObject()
      , mPlatformOfInterest()
   {
   }

   std::vector<WeaponMapData> GetWidgetData() const;

   const std::string& GetPlatformOfInterest() const { return mPlatformOfInterest; }

   void ResetData();

   void SetPlatformOfInterest(const std::string& aPlatformName);

   void PlatformChangedUpdate(const std::string& aPlatformName, const std::vector<WeaponMapData>& aData);

   void PlatformDeletedUpdate(const std::string& aPlatformName);

   void WeaponQuantityUpdate(const std::string& aPlatformName, const std::string& aWeaponName, int aQuantity);

   void MenuExpansionUpdate(const std::string& aPlatformName, const std::string& aWeaponName, bool aShow);

   void UpdateWeaponTrack(const QString& aText, const std::string& aWeaponName, int aIndex);
   void UpdateWeaponQueuedQuantity(const QString& aText, const std::string& aWeaponName, int aIndex);

   void AddWeaponCommand(const std::string& aWeaponName);
   void RemoveWeaponCommand(const std::string& aWeaponName, int aIndex);

signals:
   // this signal is necessary because of the delayed processing associated with changing the platform of interest
   void RelevantDataChanged();

private:
   std::string mPlatformOfInterest;

   // key: platform_name, which is unique in scenario
   std::unordered_map<std::string, std::vector<WeaponMapData>> mWeaponDataMap;
};
} // namespace WkWeaponBrowser

#endif
