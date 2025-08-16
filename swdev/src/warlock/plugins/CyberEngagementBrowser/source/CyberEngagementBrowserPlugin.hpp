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

#ifndef CYBERENGAGEMENTBROWSERPLUGIN_HPP
#define CYBERENGAGEMENTBROWSERPLUGIN_HPP

#include "CyberEngagementBrowserDockWidget.hpp"
#include "CyberEngagementBrowserPrefWidget.hpp"
#include "CyberEngagementBrowserSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkCyberEngagementBrowser
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

protected:
   void GuiUpdate() override;

private:
   PluginUiPointer<DockWidget> mDockWidget;
   DataContainer               mCyberEngagementBrowserData;
   PluginUiPointer<PrefWidget> mPrefWidgetPtr;
   const PrefObject*           mPrefObjectPtr;
   int                         mPurgeTime;
   bool                        mPurge;
};
} // namespace WkCyberEngagementBrowser
#endif
