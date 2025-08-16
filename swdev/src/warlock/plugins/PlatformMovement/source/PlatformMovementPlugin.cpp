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

#include "PlatformMovementPlugin.hpp"

#include <QMenu>

#include "PlatformMovementDockWidget.hpp"
#include "PlatformMovementSimCommands.hpp"
#include "PlatformMovementSimInterface.hpp"
#include "RouteBrowserInterface.hpp"
#include "UtQtMemory.hpp"
#include "VaUtils.hpp"
#include "WkPlugin.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfViewer.hpp"
#include "route/WkfRouteBrowserDockWidget.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   WkPlatformMovement::Plugin,
   "Platform Movement",
   "The Platform Movement plugin provides a Route Browser for displaying and editing routes within the simulation. It "
   "also provides a dialog to send movement commands to a platform, such as Go To Altitude, Go To Location, Go To "
   "Speed, and Turn to Heading. Both of these dialogs are accessed from the View menu.",
   "warlock")

WkPlatformMovement::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : warlock::PluginT<SimInterface>(aPluginName, aUniqueId)
{
   wkf::MainWindow* mainWindowPtr = wkfEnv.GetMainWindow();

   mRouteBrowserInterface = new RouteBrowserInterface(GetInterface());

   mDockWidgetPtr = new DockWidget(GetInterface(), mRouteBrowserInterface, mainWindowPtr);
   mainWindowPtr->addDockWidget(Qt::RightDockWidgetArea, mDockWidgetPtr);
   mDockWidgetPtr->hide();

   mShortcutPtr = new Shortcut(*GetInterface(), this);

   mPlatformCallbacks.Add(wkf::Observer::PlatformsAdded.Connect(&Plugin::PlatformsAddedHandler, this));

   connect(mDockWidgetPtr,
           &DockWidget::RouteComboBoxSelected,
           mRouteBrowserInterface,
           &RouteBrowserInterface::RouteComboBoxSelectedHandler);

   QTreeWidgetItem* parent = RegisterOptionGroup(nullptr, "Movement");
   RegisterOption(parent, "Routes");

   wkf::Action* headingAction = new wkf::Action("Turn to Heading", wkfEnv.GetMainWindow(), QKeySequence(Qt::Key_H));
   connect(headingAction, &QAction::triggered, this, [this]() { mShortcutPtr->StartHeading(); });
   mActions.push_back(headingAction);

   wkf::Action* gotoAction = new wkf::Action("Go to Location", wkfEnv.GetMainWindow(), QKeySequence(Qt::Key_G));
   connect(gotoAction, &QAction::triggered, this, [this]() { mShortcutPtr->StartGoto(); });
   mActions.push_back(gotoAction);
}

void WkPlatformMovement::Plugin::ResetOptionStates()
{
   wkf::Scenario* scenPtr = vaEnv.GetStandardScenario();
   if (scenPtr)
   {
      const std::map<unsigned int, wkf::Platform*>& platMap = scenPtr->GetIndexPlatformMap();
      for (auto& it : platMap)
      {
         wkf::AttachmentRoute* routePtr = dynamic_cast<wkf::AttachmentRoute*>(it.second->FindAttachment("route"));
         if (routePtr)
         {
            it.second->RemoveAttachment(routePtr->GetUniqueId());
         }
      }
   }
}

void WkPlatformMovement::Plugin::SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatformPtr)
{
   wkf::AttachmentRoute* routePtr = dynamic_cast<wkf::AttachmentRoute*>(aPlatformPtr->FindAttachment("route"));

   if (routePtr == nullptr)
   {
      routePtr = vespa::make_attachment<wkf::AttachmentRoute>(*aPlatformPtr, vaEnv.GetStandardViewer());
      wkf::AttachmentRoute::LoadAttachment(*routePtr);
   }
   routePtr->SetStateVisibility(aState);
   routePtr->ShowLabels(false);
}

Qt::CheckState WkPlatformMovement::Plugin::GetPlatformOptionState(int aOptionId, const wkf::Platform* aPlatformPtr) const
{
   wkf::AttachmentRoute* routePtr = dynamic_cast<wkf::AttachmentRoute*>(aPlatformPtr->FindAttachment("route"));
   if (routePtr != nullptr)
   {
      return (routePtr->GetStateVisibility()) ? Qt::Checked : Qt::Unchecked;
   }
   return Qt::Unchecked;
}

QList<wkf::Action*> WkPlatformMovement::Plugin::GetActions() const
{
   return mActions;
}

void WkPlatformMovement::Plugin::GuiUpdate()
{
   mInterfacePtr->ProcessEvents(mRouteBrowserInterface);
   mDockWidgetPtr->Update();
}

void WkPlatformMovement::Plugin::PlatformsAddedHandler(const wkf::PlatformList& aPlatforms)
{
   for (const auto& platform : aPlatforms)
   {
      mInterfacePtr->AddSimCommand(
         ut::make_unique<LocalRouteRequestCommand>(GetInterface(), platform->GetName(), LocalRouteRequestCommand::ROUTE));
   }
}
