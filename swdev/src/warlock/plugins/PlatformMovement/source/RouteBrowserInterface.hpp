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
#ifndef RouteBrowserInterface_HPP
#define RouteBrowserInterface_HPP

#include <QValidator>

#include "PlatformMovementSimInterface.hpp"
#include "RouteContainer.hpp"
#include "VaCallbackHolder.hpp"
#include "VaEntity.hpp"
#include "WkfScenario.hpp"
#include "route/WkfAttachmentRoute.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

namespace WkPlatformMovement
{
class RouteBrowserInterface : public wkf::RouteBrowserInterface
{
public:
   RouteBrowserInterface(SimInterface* aSimInterace);
   ~RouteBrowserInterface() override = default;

   int RouteComboBoxSelectedHandler(const QString& aRouteName);

   void Clear() override;

   RouteContainer& GetGlobalRoutes() { return mGlobalRoutes; }         // All global routes
   RouteContainer& GetUserGlobalRoutes() { return mUserGlobalRoutes; } // user-created global routes

   virtual void DeleteRoute(const QString& aRouteName) override;

   void HideGlobalRoute(const QString& aGlobalRouteName);

private:
   void Connect();

   void ScenarioAdded(wkf::Scenario* aScenarioPtr) override;

   void SelectRoute(const std::string& aPlatformName) override;

   SimInterface*  mSimInterface;
   RouteContainer mGlobalRoutes;     // All global routes
   RouteContainer mUserGlobalRoutes; // user-created global routes
};
} // namespace WkPlatformMovement
#endif
