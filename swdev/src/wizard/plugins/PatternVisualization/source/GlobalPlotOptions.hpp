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

#ifndef GLOBALPLOTOPTIONS_HPP
#define GLOBALPLOTOPTIONS_HPP

#include <QObject>

#include "DataScale.hpp"

class QSettings;

namespace PatternVisualizer
{
class Session;

enum class PlotStyle
{
   PolarConstantAzimuth   = 0,
   PolarConstantElevation = 1,
   Generic3D              = 2
};

enum class FrequencyUnits
{
   cFREQUENCY_UNITS_HERTZ     = 0,
   cFREQUENCY_UNITS_KILOHERTZ = 1,
   cFREQUENCY_UNITS_MEGAHERTZ = 2,
   cFREQUENCY_UNITS_GIGAHERTZ = 3
};

class GlobalPlotOptions : public QObject
{
   Q_OBJECT
public:
   explicit GlobalPlotOptions(Session* aSessionPtr);
   ~GlobalPlotOptions() = default;

   void LoadSettings(QSettings& aSettings);
   void SaveSettings(QSettings& aSettings);

   PlotStyle GetPlotStyle() const { return mPlotStyle; }
   bool      IsPlot3D() const { return mPlotStyle == PlotStyle::Generic3D; }
   bool      IsPlot2D() const { return !IsPlot3D(); }

   const DataScale& GetDataScale() const { return mDataScale; }
   DataScale&       GetDataScale() { return mDataScale; }

   float          GetFrequency() const { return mFrequency; }
   float          GetFrequencyHz() const;
   FrequencyUnits GetFrequencyUnits() const { return mFrequencyUnits; }

public slots:
   void SetPlotStyle(PlotStyle aStyle);
   void SetScaleManual(bool aManual);
   void SetScaleMinDb(float aMinDb);
   void SetScaleMaxDb(float aMaxDb);
   void SetScaleIncDb(float aIncDb);
   void SetScale(float aMinDb, float aMaxDb, float aIncDb);
   void ResetScale();
   void SetFrequency(float aValue);
   void SetFrequencyUnits(FrequencyUnits aUnits);
   void ResetFrequencyOptions();

signals:
   void PlotStyleChanged(PlotStyle aStyle);
   void ManualScalingChanged(bool aIsManual);
   void ScaleChanged(float aMinDb, float aMaxDb, float aIncDb);
   void ScaleRangeIsInverted(bool aIsInverted);
   void FrequencyChanged(float aValue);
   void FrequencyUnitsChanged(FrequencyUnits aUnits);

private:
   float UnitFactor() const;
   void  EmitScaleSignals();

   Session*       mSessionPtr;
   PlotStyle      mPlotStyle;
   DataScale      mDataScale;
   float          mFrequency;
   FrequencyUnits mFrequencyUnits;
};
} // namespace PatternVisualizer

#endif // GLOBALPLOTOPTIONS_HPP
