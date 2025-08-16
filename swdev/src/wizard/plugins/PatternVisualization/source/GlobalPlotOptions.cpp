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

#include "GlobalPlotOptions.hpp"

#include <QSettings>

#include "Session.hpp"

namespace PatternVisualizer
{

namespace
{
const char* cSETTINGS_PLOT_STYLE     = "PlotStyle";
const char* cSETTINGS_MANUAL_SCALING = "ManualScaling";
const char* cSETTINGS_SCALE_MIN      = "ScaleMin";
const char* cSETTINGS_SCALE_MAX      = "ScaleMax";
const char* cSETTINGS_SCALE_INC      = "ScaleInc";
const char* cSETTINGS_FREQ_VALUE     = "FreqValue";
const char* cSETTINGS_FREQ_UNITS     = "FreqUnits";

const PlotStyle      cDEFAULT_PLOT_STYLE = PlotStyle::Generic3D;
const float          cDEFAULT_SCALE_MIN  = 0.0f;
const float          cDEFAULT_SCALE_MAX  = 20.0f;
const float          cDEFAULT_SCALE_INC  = 5.0f;
const float          cDEFAULT_FREQ       = 1000.0f;
const FrequencyUnits cDEFAULT_FREQ_UNITS = FrequencyUnits::cFREQUENCY_UNITS_HERTZ;
} // namespace

GlobalPlotOptions::GlobalPlotOptions(Session* aSessionPtr)
   : mSessionPtr{aSessionPtr}
   , mPlotStyle{cDEFAULT_PLOT_STYLE}
   , mFrequency{cDEFAULT_FREQ}
   , mFrequencyUnits{cDEFAULT_FREQ_UNITS}
{
}

void GlobalPlotOptions::LoadSettings(QSettings& aSettings)
{
   // Initialize the plot style
   PlotStyle style{cDEFAULT_PLOT_STYLE};
   if (aSettings.contains(cSETTINGS_PLOT_STYLE))
   {
      style = static_cast<PlotStyle>(aSettings.value(cSETTINGS_PLOT_STYLE).toInt());
   }
   mPlotStyle = style;
   emit PlotStyleChanged(mPlotStyle);

   // Initialize the manual scaling check box
   bool manual{false};
   if (aSettings.contains(cSETTINGS_MANUAL_SCALING))
   {
      manual = aSettings.value(cSETTINGS_MANUAL_SCALING).toBool();
   }
   mDataScale.SetManualScaling(manual);
   emit ManualScalingChanged(manual);

   // Initialize the manual scaling values, if applicable
   if (manual)
   {
      float minDB{cDEFAULT_SCALE_MIN};
      if (aSettings.contains(cSETTINGS_SCALE_MIN))
      {
         minDB = aSettings.value(cSETTINGS_SCALE_MIN).toFloat();
      }
      float maxDB{cDEFAULT_SCALE_MAX};
      if (aSettings.contains(cSETTINGS_SCALE_MAX))
      {
         maxDB = aSettings.value(cSETTINGS_SCALE_MAX).toFloat();
      }
      float incDB{cDEFAULT_SCALE_INC};
      if (aSettings.contains(cSETTINGS_SCALE_INC))
      {
         incDB = aSettings.value(cSETTINGS_SCALE_INC).toFloat();
      }
      if (minDB >= maxDB)
      {
         // User min/max dB values from last session are out of sequence, so use default min/max
         minDB = cDEFAULT_SCALE_MIN;
         maxDB = cDEFAULT_SCALE_MAX;
      }

      mDataScale.SetMinMaxDB(minDB, maxDB);
      mDataScale.SetIncDBManual(incDB);
      EmitScaleSignals();
   }

   // Initialize the frequency value and units
   float freq{cDEFAULT_FREQ};
   if (aSettings.contains(cSETTINGS_FREQ_VALUE))
   {
      freq = aSettings.value(cSETTINGS_FREQ_VALUE).toFloat();
   }
   FrequencyUnits units{cDEFAULT_FREQ_UNITS};
   if (aSettings.contains(cSETTINGS_FREQ_UNITS))
   {
      units = static_cast<FrequencyUnits>(aSettings.value(cSETTINGS_FREQ_UNITS).toInt());
   }
   mFrequency = freq;
   emit FrequencyChanged(mFrequency);
   mFrequencyUnits = units;
   emit FrequencyUnitsChanged(mFrequencyUnits);
}

void GlobalPlotOptions::SaveSettings(QSettings& aSettings)
{
   // Save the plot style setting
   aSettings.setValue(cSETTINGS_PLOT_STYLE, static_cast<int>(mPlotStyle));

   // Save the manual scaling settings
   aSettings.setValue(cSETTINGS_MANUAL_SCALING, mDataScale.IsManualScaling());
   aSettings.setValue(cSETTINGS_SCALE_MIN, mDataScale.GetMinDB());
   aSettings.setValue(cSETTINGS_SCALE_MAX, mDataScale.GetMaxDB());
   aSettings.setValue(cSETTINGS_SCALE_INC, mDataScale.GetIncDB());

   // Save the frequency value and units
   aSettings.setValue(cSETTINGS_FREQ_VALUE, mFrequency);
   aSettings.setValue(cSETTINGS_FREQ_UNITS, static_cast<int>(mFrequencyUnits));
}

float GlobalPlotOptions::GetFrequencyHz() const
{
   float retval{mFrequency};
   return retval * UnitFactor();
}

void GlobalPlotOptions::SetPlotStyle(PlotStyle aStyle)
{
   if (aStyle != mPlotStyle)
   {
      mPlotStyle = aStyle;
      emit PlotStyleChanged(mPlotStyle);
   }
}

void GlobalPlotOptions::SetScaleManual(bool aManual)
{
   mDataScale.SetManualScaling(aManual);
   emit ManualScalingChanged(aManual);
}

void GlobalPlotOptions::SetScaleMinDb(float aMinDb)
{
   if (mDataScale.SetMinDBManual(aMinDb))
   {
      EmitScaleSignals();
   }
}

void GlobalPlotOptions::SetScaleMaxDb(float aMaxDb)
{
   if (mDataScale.SetMaxDBManual(aMaxDb))
   {
      EmitScaleSignals();
   }
}

void GlobalPlotOptions::SetScaleIncDb(float aIncDb)
{
   if (mDataScale.SetIncDBManual(aIncDb))
   {
      EmitScaleSignals();
   }
}

void GlobalPlotOptions::SetScale(float aMinDb, float aMaxDb, float aIncDb)
{
   if (aMinDb != mDataScale.GetMinDB() || aMaxDb != mDataScale.GetMaxDB() || aIncDb != mDataScale.GetIncDB())
   {
      mDataScale.SetMinMaxDB(aMinDb, aMaxDb);
      mDataScale.SetIncDBManual(aIncDb);
      EmitScaleSignals();
   }
}

void GlobalPlotOptions::ResetScale()
{
   mDataScale.SetResettingScale(true);
   mSessionPtr->UpdateScaleDBRange();
   mDataScale.SetResettingScale(false);
}

void GlobalPlotOptions::SetFrequency(float aValue)
{
   if (aValue != mFrequency)
   {
      mFrequency = aValue;
      emit FrequencyChanged(mFrequency);
   }
}

void GlobalPlotOptions::SetFrequencyUnits(FrequencyUnits aUnits)
{
   if (aUnits != mFrequencyUnits)
   {
      mFrequencyUnits = aUnits;
      emit FrequencyUnitsChanged(mFrequencyUnits);
   }
}

void GlobalPlotOptions::ResetFrequencyOptions()
{
   SetFrequency(cDEFAULT_FREQ);
   SetFrequencyUnits(cDEFAULT_FREQ_UNITS);
}

float GlobalPlotOptions::UnitFactor() const
{
   float retval{1.0};
   switch (mFrequencyUnits)
   {
   case FrequencyUnits::cFREQUENCY_UNITS_HERTZ:
      retval = 1.0;
      break;
   case FrequencyUnits::cFREQUENCY_UNITS_KILOHERTZ:
      retval = 1.0e3;
      break;
   case FrequencyUnits::cFREQUENCY_UNITS_MEGAHERTZ:
      retval = 1.0e6;
      break;
   case FrequencyUnits::cFREQUENCY_UNITS_GIGAHERTZ:
      retval = 1.0e9;
      break;
   }
   return retval;
}

void GlobalPlotOptions::EmitScaleSignals()
{
   emit ScaleRangeIsInverted(mDataScale.GetMaxDB() <= mDataScale.GetMinDB());
   emit ScaleChanged(mDataScale.GetMinDB(), mDataScale.GetMaxDB(), mDataScale.GetIncDB());
}
} // namespace PatternVisualizer
