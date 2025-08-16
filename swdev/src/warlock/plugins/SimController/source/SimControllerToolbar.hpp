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
#ifndef SIMCONTROLLERTOOLBAR_HPP
#define SIMCONTROLLERTOOLBAR_HPP

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
#include "ui_SimControllerAdvanceTime.h"
#include "ui_SimControllerClockRateSlider.h"

namespace wkf
{
class Action;
}

namespace WkSimController
{
class Toolbar : public QToolBar
{
   friend class Plugin;

   Q_OBJECT

public:
   Toolbar(SimInterface* aSimInterfacePtr, SimControllerDataContainer& aSimStateData, const PrefObject* aPrefObjectPtr);
   ~Toolbar() override = default;

   void SetSendDIS_PDUs(bool aSend) { mSendDIS_PDUs = aSend; }

signals:
   void RequestStartupDialog();

protected:
   void showEvent(QShowEvent*) override;

private:
   void SimulationStarting();
   void SimulationPausing();
   void SimulationResuming();
   void SimulationComplete();
   void SimulationClockRateChange(double aClockRate);
   void SimulationRealTimeChange(bool aIsRealTime);

   void UpdateWidgets(bool aPaused, bool aIsRealTime);

   // Toolbar slots
   void CreateThumbnailToolbar();
   void PopupTriggered(QWidget* aWidgetPtr);
   void ClockRateSliderMoved(int value);

   void HandleTerminateSimAction();
   void HandleRestartSimAction();

   void MovePopup(QWidget* aPopupWidgetPtr, QAction* aActionPtr);

   void AdvanceToTimeExec();

   wkf::Action* mPauseActionPtr;
   wkf::Action* mResumeActionPtr;
   wkf::Action* mTerminateActionPtr;
   wkf::Action* mRestartActionPtr;
#ifdef _WIN32
   bool                     mCreateThumbToolbar;
   QWinThumbnailToolBar*    mThumbToolbarPtr;
   QWinThumbnailToolButton* mThumbToggleRunButtonPtr;
#endif

   bool mSendDIS_PDUs;

   SimInterface*               mSimInterfacePtr;
   SimControllerDataContainer& mSimDataContainer;
   const PrefObject*           mPrefObjectPtr;

   Ui::SimControllerClockRate   mClockRateUi;
   Ui::SimControllerAdvanceTime mAdvanceTimeUi;
};
} // namespace WkSimController
#endif // SIMCONTROLLERTOOLBAR_HPP
