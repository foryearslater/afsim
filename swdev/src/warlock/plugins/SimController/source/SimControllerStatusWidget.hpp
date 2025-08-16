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
#ifndef SIMCONTROLLERSTATUSWIDGET_HPP
#define SIMCONTROLLERSTATUSWIDGET_HPP

#include <QObject>
#include <QWidget>
#ifdef _WIN32
class QWinTaskbarProgress;
#endif

#include "SimControllerDataContainer.hpp"
#include "SimControllerPrefObject.hpp"
#include "WkfUnitsObject.hpp"
#include "ui_SimControllerStatusBarWidget.h"

namespace WkSimController
{
class StatusWidget : public QWidget
{
   Q_OBJECT

public:
   StatusWidget(SimControllerDataContainer& aSimStateData, const PrefObject* aPrefObjectPtr, size_t aPluginUniqueId);
   ~StatusWidget() override = default;

   void Update();

   void SimulationStarting();
   void SimulationPausing();
   void SimulationResuming();
   void SimulationComplete(double aSimTime);

protected:
   void showEvent(QShowEvent*) override;

private:
   void ShowClockRateHandler(bool aShow);
   void ShowProgressBarHandler(bool aShow);
   void ShowSimModeHandler(bool aShow);
   void ShowSimStateHandler(bool aShow);
   void ShowSimTimeHandler(bool aShow);
   void ShowTimeBehindHandler(bool aShow);
   void ShowWallTimeHandler(bool aShow);
   bool eventFilter(QObject* aObj, QEvent* aEvent) override;
   void DateFormatChanged(wkf::UnitsData::DateFormat aFormat);

   SimControllerDataContainer&      mSimStateData;
   const PrefObject*                mPrefObjectPtr;
   Ui::SimControllerStatusBarWidget mUi;
#ifdef _WIN32
   QWinTaskbarProgress* mTaskbarProgressPtr;
#endif
   size_t mPluginUniqueId;

   wkf::UnitsData::DateFormat mDateFormat;
};
} // namespace WkSimController

#endif // SIMCONTROLLERSTATUSWIDGET_HPP
