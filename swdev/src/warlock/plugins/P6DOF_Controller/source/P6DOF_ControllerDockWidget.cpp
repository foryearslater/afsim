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

#include "P6DOF_ControllerDockWidget.hpp"

#include <QMenu>

#include "SDL_AudioManager.hpp"
#include "SDL_JoystickManager.hpp"
#include "UtMath.hpp"
#include "VaAttachmentModel.hpp"
#include "VaCamera.hpp"
#include "VaCameraMotionTethered.hpp"
#include "VaObserver.hpp"
#include "VaOverlayTexture.hpp"
#include "VaUtils.hpp"
#include "VaViewer.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPlatform.hpp"
#include "WkfSharedMapPreferencesObject.hpp"
#include "WkfVtkEnvironment.hpp"

WkP6DOF_Controller::DockWidget::DockWidget(sdl::JoystickManager*          aJoystickManager,
                                           sdl::AudioManager*             aAudioManager,
                                           const QString&                 aPlatformName,
                                           const QList<wkf::Action*>&     aActions,
                                           SimInterface*                  aSimInterface,
                                           P6DOF_ControllerDataContainer* aDataContainer,
                                           QMainWindow*                   aParent,
                                           Qt::WindowFlags                aFlags)
   : wkf::ImmersiveViewDockWidget("WkP6dofController", aPlatformName, aParent, aFlags)
   , mJoystickManagerPtr(aJoystickManager)
   , mAudioManagerPtr(aAudioManager)
   , mSimInterface(aSimInterface)
   , mDataContainer(aDataContainer)
{
   setFocusPolicy(Qt::StrongFocus);

   mActions = aActions;
   mCallbacks.Add(vespa::VaObserver::ViewerResized.Connect(&DockWidget::ViewerResizedCB, this));
}

void WkP6DOF_Controller::DockWidget::SetDefaultWidth(double aWidth)
{
   mDefaultWidth = aWidth;
   setFixedWidth(aWidth);
}

void WkP6DOF_Controller::DockWidget::SetDefaultHeight(double aHeight)
{
   mDefaultHeight = aHeight;
   setFixedHeight(aHeight);
}

void WkP6DOF_Controller::DockWidget::BuildContextMenuPrivate(QMenu& aMenu, const QPoint& pos)
{
   for (auto&& it : mActions)
   {
      aMenu.addAction(it);
   }
   aMenu.addSeparator();
   QMenu* controllers = new QMenu("Controllers", &aMenu);
   controllers->setIcon(QIcon::fromTheme("controller"));
   std::list<std::string> controllerList   = GetControllerList();
   std::string            activeController = GetActiveController();

   for (const auto& controller : controllerList)
   {
      QAction* action = new QAction(QString::fromStdString(controller), controllers);
      action->setCheckable(true);
      action->setChecked(activeController == controller);
      action->setData(-2);
      connect(action, &QAction::triggered, this, &DockWidget::ActionTriggered);
      controllers->addAction(action);
   }

   QAction* cmon = new QAction("Device Monitor...", controllers);
   cmon->setData(-3);
   connect(cmon, &QAction::triggered, this, &DockWidget::ActionTriggered);
   controllers->addAction(cmon);

   aMenu.addMenu(controllers);
}

void WkP6DOF_Controller::DockWidget::ActionTriggered()
{
   QAction* senderAction = dynamic_cast<QAction*>(sender());
   if (senderAction != nullptr)
   {
      int data = senderAction->data().toInt();

      if (data == -2)
      {
         SwitchController(senderAction->text());
      }
      else if (data == -3)
      {
         LaunchDeviceMonitor(this);
      }
   }
}

void WkP6DOF_Controller::DockWidget::OverridePerspective(double aVFOV)
{
   mVFOV          = aVFOV;
   int viewWidth  = mViewerPtr->GetWidth();
   int viewHeight = mViewerPtr->GetHeight();

   // Determine the ratio between horizontal/vertical
   double xFactor = (double)viewWidth / (double)viewHeight;

   // Set the target/desired vertical half FOV angle
   double tgtHalfFovY_deg = 0.5 * aVFOV;
   double tgtHalfFovY_rad = tgtHalfFovY_deg * UtMath::cRAD_PER_DEG;

   // Set near/far clipping planes, as desired
   double near_m = 0.01; // 1 cm in front
   double far_m  = 20.0; // 20 m distant

   // To find Y at the near distance, we have tan(tgtHalfFovY_rad) = y/near
   // Thus, we have y = near * tan(tgtHalfFovY_rad)
   double yMax_m = near_m * tan(tgtHalfFovY_rad);
   double yMin_m = -yMax_m;
   double xMin_m = -xFactor * yMax_m;
   double xMax_m = xFactor * yMax_m;

   // Re-adjust near/far clipping planes, as desired
   near_m = 1.0;      // 1 meter in front
   far_m  = 200000.0; // 200 km distant

   // Re-calculate x & y values
   yMax_m = near_m * tan(tgtHalfFovY_rad);
   yMin_m = -yMax_m;
   xMin_m = -xFactor * yMax_m;
   xMax_m = xFactor * yMax_m;

   mViewerPtr->OverridePerspective(xMin_m, xMax_m, yMin_m, yMax_m, near_m, far_m);

   if (mHudPtr)
   {
      mHudPtr->SetupHudProjection(aVFOV);
   }
}

void WkP6DOF_Controller::DockWidget::SetFirstPersonPrivate(bool aState)
{
   mFirstPerson = aState;
}

void WkP6DOF_Controller::DockWidget::ClearDataPrivate()
{
   mHudPtr = nullptr;
}

std::list<std::string> WkP6DOF_Controller::DockWidget::GetControllerList() const
{
   std::list<std::string> deviceList;
   if (nullptr != mJoystickManagerPtr)
   {
      mJoystickManagerPtr->GetDevices(deviceList);
   }
   return deviceList;
}

std::string WkP6DOF_Controller::DockWidget::GetActiveController() const
{
   if (nullptr != mJoystickManagerPtr)
   {
      if (mJoystickManagerPtr->HasDevice(0))
      {
         return mJoystickManagerPtr->GetDeviceName(0);
      }
   }
   return "";
}

void WkP6DOF_Controller::DockWidget::SwitchController(const QString& aControllerName)
{
   if (nullptr != mJoystickManagerPtr)
   {
      std::list<std::string> devices;
      devices.push_back(aControllerName.toStdString());
      mJoystickManagerPtr->ReleaseDevice(0);
      mJoystickManagerPtr->UseBestDevice(0, devices);
   }
}

void WkP6DOF_Controller::DockWidget::LaunchDeviceMonitor(QWidget* aParent)
{
   if (nullptr != mJoystickManagerPtr)
   {
      mJoystickManagerPtr->OpenSDL_DeviceSpy(aParent);
   }
}

void WkP6DOF_Controller::DockWidget::ViewerInitializedPrivate()
{
   CompleteConstruction();
}


bool WkP6DOF_Controller::DockWidget::HUD_Visible() const
{
   if (mHudPtr)
   {
      return mHudPtr->IsVisible();
   }
   else
   {
      return false;
   }
}

WkP6DOF_Controller::HUD* WkP6DOF_Controller::DockWidget::GetHUD() const
{
   return mHudPtr;
}

void WkP6DOF_Controller::DockWidget::ShowHUD(bool aState)
{
   if (aState && !mHudPtr)
   {
      mHudPtr = new WkP6DOF_Controller::HUD("hud", vespa::VaOverlay::cOVERLAY_WINDOW, mVFOV);
      AddOverlay(mHudPtr);
   }
   if (mHudPtr)
   {
      mHudPtr->SetVisible(aState);
   }
}

void WkP6DOF_Controller::DockWidget::ViewerResizedCB(vespa::VaViewer* aViewerPtr)
{
   if (IsMyViewer(aViewerPtr))
   {
      OverridePerspective(mVFOV);
   }
}
