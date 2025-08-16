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

#ifndef POLARCONSTANTAZELPLOTSETTINGS_HPP
#define POLARCONSTANTAZELPLOTSETTINGS_HPP

#include <QSettings>

#include "PlaneType.hpp"
#include "ui_PolarConstantAzElPlotSettings.h"

class QEvent;

namespace PatternVisualizer
{
class Angle;
class PolarPlotOptions;

class PolarConstantAzElPlotSettings : public QWidget, public Ui::PolarConstantAzElPlotSettings
{
   Q_OBJECT

public:
   explicit PolarConstantAzElPlotSettings(QWidget* aParentPtr = nullptr);
   ~PolarConstantAzElPlotSettings() override = default;

   void FinalizeUi(PolarPlotOptions* aOptionsPtr);
   void SetPlaneType(PlaneType aType);

   // Reimplemented methods
   bool eventFilter(QObject* aObjPtr, QEvent* aEventPtr) override;

private slots:
   void on_angleResetButton_clicked();
   void on_originResetButton_clicked();
   void on_zoomResetButton_clicked();

private:
   void ChangeAngle(int aValue);
   void ChangeOriginX(int aValue);
   void ChangeOriginY(int aValue);
   void ChangeZoom(int aValue);
   void UpdateAngle(Angle aValue);
   void UpdateOrigin(QPoint aValue);
   void UpdateZoom(float aValue);

   PolarPlotOptions* mPolarPlotOptionsPtr;
   PlaneType         mPlaneType;
   bool              mInternalChange{false};
};
} // namespace PatternVisualizer

#endif // POLARCONSTANTAZELPLOTSETTINGS_HPP
