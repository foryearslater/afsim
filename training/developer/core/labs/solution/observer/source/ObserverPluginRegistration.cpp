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

#include "observer_exercise_export.h"
#include "RegisterUDP_Observer.hpp"

#include "WsfPlugin.hpp"

#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "UtMemory.hpp"

extern "C"
{
   OBSERVER_EXERCISE_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
   {
      // EXERCISE 1 TASK 1
      // Set the plugin version by invoking UtPluginVersion
      // Using the macros and static constant defined in WsfPlugin.hpp, the first argument
      // should be the major version number, the second argument should be the minor version
      // number, and the third argument should be the compiler string
      aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                 WSF_PLUGIN_API_MINOR_VERSION,
                                 WSF_PLUGIN_API_COMPILER_STRING);
   }
   OBSERVER_EXERCISE_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
   {
      // EXERCISE 1 TASK 2
      // Register the default application extension with aApplication
      // Name this application extension "register_udp_observer".
      // instantiate a unique_ptr to a WsfDefaultApplicationExtension, templated upon
      // the RegisterUDP_Observer scenario extension
      aApplication.RegisterExtension("register_udp_observer",
                                     ut::make_unique<WsfDefaultApplicationExtension<RegisterUDP_Observer>>());
   }
}
