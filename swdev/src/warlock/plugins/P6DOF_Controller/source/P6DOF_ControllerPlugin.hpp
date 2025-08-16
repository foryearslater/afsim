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
#ifndef P6DOF_CONTROLLERPLUGIN_HPP
#define P6DOF_CONTROLLERPLUGIN_HPP

#include <osg/Group>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Transform>
#include <osg/ref_ptr>

#include "P6DOF_ControllerPluginObject.hpp"
#include "P6DOF_ControllerPrefWidget.hpp"
#include "P6DOF_ControllerSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkP6DOF_Controller
{
// The WkP6DOF_Controller::Plugin is a Warlock plug-in that provides the
// ability to manually control (fly) P6DOF movers using gaming
// joysticks/controllers for user input while providing an out-the-window
// (OTW) view with a head-up display (HUD) overlay and basic audio output
// (for cockpit sounds such as warning tones).
class Plugin : public warlock::PluginT<SimInterface>
{
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override;

   // This is used to build the list of actions possible. In this case,
   // it will verify that the entity is a platform and is a P6DOF mover,
   // and if so, add "P6DOF Controller" to the list
   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

   // This updates the plug-in GUI
   void GuiUpdate() override;

   // This returns the Wkf actions that are supported
   QList<wkf::Action*> GetActions() const override;

   // This returns a list of preference widgets
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

protected:
   PluginUiPointer<PrefWidget> mPrefWidgetPtr;

private:
   virtual void ConnectToPlatformActionHandler();

   PluginObject* mControllerObjectPtr;
};
} // namespace WkP6DOF_Controller
#endif
