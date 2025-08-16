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
#ifndef P6DOF_CONTROLLERDOCKWIDGET_HPP
#define P6DOF_CONTROLLERDOCKWIDGET_HPP

#include <osg/Group>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Transform>
#include <osg/ref_ptr>

#include "P6DOF_ControllerDataContainer.hpp"
#include "P6DOF_ControllerHUD.hpp"
#include "P6DOF_ControllerSimInterface.hpp"
#include "misc/WkfImmersiveViewDockWidget.hpp"

namespace sdl
{
class AudioManager;
class JoystickManager;
} // namespace sdl

namespace vespa
{
class VaOverlayTexture;
}

namespace WkP6DOF_Controller
{
// The WkP6DOF_Controller::DockWidget is the main widget of the P6DofController.
// It provides a tether window (immersive view) with a HUD overlay.
class DockWidget : public wkf::ImmersiveViewDockWidget
{
public:
   DockWidget(sdl::JoystickManager*          aJoystickManager,
              sdl::AudioManager*             aAudioManager,
              const QString&                 aPlatformName,
              const QList<wkf::Action*>&     aActions,
              SimInterface*                  aSimInterface,
              P6DOF_ControllerDataContainer* aDataContainer,
              QMainWindow*                   aParent = nullptr,
              Qt::WindowFlags                aFlags  = Qt::WindowFlags());

   // This returns true if the HUD is visible/shown
   bool HUD_Visible() const;

   // This set the FOV when overriding perspective
   void OverridePerspective(double aVFOV);

   // This returns a pointer to the HUD object
   WkP6DOF_Controller::HUD* GetHUD() const;

   // This shows/hides the HUD based on the state argument
   void ShowHUD(bool aState);

   // This sets the default width for the widget
   void SetDefaultWidth(double aWidth);

   // This sets the default height for the widget
   void SetDefaultHeight(double aHeight);

protected:
   void BuildContextMenuPrivate(QMenu& aMenu, const QPoint& pos) override;
   void SetFirstPersonPrivate(bool aState) override;
   void ClearDataPrivate() override;

private:
   void                   ViewerResizedCB(vespa::VaViewer* aViewerPtr);
   void                   ActionTriggered();
   std::list<std::string> GetControllerList() const;
   std::string            GetActiveController() const;
   void                   SwitchController(const QString& aControllerName);
   void                   LaunchDeviceMonitor(QWidget* aParent);
   void                   ViewerInitializedPrivate() override;

   sdl::JoystickManager*          mJoystickManagerPtr = nullptr;
   sdl::AudioManager*             mAudioManagerPtr    = nullptr;
   SimInterface*                  mSimInterface       = nullptr;
   P6DOF_ControllerDataContainer* mDataContainer      = nullptr;
   WkP6DOF_Controller::HUD*       mHudPtr             = nullptr;
   bool                           mFirstPerson        = true;
   double                         mVFOV               = 44.0;
   double                         mDefaultWidth       = 0;
   double                         mDefaultHeight      = 0;
   QList<wkf::Action*>            mActions;
};
} // namespace WkP6DOF_Controller
#endif // P6DOF_CONTROLLERDOCKWIDGET_HPP
