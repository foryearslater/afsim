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
#include "SimControllerToolbar.hpp"

#include <functional>

#include <QMenu>
#include <QMessageBox>
#include <QStyleOptionSlider>
#include <QStylePainter>
#include <QTimer>
#ifdef _WIN32
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
#endif

#include "SimControllerPrefObject.hpp"
#include "SimControllerSimCommands.hpp"
#include "SimControllerSimInterface.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "WkRunManager.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPermissionManager.hpp"

WkSimController::Toolbar::Toolbar(SimInterface*               aSimInterfacePtr,
                                  SimControllerDataContainer& aSimStateData,
                                  const PrefObject*           aPrefObjectPtr)
   : QToolBar("Run")
   , mSimInterfacePtr(aSimInterfacePtr)
   , mSimDataContainer(aSimStateData)
   , mPrefObjectPtr(aPrefObjectPtr)
#ifdef _WIN32
   , mCreateThumbToolbar(true)
   , mThumbToolbarPtr(nullptr)
#endif
{
   setIconSize(QSize(16, 16));
   setEnabled(false); // Wait for SimulationStarting signal to enable toolbar
   setObjectName("SimControllerToolbar");

   auto& enableToolbar =
      wkfEnv.GetPermissions().AddBool("Sim Controller",
                                      "Enable Simulation Control",
                                      "Allows the user to control the simulation state (ie: play/pause)",
                                      true);
   connect(&enableToolbar,
           &wkf::permission::BoolPermission::Changed,
           [this](bool aEnable)
           {
              this->setEnabled(aEnable);
#ifdef _WIN32
              mCreateThumbToolbar = aEnable;
              if (!aEnable && mThumbToolbarPtr)
              {
                 mThumbToolbarPtr->clear();
                 delete mThumbToolbarPtr;
                 mThumbToolbarPtr = nullptr;
              }
              CreateThumbnailToolbar();
#endif
           });

   // Add actions to toolbar to create tool buttons
   QActionGroup* pauseResumeGroup = new QActionGroup(this);
   mPauseActionPtr = new wkf::Action(QIcon::fromTheme("pause"), "Pause", pauseResumeGroup, QKeySequence(Qt::Key_F5));
   connect(mPauseActionPtr,
           &QAction::triggered,
           [this] { mSimInterfacePtr->AddSimCommand(ut::make_unique<PauseCommand>(mSendDIS_PDUs)); });
   mResumeActionPtr = new wkf::Action(QIcon::fromTheme("play"), "Resume", pauseResumeGroup, QKeySequence(Qt::Key_F5));
   connect(mResumeActionPtr,
           &QAction::triggered,
           [this] { mSimInterfacePtr->AddSimCommand(ut::make_unique<ResumeCommand>(mSendDIS_PDUs)); });

   addActions(pauseResumeGroup->actions());

   mTerminateActionPtr =
      new wkf::Action(QIcon::fromTheme("stop"), "Terminate", this, QKeySequence(Qt::SHIFT + Qt::Key_F5));
   connect(mTerminateActionPtr, &QAction::triggered, this, &Toolbar::HandleTerminateSimAction);

   mRestartActionPtr =
      new wkf::Action(QIcon::fromTheme("restart"), "Restart", this, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F5));
   connect(mRestartActionPtr, &QAction::triggered, this, &Toolbar::HandleRestartSimAction);

   addSeparator();

   wkf::Action* clockRateActionPtr     = new wkf::Action(QIcon::fromTheme("fast_forward"), "Set Clock Rate", this);
   wkf::Action* advanceToTimeActionPtr = new wkf::Action(QIcon::fromTheme("time"), "Advance to Time", this);

   QMenu* runMenuPtr = wkfEnv.GetMainWindow()->FindOrCreateMenu(windowTitle());
   runMenuPtr->addActions(actions());

   // Create popup frame and slider to control clock rate
   QFrame* clockFramePtr = new QFrame(this, Qt::Popup);
   mClockRateUi.setupUi(clockFramePtr);
   // Clock rate slider is a logarithmic scale
   mClockRateUi.mSlider->setValue(UtMath::LinearToDB(mSimDataContainer.GetClockRate()));
   mClockRateUi.mSpinBox->setMinimum(UtMath::DB_ToLinear(mClockRateUi.mSlider->minimum()));
   mClockRateUi.mSpinBox->setMaximum(UtMath::DB_ToLinear(mClockRateUi.mSlider->maximum()));
   mClockRateUi.mSpinBox->setValue(mSimDataContainer.GetClockRate());

   // Create popup frame to display "advance to time" dialog
   QFrame* advTimeFramePtr = new QFrame(this, Qt::Popup);
   mAdvanceTimeUi.setupUi(advTimeFramePtr);

   connect(clockRateActionPtr, &QAction::triggered, this, [=]() { PopupTriggered(clockFramePtr); });
   connect(advanceToTimeActionPtr, &QAction::triggered, this, [=]() { PopupTriggered(advTimeFramePtr); });

   connect(mClockRateUi.mSlider,
           &QSlider::valueChanged,
           [this](int aValue)
           { mSimInterfacePtr->AddSimCommand(ut::make_unique<SetClockRateCommand>(UtMath::DB_ToLinear(aValue))); });
   connect(mClockRateUi.mSlider, &QSlider::sliderMoved, this, &Toolbar::ClockRateSliderMoved);

   connect(mClockRateUi.mSpinBox,
           static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
           [this](double aValue) { mSimInterfacePtr->AddSimCommand(ut::make_unique<SetClockRateCommand>(aValue)); });

   connect(mAdvanceTimeUi.buttonBox, &QDialogButtonBox::accepted, this, &Toolbar::AdvanceToTimeExec);
   connect(mAdvanceTimeUi.mTime, &QLineEdit::returnPressed, this, &Toolbar::AdvanceToTimeExec);

   connect(&mSimDataContainer, &SimControllerDataContainer::Initialized, this, &Toolbar::SimulationStarting);
   connect(&mSimDataContainer, &SimControllerDataContainer::SimPausing, this, &Toolbar::SimulationPausing);
   connect(&mSimDataContainer, &SimControllerDataContainer::SimResuming, this, &Toolbar::SimulationResuming);
   connect(&mSimDataContainer, &SimControllerDataContainer::SimComplete, this, &Toolbar::SimulationComplete);
   connect(&mSimDataContainer, &SimControllerDataContainer::SimClockRateChanged, this, &Toolbar::SimulationClockRateChange);
   connect(&mSimDataContainer, &SimControllerDataContainer::RealTimeChanged, this, &Toolbar::SimulationRealTimeChange);
}

void WkSimController::Toolbar::SimulationStarting()
{
   UpdateWidgets(mSimDataContainer.GetPaused(), mSimDataContainer.GetRealTime());
}

void WkSimController::Toolbar::SimulationPausing()
{
   UpdateWidgets(true, mSimDataContainer.GetRealTime());
}

void WkSimController::Toolbar::SimulationResuming()
{
   UpdateWidgets(false, mSimDataContainer.GetRealTime());
}

void WkSimController::Toolbar::SimulationComplete()
{
   UpdateWidgets(false, false);
   mRestartActionPtr->setEnabled(true);
}

void WkSimController::Toolbar::SimulationClockRateChange(double aClockRate)
{
   // Prevent valueChanged() signal from being emitted causing feedback loop
   mClockRateUi.mSpinBox->blockSignals(true);
   mClockRateUi.mSlider->blockSignals(true);
   mClockRateUi.mSpinBox->setValue(aClockRate);
   mClockRateUi.mSlider->setValue(UtMath::LinearToDB(aClockRate));
   mClockRateUi.mSpinBox->blockSignals(false);
   mClockRateUi.mSlider->blockSignals(false);

   mClockRateUi.mSpinBox->ValueChanged(aClockRate);

   UpdateWidgets(mSimDataContainer.GetPaused(), mSimDataContainer.GetRealTime());
}

void WkSimController::Toolbar::SimulationRealTimeChange(bool aIsRealTime)
{
   UpdateWidgets(mSimDataContainer.GetPaused(), aIsRealTime);
}

void WkSimController::Toolbar::UpdateWidgets(bool aPaused, bool aIsRealTime)
{
   mPauseActionPtr->setVisible(!aPaused);
   mResumeActionPtr->setVisible(aPaused);

   foreach (QAction* a, actions())
   {
      a->setEnabled(aIsRealTime);
   }

#ifdef _WIN32
   if (mThumbToolbarPtr)
   {
      foreach (QWinThumbnailToolButton* button, mThumbToolbarPtr->buttons())
      {
         button->setEnabled(aIsRealTime);
      }
      mThumbToggleRunButtonPtr->setIcon(QIcon::fromTheme(aPaused ? "play" : "pause"));
      mThumbToggleRunButtonPtr->setToolTip(aPaused ? "Resume" : "Pause");
   }
#endif
}

void WkSimController::Toolbar::PopupTriggered(QWidget* aWidgetPtr)
{
   QAction* actionPtr = qobject_cast<QAction*>(sender());
   MovePopup(aWidgetPtr, actionPtr);
   aWidgetPtr->show();
}

void WkSimController::Toolbar::ClockRateSliderMoved(int value)
{
   // Prevent SpinBox's valueChanged() signal from being emitted which would trigger a command to change clock rate.
   // The SpinBox is just "previewing" the value of the slider.
   mClockRateUi.mSpinBox->blockSignals(true);
   mClockRateUi.mSpinBox->setValue(UtMath::DB_ToLinear(value));
   mClockRateUi.mSpinBox->blockSignals(false);
}

void WkSimController::Toolbar::HandleTerminateSimAction()
{
   int response =
      QMessageBox::question(nullptr, "Terminate Simulation", "Are you sure you want to terminate the current simulation?");
   if (response == QMessageBox::Yes)
   {
      mSimInterfacePtr->AddSimCommand(ut::make_unique<TerminateCommand>());
      emit RequestStartupDialog();
   }
}

void WkSimController::Toolbar::HandleRestartSimAction()
{
   if (mSimDataContainer.GetSimState() == WsfSimulation::cCOMPLETE)
   {
      warlock::RunManager::GetInstance().ReloadScenario();
   }
   else
   {
      mSimInterfacePtr->AddSimCommand(ut::make_unique<ResetCommand>());
   }
}

void WkSimController::Toolbar::showEvent(QShowEvent*)
{
   CreateThumbnailToolbar();
}

void WkSimController::Toolbar::CreateThumbnailToolbar()
{
#ifdef _WIN32
   if (mCreateThumbToolbar && !mThumbToolbarPtr)
   {
      // Create Windows thumbnail toolbar
      mThumbToolbarPtr = new QWinThumbnailToolBar(this);
      mThumbToolbarPtr->setWindow(wkfEnv.GetMainWindow()->windowHandle());

      mThumbToggleRunButtonPtr = new QWinThumbnailToolButton(mThumbToolbarPtr);
      bool paused              = mSimDataContainer.GetPaused();
      mThumbToggleRunButtonPtr->setIcon(QIcon::fromTheme(paused ? "play" : "pause"));
      mThumbToggleRunButtonPtr->setToolTip(paused ? "Resume" : "Pause");
      mThumbToolbarPtr->addButton(mThumbToggleRunButtonPtr);
      connect(mThumbToggleRunButtonPtr,
              &QWinThumbnailToolButton::clicked,
              [this]
              {
                 if (mSimDataContainer.GetPaused())
                 {
                    mSimInterfacePtr->AddSimCommand(ut::make_unique<ResumeCommand>(mSendDIS_PDUs));
                 }
                 else
                 {
                    mSimInterfacePtr->AddSimCommand(ut::make_unique<PauseCommand>(mSendDIS_PDUs));
                 }
              });

      QWinThumbnailToolButton* termButtonPtr = new QWinThumbnailToolButton(mThumbToolbarPtr);
      termButtonPtr->setIcon(QIcon::fromTheme("stop"));
      termButtonPtr->setToolTip("Terminate");
      mThumbToolbarPtr->addButton(termButtonPtr);
      connect(termButtonPtr, &QWinThumbnailToolButton::clicked, mTerminateActionPtr, &QAction::trigger);

      QWinThumbnailToolButton* restartPtr = new QWinThumbnailToolButton(mThumbToolbarPtr);
      restartPtr->setIcon(QIcon::fromTheme("restart"));
      restartPtr->setToolTip("Restart");
      mThumbToolbarPtr->addButton(restartPtr);
      connect(restartPtr, &QWinThumbnailToolButton::clicked, mRestartActionPtr, &QAction::trigger);
   }
#endif
}

// Move popup widget to correct location in global screen coordinates, based on the action that triggered it
// and the orientation/dock area of the toolbar
void WkSimController::Toolbar::MovePopup(QWidget* aPopupWidgetPtr, QAction* aActionPtr)
{
   QPoint          localPt;
   Qt::ToolBarArea area = isFloating() ? Qt::TopToolBarArea : wkfEnv.GetMainWindow()->toolBarArea(this);
   switch (area)
   {
   case Qt::LeftToolBarArea:
      localPt.setX(width());
      localPt.setY(0);
      break;

   case Qt::BottomToolBarArea:
      localPt.setX(0);
      localPt.setY(-aPopupWidgetPtr->height());
      break;

   case Qt::RightToolBarArea:
      localPt.setX(-aPopupWidgetPtr->width());
      localPt.setY(0);
      break;

   case Qt::TopToolBarArea:
   default:
      localPt.setX(0);
      localPt.setY(height());
      break;
   }

   QPoint globalPt = widgetForAction(aActionPtr)->mapToGlobal(localPt);
   aPopupWidgetPtr->move(globalPt);
}

void WkSimController::Toolbar::AdvanceToTimeExec()
{
   if (mAdvanceTimeUi.mTime->IsValid())
   {
      mSimInterfacePtr->AddSimCommand(ut::make_unique<AdvanceToTimeCommand>(mAdvanceTimeUi.mTime->GetValue()));
   }
   mAdvanceTimeUi.mTime->clear();
   UtQtText::SetValid(true, *mAdvanceTimeUi.mTime);
   mAdvanceTimeUi.mTime->parentWidget()->hide();
}
