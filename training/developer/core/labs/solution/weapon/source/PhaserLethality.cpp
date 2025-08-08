// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2013 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
// ****************************************************************************
// Updated by Infoscitex, a DCS Company.
// ****************************************************************************

#include "PhaserLethality.hpp"

#include <iostream>
#include <string>

#include "UtSphericalEarth.hpp"
#include "UtInput.hpp"
#include "WsfAttributeContainer.hpp"
#include "WsfStringId.hpp"
#include "WsfException.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfWeaponEngagement.hpp"

PhaserLethality::PhaserLethality(WsfScenario& aScenario)
   : WsfWeaponEffects(aScenario),
     mShieldDamageRate(20.0),
     mArmorDamageRate(20.0),
     mLastUpdateTime(0.0)
{
   // Use a Pk determined at intercept, not launch (although for a DE weapon
   // like this, it is essentially the same...
   SetUseInterceptPk();
   SetInterceptPk(1.0);
}

PhaserLethality::PhaserLethality(const PhaserLethality& aSrc)
   : WsfWeaponEffects(aSrc),
     mShieldDamageRate(aSrc.mShieldDamageRate),
     mArmorDamageRate(aSrc.mArmorDamageRate),
     mLastUpdateTime(0.0)
{
}

//virtual
bool PhaserLethality::ProcessInput(UtInput& aInput)
{
   bool myCommand = true;
   std::string command = aInput.GetCommand();
   if (command == "shield_damage_rate")
   {
      aInput.ReadValue(mShieldDamageRate);
   }
   else if (command == "armor_damage_rate")
   {
      // EXERCISE 2 TASK 2
      aInput.ReadValue(mArmorDamageRate);
   }
   return myCommand;
}

//virtual
bool PhaserLethality::Initialize(double                     aSimTime,
                                 const WsfWeaponEngagement* aEngagementPtr)
{
   bool success = WsfWeaponEffects::Initialize(aSimTime, aEngagementPtr);
   mLastUpdateTime = aSimTime;  // Start applying the effect now
   return success;
}

//virtual
//! Called every integration interval while firing, this method applies damage
//! to the target platform's shields or armor.  If armor is depleted, the platform
//! is destroyed.
void PhaserLethality::ApplyEffectTo(double       aSimTime,
                                    WsfPlatform* aTargetPtr)
{
   WsfPlatform* weaponPlatformPtr = GetEngagement()->GetFiringPlatform();
   double integrationTime = aSimTime - mLastUpdateTime;
   double damage = 1.0e-6; // always apply some damage.

   if ((weaponPlatformPtr != nullptr) && (integrationTime > 0.0))
   {
      double shields(0.0);
      double armor(100.0);

      // Get the shield and armor data from the auxiliary data container on the target platform
      WsfUtil::GetAuxValue(aTargetPtr, "phaser_shields", shields);
      WsfUtil::GetAuxValue(aTargetPtr, "phaser_armor", armor);

      // EXERCISE 2 TASK 6
      // Reduce shields based on damage rate
      shields -= mShieldDamageRate * integrationTime;
      if (shields <= 0.0)
      {
         // If shields are down then reduce armor base on damage rate
         shields = 0.0;
         armor = armor - mArmorDamageRate * integrationTime;
      }

      // If armor is depleted, target is killed and removed
      if (armor <= 0.0)
      {
         armor = 0.0;
         damage = WsfWeaponEffects::cKILLED;
      }

      // Set the new shield and armor values in the auxiliary data container
      aTargetPtr->GetAuxData().Assign("phaser_shields", shields);
      aTargetPtr->GetAuxData().Assign("phaser_armor", armor);
   }

   //! Reflect the last update time.
   mLastUpdateTime = aSimTime;

   // Call the base class to apply any damage to the target.
   ApplyEffectIncrement(aSimTime, aTargetPtr, damage);
}
