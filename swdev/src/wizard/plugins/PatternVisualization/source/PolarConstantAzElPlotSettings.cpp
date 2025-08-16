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

#include "PolarConstantAzElPlotSettings.hpp"

#include <QStyle>

#include "PolarPlotOptions.hpp"
#include "SpinBoxEventFilter.hpp"

namespace PatternVisualizer
{
PolarConstantAzElPlotSettings::PolarConstantAzElPlotSettings(QWidget* aParentPtr)
   : QWidget(aParentPtr)
   , mPolarPlotOptionsPtr{nullptr}
{
   setupUi(this);

   zoomSpinBox->installEventFilter(this);
   angleSpinBox->installEventFilter(this);
   originXSpinBox->installEventFilter(this);
   originYSpinBox->installEventFilter(this);

   zoomResetButton->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
   angleResetButton->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
   originResetButton->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
}

void PolarConstantAzElPlotSettings::ChangeAngle(int aValue)
{
   mInternalChange = true;
   Angle angle     = Angle::FromDegrees(aValue);
   switch (mPlaneType)
   {
   case PlaneType::AZ:
      mPolarPlotOptionsPtr->SetConstantAzimuth(angle);
      break;
   case PlaneType::EL:
      mPolarPlotOptionsPtr->SetConstantElevation(angle);
      break;
   default:
      break;
   }
   mInternalChange = false;
}

void PolarConstantAzElPlotSettings::ChangeOriginX(int aValue)
{
   mInternalChange     = true;
   QPoint canvasOrigin = mPolarPlotOptionsPtr->GetOriginOffset();
   mPolarPlotOptionsPtr->SetOriginOffset(QPoint(aValue, canvasOrigin.y()));
   mInternalChange = false;
}

void PolarConstantAzElPlotSettings::ChangeOriginY(int aValue)
{
   mInternalChange     = true;
   QPoint canvasOrigin = mPolarPlotOptionsPtr->GetOriginOffset();
   mPolarPlotOptionsPtr->SetOriginOffset(QPoint(canvasOrigin.x(), aValue));
   mInternalChange = false;
}

void PolarConstantAzElPlotSettings::ChangeZoom(int aValue)
{
   mInternalChange = true;
   float zoom      = aValue / 100.0f;
   mPolarPlotOptionsPtr->SetZoom2D(zoom);
   mInternalChange = false;
}

bool PolarConstantAzElPlotSettings::eventFilter(QObject* aObjPtr, QEvent* aEventPtr)
{
   if (aObjPtr == zoomSpinBox)
   {
      std::function<void(int)> func = [this](int aValue) { ChangeZoom(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else if (aObjPtr == angleSpinBox)
   {
      std::function<void(double)> func = [this](double aValue) { ChangeAngle(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QDoubleSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else if (aObjPtr == originXSpinBox)
   {
      std::function<void(double)> func = [this](double aValue) { ChangeOriginX(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QDoubleSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else if (aObjPtr == originYSpinBox)
   {
      std::function<void(double)> func = [this](double aValue) { ChangeOriginY(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QDoubleSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else
   {
      return QWidget::eventFilter(aObjPtr, aEventPtr);
   }
}

void PolarConstantAzElPlotSettings::FinalizeUi(PolarPlotOptions* aOptionsPtr)
{
   mPolarPlotOptionsPtr = aOptionsPtr;

   connect(mPolarPlotOptionsPtr, &PolarPlotOptions::OriginOffsetChanged, this, &PolarConstantAzElPlotSettings::UpdateOrigin);
   connect(mPolarPlotOptionsPtr, &PolarPlotOptions::Zoom2D_Changed, this, &PolarConstantAzElPlotSettings::UpdateZoom);

   UpdateOrigin(mPolarPlotOptionsPtr->GetOriginOffset());
   UpdateZoom(mPolarPlotOptionsPtr->GetZoom2D());
}

void PolarConstantAzElPlotSettings::on_angleResetButton_clicked()
{
   switch (mPlaneType)
   {
   case PlaneType::AZ:
      mPolarPlotOptionsPtr->ResetConstantAzimuth();
      break;
   case PlaneType::EL:
      mPolarPlotOptionsPtr->ResetConstantElevation();
      break;
   default:
      break;
   }
}

void PolarConstantAzElPlotSettings::on_originResetButton_clicked()
{
   mPolarPlotOptionsPtr->ResetOriginOffset();
}

void PolarConstantAzElPlotSettings::on_zoomResetButton_clicked()
{
   mPolarPlotOptionsPtr->ResetZoom2D();
}

void PolarConstantAzElPlotSettings::SetPlaneType(PlaneType aType)
{
   mPlaneType = aType;

   switch (mPlaneType)
   {
   case PlaneType::AZ:
      angleLabel->setText("Azimuth");
      connect(mPolarPlotOptionsPtr, &PolarPlotOptions::ConstantAzimuthChanged, this, &PolarConstantAzElPlotSettings::UpdateAngle);
      UpdateAngle(mPolarPlotOptionsPtr->GetConstantAzimuth());
      break;
   case PlaneType::EL:
      angleLabel->setText("Elevation");
      connect(mPolarPlotOptionsPtr,
              &PolarPlotOptions::ConstantElevationChanged,
              this,
              &PolarConstantAzElPlotSettings::UpdateAngle);
      UpdateAngle(mPolarPlotOptionsPtr->GetConstantElevation());
      break;
   default:
      break;
   }
}

void PolarConstantAzElPlotSettings::UpdateAngle(Angle aValue)
{
   if (!mInternalChange)
   {
      int angle = static_cast<int>(round(aValue.GetDegrees()));
      angleSpinBox->setValue(angle);
   }
}

void PolarConstantAzElPlotSettings::UpdateOrigin(QPoint aValue)
{
   if (!mInternalChange)
   {
      originXSpinBox->setValue(aValue.x());
      originYSpinBox->setValue(aValue.y());
   }
}

void PolarConstantAzElPlotSettings::UpdateZoom(float aValue)
{
   if (!mInternalChange)
   {
      int zoom = static_cast<int>(round(aValue * 100.0f));
      zoomSpinBox->setValue(zoom);
   }
}
} // namespace PatternVisualizer
