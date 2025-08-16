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

#include "OrbitSizeWidget.hpp"

#include <limits>

#include <QComboBox>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QStackedWidget>

#include "ConstellationMakerOptions.hpp"
#include "UtSphericalEarth.hpp"
#include "WsfConstellationOptions.hpp"
#include "WsfSpaceOrbitSize.hpp"

namespace // {anonymous}
{

constexpr int cALTITUDE_INDEX        = 0;
constexpr int cSEMI_MAJOR_AXIS_INDEX = 1;
constexpr int cPERIOD_INDEX          = 2;
constexpr int cREVS_PER_DAY_INDEX    = 3;

} // namespace

namespace SpaceTools
{

OrbitSizeWidget::OrbitSizeWidget(QWidget* aParentPtr)
   : QWidget(aParentPtr)
{
   mSelector     = new QComboBox{};
   mWidgetStack  = new QStackedWidget{};
   mAltitudeEdit = new UtQtLengthValueEdit{nullptr};
   mAltitudeEdit->SetValue(UtLengthValue{0.0, UtUnitLength::cKILOMETERS});
   mSMA_Edit = new UtQtLengthValueEdit{nullptr};
   mSMA_Edit->SetValue(UtLengthValue{0.0, UtUnitLength::cKILOMETERS});
   mPeriodEdit = new UtQtTimeValueEdit{nullptr};
   mPeriodEdit->SetValue(UtTimeValue{0.0, UtUnitTime::cMINUTES});
   mRevsPerDayEdit = new QLineEdit{};
   mRevsPerDayEdit->setValidator(new QDoubleValidator(0.0, std::numeric_limits<double>::max(), 8, mRevsPerDayEdit));

   setFocusProxy(mSelector);

   // Add them to the stacked widget, and use their index as the user data
   // for the combo box.
   mSelector->insertItem(cALTITUDE_INDEX, QString{"Orbit Altitude:"}, mWidgetStack->addWidget(mAltitudeEdit));
   mSelector->insertItem(cSEMI_MAJOR_AXIS_INDEX, QString{"Semi-major Axis:"}, mWidgetStack->addWidget(mSMA_Edit));
   mSelector->insertItem(cPERIOD_INDEX, QString{"Orbital Period:"}, mWidgetStack->addWidget(mPeriodEdit));
   mSelector->insertItem(cREVS_PER_DAY_INDEX, QString{"Revs. Per Day:"}, mWidgetStack->addWidget(mRevsPerDayEdit));

   QHBoxLayout* layoutPtr = new QHBoxLayout{};
   layoutPtr->addWidget(mSelector, 1);
   layoutPtr->addWidget(mWidgetStack, 1);
   layoutPtr->setMargin(0);

   setLayout(layoutPtr);

   connect(mSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OrbitSizeWidget::OnSelectorChanged);
   connect(mAltitudeEdit, &UtQtLengthValueEdit::ValueChanged, this, &OrbitSizeWidget::OnAltitudeEdited);
   connect(mSMA_Edit, &UtQtLengthValueEdit::ValueChanged, this, &OrbitSizeWidget::OnSMA_Edited);
   connect(mPeriodEdit, &UtQtTimeValueEdit::ValueChanged, this, &OrbitSizeWidget::OnPeriodEdited);
   connect(mRevsPerDayEdit, &QLineEdit::textChanged, this, &OrbitSizeWidget::OnRevsEdited);
}

void OrbitSizeWidget::GetValue(WsfConstellationOptions& aOptions)
{
   const QWidget* currentWidgetPtr = mWidgetStack->currentWidget();
   if (currentWidgetPtr == mAltitudeEdit)
   {
      aOptions.SetCircularAltitude(mAltitudeEdit->GetValue());
   }
   else if (currentWidgetPtr == mSMA_Edit)
   {
      aOptions.SetSemiMajorAxis(mSMA_Edit->GetValue());
   }
   else if (currentWidgetPtr == mPeriodEdit)
   {
      aOptions.SetOrbitalPeriod(mPeriodEdit->GetValue());
   }
   else if (currentWidgetPtr == mRevsPerDayEdit)
   {
      aOptions.SetRevolutionsPerDay(mRevsPerDayEdit->text().toDouble());
   }
   else
   {
      throw std::runtime_error{"Combo box in Constellation Maker somehow in invalid state."};
   }
}

void OrbitSizeWidget::SetValue(const WsfConstellationOptions& aOptions)
{
   if (aOptions.SpecifiedAsCircularAltitude())
   {
      mSelector->setCurrentIndex(cALTITUDE_INDEX);
      mAltitudeEdit->SetValue(aOptions.GetCircularAltitude());
      OnAltitudeEdited();
   }
   else if (aOptions.SpecifiedAsSemiMajorAxis())
   {
      mSelector->setCurrentIndex(cSEMI_MAJOR_AXIS_INDEX);
      mSMA_Edit->SetValue(aOptions.GetSemiMajorAxis());
      OnSMA_Edited();
   }
   else if (aOptions.SpecifiedAsOrbitalPeriod())
   {
      mSelector->setCurrentIndex(cPERIOD_INDEX);
      mPeriodEdit->SetValue(aOptions.GetOrbitalPeriod());
      OnPeriodEdited();
   }
   else if (aOptions.SpecifiedAsRevolutionsPerDay())
   {
      mSelector->setCurrentIndex(cREVS_PER_DAY_INDEX);
      mRevsPerDayEdit->setText(QString::number(aOptions.GetRevolutionsPerDay()));
      OnRevsEdited(QString{});
   }
   else
   {
      throw std::runtime_error{"Unable to set orbit size value: missing or unknown data."};
   }
}

void OrbitSizeWidget::ValidateInput(QStringList& aErrors)
{
   int currentIndex = mSelector->currentIndex();
   switch (currentIndex)
   {
   case cALTITUDE_INDEX:
      if (!mAltitudeEdit->IsValid() || mAltitudeEdit->GetValue().GetAsUnit(UtUnitLength::cKILOMETERS) < 100.0)
      {
         aErrors.push_back(QString{"Altitude must be greater than 100 km."});
      }
      break;
   case cSEMI_MAJOR_AXIS_INDEX:
      if (!mSMA_Edit->IsValid() ||
          mSMA_Edit->GetValue().GetAsUnit(UtUnitLength::cMETERS) < UtSphericalEarth::cEARTH_MEAN_RADIUS)
      {
         aErrors.push_back(QString{"Orbit must have a semi-major axis of at least 6471 km."});
      }
      break;
   case cPERIOD_INDEX:
      if (!mPeriodEdit->IsValid() || mPeriodEdit->GetValue().GetAsUnit(UtUnitTime::cHOURS) < 1.43902)
      {
         aErrors.push_back(QString{"Orbital period must be at least 1.43902 hours."});
      }
      break;
   case cREVS_PER_DAY_INDEX:
      if (mRevsPerDayEdit->text().toDouble() > 16.6781)
      {
         aErrors.push_back(QString{"Revolutions per day should be less than 16.6781."});
      }
      break;
   default:
      aErrors.push_back(QString{"Unknown error with orbit size input."});
      break;
   }
}

void OrbitSizeWidget::OnSelectorChanged(int aIndex)
{
   int panelIndex = mSelector->currentData().toInt();
   mWidgetStack->setCurrentIndex(panelIndex);
}

void OrbitSizeWidget::OnSMA_Edited()
{
   if (mWidgetStack->currentWidget() == mSMA_Edit && mSMA_Edit->IsValid())
   {
      auto orbitSize = wsf::space::OrbitSize::FromSemiMajorAxis(mSMA_Edit->GetValue());
      SetAltitudeText(orbitSize.GetCircularAltitude());
      SetPeriodText(orbitSize.GetOrbitalPeriod());
      SetRevsPerDayText(orbitSize.GetRevolutionsPerDay());
   }
}

void OrbitSizeWidget::OnAltitudeEdited()
{
   if (mWidgetStack->currentWidget() == mAltitudeEdit && mAltitudeEdit->IsValid())
   {
      auto orbitSize = wsf::space::OrbitSize::FromCircularAltitude(mAltitudeEdit->GetValue());
      SetSMA_Text(orbitSize.GetSemiMajorAxis());
      SetPeriodText(orbitSize.GetOrbitalPeriod());
      SetRevsPerDayText(orbitSize.GetRevolutionsPerDay());
   }
}

void OrbitSizeWidget::OnPeriodEdited()
{
   if (mWidgetStack->currentWidget() == mPeriodEdit && mPeriodEdit->IsValid())
   {
      auto orbitSize = wsf::space::OrbitSize::FromOrbitalPeriod(mPeriodEdit->GetValue());
      SetSMA_Text(orbitSize.GetSemiMajorAxis());
      SetRevsPerDayText(orbitSize.GetRevolutionsPerDay());
      SetAltitudeText(orbitSize.GetCircularAltitude());
   }
}

void OrbitSizeWidget::OnRevsEdited(const QString& aText)
{
   if (mWidgetStack->currentWidget() == mRevsPerDayEdit)
   {
      auto orbitSize = wsf::space::OrbitSize::FromRevolutionsPerDay(mRevsPerDayEdit->text().toDouble());
      SetSMA_Text(orbitSize.GetSemiMajorAxis());
      SetAltitudeText(orbitSize.GetCircularAltitude());
      SetPeriodText(orbitSize.GetOrbitalPeriod());
   }
}

void OrbitSizeWidget::SetSMA_Text(UtLengthValue aSMA)
{
   aSMA.ConvertToUnit(mSMA_Edit->GetValue().GetUnit());
   mSMA_Edit->SetValue(aSMA);
}

void OrbitSizeWidget::SetAltitudeText(UtLengthValue aAltitude)
{
   aAltitude.ConvertToUnit(mAltitudeEdit->GetValue().GetUnit());
   mAltitudeEdit->SetValue(aAltitude);
}

void OrbitSizeWidget::SetPeriodText(UtTimeValue aPeriod)
{
   aPeriod.ConvertToUnit(mPeriodEdit->GetValue().GetUnit());
   mPeriodEdit->SetValue(aPeriod);
}

void OrbitSizeWidget::SetRevsPerDayText(double aRevsPerDay)
{
   mRevsPerDayEdit->setText(QString::number(aRevsPerDay));
}

} // namespace SpaceTools
