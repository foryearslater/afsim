// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "weapon_exercise_export.h"

#include "PhaserLethality.hpp"
#include "PhaserWeapon.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfScenario.hpp"
#include "WsfWeaponEffectsTypes.hpp"
#include "WsfWeaponTypes.hpp"

//! Provide a way to register the platform control service with the simulation using a WsfApplicationExtension.
class RegisterPhaserWeapon : public WsfApplicationExtension
{
   public:
      RegisterPhaserWeapon() = default;

      ~RegisterPhaserWeapon() noexcept override = default;

      void ScenarioCreated(WsfScenario& aScenario) override
      {
         // EXERCISE 1 TASK 3a
         //! Using the scenario's WeaponTypes, Add a prototype PhaserWeapon object called "PHASER_WEAPON"
         //! Call the static “Get” Method in WsfWeaponTypes with the argument of "aScenario".
         //! Then call "Add", calling the weapon "PHASER_WEAPON", and provide a prototype instance of PhaserWeapon.
         // PLACE YOUR CODE HERE

         // EXERCISE 1 TASK 3b
         //! Using the scenario's WeaponEffectsTypes, Add a prototype PhaserLethality object called "PHASER_LETHALITY"
         //! Call the static “Get” Method in WsfWeaponEffectTypes with the argument of "aScenario"
         //! Then call "Add", calling the weapon effect "PHASER_LETHALITY", and provide a prototype instance of PhaserLethality.
         // PLACE YOUR CODE HERE
      }
};
