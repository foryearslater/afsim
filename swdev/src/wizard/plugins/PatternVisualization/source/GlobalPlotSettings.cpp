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

#include "GlobalPlotSettings.hpp"

#include "GlobalPlotOptions.hpp"
#include "Session.hpp"
#include "SpinBoxEventFilter.hpp"

namespace PatternVisualizer
{
namespace
{
const QString cRANGE_INVERTED_MESSAGE{"Error: Range must have min < max."};
}

GlobalPlotSettings::GlobalPlotSettings(QWidget* aParentPtr)
   : QWidget{aParentPtr}
   , mSessionPtr{nullptr}
{
   setupUi(this);
   errorMsg->setText("");

   // Install the event filter to process user inputs
   frequencySpinBox->installEventFilter(this);
   dbMinSpinBox->installEventFilter(this);
   dbMaxSpinBox->installEventFilter(this);
   dbIncSpinBox->installEventFilter(this);

   scaleResetButton->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
   frequencyResetButton->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
}

void GlobalPlotSettings::FinalizeUi(Session* aSessionPtr, GlobalPlotOptions* aGlobalPlotOptionsPtr)
{
   mSessionPtr = aSessionPtr;

   mGlobalPlotOptionsPtr = aGlobalPlotOptionsPtr;

   connect(mGlobalPlotOptionsPtr, &GlobalPlotOptions::PlotStyleChanged, this, &GlobalPlotSettings::UpdatePlotStyle);

   connect(scaleResetButton, &QPushButton::clicked, mGlobalPlotOptionsPtr, &GlobalPlotOptions::ResetScale);
   connect(manualScalingCheckBox, &QCheckBox::clicked, mGlobalPlotOptionsPtr, &GlobalPlotOptions::SetScaleManual);
   connect(mGlobalPlotOptionsPtr,
           &GlobalPlotOptions::ManualScalingChanged,
           this,
           &GlobalPlotSettings::on_manualScalingCheckBox_clicked);
   connect(mGlobalPlotOptionsPtr, &GlobalPlotOptions::ScaleChanged, this, &GlobalPlotSettings::UpdateScale);
   connect(mGlobalPlotOptionsPtr, &GlobalPlotOptions::ScaleRangeIsInverted, this, &GlobalPlotSettings::UpdateScaleRangeWarning);

   connect(mGlobalPlotOptionsPtr, &GlobalPlotOptions::FrequencyChanged, this, &GlobalPlotSettings::UpdateFrequency);
   connect(mGlobalPlotOptionsPtr, &GlobalPlotOptions::FrequencyUnitsChanged, this, &GlobalPlotSettings::UpdateFrequencyUnits);
   connect(frequencyResetButton, &QPushButton::clicked, mGlobalPlotOptionsPtr, &GlobalPlotOptions::ResetFrequencyOptions);
}

bool GlobalPlotSettings::eventFilter(QObject* aObjPtr, QEvent* aEventPtr)
{
   if (aObjPtr == dbMinSpinBox)
   {
      std::function<void(double)> func = [this](double aValue) { ChangeScaleMinDb(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QDoubleSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else if (aObjPtr == dbMaxSpinBox)
   {
      std::function<void(double)> func = [this](double aValue) { ChangeScaleMaxDb(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QDoubleSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else if (aObjPtr == dbIncSpinBox)
   {
      std::function<void(int)> func = [this](int aValue) { ChangeScaleIncDb(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else if (aObjPtr == frequencySpinBox)
   {
      std::function<void(int)> func = [this](int aValue) { ChangeFrequency(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QSpinBox*>(aObjPtr), func, aEventPtr, false);
   }
   else
   {
      return QWidget::eventFilter(aObjPtr, aEventPtr);
   }
}

void GlobalPlotSettings::ChangeScaleMinDb(double aValue)
{
   mInternalChange = true;
   mGlobalPlotOptionsPtr->SetScaleMinDb(aValue);
   mInternalChange = false;
}

void GlobalPlotSettings::ChangeScaleMaxDb(double aValue)
{
   mInternalChange = true;
   mGlobalPlotOptionsPtr->SetScaleMaxDb(aValue);
   mInternalChange = false;
}

void GlobalPlotSettings::ChangeScaleIncDb(double aValue)
{
   mInternalChange = true;
   mGlobalPlotOptionsPtr->SetScaleIncDb(aValue);
   mInternalChange = false;
}

void GlobalPlotSettings::ChangeFrequency(double aValue)
{
   mInternalChange = true;
   mGlobalPlotOptionsPtr->SetFrequency(aValue);
   mInternalChange = false;
}

void GlobalPlotSettings::on_plotStyleComboBox_currentIndexChanged(int aValue)
{
   PlotStyle plotStyle = static_cast<PlotStyle>(aValue);
   mGlobalPlotOptionsPtr->SetPlotStyle(plotStyle);
}

void GlobalPlotSettings::on_frequencyUnits_currentIndexChanged(int aValue)
{
   // inform the options object
   FrequencyUnits value = static_cast<FrequencyUnits>(aValue);
   mGlobalPlotOptionsPtr->SetFrequencyUnits(value);

   // Set number of decimals based on the units
   if (aValue == 0)
   {
      frequencySpinBox->setDecimals(0);
      frequencySpinBox->setMinimum(1);
   }
   else
   {
      frequencySpinBox->setDecimals(3);
      frequencySpinBox->setMinimum(0.001);
   }
}

void GlobalPlotSettings::on_manualScalingCheckBox_clicked(bool aValue)
{
   // Enable/disable the manual scaling widgets
   dbMinSpinBox->setEnabled(aValue);
   dbMaxSpinBox->setEnabled(aValue);
   dbIncSpinBox->setEnabled(aValue);
   scaleResetButton->setEnabled(aValue);
   manualScalingCheckBox->setChecked(aValue);
}


void GlobalPlotSettings::UpdatePlotStyle(PlotStyle aPlotStyle)
{
   plotStyleComboBox->setCurrentIndex(static_cast<int>(aPlotStyle));
}

void GlobalPlotSettings::UpdateScale(float aMinDB, float aMaxDB, float aIncDB)
{
   if (!mInternalChange)
   {
      dbMinSpinBox->setValue(aMinDB);
      dbMaxSpinBox->setValue(aMaxDB);
      dbIncSpinBox->setValue(aIncDB);
   }
}

void GlobalPlotSettings::UpdateScaleRangeWarning(bool aIsInverted)
{
   if (aIsInverted)
   {
      errorMsg->setText(cRANGE_INVERTED_MESSAGE);
   }
   else
   {
      errorMsg->setText(QString());
   }
}

void GlobalPlotSettings::UpdateFrequency(float aValue)
{
   frequencySpinBox->setValue(aValue);
}

void GlobalPlotSettings::UpdateFrequencyUnits(FrequencyUnits aUnits)
{
   switch (aUnits)
   {
   case FrequencyUnits::cFREQUENCY_UNITS_HERTZ:
      frequencyUnits->setCurrentText("Hz");
      break;
   case FrequencyUnits::cFREQUENCY_UNITS_KILOHERTZ:
      frequencyUnits->setCurrentText("kHz");
      break;
   case FrequencyUnits::cFREQUENCY_UNITS_MEGAHERTZ:
      frequencyUnits->setCurrentText("MHz");
      break;
   case FrequencyUnits::cFREQUENCY_UNITS_GIGAHERTZ:
      frequencyUnits->setCurrentText("GHz");
      break;
   }
}
} // namespace PatternVisualizer
