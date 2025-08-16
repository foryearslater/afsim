// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "VisualEffectsPlugin.hpp"

#include "VaTimeManager.hpp"
#include "WkfEnvironment.hpp"
#include "WkfVtkEnvironment.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkVisualEffects::Plugin,
                          "Visual Effects",
                          "The Visual Effects plugin adds particle effects to the world.",
                          "warlock")

WkVisualEffects::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mEnabled(false)
   , mPrefWidgetPtr(new wkf::VisualEffectsPrefWidget())
   , mPrefObjectPtr(mPrefWidgetPtr->GetPreferenceObject())
   , mDisplayInterfacePtr(new wkf::VisualEffectsDisplayInterface(this, *mPrefObjectPtr))
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

void WkVisualEffects::Plugin::GuiUpdate()
{
   if (mEnabled)
   {
      // Clean up old effects.
      mDisplayInterfacePtr->Update(vaEnv.GetTime()->GetWallClockTime());

      // Process events and display new effects as necessary.
      mInterfacePtr->ProcessEvents(mDisplayInterfacePtr);
   }
}

QList<wkf::PrefWidget*> WkVisualEffects::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}
