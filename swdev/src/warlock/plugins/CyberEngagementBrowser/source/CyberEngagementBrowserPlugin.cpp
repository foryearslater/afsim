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

#include "CyberEngagementBrowserPlugin.hpp"

#include "CyberEngagementBrowserDockWidget.hpp"
#include "CyberEngagementBrowserPrefWidget.hpp"
#include "CyberEngagementBrowserSimInterface.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkCyberEngagementBrowser::Plugin,
                          "Cyber Engagement Browser",
                          "The Cyber Engagement Browser plugin provides a dialog, accessed through the View menu, that "
                          "displays perceived cyber scans and attacks.",
                          "warlock")

WkCyberEngagementBrowser::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
   , mPrefWidgetPtr(new WkCyberEngagementBrowser::PrefWidget)
   , mPrefObjectPtr(mPrefWidgetPtr->GetPreferenceObject())
   , mPurgeTime(0)
   , mPurge(false)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();
   mDockWidget                    = new DockWidget(GetInterface(), mCyberEngagementBrowserData, mainWindowPtr);
   connect(mPrefObjectPtr,
           &WkCyberEngagementBrowser::PrefObject::UpdatePurgeInfo,
           this,
           [this](int aPurgeTime, bool aPurge)
           {
              mPurgeTime = aPurgeTime;
              mPurge     = aPurge;
           });
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidget);
   mDockWidget->installEventFilter(this);
   mDockWidget->hide();
}

void WkCyberEngagementBrowser::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mCyberEngagementBrowserData);
   mDockWidget->Update(mPurge, mPurgeTime);
}

QList<wkf::PrefWidget*> WkCyberEngagementBrowser::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> list;
   list.push_back(mPrefWidgetPtr);
   return list;
}
