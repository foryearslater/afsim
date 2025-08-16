// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "LogServerPrefObject.hpp"

#include <QSettings>

LogServer::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, cNAME)
{
}

void LogServer::PrefObject::EmitAllSignals()
{
   PrefData& pd = mCurrentPrefs;
   emit      PreferencesChanged(QVariant::fromValue(pd));
}

void LogServer::PrefObject::Apply()
{
   EmitAllSignals();
}

LogServer::PrefData LogServer::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData        prefs;
   const PrefData& defaults = mDefaultPrefs;
   prefs.mLogServerPort     = aSettings.value("logserverport", defaults.mLogServerPort).toInt();

   return prefs;
}

void LogServer::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("logserverport", mCurrentPrefs.mLogServerPort);
}
