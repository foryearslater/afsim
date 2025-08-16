// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ORBITSIZEWIDGET_HPP
#define ORBITSIZEWIDGET_HPP

#include <QWidget>

class QComboBox;
class QStackedWidget;
class QLineEdit;

#include "UtQtUnitValueEdit.hpp"

class WsfConstellationOptions;

namespace SpaceTools
{

class OrbitSizeWidget : public QWidget
{
   Q_OBJECT

public:
   explicit OrbitSizeWidget(QWidget* aParentPtr = nullptr);
   ~OrbitSizeWidget() override = default;

   void GetValue(WsfConstellationOptions& aOptions);
   void SetValue(const WsfConstellationOptions& aOptions);

   void ValidateInput(QStringList& aErrors);

private:
   void OnSelectorChanged(int aIndex);
   void OnSMA_Edited();
   void OnAltitudeEdited();
   void OnPeriodEdited();
   void OnRevsEdited(const QString& aText);

   void SetSMA_Text(UtLengthValue aSMA);
   void SetAltitudeText(UtLengthValue aAltitude);
   void SetPeriodText(UtTimeValue aPeriod);
   void SetRevsPerDayText(double aRevsPerDay);

   QComboBox*           mSelector{nullptr};
   QStackedWidget*      mWidgetStack{nullptr};
   UtQtLengthValueEdit* mAltitudeEdit{nullptr};
   UtQtLengthValueEdit* mSMA_Edit{nullptr};
   UtQtTimeValueEdit*   mPeriodEdit{nullptr};
   QLineEdit*           mRevsPerDayEdit{nullptr};
};

} // namespace SpaceTools

#endif // ORBITSIZEWIDGET_HPP
