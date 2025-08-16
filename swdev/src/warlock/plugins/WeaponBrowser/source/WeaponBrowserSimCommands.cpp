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

#include "WeaponBrowserSimCommands.hpp"

#include "WkObserver.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfTrack.hpp"
#include "WsfTrackManager.hpp"
#include "WsfWeapon.hpp"
#include "xio/WsfXIO_WeaponObjectInfo.hpp"

void WkWeaponBrowser::FireCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformName);
   // only display a maximum of one QMessageBox error per attempted firing command
   if (platform)
   {
      WsfTrack* track = platform->GetTrackManager().FindTrack(WsfTrackId(mPlatformName, mTrackIndex));
      if (!platform->IsExternallyControlled())
      {
         WsfWeapon* weapon = platform->GetComponent<WsfWeapon>(mWeaponName);
         if (track && weapon && mQuantity >= 1)
         {
            weapon->FireSalvo(aSimulation.GetSimTime(), WsfWeapon::FireTarget(track), WsfWeapon::SalvoOptions(mQuantity));
            WkObserver::SimulationUserAction(
               aSimulation)("Weapon " + mWeaponName + " fired with quantity of " + std::to_string(mQuantity), platform);
         }
         else if (!weapon)
         {
            QString message = "The weapon \"" + QString::fromStdString(mWeaponName) +
                              "\" could not be found on platform \"" + QString::fromStdString(mPlatformName) + ".\"";
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning, "Weapon Not Found", message);
         }
         else if (!track)
         {
            QString message = "The local track with id " + QString::number(mTrackIndex) +
                              " could not be found on platform \"" + QString::fromStdString(mPlatformName) + ".\"";
            wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning, "Track Not Found", message);
         }
      }
      else
      {
         WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(platform);
         if (info != nullptr)
         {
            WsfXIO_PlatformPartInfo*   weapon    = info->FindPart(mWeaponName, cWSF_COMPONENT_WEAPON);
            WsfXIO_ExplicitWeaponInfo* weaponPtr = dynamic_cast<WsfXIO_ExplicitWeaponInfo*>(weapon);
            if (weaponPtr && track && mQuantity >= 1)
            {
               weaponPtr->FireSalvo(WsfTrackId(mPlatformName, mTrackIndex), mQuantity);
               WkObserver::SimulationUserAction(aSimulation)("Fire Weapon", platform);
            }
            else if (!weaponPtr)
            {
               QString message = "The weapon \"" + QString::fromStdString(mWeaponName) +
                                 "\" could not be found on platform \"" + QString::fromStdString(mPlatformName) + ".\"";
               wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning, "Weapon Not Found", message);
            }
            else if (!track)
            {
               QString message = "The local track with id " + QString::number(mTrackIndex) +
                                 " could not be found on platform \"" + QString::fromStdString(mPlatformName) + ".\"";
               wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning, "Track Not Found", message);
            }
         }
      }
   }
   else
   {
      QString message = "The platform \"" + QString::fromStdString(mPlatformName) + "\" no longer exists.";
      wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Warning, "Platform Not Found", message);
   }
}
