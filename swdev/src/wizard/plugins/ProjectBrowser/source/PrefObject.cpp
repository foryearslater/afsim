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

#include "PrefObject.hpp"

ProjectBrowser::PrefObject::PrefObject(QObject* parent /*= nullptr*/)
   : wkf::PrefObjectT<PrefData>(parent, cNAME)
{
}

void ProjectBrowser::PrefObject::Apply()
{
   emit Changed();
}

ProjectBrowser::PrefData ProjectBrowser::PrefObject::ReadSettings(QSettings& aSettings) const
{
   // someday maybe we will expose this to the user
   PrefData pData;
   pData.mHiddenFileExtensions = mDefaultPrefs.mHiddenFileExtensions;
   return pData;
}

void ProjectBrowser::PrefObject::SaveSettingsP(QSettings& aSettings) const {}

QSet<QString> ProjectBrowser::PrefObject::GetHiddenFileExtensions() const
{
   return mCurrentPrefs.mHiddenFileExtensions;
}
