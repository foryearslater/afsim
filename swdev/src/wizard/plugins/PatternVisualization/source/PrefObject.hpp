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

#ifndef PREFOBJECT_HPP
#define PREFOBJECT_HPP

#include "GlobalPlotOptions.hpp"
#include "Session.hpp"
#include "WkfPrefObject.hpp"

namespace PatternVisualizer
{
struct PrefData
{
   bool   mShowLocator{false};
   bool   mUseManualScaling{false};
   float  mCameraZoom{1.0f};
   float  mCameraAzimuth{45.0f};
   float  mCameraElevation{20.0f};
   float  mMinimumScaling{0.0f};
   float  mMaximumScaling{20.0f};
   float  mScalingIncrement{5.0f};
   float  mFrequency{1000.0f};
   float  m2DZoom{1.0f};
   double mConstantAzimuth{0.0};
   double mConstantElevation{0.0};
   int    mFrequencyUnits{0};
   int    mPlotStyle{2};
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
public:
   static constexpr const char* cNAME = "PatternVisualizerSettings";

   explicit PrefObject(QObject* parent = nullptr);
   ~PrefObject() override = default;

   void     Apply() override{};
   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;

   void SetSession(Session* aSessionPtr);

private:
   Session* mSessionPtr;
};
} // namespace PatternVisualizer

Q_DECLARE_METATYPE(PatternVisualizer::PrefData)

#endif // !PREFOBJECT_HPP
