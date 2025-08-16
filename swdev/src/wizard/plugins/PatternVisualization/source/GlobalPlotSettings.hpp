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

#ifndef GLOBALPLOTSETTINGS_HPP
#define GLOBALPLOTSETTINGS_HPP

#include <QSettings>

#include "GlobalPlotOptions.hpp"
#include "ui_GlobalPlotSettings.h"

namespace PatternVisualizer
{

enum class PlotStyle;
class Session;

class GlobalPlotSettings : public QWidget, public Ui::GlobalPlotSettings
{
   Q_OBJECT

public:
   explicit GlobalPlotSettings(QWidget* aParentPtr = nullptr);
   ~GlobalPlotSettings() override = default;

   void FinalizeUi(Session* aSessionPtr, GlobalPlotOptions* aGlobalPlotOptionsPtr);

   // Reimplemented methods
   bool eventFilter(QObject* aObjPtr, QEvent* aEventPtr) override;

private slots:
   void on_frequencyUnits_currentIndexChanged(int aValue);
   void on_manualScalingCheckBox_clicked(bool aValue);
   void on_plotStyleComboBox_currentIndexChanged(int aValue);
   void UpdatePlotStyle(PlotStyle aPlotStyle);
   void UpdateScale(float aMinDB, float aMaxDB, float aIncDB);
   void UpdateScaleRangeWarning(bool aIsInverted);
   void UpdateFrequency(float aValue);
   void UpdateFrequencyUnits(FrequencyUnits aUnits);

private:
   void ChangeScaleMinDb(double aValue);
   void ChangeScaleMaxDb(double aValue);
   void ChangeScaleIncDb(double aValue);
   void ChangeFrequency(double aValue);

   Session*           mSessionPtr;
   GlobalPlotOptions* mGlobalPlotOptionsPtr;
   bool               mInternalChange{false};
};
} // namespace PatternVisualizer

#endif
