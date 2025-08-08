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

#include "xio_exercise_export.h"

#include "UtPlugin.hpp"
#include "UtMemory.hpp"

#include "PlatformControlService.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfPlugin.hpp"
#include "WsfSimulation.hpp"
#include "WsfScenarioExtension.hpp"

//! Provide a way to register the platform control service
//! with the simulation using a WsfScenarioExtension.
class RegisterPlatformController : public WsfScenarioExtension
{
   public:

      ~RegisterPlatformController() noexcept override = default;

      void SimulationCreated(WsfSimulation& aSimulation) override
      {
         // EXERCISE 1 TASK 1
         // Use the simulation object to register an extension
         // Name this extension "platform_controller"
         // Provide a new instance of PlatformControlService as a parameter
         aSimulation.RegisterExtension("platform_controller",
                                       ut::make_unique<PlatformControlService>());
      }
};

extern "C"
{
   //! This method is called to check the plugin version and compiler type.
   //! If values do not match the plugin will not load.
   XIO_EXERCISE_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
   {
      aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                 WSF_PLUGIN_API_MINOR_VERSION,
                                 WSF_PLUGIN_API_COMPILER_STRING);
   }

   //! This method is called to register the plugin with the application.
   XIO_EXERCISE_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
   {
      // Make an application extension that creates a scenario extension for every scenario.
      aApplication.RegisterExtension("platform_controller_registration",
                                      ut::make_unique<WsfDefaultApplicationExtension<RegisterPlatformController>>());
      aApplication.ExtensionDepends("platform_controller_registration", "wsf_p6dof", true);
   }
}
