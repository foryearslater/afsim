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

#include "WeaponBrowserSimInterface.hpp"

#include "UtMemory.hpp"
#include "WkSimEnvironment.hpp"
#include "WkXIO_DataContainer.hpp"
#include "WsfComponentList.hpp"
#include "WsfExplicitWeapon.hpp"
#include "WsfPlatform.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfSimulation.hpp"
#include "WsfWeapon.hpp"
#include "WsfWeaponEngagement.hpp"
#include "WsfWeaponObserver.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"
#include "xio/WsfXIO_RemotePlatformList.hpp"
#include "xio/WsfXIO_WeaponInterface.hpp"
#include "xio/WsfXIO_WeaponObjectInfo.hpp"
#include "xio_sim/WsfXIO_Extension.hpp"

WkWeaponBrowser::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<WeaponEvent>(aPluginName)
   , mChangedPlatform(false)
{
}

void WkWeaponBrowser::SimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   if (IsEnabled())
   {
      AddSimEvent(ut::make_unique<SimulationCompleteEvent>());
   }
}

void WkWeaponBrowser::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   if (!IsEnabled())
   {
      return;
   }

   QMutexLocker locker(&mMutex);

   mCallbacks.Clear();

   // The following callbacks add events to modify the data being stored in the data container
   // WsfWeapon's mQuantityRemaining is only modified during construction/initialization, and in the CompleteReloading
   // and SetQuantityRemaining functions
   // The decrement function for explicit weapons is only called when a weapon platform is added (when firing) and for a
   // deferred launch. The WeaponQuantityChanged signal has been added to these functions, which should address all
   // cases of a weapon changing quantity. Note: Sometimes AFSIM can make a platform fire more missiles than it actually
   // has; the quantity will have a minimum of 0 in this case

   mCallbacks.Add(WsfObserver::WeaponQuantityChanged(&aSimulation)
                     .Connect(
                        [this, &aSimulation](double aSimTime, const WsfWeapon* aWeaponPtr)
                        {
                           if (aWeaponPtr)
                           {
                              AddSimEvent(ut::make_unique<WeaponQuantityEvent>(aWeaponPtr->GetPlatform()->GetName(),
                                                                               aWeaponPtr->GetName(),
                                                                               aWeaponPtr->GetQuantityRemaining()));
                           }
                        }));

   // Platform deleted event
   mCallbacks.Add(WsfObserver::PlatformDeleted(&aSimulation)
                     .Connect(
                        [this](double aSimTime, const WsfPlatform* aPlatformPtr)
                        {
                           if (aPlatformPtr)
                           {
                              AddSimEvent(ut::make_unique<PlatformDeletedEvent>(aPlatformPtr->GetName()));
                           }
                        }));
}

void WkWeaponBrowser::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   QMutexLocker locker(&mMutex);

   // the RemotePlatformList is not created until WkCoreSimInterface's SimulationInitializing function
   // This callback is handled in SimulationStarting to guarantee the creation of RemotePlatformList has already happened
   WsfXIO_RemotePlatformList* platformList = simEnv.GetXIO_Info().GetPlatformList();
   if (platformList)
   {
      mCallbacks.Add(platformList->PlatformPartStateChanged.Connect(
         [this, &aSimulation](WsfXIO_PlatformInfo& aPlatformInfo, WsfXIO_PlatformPartInfo& aPartInfo)
         {
            WsfXIO_PlatformPartInfo*   partInfo  = &aPartInfo;
            WsfXIO_ExplicitWeaponInfo* weaponPtr = dynamic_cast<WsfXIO_ExplicitWeaponInfo*>(partInfo);
            if (weaponPtr)
            {
               AddSimEvent(ut::make_unique<WeaponQuantityEvent>(aPlatformInfo.GetName(),
                                                                weaponPtr->GetName(),
                                                                weaponPtr->mQuantityRemaining));
            }
         }));
   }
}

void WkWeaponBrowser::SimInterface::SetPlatformOfInterest(const std::string& aPlatformName)
{
   QMutexLocker locker(&mMutex);
   // Getting weapons requires either a weaponObserver callback or being connected to the simulation.
   // When changing platform of interest, neither of these conditions are met, so defer to WallClockRead
   mChangedPlatform    = true;
   mPlatformOfInterest = aPlatformName;
}

std::string WkWeaponBrowser::SimInterface::GetPlatformOfInterest() const
{
   QMutexLocker locker(&mMutex);
   return mPlatformOfInterest;
}

void WkWeaponBrowser::SimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   mMutex.lock();
   if (mChangedPlatform)
   {
      mChangedPlatform = false;

      const WsfPlatform* platform = nullptr;
      if (!mPlatformOfInterest.empty())
      {
         platform = aSimulation.GetPlatformByName(mPlatformOfInterest);
      }
      mMutex.unlock();

      if (platform)
      {
         std::vector<WeaponMapData> weaponData;
         if (platform->IsExternallyControlled())
         {
            WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
            if (info != nullptr)
            {
               std::vector<WsfXIO_PlatformPartInfo*> partList = info->GetParts();
               for (const auto& it : partList)
               {
                  int partType = it->GetPartType();
                  // when firing salvos with firing delays, it appears that XIO processes all the weapons without a delay
                  if (partType == cWSF_COMPONENT_WEAPON)
                  {
                     WsfXIO_ExplicitWeaponInfo* weaponPtr = dynamic_cast<WsfXIO_ExplicitWeaponInfo*>(it);
                     if (weaponPtr)
                     {
                        std::string nameString = weaponPtr->GetName();
                        std::string typeString = weaponPtr->GetInstantiationType();
                        int         quantity   = weaponPtr->mQuantityRemaining;
                        weaponData.emplace_back(
                           WeaponMapData(nameString, typeString, quantity, ut::optional<double>(), ut::optional<double>()));
                     }
                  }
               }
            }
         }
         else
         {
            for (WsfComponentList::RoleIterator<WsfWeapon> iter(*platform); !iter.AtEnd(); ++iter)
            {
               WsfExplicitWeapon* weaponPtr = dynamic_cast<WsfExplicitWeapon*>(*iter);
               if (weaponPtr != nullptr)
               {
                  // Add the No-Escape/Maneuver to the weapon map data if they exist.
                  ut::optional<double> noEscape;
                  ut::optional<double> noManeuver;
                  if (weaponPtr->HasAuxData())
                  {
                     auto escapePtr = weaponPtr->GetAuxData().FindAttribute("NO_ESCAPE_FT");
                     if (escapePtr != nullptr)
                     {
                        noEscape = escapePtr->GetDouble();
                     }

                     auto maneuverPtr = weaponPtr->GetAuxData().FindAttribute("NO_MANEUVER_FT");
                     if (maneuverPtr != nullptr)
                     {
                        noManeuver = maneuverPtr->GetDouble();
                     }
                  }

                  weaponData.emplace_back(
                     WeaponMapData(iter->GetName(), iter->GetType(), iter->GetQuantityRemaining(), noEscape, noManeuver));
               }
            }
         }
         AddSimEvent(ut::make_unique<PlatformChangedEvent>(platform->GetName(), weaponData));
      }
   }
   else
   {
      mMutex.unlock();
   }
}
