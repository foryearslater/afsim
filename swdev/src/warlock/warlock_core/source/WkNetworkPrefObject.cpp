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

#include "WkNetworkPrefObject.hpp"

warlock::net::PrefObject::PrefObject(QObject* aParent /*= nullptr*/)
   : wkf::PrefObjectT<PrefData>(aParent, "Network")
{
}

void warlock::net::PrefObject::Apply()
{
   if (mNetworkChanged)
   {
      emit NetworkChanged(mCurrentPrefs.mAddress, mCurrentPrefs.mInterface, mCurrentPrefs.mPort);
      mNetworkChanged = false;
   }
}

void warlock::net::PrefObject::SetPreferenceDataP(const PrefData& aPrefData)
{
   if (aPrefData.mAddress != mCurrentPrefs.mAddress || aPrefData.mInterface != mCurrentPrefs.mInterface ||
       aPrefData.mPort != mCurrentPrefs.mPort)
   {
      mNetworkChanged = true;
   }

   mCurrentPrefs = aPrefData;
}

warlock::net::PrefData warlock::net::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData retval;
   retval.mAddress   = aSettings.value("address", mDefaultPrefs.mAddress.toString()).toString();
   retval.mInterface = aSettings.value("interface", mDefaultPrefs.mInterface.toString()).toString();
   retval.mPort      = aSettings.value("port", mDefaultPrefs.mPort).toInt();
   return retval;
}

void warlock::net::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("address", mCurrentPrefs.mAddress.toString());
   aSettings.setValue("interface", mCurrentPrefs.mInterface.toString());
   aSettings.setValue("port", mCurrentPrefs.mPort);
}
