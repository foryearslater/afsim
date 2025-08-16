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
#ifndef PLUGINSIMCONTROLLER_HPP
#define PLUGINSIMCONTROLLER_HPP

#include "SimControllerDataContainer.hpp"
#include "SimControllerPrefWidget.hpp"
#include "SimControllerSimInterface.hpp"
#include "SimControllerStatusWidget.hpp"
#include "SimControllerToolbar.hpp"
#include "WkPlugin.hpp"

namespace WkSimController
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;
   QList<wkf::Action*>     GetActions() const override;

private:
   // Slots executed on Gui thread
   void GuiUpdate() override;

   void LoadActionHandler();
   void StartupDialogRequested();
   void RecentActionHandler();
   void RemoteSimulationBehindHandler();

   void                          AddLoadScenarioActionsToFileMenu();
   void                          PopulateRecentMenu();
   std::unique_ptr<wkf::Updater> GetUpdater(const QString&     aPlatformName,
                                            const QString&     aDatum,
                                            const unsigned int aContext = 0) const override;

   QMenu* mRecentScenariosMenu;

   // Use guarded pointers because objects will be owned by main window
   PluginUiPointer<PrefWidget>   mPrefWidgetPtr;
   PluginUiPointer<StatusWidget> mStatusWidgetPtr;
   PluginUiPointer<Toolbar>      mToolbarPtr;

   SimControllerDataContainer mSimulationState;
};
} // namespace WkSimController
#endif
