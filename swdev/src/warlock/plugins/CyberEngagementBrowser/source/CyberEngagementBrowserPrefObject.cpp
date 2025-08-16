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
#include "CyberEngagementBrowserPrefObject.hpp"

#include <QSettings>

WkCyberEngagementBrowser::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, "CyberEngagementBrowserOptions")
{
}

void WkCyberEngagementBrowser::PrefObject::Apply()
{
   emit UpdatePurgeInfo(mCurrentPrefs.mPurgeTime, mCurrentPrefs.mPurge);
}

WkCyberEngagementBrowser::PrefData WkCyberEngagementBrowser::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   pData.mPurgeTime = aSettings.value("purgeTime", mDefaultPrefs.mPurgeTime).toInt();
   pData.mPurge     = aSettings.value("purge", mDefaultPrefs.mPurge).toBool();
   return pData;
}

void WkCyberEngagementBrowser::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("purgeTime", mCurrentPrefs.mPurgeTime);
   aSettings.setValue("purge", mCurrentPrefs.mPurge);
}
