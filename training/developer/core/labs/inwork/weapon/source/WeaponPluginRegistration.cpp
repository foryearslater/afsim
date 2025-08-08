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

#include "weapon_exercise_export.h"

#include "RegisterPhaserWeapon.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfPlugin.hpp"

extern "C"
{
   //! This method is called to check the plugin version and compiler type.
   //! If values do not match the plugin will not load.
   WEAPON_EXERCISE_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
   {
      // EXERCISE 1 TASK 1
      // Initialize the plugin version object using the provided macros in UtPlugin.hpp
      // Use the method UtPluginVersion(...)
	   // PLACE YOUR CODE HERE
      
   }

   //! This method is called to register the plugin with the application.
   WEAPON_EXERCISE_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
   {
      // EXERCISE 1 TASK 2a
      // Use the aApplication object to Register an extension.
      // Name this extension "register_phaser_weapon".
      // Make it of type RegisterPhaserWeapon
      // PLACE YOUR CODE HERE
      
      // EXERCISE 1 TASK 2b
      // Tell the application that this extension, "register_phaser_weapon", depends on "wsf_mil"
      // Make sure that wsf_mil is loaded before we try to register our new types.
      // (WsfWeaponTypes and WsfWeaponEffectsTypes need to be valid)
      // PLACE YOUR CODE HERE
   }
}
