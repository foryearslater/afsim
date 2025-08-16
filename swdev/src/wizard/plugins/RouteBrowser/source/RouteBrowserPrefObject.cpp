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
#include "RouteBrowserPrefObject.hpp"

#include <QSettings>

RouteBrowser::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, cNAME)
{
}

void RouteBrowser::PrefObject::EmitAllSignals()
{
   PrefData& pd = mCurrentPrefs;
   emit      PreferencesChanged(QVariant::fromValue(pd));
}

void RouteBrowser::PrefObject::Apply()
{
   EmitAllSignals();
}

RouteBrowser::PrefData RouteBrowser::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData        prefs;
   const PrefData& defaults = mDefaultPrefs;

   prefs.mShowGlobalRoutes        = aSettings.value("showglobalroutes", defaults.mShowGlobalRoutes).toBool();
   prefs.mUseDefaultRouteName     = aSettings.value("usedefaultroutenames", defaults.mUseDefaultRouteName).toBool();
   prefs.mUseDefaultWaypointLabel = aSettings.value("usedefaultwaypointlabel", defaults.mUseDefaultWaypointLabel).toBool();

   prefs.mDefaultRouteName     = aSettings.value("defaultroutename", defaults.mDefaultRouteName).toString();
   prefs.mDefaultWaypointLabel = aSettings.value("defaultwaypointlabel", defaults.mDefaultWaypointLabel).toString();

   return prefs;
}

void RouteBrowser::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("showglobalroutes", mCurrentPrefs.mShowGlobalRoutes);
   aSettings.setValue("usedefaultroutenames", mCurrentPrefs.mUseDefaultRouteName);
   aSettings.setValue("usedefaultwaypointlabel", mCurrentPrefs.mUseDefaultWaypointLabel);
   aSettings.setValue("defaultroutename", mCurrentPrefs.mDefaultRouteName);
   aSettings.setValue("defaultwaypointlabel", mCurrentPrefs.mDefaultWaypointLabel);
}
