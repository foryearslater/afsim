// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TrainingPlugin.hpp"

#include "UtQtMemory.hpp"
#include "VaUtils.hpp"
#include "annotation/WkfAttachmentRangeRing.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

#include "TrainingDockWidget.hpp"

// EXERCISE 1 TASK 1b
// Add the macro WKF_PLUGIN_DEFINE_SYMBOLS
// The PLUGIN_TAGS should either "wizard | warlock" or "all",
//  so that the plugin will be loaded by both Wizard and Warlock
// PLACE YOUR CODE HERE

Training::Plugin::Plugin(const QString& aPluginName,
                         const size_t   aUniqueId)
   : wkf::Plugin(aPluginName, aUniqueId)
{
   // EXERCISE 1 TASK 2a
   // Get wkf::MainWindow pointer from WkfEnvironment
   // PLACE YOUR CODE HERE
   // 
   // EXERCISE 1 TASK 2b
   // Add mDockWidget to the Main Window, 
   //  this will also add the action to toggle the visibility of the DockWidget to the View menu
   // PLACE YOUR CODE HERE

   //EXERCISE 2 TASK 1
   // Add a connect statement that connects RangeRing::Plugin::SelectionChanged
   //  to the wkf::Environment::PlatformSelectionChanged signal
   // PLACE YOUR CODE HERE
}

void Training::Plugin::SelectionChanged(const wkf::PlatformList& aSelected,
                                        const wkf::PlatformList& aUnselected)
{
   for (auto platform : aSelected)
   {
      //EXERCISE 2 TASK 2a
      // First check to see if an attachment named "range_ring" is on the platform
      // If the attachment does not exist, create an attachment using vespa::make_attachment
      // The attachment constructor takes three arguments:
      // 1. Parent: which the the platform
      // 2. Viewer: which is the Standard Viewer
      // 3. Unique Name: name the attachment "range_ring"
      // PLACE YOUR CODE HERE
   }

   for (auto platform : aUnselected)
   {
      //EXERCISE 2 TASK 2b
      // Find the attachment on the platform with the name "range_ring"
      // Remove the attachment from the platform using the attachment's GetUniqueId()
      // PLACE YOUR CODE HERE
   }
}
