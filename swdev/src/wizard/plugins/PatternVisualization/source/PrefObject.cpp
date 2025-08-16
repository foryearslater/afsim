// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PrefObject.hpp"

#include "Angle.hpp"

PatternVisualizer::PrefObject::PrefObject(QObject* parent)
   : wkf::PrefObjectT<PrefData>(parent, cNAME)
   , mSessionPtr(nullptr)
{
}

PatternVisualizer::PrefData PatternVisualizer::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   pData.mCameraZoom      = aSettings.value("cameraZoom", mDefaultPrefs.mCameraZoom).toFloat();
   pData.mCameraAzimuth   = aSettings.value("cameraAzimuth", mDefaultPrefs.mCameraAzimuth).toFloat();
   pData.mCameraElevation = aSettings.value("cameraElevation", mDefaultPrefs.mCameraElevation).toFloat();

   pData.mShowLocator = aSettings.value("showLocator", mDefaultPrefs.mShowLocator).toBool();

   pData.mPlotStyle        = aSettings.value("plotStyle", mDefaultPrefs.mPlotStyle).toInt();
   pData.mUseManualScaling = aSettings.value("useManualScaling", mDefaultPrefs.mUseManualScaling).toBool();
   pData.mMinimumScaling   = aSettings.value("minimumScaling", mDefaultPrefs.mMinimumScaling).toFloat();
   pData.mMaximumScaling   = aSettings.value("maximumScaling", mDefaultPrefs.mMaximumScaling).toFloat();
   pData.mScalingIncrement = aSettings.value("scalingIncrement", mDefaultPrefs.mScalingIncrement).toFloat();
   pData.mFrequency        = aSettings.value("frequency", mDefaultPrefs.mFrequency).toFloat();
   pData.mFrequencyUnits   = aSettings.value("frequencyUnits", mDefaultPrefs.mFrequencyUnits).toInt();

   pData.m2DZoom            = aSettings.value("2DZoom", mDefaultPrefs.m2DZoom).toFloat();
   pData.mConstantAzimuth   = aSettings.value("constantAzimuth", mDefaultPrefs.mConstantAzimuth).toFloat();
   pData.mConstantElevation = aSettings.value("constantElevation", mDefaultPrefs.mConstantElevation).toFloat();
   return pData;
}

void PatternVisualizer::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   if (mSessionPtr != nullptr)
   {
      aSettings.setValue("cameraZoom", mSessionPtr->GetCamera()->GetZoom());
      aSettings.setValue("cameraAzimuth", mSessionPtr->GetCamera()->GetAzimuth().GetRadians());
      aSettings.setValue("cameraElevation", mSessionPtr->GetCamera()->GetElevation().GetRadians());

      aSettings.setValue("showLocator", mSessionPtr->GetLocatorOptions()->GetShowLocator());

      aSettings.setValue("plotStyle", static_cast<int>(mSessionPtr->GetGlobalPlotOptions()->GetPlotStyle()));
      aSettings.setValue("useManualScaling", mSessionPtr->GetGlobalPlotOptions()->GetDataScale().IsManualScaling());
      aSettings.setValue("minimumScaling", mSessionPtr->GetGlobalPlotOptions()->GetDataScale().GetMinDB());
      aSettings.setValue("maximumScaling", mSessionPtr->GetGlobalPlotOptions()->GetDataScale().GetMaxDB());
      aSettings.setValue("scalingIncrement", mSessionPtr->GetGlobalPlotOptions()->GetDataScale().GetIncDB());
      aSettings.setValue("frequency", mSessionPtr->GetGlobalPlotOptions()->GetFrequency());
      aSettings.setValue("frequencyUnits", static_cast<int>(mSessionPtr->GetGlobalPlotOptions()->GetFrequencyUnits()));

      aSettings.setValue("2DZoom", mSessionPtr->GetPolarPlotOptions()->GetZoom2D());
      aSettings.setValue("constantAzimuth", mSessionPtr->GetPolarPlotOptions()->GetConstantAzimuth().GetRadians());
      aSettings.setValue("constantElevation", mSessionPtr->GetPolarPlotOptions()->GetConstantElevation().GetRadians());
   }
}

void PatternVisualizer::PrefObject::SetSession(Session* aSessionPtr)
{
   mSessionPtr = aSessionPtr;

   if (mSessionPtr != nullptr)
   {
      mSessionPtr->GetCamera()->SetZoom(mCurrentPrefs.mCameraZoom);
      mSessionPtr->GetCamera()->SetAzimuth(Angle::FromRadians(mCurrentPrefs.mCameraAzimuth));
      mSessionPtr->GetCamera()->SetElevation(Angle::FromRadians(mCurrentPrefs.mCameraElevation));

      mSessionPtr->GetLocatorOptions()->SetLocatorShown(mCurrentPrefs.mShowLocator);

      mSessionPtr->GetGlobalPlotOptions()->SetPlotStyle(static_cast<PatternVisualizer::PlotStyle>(mCurrentPrefs.mPlotStyle));
      mSessionPtr->GetGlobalPlotOptions()->SetScaleManual(mCurrentPrefs.mUseManualScaling);
      mSessionPtr->GetGlobalPlotOptions()->SetScaleMinDb(mCurrentPrefs.mMinimumScaling);
      mSessionPtr->GetGlobalPlotOptions()->SetScaleMaxDb(mCurrentPrefs.mMaximumScaling);
      mSessionPtr->GetGlobalPlotOptions()->SetScaleIncDb(mCurrentPrefs.mScalingIncrement);
      mSessionPtr->GetGlobalPlotOptions()->SetFrequency(mCurrentPrefs.mFrequency);
      mSessionPtr->GetGlobalPlotOptions()->SetFrequencyUnits(
         static_cast<PatternVisualizer::FrequencyUnits>(mCurrentPrefs.mFrequencyUnits));

      mSessionPtr->GetPolarPlotOptions()->SetZoom2D(mCurrentPrefs.m2DZoom);
      mSessionPtr->GetPolarPlotOptions()->SetConstantAzimuth(Angle::FromRadians(mCurrentPrefs.mConstantAzimuth));
      mSessionPtr->GetPolarPlotOptions()->SetConstantElevation(Angle::FromRadians(mCurrentPrefs.mConstantElevation));
   }
}
