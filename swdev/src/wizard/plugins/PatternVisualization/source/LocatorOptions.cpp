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

#include "LocatorOptions.hpp"

#include <QSettings>

#include "Session.hpp"

namespace PatternVisualizer
{
namespace
{
const char* cSETTINGS_SHOW_LOCATOR = "ShowLocator";
}

LocatorOptions::LocatorOptions(const Session* aSessionPtr)
   : mAzimuth{0.0_deg}
   , mElevation{0.0_deg}
   , mSessionPtr{aSessionPtr}
   , mLocatorShown{false}
{
}

void LocatorOptions::LoadSettings(QSettings& aSettings)
{
   bool showLocator{false};
   if (aSettings.contains(cSETTINGS_SHOW_LOCATOR))
   {
      showLocator = aSettings.value(cSETTINGS_SHOW_LOCATOR).toBool();
   }
   SetLocatorShown(showLocator);
}

void LocatorOptions::SaveSettings(QSettings& aSettings)
{
   aSettings.setValue(cSETTINGS_SHOW_LOCATOR, mLocatorShown);
}

void LocatorOptions::SetAzEl(Angle aAzimuth, Angle aElevation)
{
   if (mAzimuth != aAzimuth || mElevation != aElevation)
   {
      mAzimuth   = aAzimuth;
      mElevation = aElevation;
      emit AzElChanged(mAzimuth, mElevation);
      RecomputeDockValue();
   }
}

void LocatorOptions::RecomputeDockValue()
{
   bool  valid{false};
   float value{-1.0f};
   if (mLocatorShown)
   {
      value = mSessionPtr->GetCurrentValueAt(mAzimuth, mElevation, valid);
   }
   emit UpdateDockValue(value, valid);
}

void LocatorOptions::SetLocatorShown(bool aShown)
{
   mLocatorShown = aShown;
   emit ShowLocatorChanged(mLocatorShown);
   RecomputeDockValue();
}
} // namespace PatternVisualizer
