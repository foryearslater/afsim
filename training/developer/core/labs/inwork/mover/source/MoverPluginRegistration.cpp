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

#include "mover_exercise_export.h"

#include "MATLABBallisticMover.hpp"
#include "UtPlugin.hpp"
#include "UtLog.hpp"
#include "UtMemory.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"
#include "mover/WsfMoverTypes.hpp"
#include "WsfPlugin.hpp"
#include "WsfScenario.hpp"

class RegisterMATLAB_BallisticMover : public WsfApplicationExtension
{
   public:
      RegisterMATLAB_BallisticMover()
      {
      }
      ~RegisterMATLAB_BallisticMover() noexcept override
      {
         // Clean up the Matlab Compiler Runtime.
         libAFSIM_MoverTerminate();
         mclTerminateApplication();
      }
      void ScenarioCreated(WsfScenario& aScenario) override
      {
         // To use a MATLAB shared library, you must initialize and terminate
         // the MATLAB Compiler Runtime instance correctly.
         if (!mclInitializeApplication(NULL, 0) ||
             !libAFSIM_MoverInitialize())
         {
            ut::log::fatal() << "Could not initialize MATLAB libraries!";
            exit(-1);
         }
         // EXERCISE 1 TASK 1
         // PLACE YOUR CODE HERE
         
      }
};

extern "C"
{
   MOVER_EXERCISE_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
   {
      aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                 WSF_PLUGIN_API_MINOR_VERSION,
                                 WSF_PLUGIN_API_COMPILER_STRING);
   }
   MOVER_EXERCISE_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
   {
      aApplication.RegisterExtension("register_matlab_ballistic_mover", ut::make_unique<RegisterMATLAB_BallisticMover>());
   }
}
