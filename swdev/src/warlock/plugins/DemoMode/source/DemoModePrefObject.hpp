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
#ifndef WKDEMOMODEPREFOBJECT_HPP
#define WKDEMOMODEPREFOBJECT_HPP

class QSettings;
#include "WkfPrefObject.hpp"

namespace WkDemoMode
{
struct PrefData
{
   bool        mEnabled{false};
   double      mPauseLength_sec{0.0};
   double      mRunSpeedMultiplier{1.0};
   bool        mSelectPlatform{false};
   std::string mSelectPlatformName{};
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   PrefObject(QObject* parent = nullptr);

   bool GetEnabled() const { return mCurrentPrefs.mEnabled; }
   void SetEnabled(bool aEnabled) { mCurrentPrefs.mEnabled = aEnabled; }

   double GetPauseLength() const { return mCurrentPrefs.mPauseLength_sec; }
   void   SetPauseLength(double aSec) { mCurrentPrefs.mPauseLength_sec = aSec; }

   double GetRunSpeedMultiplier() const { return mCurrentPrefs.mRunSpeedMultiplier; }
   void   SetRunSpeedMultiplier(double aMultiplier) { mCurrentPrefs.mRunSpeedMultiplier = aMultiplier; }

   bool        GetSelectPlatform() const { return mCurrentPrefs.mSelectPlatform; }
   std::string GetSelectedPlatformName() const { return mCurrentPrefs.mSelectPlatformName; }
   void        SetSelectPlatform(const std::string& aPlatformName, bool aSelect);

signals:
   void EnabledChanged(bool aEnabled);
   void RunSpeedMultiplierChanged(double aMultiplier);

private:
   void Apply() override;

   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;
};
} // namespace WkDemoMode

Q_DECLARE_METATYPE(WkDemoMode::PrefData)
#endif
