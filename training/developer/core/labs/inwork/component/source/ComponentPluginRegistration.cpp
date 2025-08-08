// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "component_exercise_export.h"

#include "ComponentTypesRegistration.hpp"

#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfPlugin.hpp"
#include "WsfScenario.hpp"
#include "WsfScenarioExtension.hpp"
#include "WsfSimulation.hpp"

//! Provide a way to register the platform control service with the simulation using a WsfApplicationExtension.
class RegisterShieldComponent : public WsfApplicationExtension
{
   public:
      RegisterShieldComponent() = default;

      void AddedToApplication(WsfApplication& aApplication) override
      {
         // EXERCISE 1 TASK 1a
         // Use the application object to register the shield component's script type(s)
         // PLACE YOUR CODE HERE

         // EXERCISE 1 TASK 1b
         // Use the application object to register the "Shields" accessor that extends the WsfPlatform script type.
         // PLACE YOUR CODE HERE

         // Use the application object to register the latinum component's script type(s)
         LatinumComponent::RegisterScriptTypes(*aApplication.GetScriptTypes());

         // Use the application object to register the "Latinum" accessor that extends WsfPlatform
         LatinumComponent::RegisterScriptMethods(*aApplication.GetScriptTypes());
      }
      void ScenarioCreated(WsfScenario& aScenario) override
      {
         // EXERCISE 1 TASK 2 
         // Register the scenario extension that allows us to reference the new type lists.
         // Use the scenario object to register an extension called "shield_types",
         //   of type ComponentTypesRegistration().
         // PLACE YOUR CODE HERE

         // EXERCISE 1 TASK 3 
         // Call the CyberSensorEffect's static method RegisterComponentFactory.
         // PLACE YOUR CODE HERE

      }
};

extern "C"
{
   //! This method is called to check the plugin version and compiler type.
   //! If values do not match the plugin will not load.
   COMPONENT_EXERCISE_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
   {
      aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                 WSF_PLUGIN_API_MINOR_VERSION,
                                 WSF_PLUGIN_API_COMPILER_STRING);
   }

   //! This method is called to register the plugin with the application.
   COMPONENT_EXERCISE_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
   {
      // Use the aApplication object to Register an extension.
      aApplication.RegisterExtension("register_shield_component",
                                      ut::make_unique<RegisterShieldComponent>());
   }
}
