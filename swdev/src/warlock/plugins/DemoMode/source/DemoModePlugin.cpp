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
#include "DemoModePlugin.hpp"

#include <QTimer>

#include "DemoModeSimCommands.hpp"
#include "UtMemory.hpp"
#include "VaEntity.hpp"
#include "WkRunManager.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfPlatform.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkDemoMode::Plugin,
                          "Demo Mode",
                          "The Demo Mode plugin provides useful options for automatically configuring and restarting "
                          "Warlock when it is running as a demo, such as at a trade show.",
                          "warlock")

WkDemoMode::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mEnabled(false)
   , mPrefWidgetPtr(new WkDemoMode::PrefWidget)
   , mPrefObjectPtr(mPrefWidgetPtr->GetPreferenceObject())
{
   connect(mPrefObjectPtr,
           &WkDemoMode::PrefObject::EnabledChanged,
           this,
           [this](bool aEnabled)
           {
              mEnabled = aEnabled;
              mInterfacePtr->SetEnabled(aEnabled);
           });
   connect(mPrefObjectPtr,
           &WkDemoMode::PrefObject::RunSpeedMultiplierChanged,
           this,
           [this](double aMultiplier)
           {
              mInterfacePtr->SetRunSpeedMultiplier(aMultiplier);
              mInterfacePtr->AddSimCommand(ut::make_unique<RunSpeedCommand>(aMultiplier));
           });

   mCallbacks.Add(wkf::Observer::PlatformsAdded.Connect(&WkDemoMode::Plugin::PlatformsAddedHandler, this));

   wkf::Action* shortcut = new wkf::Action("Toggle Demo Mode", wkfEnv.GetMainWindow(), QKeySequence("Ctrl+D"));
   connect(shortcut,
           &QAction::triggered,
           this,
           [this]
           {
              mEnabled = !mEnabled;
              mInterfacePtr->SetEnabled(mEnabled);
           });
   mActions.push_back(shortcut);
}

QList<wkf::PrefWidget*> WkDemoMode::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

void WkDemoMode::Plugin::HandleSimulationComplete()
{
   if (mEnabled)
   {
      double timeDelay = mPrefObjectPtr->GetPauseLength();
      if (timeDelay > 0)
      {
         QTimer::singleShot(timeDelay * 1000, this, [this]() { Restart(); });
      }
      else
      {
         Restart();
      }
   }
}

void WkDemoMode::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(*this);
}

void WkDemoMode::Plugin::PlatformsAddedHandler(const wkf::PlatformList& aPlatforms)
{
   if (mEnabled && mPrefObjectPtr->GetSelectPlatform())
   {
      for (const auto& platform : aPlatforms)
      {
         if (platform->GetName() == mPrefObjectPtr->GetSelectedPlatformName())
         {
            wkfEnv.SetPlatformsSelected({platform});
         }
      }
   }
}

void WkDemoMode::Plugin::Restart()
{
   auto recents = warlock::RunManager::GetInstance().GetRecentScenarios();
   if (!recents.empty())
   {
      warlock::RunManager::GetInstance().LoadScenario(recents.first());
   }
}
