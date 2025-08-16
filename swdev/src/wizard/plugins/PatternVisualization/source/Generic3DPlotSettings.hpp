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

#ifndef GENERIC3DPLOTSETTINGS_HPP
#define GENERIC3DPLOTSETTINGS_HPP

#include <QWidget>

#include "Angle.hpp"
#include "ui_Generic3DPlotSettings.h"

class QEvent;

namespace PatternVisualizer
{
class Session;

class Generic3DPlotSettings : public QWidget, public Ui::Generic3DPlotSettings
{
   Q_OBJECT

public:
   explicit Generic3DPlotSettings(QWidget* aParentPtr = nullptr);
   ~Generic3DPlotSettings() override = default;

   void FinalizeUi(Session* aSessionPtr);

   // Reimplemented methods
   bool eventFilter(QObject* aObjPtr, QEvent* aEventPtr) override;

public slots:
   void SetLocatorAzEl(Angle aAzimuth, Angle aElevation);
   void EmitLocatorAzEl();
   void SetLocatorValue(float aValue, bool aValid);

signals:
   void AzElChanged(Angle aAzimuth, Angle aElevation);
   void CameraAzimuthInputChanged(Angle aAzimuth);
   void CameraElevationInputChanged(Angle aAzimuth);
   void CameraZoomInputChanged(float aZoom);

private slots:
   void OnCameraRotationChanged(const Angle& aAzimuth, const Angle& aElevation);
   void OnCameraZoomChanged(float aZoom);

private:
   void ChangeAzimuth(double aValue);
   void ChangeElevation(double aValue);
   void ChangeZoom(int aValue);
   void SetAzSpinBoxValue(Angle aAngle);
   void SetElSpinBoxValue(Angle aAngle);
   void SetZoomSpinBoxValue(float aValue);

   bool mInternalChange = false;
};
} // namespace PatternVisualizer

#endif
