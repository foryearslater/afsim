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

#include "sensor_exercise_export.h"
#include "TricorderSensor.hpp"

#include "UtMemory.hpp"
#include "UtPlugin.hpp"
#include "UtScriptTypes.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "WsfPlugin.hpp"
#include "WsfSensorTypes.hpp"

class TricorderSensorRegistration : public WsfApplicationExtension
{
   public:
      TricorderSensorRegistration() = default;

      ~TricorderSensorRegistration() noexcept override = default;

      void AddedToApplication(WsfApplication& aApplication) override
      {
         // EXERCISE 1 TASK 1
         // Use the application object parameter to add the script tricorder class
         // to the script types
         // Call the script class "TricorderSensor"
         // PLACE YOUR CODE HERE
      }
      void ScenarioCreated(WsfScenario& aScenario) override
      {
         // EXERCISE 1 TASK 2
         // Use the scenario object parameter to add the TricorderSensor class to the
         // list of sensor types
         // Name the new sensor type "TRICORDER_SENSOR"
         // PLACE YOUR CODE HERE
      }
};

extern "C"
{
   SENSOR_EXERCISE_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
   {
      aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                 WSF_PLUGIN_API_MINOR_VERSION,
                                 WSF_PLUGIN_API_COMPILER_STRING);
   }
   SENSOR_EXERCISE_EXPORT void WsfPluginSetup(WsfApplication& aApplicationPtr)
   {
      aApplicationPtr.RegisterExtension("tricorder_sensor_registration",
                                        ut::make_unique<TricorderSensorRegistration>());
   }
}
