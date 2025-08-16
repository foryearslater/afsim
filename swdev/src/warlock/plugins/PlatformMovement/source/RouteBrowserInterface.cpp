// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "RouteBrowserInterface.hpp"

#include <QMenu>
#include <QMessageBox>

#include "PlatformMovementSimCommands.hpp"
#include "PlatformMovementSimInterface.hpp"
#include "UtMemory.hpp"
#include "UtSphericalEarth.hpp"
#include "VaCallbackHolder.hpp"
#include "VaDefs.hpp"
#include "VaEntity.hpp"
#include "VaObserver.hpp"
#include "WkPermissions.hpp"
#include "WkSimInterface.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfObserver.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPlatform.hpp"
#include "WsfRoute.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "route/WkfRouteBrowserCreateDialog.hpp"
#include "route/WkfRouteBrowserDockWidget.hpp"
#include "route/WkfRouteBrowserEditDialog.hpp"

WkPlatformMovement::RouteBrowserInterface::RouteBrowserInterface(SimInterface* aSimInterface)
   : wkf::RouteBrowserInterface(wkfEnv.GetMainWindow(), vaEnv.GetStandardViewer())
   , mSimInterface(aSimInterface)
{
   Connect();
}

// When the user selects a route from the platform movement follow route combo box, select it in the route browser
int WkPlatformMovement::RouteBrowserInterface::RouteComboBoxSelectedHandler(const QString& aRouteName)
{
   mDockWidget->CollapseAllRoutes();
   return mDockWidget->ExpandGlobalRoute(aRouteName);
}

void WkPlatformMovement::RouteBrowserInterface::Clear()
{
   wkf::RouteBrowserInterface::Clear();
   mGlobalRoutes.Clear();
}

void WkPlatformMovement::RouteBrowserInterface::DeleteRoute(const QString& aRouteName)
{
   mDockWidget->DeleteRouteHandler();
   mGlobalRoutes.DeleteRoute(aRouteName);
   mUserGlobalRoutes.DeleteRoute(aRouteName);
   wkf::RouteBrowserInterface::DeleteRoute(aRouteName);
}

void WkPlatformMovement::RouteBrowserInterface::HideGlobalRoute(const QString& aGlobalRouteNames)
{
   if (mViewer != nullptr)
   {
      vespa::VaEntity* anchor = vaEnv.FindEntity(aGlobalRouteNames.toStdString() + "_anchor");
      if (anchor != nullptr)
      {
         auto* routeAttachmentPtr =
            dynamic_cast<wkf::AttachmentRoute*>(anchor->FindAttachment(aGlobalRouteNames.toStdString()));
         if (routeAttachmentPtr)
         {
            routeAttachmentPtr->SetStateVisibility(false);
         }
      }
   }
}

void WkPlatformMovement::RouteBrowserInterface::Connect()
{
   // DockWidget Connections --------------------------------------------------------------------------------------
   connect(mDockWidget,
           &wkf::RouteBrowserDockWidget::LocalRouteEditDialogRequested,
           this,
           [this](const QString& aPlatformName)
           {
              mSimInterface->AddSimCommand(
                 ut::make_unique<LocalRouteRequestCommand>(mSimInterface,
                                                           aPlatformName.toStdString(),
                                                           LocalRouteRequestCommand::ROUTE_DIALOG));
           });

   connect(mDockWidget->GetEditDialog(),
           &wkf::RouteBrowserEditDialog::Apply,
           this,
           [this](const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo)
           {
              auto* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(aRouteInfo.mPlatformName);
              if (platformPtr && !warlock::HasPermissionToControlPlatform(*platformPtr))
              {
                 QMessageBox::warning(nullptr,
                                      "Route Not Modified",
                                      "User does not have permissions to modify platform " + aRouteInfo.mPlatformName + ".");
              }
              else
              {
                 mSimInterface->AddSimCommand(ut::make_unique<ModifyRouteCommand>(aRouteInfo));
              }
           });

   connect(mDockWidget->GetEditDialog(),
           &wkf::RouteBrowserEditDialog::RouteEdited,
           this,
           &wkf::RouteBrowserInterface::RouteEditedHandler);

   connect(mDockWidget->GetEditDialog(),
           &wkf::RouteBrowserEditDialog::RequestRouteInfo,
           this,
           [this](const QString& aPlatformName)
           {
              mSimInterface->AddSimCommand(
                 ut::make_unique<LocalRouteRequestCommand>(mSimInterface,
                                                           aPlatformName.toStdString(),
                                                           LocalRouteRequestCommand::ROUTE_DIALOG));
           });

   // Create Dialog Connections -----------------------------------------------------------------------------------
   connect(mDockWidget->GetCreateDialog(),
           &wkf::RouteBrowserCreateDialog::GlobalRouteCreated,
           &mUserGlobalRoutes,
           static_cast<void (RouteContainer::*)(const RouteInfo&)>(&RouteContainer::CreateRoute));

   connect(mDockWidget->GetCreateDialog(),
           &wkf::RouteBrowserCreateDialog::GlobalRouteEdited,
           &mUserGlobalRoutes,
           &RouteContainer::EditRoute);

   connect(mDockWidget->GetCreateDialog(),
           &wkf::RouteBrowserCreateDialog::GlobalRouteCreated,
           &mGlobalRoutes,
           static_cast<void (RouteContainer::*)(const RouteInfo&)>(&RouteContainer::CreateRoute));

   connect(mDockWidget->GetCreateDialog(),
           &wkf::RouteBrowserCreateDialog::GlobalRouteEdited,
           &mGlobalRoutes,
           &RouteContainer::EditRoute);

   connect(mDockWidget->GetCreateDialog(),
           &wkf::RouteBrowserCreateDialog::RouteAssignedToPlatform,
           [this](const QString& aPlatformName, const RouteInfo& aRouteInfo)
           {
              auto* platformPtr = vaEnv.GetStandardScenario()->FindPlatformByName(aPlatformName);
              if (platformPtr && !warlock::HasPermissionToControlPlatform(*platformPtr))
              {
                 QMessageBox::warning(nullptr,
                                      "Route Not Modified",
                                      "User does not have permissions to modify platform " + aPlatformName + ".");
              }
              else
              {
                 mSimInterface->AddSimCommand(
                    ut::make_unique<AssignNewRouteCommand>(aPlatformName.toStdString(), aRouteInfo));
              }
           });

   // Route Drawing Connections -----------------------------------------------------------------------------------

   // create new anchor point and route
   connect(&mGlobalRoutes, &RouteContainer::RouteCreated, this, &wkf::RouteBrowserInterface::GlobalRouteCreatedHandler);

   // -------------------------------------------------------------------------------------------------------------
}

void WkPlatformMovement::RouteBrowserInterface::ScenarioAdded(wkf::Scenario* aScenarioPtr)
{
   if (aScenarioPtr != nullptr)
   {
      for (auto& g : mGlobalRoutes.GetRoutes())
      {
         mGlobalRoutes.CreateRoute(g.second);
         HideGlobalRoute(g.second.mName);
      }
   }
}

// Add a sim command that gets the route info and adds an event to select / expand the route in the Route Browser
void WkPlatformMovement::RouteBrowserInterface::SelectRoute(const std::string& aPlatformName)
{
   if (wkfEnv.GetPlatformOfInterest() && wkfEnv.GetPlatformOfInterest()->GetName() == aPlatformName)
   {
      mSimInterface->AddSimCommand(
         ut::make_unique<LocalRouteRequestCommand>(mSimInterface, aPlatformName, LocalRouteRequestCommand::ROUTE_SELECT));
   }
}
