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

#include "comm_exercise_export.h"
#include "SignalCommRegistration.hpp"

#include "UtPlugin.hpp"
#include "WsfApplication.hpp"
#include "WsfApplicationExtension.hpp"

extern "C"
{
   COMM_EXERCISE_EXPORT void WsfPluginVersion(UtPluginVersion& aVersion)
   {
      aVersion = UtPluginVersion(WSF_PLUGIN_API_MAJOR_VERSION,
                                 WSF_PLUGIN_API_MINOR_VERSION,
                                 WSF_PLUGIN_API_COMPILER_STRING);
   }
   COMM_EXERCISE_EXPORT void WsfPluginSetup(WsfApplication& aApplication)
   {
      // EXERCISE 1 TASK 1
      // Invoke aApplication's RegisterExtension method
      // The first argument is the name of the application extension
      // The second argument is a newly created unique_ptr to a defaul application extension 
      // that is templated upon a scenario extension (the SignalCommRegistration class)
      // PLACE YOUR CODE HERE
      
   }
}
