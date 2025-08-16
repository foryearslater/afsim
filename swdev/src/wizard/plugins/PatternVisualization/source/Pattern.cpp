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

#include "Pattern.hpp"

#include <algorithm>
#include <array>

#include <QDir>

#include "MaterialColors.hpp"
#include "PatternUpdateManager.hpp"
#include "Session.hpp"
#include "UtMemory.hpp"
#include "UtSleep.hpp"
#include "WsfMilRadarSignature.hpp"
#include "WsfRadarSignatureTypes.hpp"
#include "WsfStandardAcousticSignature.hpp"
#include "WsfStandardInfraredSignature.hpp"


namespace PatternVisualizer
{
// static
PatternUpdateManager* Pattern::sPatternUpdateManager = nullptr;


Pattern::Pattern(const QString& aFileName, Session* aSessionPtr, const QString& aName, Pattern::Type aPatternType)
   : QObject(aSessionPtr)
   , mPatternType(aPatternType)
   , mFileName(aFileName)
   , mName(aName)
{
   if (sPatternUpdateManager == nullptr)
   {
      Pattern::sPatternUpdateManager = aSessionPtr->GetPatternUpdateManager();
   }
}

//##################################################################################################

AntPattern::AntPattern(const QString& aFileName, WsfAntennaPattern& aWsfPattern, Session* aSessionPtr, const QString& aPatternName)
   : Pattern(aFileName, aSessionPtr, aPatternName, Pattern::Type::ANTENNA_PAT)
   , mWsfPattern(aWsfPattern)
{
}

bool AntPattern::UpdatePatternData(double aFreq)
{
   if (mPatternData.get() == nullptr)
   {
      mPatternData = ut::make_unique<PatternData>(mWsfPattern, aFreq);
      return true;
   }
   else
   {
      return mPatternData->UpdateGainTable(mWsfPattern, aFreq);
   }
}

//##################################################################################################

// static
QStringList RadarSigPattern::sPolLabels = QStringList() << "default"
                                                        << "horizontal"
                                                        << "vertical"
                                                        << "slant 45"
                                                        << "slant 135"
                                                        << "left circular"
                                                        << "right circular";

RadarSigPattern::RadarSigPattern(const QString&            aFileName,
                                 WsfRadarSignature&        aWsfPattern,
                                 Session*                  aSessionPtr,
                                 const QString&            aSigName,
                                 const QString&            aState,
                                 WsfEM_Types::Polarization aPolarization)
   : Pattern(aFileName, aSessionPtr, aSigName, Pattern::Type::RADAR_SIG)
   , mWsfPattern(aWsfPattern)
   , mState(aState.toStdString().c_str())
   , mPolarization(aPolarization)
{
   mName += "/" + aState + "/" + QString(sPolLabels[mPolarization]).replace(" ", "");
}

bool RadarSigPattern::UpdatePatternData(double aFreq)
{
   if (mPatternData.get() == nullptr)
   {
      mPatternData = ut::make_unique<PatternData>(mWsfPattern, mState, mPolarization, aFreq);
      return true;
   }
   else
   {
      return mPatternData->UpdateGainTable(mWsfPattern, aFreq);
   }
}

//##################################################################################################

OpticalSigPattern::OpticalSigPattern(const QString&       aFileName,
                                     WsfOpticalSignature& aWsfPattern,
                                     Session*             aSessionPtr,
                                     const QString&       aSigName,
                                     const QString&       aState)
   : Pattern(aFileName, aSessionPtr, aSigName, Pattern::Type::OPTICAL_SIG)
   , mWsfPattern(aWsfPattern)
   , mState(aState.toStdString().c_str())
{
   mName += "/" + aState;
}

bool OpticalSigPattern::UpdatePatternData(double aFreq)
{
   if (mPatternData.get() == nullptr)
   {
      mPatternData = ut::make_unique<PatternData>(mWsfPattern, mState);
      return true;
   }
   else
   {
      // Nothing to do since optical patterns are not frequency dependent
      return false;
   }
}

//##################################################################################################

InfraredSigPattern::InfraredSigPattern(const QString&        aFileName,
                                       WsfInfraredSignature& aWsfPattern,
                                       Session*              aSessionPtr,
                                       const QString&        aSigName,
                                       const QString&        aState)
   : Pattern(aFileName, aSessionPtr, aSigName, Pattern::Type::INFRARED_SIG)
   , mWsfPattern(aWsfPattern)
   , mState(aState.toStdString().c_str())
{
   mName += "/" + aState;
}

// static
WsfEM_Types::InfraredBand InfraredSigPattern::GetBand(double aFreq)
{
   static const double vlight     = 299792458.0; // m/s : speed of light
   double              wavelength = vlight / aFreq;
   if (wavelength < 1.0e-6)
   {
      return WsfEM_Types::InfraredBand::cIRBAND_DEFAULT;
   }
   else if (wavelength < 3.0e-6)
   {
      return WsfEM_Types::InfraredBand::cIRBAND_SHORT;
   }
   else if (wavelength < 5.0e-6)
   {
      return WsfEM_Types::InfraredBand::cIRBAND_MEDIUM;
   }
   else if (wavelength < 8.0e-6)
   {
      return WsfEM_Types::InfraredBand::cIRBAND_DEFAULT;
   }
   else if (wavelength < 14.0e-6)
   {
      return WsfEM_Types::InfraredBand::cIRBAND_LONG;
   }
   else if (wavelength < 30.0e-6)
   {
      return WsfEM_Types::InfraredBand::cIRBAND_VERY_LONG;
   }
   else
   {
      return WsfEM_Types::InfraredBand::cIRBAND_DEFAULT;
   }
}

bool InfraredSigPattern::UpdatePatternData(double aFreq)
{
   if (mPatternData.get() == nullptr)
   {
      mPatternData = ut::make_unique<PatternData>(mWsfPattern, mState, GetBand(aFreq));
      return true;
   }
   else
   {
      return mPatternData->UpdateGainTable(mWsfPattern, GetBand(aFreq));
   }
}

//##################################################################################################

AcousticSigPattern::AcousticSigPattern(const QString&        aFileName,
                                       WsfAcousticSignature& aWsfPattern,
                                       Session*              aSessionPtr,
                                       const QString&        aSigName,
                                       const QString&        aState)
   : Pattern(aFileName, aSessionPtr, aSigName, Pattern::Type::ACOUSTIC_SIG)
   , mWsfPattern(aWsfPattern)
   , mState(aState.toStdString().c_str())
{
   mName += "/" + aState;
}

bool AcousticSigPattern::UpdatePatternData(double aFreq)
{
   if (mPatternData.get() == nullptr)
   {
      mPatternData = ut::make_unique<PatternData>(mWsfPattern, mState, aFreq);
      return true;
   }
   else
   {
      return mPatternData->UpdateGainTable(mWsfPattern, aFreq);
   }
}
} // namespace PatternVisualizer
