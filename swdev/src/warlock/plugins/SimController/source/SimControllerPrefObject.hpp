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
#ifndef WKSIMCONTROLLERPREFOBJECT_HPP
#define WKSIMCONTROLLERPREFOBJECT_HPP

class QSettings;
#include "WkfPrefObject.hpp"

namespace WkSimController
{
struct PrefData
{
   bool mShowClockRate{true};
   bool mShowProgressBar{true};
   bool mShowSimMode{true};
   bool mShowSimState{true};
   bool mShowSimTime{true};
   bool mShowTimeBehind{true};
   bool mShowWallTime{true};

   bool   mSendDIS_PDUs{true};
   bool   mAutoPause{true};
   double mFallingBehindThreshold{0.5};

   int          mUTC_OffsetHrs{0};
   int          mUTC_OffsetMins{0};
   Qt::TimeSpec mWallTimeSpec{Qt::UTC};
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   PrefObject(QObject* parent = nullptr);

   bool GetShowClockRate() const { return mCurrentPrefs.mShowClockRate; }
   void SetShowClockRate(bool aShow);

   bool GetShowProgressBar() const { return mCurrentPrefs.mShowProgressBar; }
   void SetShowProgressBar(bool aShow);

   bool GetShowSimMode() const { return mCurrentPrefs.mShowSimMode; };
   void SetShowSimMode(bool aShow);

   bool GetShowSimState() const { return mCurrentPrefs.mShowSimState; }
   void SetShowSimState(bool aShow);

   bool GetShowSimTime() const { return mCurrentPrefs.mShowSimTime; }
   void SetShowSimTime(bool aShow);

   bool GetShowTimeBehind() const { return mCurrentPrefs.mShowTimeBehind; }
   void SetShowTimeBehind(bool aShow);

   bool GetShowWallTime() const { return mCurrentPrefs.mShowWallTime; }
   void SetShowWallTime(bool aShow);

   bool GetSendDIS_PDUs() const { return mCurrentPrefs.mSendDIS_PDUs; }
   void SetSendDIS_PDUs(bool aSend);

   bool   GetSendAutoPause() const { return mCurrentPrefs.mAutoPause; }
   double GetFallingBehindThreshold() const { return mCurrentPrefs.mFallingBehindThreshold; }

   int  GetUTC_OffsetHrs() const { return mCurrentPrefs.mUTC_OffsetHrs; }
   void SetUTC_OffsetHrs(int aHrs);

   int  GetUTC_OffsetMins() const { return mCurrentPrefs.mUTC_OffsetMins; }
   void SetUTC_OffsetMins(int aMins);

   Qt::TimeSpec GetWallTimeSpec() const { return mCurrentPrefs.mWallTimeSpec; }
   void         SetWallTimeSpec(Qt::TimeSpec aTimeSpec);

signals:
   void ShowClockRateChanged(bool aShow);
   void ShowProgressBarChanged(bool aShow);
   void ShowSimModeChanged(bool aShow);
   void ShowSimStateChanged(bool aShow);
   void ShowSimTimeChanged(bool aShow);
   void ShowTimeBehindChanged(bool aShow);
   void ShowWallTimeChanged(bool aShow);
   void SendDIS_PDUsChanged(bool aSend);
   void WallTimeSpecChanged();
   void FallingBehindThresholdChanged(double aThreshold);

private:
   void EmitAllSignals();

   void     Apply() override;
   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;
};
} // namespace WkSimController

Q_DECLARE_METATYPE(WkSimController::PrefData)
#endif // SIMCONTORLLERPREFOBJECT_HPP
