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
#include "ChatPrefObject.hpp"

#include <QNetworkInterface>
#include <QSettings>

#include "UtQtSettingsScopes.hpp"

Chat::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, "Chat")
{
}

void Chat::PrefObject::Apply()
{
   emit UserNameChanged(mCurrentPrefs.mUserName);
   emit TimeStampsChanged(mCurrentPrefs.mWallTimes, mCurrentPrefs.mSimTimes);
}

Chat::PrefData Chat::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData        prefs;
   const PrefData& defaults = mDefaultPrefs;

   prefs.mUserName  = aSettings.value("UserName", defaults.mUserName).toString();
   prefs.mWallTimes = aSettings.value("WallTimes", defaults.mWallTimes).toBool();
   prefs.mSimTimes  = aSettings.value("SimTimes", defaults.mSimTimes).toBool();

   return prefs;
}

void Chat::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("UserName", mCurrentPrefs.mUserName);
   aSettings.setValue("WallTimes", mCurrentPrefs.mWallTimes);
   aSettings.setValue("SimTimes", mCurrentPrefs.mSimTimes);
}
