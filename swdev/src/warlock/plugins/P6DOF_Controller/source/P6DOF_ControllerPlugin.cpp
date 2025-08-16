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

#include "P6DOF_ControllerPlugin.hpp"

#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <osg/Group>
#include <osg/Matrix>
#include <osg/Transform>

#include "P6DOF_ControllerPluginObject.hpp"
#include "P6DOF_ControllerPrefWidget.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPrefWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkP6DOF_Controller::Plugin,
                          "P6DOF Controller",
                          "The P6DOF Controller plugin is deprecated. Its functionality has been implemented in the "
                          "Joystick and HeadUpView plugins. P6DOF Controller allows a user to control an AFSIM "
                          "platform with a P6DOF mover using a SDL connected joystick or gamepad.",
                          "warlock",
                          false)

WkP6DOF_Controller::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId, eSECONDARY_VIEWER)
   , mPrefWidgetPtr(new PrefWidget)
   , mControllerObjectPtr(nullptr)
{
   mControllerObjectPtr = new PluginObject(GetInterface(), mPrefWidgetPtr->GetPreferenceObject());
}

WkP6DOF_Controller::Plugin::~Plugin()
{
   delete mControllerObjectPtr;
}

QList<wkf::PrefWidget*> WkP6DOF_Controller::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs;
   prefs << mPrefWidgetPtr;
   return prefs;
}

void WkP6DOF_Controller::Plugin::BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr)
{
   if (aEntityPtr->IsA_TypeOf<wkf::Platform>())
   {
      QString name = aEntityPtr->GetName().c_str();

      if (mControllerObjectPtr->GetDataContainer().IsPlatformP6DOF_Entity(name))
      {
         wkf::Action* connectAction = new wkf::Action(QIcon::fromTheme("controller"), QString("P6DOF Controller"), aMenu);
         connectAction->setData(name);
         connect(connectAction, &QAction::triggered, this, &Plugin::ConnectToPlatformActionHandler);
      }
   }
}

void WkP6DOF_Controller::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mControllerObjectPtr->GetDataContainer()); // Process all the new SimEvents
   mControllerObjectPtr->Update();
}

void WkP6DOF_Controller::Plugin::ConnectToPlatformActionHandler()
{
   QAction* sender       = dynamic_cast<QAction*>(QObject::sender());
   QString  platformName = sender->data().toString();
   mControllerObjectPtr->ConnectToPlatform(platformName, wkfEnv.GetMainWindow());
}

QList<wkf::Action*> WkP6DOF_Controller::Plugin::GetActions() const
{
   return mControllerObjectPtr->GetActions();
}
