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
#ifndef PLUGINTUNER_HPP
#define PLUGINTUNER_HPP

#include "TunerCommandDialog.hpp"
#include "TunerMainWidget.hpp"
#include "TunerPrefWidget.hpp"
#include "TunerSimInterface.hpp"
#include "VaCallbackHolder.hpp"
#include "WkPlugin.hpp"
#include "WkfDockWidget.hpp"

namespace WkTuner
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

protected:
   // Slots executed on Gui thread
   void GuiUpdate() override;

   void PlatformsAddedHandler(const wkf::PlatformList& aPlatformPtr);

   void CreateWidgets();

   void InitialAutoTuneSettings(int aPid);

   vespa::VaCallbackHolder mCallbacks;

   PluginUiPointer<CommandDialog>   mCommandDialog;
   PluginUiPointer<wkf::DockWidget> mTunerDockWidget;
   MainWidget*                      mTunerMainWidget;

   PluginUiPointer<PrefWidget> mPreferencesWidget;
};
} // namespace WkTuner
#endif
