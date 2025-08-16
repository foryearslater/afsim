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
#ifndef WKPLUGINDEMOMODE_HPP
#define WKPLUGINDEMOMODE_HPP

#include "DemoModePrefObject.hpp"
#include "DemoModePrefWidget.hpp"
#include "DemoModeSimInterface.hpp"
#include "VaCallbackHolder.hpp"
#include "WkPlugin.hpp"

namespace vespa
{
class VaEntity;
}

namespace wkf
{
class Platform;
}

namespace WkDemoMode
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;
   QList<wkf::Action*>     GetActions() const override { return mActions; }

   void HandleSimulationComplete();

protected:
   void GuiUpdate() override;
   void PlatformsAddedHandler(const wkf::PlatformList& aPlatformPtr);

private:
   void Restart();

   bool mEnabled;

   vespa::VaCallbackHolder mCallbacks;
   // Use guarded pointers because objects will be owned by main window
   PluginUiPointer<PrefWidget> mPrefWidgetPtr;
   // This is owned by mPrefWidgetPtr
   const PrefObject* mPrefObjectPtr;

   QList<wkf::Action*> mActions;
};
} // namespace WkDemoMode
#endif
