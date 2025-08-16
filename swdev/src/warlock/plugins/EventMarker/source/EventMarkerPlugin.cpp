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
#include "EventMarkerPlugin.hpp"

#include "EventMarkerSimEvents.hpp"
#include "WkfEnvironment.hpp"
#include "WkfVtkEnvironment.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkEventMarker::Plugin,
                          "Event Marker",
                          "The Event Marker plugin visualizes AFSIM events as color coded icons on the map. Events "
                          "include platform damage, platform removed, weapon hit, weapon miss.",
                          "warlock")

WkEventMarker::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface, wkf::EventMarkerPluginBase>(aPluginName, aUniqueId)
   , mEnabled(false)
{
   connect(&wkfEnv,
           &wkf::Environment::Initialize,
           this,
           [this]()
           {
              mEnabled = (vaEnv.GetStandardViewer() != nullptr);
              mInterfacePtr->SetEnabled(mEnabled);
           });
}

void WkEventMarker::Plugin::GuiUpdate()
{
   if (mEnabled)
   {
      GetDisplayInterface()->CleanUp(vaEnv.GetDisplayedTime());

      // Process all the new SimEvents
      mInterfacePtr->ProcessEvents(GetDisplayInterface(), GetPrefObject());
   }
}
