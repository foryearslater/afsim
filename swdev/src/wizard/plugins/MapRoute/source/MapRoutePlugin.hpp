// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef MAPROUTEPLUGIN_HPP
#define MAPROUTEPLUGIN_HPP

#include "Plugin.hpp"
#include "Route.hpp"
#include "RouteTerrainQuery.hpp"
#include "UtOptional.hpp"
#include "WkfAction.hpp"
#include "WkfMainWindow.hpp"
#include "orbit/WkfOrbitPrefWidget.hpp"
#include "route/WkfAttachmentRoute.hpp"

class UtoMapProjection;

namespace wizard
{
class Platform;
}

namespace MapRoute
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

   void           ResetOptionStates() override;
   void           SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatformPtr) override;
   Qt::CheckState GetPlatformOptionState(int aOptionId, const wkf::Platform* aPlatformPtr) const override;
   bool           GetVisibility(const wkf::Platform* aPlaformPtr);
   void           PlatformUpdated(const QString& aPlatformPtr) override;
   void           PlatformDeleted(const QString& aPlatformPtr) override;

   void MapProjectionChangedCB(unsigned int aViewerId, const UtoMapProjection* aProjection);
   void AttachmentSelectionChangedCB(vespa::VaViewer*                aViewer,
                                     int                             aSelected,
                                     vespa::VaAttachment*            aAttachment,
                                     const vespa::VaAttachmentSubId& aId);
   void UpdateEpoch(const UtCalendar& aCalendar);
   void OnProxyAvailable(WsfPProxy* aProxyPtr);

   //! Builds a wkf::AttachmentRoute from a (global) proxy route
   //! @param aRoute is the proxy route
   //! @post If the route fails to build, then Wizard marks the corresponding input as invalid.
   void GlobalRouteAdded(WsfPM_Route& aRoute);

   //! Builds a wkf::AttachmentRoute from a (global) proxy route
   //! @param aRoute is the proxy route
   //! @param aRoutePtr is the wkf::AttachmentRoute to build
   //! @retval Route::BuildStatus::NoPosition if the route-building with a offset-only route
   //! @retval Route::BuildStatus::InitialPosition if the route-building with a offset-only route containing an initial
   //! position waypoint
   //! @retval Route::BuildStatus::SecondPosition if the route-building succeeded with a position-only route
   //! @retval Route::BuildStatus::MixedWaypointError if the route-building failed
   Route::BuildStatus GlobalRouteAddWaypoints(const WsfPM_Route& aRoute, wkf::AttachmentRoute* aRoutePtr);

   //! Get the goto index corresponding to the first occurrence of the given goto label
   //! @param aRoute is the proxy route
   //! @param aLabel is the given goto label
   //! @return the goto index corresponding to aLabel
   int  GetWaypointGotoIndex(const WsfPM_Route& aRoute, const std::string& aLabel);
   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;
   void BuildAttachmentContextMenu(QMenu* aMenu, vespa::VaAttachment* aAttachmentPtr, unsigned int aSubPartId) override;
   void BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr) override;
   void AddRouteToPlatformHandler(wizard::Platform* aPlatformPtr);


protected:
private:
   //! Builds the first wkf::Waypoint from the first proxy waypoint
   //! @param aProxyWaypoint is the proxy waypoint
   //! @param aWaypoints is a list of waypoint data to build into the route
   //! @retval Route::BuildStatus::NoPosition if the route-building with a offset-only route
   //! @retval Route::BuildStatus::InitialPosition if the route-building with a offset-only route containing an initial
   //! position waypoint
   //! @retval Route::BuildStatus::SecondPosition if the route-building succeeded with a position-only route
   //! @retval Route::BuildStatus::MixedWaypointError if the route-building failed
   Route::BuildStatus GlobalRouteAddFirstWaypoint(const WsfPM_Waypoint&                   aProxyWaypoint,
                                                  wkf::AttachmentRoute::WaypointDataList& aWaypoints);

   void DeleteWaypointActionHandler(wkf::AttachmentRoute* aRoutePtr, int aWaypointIndex = -1);
   void CreateWaypointHandler(wkf::AttachmentRoute* aRoutePtr, unsigned int aWaypointIdx);
   void EditGlobalRoute(wkf::AttachmentRoute* aRoutePtr);
   void CreateWaypointAtLocationHandler(wkf::AttachmentRoute* aRoutePtr, unsigned int aWaypointIdx);

   void       LineWidthChanged(int aWidth);
   void       PeriodsChanged(unsigned int aPeriods);
   void       TeamColorsChanged();
   void       UpdateGlobalRoute(wkf::AttachmentRoute* aRoutePtr);
   WsfPM_Root mProxyRoot;

   std::map<QString, Route*>    mRouteMap;
   int                          mOptionType;
   std::map<unsigned int, bool> mState;

   vespa::VaCallbackHolder                            mVaCallbacks;
   UtCallbackHolder                                   mProxyAvailableCallback;
   size_t                                             mDtedRegistrationId;
   RouteTerrainQuery                                  mTerrainQuery;
   ut::optional<std::pair<std::string, unsigned int>> mWaypointSelectionRequest;

   //! Used to keep track of the last waypoint that defined the "altitude" attribute
   //! @see GlobalRouteAddWaypoints for usage
   size_t mLastWaypointWithDefinedAltitude{0};

   //! Handles the preferences for orbits
   PluginUiPointer<wkf::OrbitPrefWidget> mOrbitPrefWidgetPtr;
};
} // namespace MapRoute
#endif
