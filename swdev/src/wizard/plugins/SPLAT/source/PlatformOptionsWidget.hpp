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

#ifndef PLATFORMOPTIONSWIDGET_HPP
#define PLATFORMOPTIONSWIDGET_HPP

#include <QValidator>

#include "PlotOptionsWidget.hpp"
#include "UtUnitTypes.hpp"
#include "ui_PlatformOptionsWidget.h"

namespace SPLAT
{
class PlatformTypeValidator;

class PlatformOptionsWidget : public PlotOptionsWidget
{
   Q_OBJECT
public:
   struct TargetOptions
   {
      UtLengthValue mAltitude;
      UtSpeedValue  mSpeed;
      UtAngleValue  mHeading;
      UtAngleValue  mYaw;
      UtAngleValue  mPitch;
      UtAngleValue  mRoll;

      TargetOptions()
         : mAltitude(UtLengthValue(25000.0, UtUnitLength::cFEET))
         , mSpeed(UtSpeedValue(250, UtUnitSpeed::cMETERS_PER_SECOND))
         , mHeading(UtAngleValue(0.0, UtUnitAngle::cDEGREES))
         , mYaw(UtAngleValue(0.0, UtUnitAngle::cDEGREES))
         , mPitch(UtAngleValue(0.0, UtUnitAngle::cDEGREES))
         , mRoll(UtAngleValue(0.0, UtUnitAngle::cDEGREES))
      {
      }
   };

   struct SensorControl
   {
      QString       mModeName;
      QString       mPlatformToDetect;
      UtLengthValue mAltitude{0.0, UtUnitLength::cFEET};
      UtAngleValue  mYaw{0.0, UtUnitAngle::cDEGREES};
      UtAngleValue  mPitch{0.0, UtUnitAngle::cDEGREES};
      UtAngleValue  mRoll{0.0, UtUnitAngle::cDEGREES};
      double        mRequiredPD{0.5};
      bool          mAutoCue{true};
   };

   struct JammerControl
   {
      QString      mPlatformName;
      UtRatioValue mSigRef; // Jammer to Signal Reference

      JammerControl()
         : mPlatformName("")
         , mSigRef(UtRatioValue(0.0, UtUnitRatio::cDB))
      {
      }
   };


   PlatformOptionsWidget(PlotType aPlotType, PlotOptionsDialog* aParent);
   ~PlatformOptionsWidget() override;

   void WriteData(std::ostream& aStream) override;
   bool ApplyChanges() override;
   void RestoreDefaults() override;

   void RequiredJammingPowerPressed(bool aChecked);
   void UpdateModes(const QStringList& aModeNames);
   void UpdatePlatformTypes(const QStringList& aPlatformTypes);
   void CheckValidity();

private:
   void Initialize();
   void Populate();
   void Connect();
   void SetPlatformOptions();
   bool PlatformOptionsValid();
   void TextChangedHandler();

   Ui::PlatformOptionsWidget mUi;

   PlatformTypeValidator* mTargetTypeValidator;

   PlotType mPlotType;

   QStringList   mModeNames;
   TargetOptions mTargetOptions;
   SensorControl mSensorControl;
   JammerControl mJammerControl;
};


class PlatformTypeValidator : public QValidator
{
   Q_OBJECT
public:
   PlatformTypeValidator(QObject* aParent = nullptr)
      : QValidator(aParent)
      , mList(QStringList())
      , mState(QValidator::Acceptable)
   {
   }

   void setValidList(const QStringList& aList) { mList = aList; }

   QValidator::State validate(QString& s, int& i) const override
   {
      QValidator::State valid = QValidator::Acceptable;
      if (!s.compare("TARGET_PLATFORM_TYPE") || (!s.isEmpty() && !mList.contains(s)))
      {
         valid = QValidator::Intermediate;
      }
      return valid;
   }

private:
   QStringList       mList;
   QValidator::State mState;
};
} // namespace SPLAT
#endif
