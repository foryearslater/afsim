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

#include "Generic3DPlotSettings.hpp"

#include <QDoubleValidator>

#include "Session.hpp"
#include "SpinBoxEventFilter.hpp"

namespace PatternVisualizer
{
Generic3DPlotSettings::Generic3DPlotSettings(QWidget* aParentPtr)
   : QWidget{aParentPtr}
{
   setupUi(this);
   cameraResetButton->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
   zoomResetButton->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
}

void Generic3DPlotSettings::ChangeAzimuth(double aValue)
{
   mInternalChange = true;
   Angle azimuth   = Angle::FromDegrees(aValue);
   emit  CameraAzimuthInputChanged(azimuth);
   mInternalChange = false;
}

void Generic3DPlotSettings::ChangeElevation(double aValue)
{
   mInternalChange = true;
   Angle elevation = Angle::FromDegrees(aValue);
   emit  CameraElevationInputChanged(elevation);
   mInternalChange = false;
}

void Generic3DPlotSettings::ChangeZoom(int aValue)
{
   mInternalChange = true;
   float zoom      = static_cast<float>(aValue) / 100.0f;
   emit  CameraZoomInputChanged(zoom);
   mInternalChange = false;
}

void Generic3DPlotSettings::EmitLocatorAzEl()
{
   float azimuth   = mAzimuthLineEdit->text().toFloat();
   float elevation = mElevationLineEdit->text().toFloat();
   emit  AzElChanged(Angle::FromDegrees(azimuth), Angle::FromDegrees(elevation));
}

bool Generic3DPlotSettings::eventFilter(QObject* aObjPtr, QEvent* aEventPtr)
{
   if (aObjPtr == cameraAzDoubleSpinBox)
   {
      std::function<void(double)> func = [this](double aValue) { ChangeAzimuth(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QDoubleSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else if (aObjPtr == cameraElDoubleSpinBox)
   {
      std::function<void(double)> func = [this](double aValue) { ChangeElevation(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QDoubleSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else if (aObjPtr == zoomSpinBox)
   {
      std::function<void(int)> func = [this](int aValue) { ChangeZoom(aValue); };
      return SpinBoxEventFilter::Filter(static_cast<QSpinBox*>(aObjPtr), func, aEventPtr);
   }
   else
   {
      return QWidget::eventFilter(aObjPtr, aEventPtr);
   }
}

void Generic3DPlotSettings::FinalizeUi(Session* aSessionPtr)
{
   // NOTE: This method is required because the system currently is setting up the UI
   // automatically from the related .ui files for the various widgets in this dock,
   // but we need to have the session.
   Camera* cameraPtr = aSessionPtr->GetCamera();

   // Set the camera controls
   SetAzSpinBoxValue(cameraPtr->GetAzimuth());
   SetElSpinBoxValue(cameraPtr->GetElevation());
   SetZoomSpinBoxValue(cameraPtr->GetZoom());

   // Create connections between this widget and the Camera object to allow the user to
   // change camera settings with the widgets or by mouse action on the canvas
   connect(cameraResetButton, &QPushButton::clicked, cameraPtr, &Camera::ResetCameraAngles);
   connect(zoomResetButton, &QPushButton::clicked, cameraPtr, &Camera::ResetCameraZoom);
   connect(cameraPtr, &Camera::CameraRotationChanged, this, &Generic3DPlotSettings::OnCameraRotationChanged);
   connect(cameraPtr, &Camera::CameraZoomChanged, this, &Generic3DPlotSettings::OnCameraZoomChanged);
   connect(this, &Generic3DPlotSettings::CameraAzimuthInputChanged, cameraPtr, &Camera::SetAzimuth);
   connect(this, &Generic3DPlotSettings::CameraElevationInputChanged, cameraPtr, &Camera::SetElevation);
   connect(this, &Generic3DPlotSettings::CameraZoomInputChanged, cameraPtr, &Camera::SetZoom);

   // Install event filters for the spin boxes to prevent intermediate image updates while the
   // user is typing new values
   zoomSpinBox->installEventFilter(this);
   cameraAzDoubleSpinBox->installEventFilter(this);
   cameraElDoubleSpinBox->installEventFilter(this);

   // Set validators for the locator plane related controls
   mAzimuthLineEdit->setValidator(new QDoubleValidator(-180.0, 180.0, 2));
   mElevationLineEdit->setValidator(new QDoubleValidator(-90.0, 90.0, 2));

   // Create connections between this widget and the locator plane controls
   LocatorOptions* locatorOptionsPtr = aSessionPtr->GetLocatorOptions();
   connect(mAzimuthLineEdit, &QLineEdit::editingFinished, this, &Generic3DPlotSettings::EmitLocatorAzEl);
   connect(mElevationLineEdit, &QLineEdit::editingFinished, this, &Generic3DPlotSettings::EmitLocatorAzEl);
   connect(this, &Generic3DPlotSettings::AzElChanged, locatorOptionsPtr, &LocatorOptions::SetAzEl);
   connect(locatorOptionsPtr, &LocatorOptions::AzElChanged, this, &Generic3DPlotSettings::SetLocatorAzEl);
   connect(locatorOptionsPtr, &LocatorOptions::UpdateDockValue, this, &Generic3DPlotSettings::SetLocatorValue);
   connect(locatorOptionsPtr, &LocatorOptions::ShowLocatorChanged, mShowLocator, &QGroupBox::setChecked);
   connect(mShowLocator, &QGroupBox::clicked, locatorOptionsPtr, &LocatorOptions::SetLocatorShown);
}

void Generic3DPlotSettings::OnCameraRotationChanged(const Angle& aAzimuth, const Angle& aElevation)
{
   if (!mInternalChange)
   {
      SetAzSpinBoxValue(aAzimuth);
      SetElSpinBoxValue(aElevation);
   }
}

void Generic3DPlotSettings::OnCameraZoomChanged(float aZoom)
{
   if (!mInternalChange)
   {
      SetZoomSpinBoxValue(aZoom);
   }
}

void Generic3DPlotSettings::SetAzSpinBoxValue(Angle aAngle)
{
   cameraAzDoubleSpinBox->setValue(aAngle.GetDegrees());
}

void Generic3DPlotSettings::SetElSpinBoxValue(Angle aAngle)
{
   cameraElDoubleSpinBox->setValue(aAngle.GetDegrees());
}

void Generic3DPlotSettings::SetLocatorAzEl(Angle aAzimuth, Angle aElevation)
{
   mAzimuthLineEdit->setText(QString::number(aAzimuth.GetDegrees(), 'f', 2));
   mElevationLineEdit->setText(QString::number(aElevation.GetDegrees(), 'f', 2));
}

void Generic3DPlotSettings::SetLocatorValue(float aValue, bool aValid)
{
   if (aValid)
   {
      mValueLineEdit->setText(QString::number(aValue, 'f', 4));
   }
   else
   {
      mValueLineEdit->setText(QString(""));
   }
}

void Generic3DPlotSettings::SetZoomSpinBoxValue(float aValue)
{
   int zoom = static_cast<int>(round(aValue * 100.0f));
   zoomSpinBox->setValue(zoom);
}
} // namespace PatternVisualizer
