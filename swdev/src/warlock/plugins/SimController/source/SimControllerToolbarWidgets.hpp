// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SIM_CONTROLLER_TOOLBAR_WIDGETS_HPP
#define SIM_CONTROLLER_TOOLBAR_WIDGETS_HPP

#include <QDoubleSpinBox>
#include <QSlider>
#include <QToolBar>
#ifdef _WIN32
class QWinThumbnailToolBar;
class QWinThumbnailToolButton;
#endif

#include "SimControllerDataContainer.hpp"
#include "SimControllerPrefObject.hpp"
#include "SimControllerSimInterface.hpp"

namespace wkf
{
class Action;
}

namespace WkSimController
{
class ClockRateSpinBox : public QDoubleSpinBox
{
   friend class Toolbar;

   Q_OBJECT

public:
   ClockRateSpinBox(QWidget* parent);

private:
   void     ValueChanged(double value);
   void     Reset();
   QAction* mResetActionPtr;
};

// Subclass QSlider and re-implement paintEvent to draw tick marks correctly based on palette colors
class ClockRateSlider : public QSlider
{
public:
   ClockRateSlider(QWidget* parent);

protected:
   void paintEvent(QPaintEvent* ev) override;
};
} // namespace WkSimController

#endif
