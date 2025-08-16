// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WkSimPrefObject.hpp"

#include "WkSimEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

warlock::SimPrefObject::SimPrefObject(QObject* parent)
   : wkf::PrefObjectT<SimPrefData>(parent, cNAME)
{
}

warlock::SimPrefData warlock::SimPrefObject::ReadSettings(QSettings& aSettings) const
{
   SimPrefData pData;
   pData.startPaused          = aSettings.value("startPaused", mDefaultPrefs.startPaused).toBool();
   pData.platformsDraggable   = aSettings.value("platformsDraggable", mDefaultPrefs.platformsDraggable).toBool();
   pData.enableDIS            = aSettings.value("enableDis", mDefaultPrefs.enableDIS).toBool();
   pData.multicastIp          = aSettings.value("multicastIp", mDefaultPrefs.multicastIp).toString();
   pData.netId                = aSettings.value("netId", mDefaultPrefs.netId).toString();
   pData.port                 = aSettings.value("port", mDefaultPrefs.port).toInt();
   pData.siteId               = aSettings.value("siteId", mDefaultPrefs.siteId).toInt();
   pData.applicationId        = aSettings.value("applicationId", mDefaultPrefs.applicationId).toInt();
   pData.exerciseId           = aSettings.value("exerciseId", mDefaultPrefs.exerciseId).toInt();
   pData.enabledDeferredStart = aSettings.value("deferredStart", mDefaultPrefs.enabledDeferredStart).toBool();
   pData.deferredStartTimeSec = aSettings.value("deferredTime", mDefaultPrefs.deferredStartTimeSec).toDouble();
   pData.clockRate            = aSettings.value("clockRate", mDefaultPrefs.clockRate).toDouble();
   return pData;
}

void warlock::SimPrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("startPaused", mCurrentPrefs.startPaused);
   aSettings.setValue("platformsDraggable", mCurrentPrefs.platformsDraggable);
   aSettings.setValue("enableDis", mCurrentPrefs.enableDIS);
   aSettings.setValue("multicastIp", mCurrentPrefs.multicastIp);
   aSettings.setValue("netId", mCurrentPrefs.netId);
   aSettings.setValue("port", mCurrentPrefs.port);
   aSettings.setValue("siteId", mCurrentPrefs.siteId);
   aSettings.setValue("applicationId", mCurrentPrefs.applicationId);
   aSettings.setValue("exerciseId", mCurrentPrefs.exerciseId);
   aSettings.setValue("deferredStart", mCurrentPrefs.enabledDeferredStart);
   aSettings.setValue("deferredTime", mCurrentPrefs.deferredStartTimeSec);
   aSettings.setValue("clockRate", mCurrentPrefs.clockRate);
}

void warlock::SimPrefObject::Apply()
{
   vaEnv.EnablePlatformsDraggable(mCurrentPrefs.platformsDraggable);

   simEnv.SetSimClockInterval(1.0 / mCurrentPrefs.clockRate);
   simEnv.SetWallClockInterval(1.0 / mCurrentPrefs.clockRate);
}
