// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WIZ_ROUTE_BROWSER_INTERFACE_HPP
#define WIZ_ROUTE_BROWSER_INTERFACE_HPP

#include "Platform.hpp"
#include "RouteBrowserPrefObject.hpp"
#include "WkfAction.hpp"
#include "WsfPM_Root.hpp"
#include "route/WkfRouteBrowserInterface.hpp"

namespace wizard
{
class ProxyChange;
}

namespace RouteBrowser
{
//! Represents the route building state.
//! @par Mixed Routes
//!   Routes can have four types of waypoints:  position, MGRS, offset, and turn.
//!   Currently, WSF does not allow offset waypoints after the route defines a second position waypoint.
//!   Wizard will abort the creation of a route if this occurs.
enum class BuildStatus
{
   //! GetRouteInfo has not processed any waypoint yet.
   NoPosition,
   //! GetRouteInfo has processed the initial position waypoint.
   InitialPosition,
   //! GetRouteInfo has processed a second position waypoint.
   //! @note When in this state, succeeding waypoints MUST be position waypoints.
   SecondPosition,
   //! GetRouteInfo has processed an offset waypoint succeeding a second position waypoint.
   //! @warning GetRouteInfo will abort if it reaches this state.
   MixedWaypointError
};

class RouteBrowserInterface : public wkf::RouteBrowserInterface
{
public:
   RouteBrowserInterface();

   void SelectRoute(const std::string& aPlatformName) override;

   wkf::RouteBrowserInterface::RouteInfo GetRouteInfo(const std::string& aPlatformName);
   wkf::RouteBrowserInterface::RouteInfo GetGlobalRouteInfo(const std::string& aPlatformName);
   void                                  UpdateAllGlobalRoutes();
   void                                  UpdateGlobalRoute(const WsfPProxyNode& aNode);
   void                                  GlobalRouteCreated(const RouteInfo& aInfo);
   void                                  ToggleGlobalRouteVisibility();

protected:
   void DockGlobalRouteSelectedHandler(const QString& aPrevRouteName,
                                       int            aPrevWaypoint,
                                       const QString& aCurrRouteName,
                                       int            aCurrWaypoint) override{};
   void DockRouteSelectionChangedHandler(const QString& aPrevPlatformName,
                                         const QString& aPrevRouteName,
                                         int            aPrevWaypoint,
                                         const QString& aCurrPlatformName,
                                         const QString& aCurrRouteName,
                                         int            aCurrWaypoint) override;
   void ConstructRoute(const wkf::RouteBrowserInterface::RouteInfo& aRouteInfo, const std::string& aEntityName) override;
   void DeleteRoute(const QString& aRouteName) override;
   void PlatformDeselectedHandler(wkf::Platform* aPlatform) override;
   void PlatformSelectionChanged(const wkf::PlatformList& aSelected, const wkf::PlatformList& aUnselected) override;

private:
   //! Populate the wkf::RouteInfo
   //! @param aRoute is the proxy route from which to get the route information
   //! @param aInfo is the wkf::RouteInfo object to populate
   //! @note This routine will halt prematurely if the route is invalid
   //! @see BuildStatus
   void GetRouteInfo(WsfPM_Route& aRoute, RouteInfo& aInfo);

   //! Methods get proxy waypoint information to populate wkf::WaypointInfo
   //@{

   //! Populate the wkf::WaypointInfo's position
   //! @param aPreviousStatus is the BuildStatus from the previous waypoint
   //! @param aInfo is the wkf::WaypointInfo object to populate
   //! @param aRoute is the wkf::RouteInfo object to which to add aInfo
   //! @param aProxyWaypoint is the proxy waypoint from which to get the position
   //! @retval BuildStatus::NoPosition if the route-building with a offset-only route
   //! @retval BuildStatus::InitialPosition if the route-building with a offset-only route containing an initial
   //! position waypoint
   //! @retval BuildStatus::SecondPosition if the route-building succeeded with a position-only route
   //! @retval BuildStatus::MixedWaypointError if the route-building failed
   BuildStatus GetPositionInfo(const BuildStatus     aPreviousStatus,
                               WaypointInfo&         aInfo,
                               const RouteInfo&      aRoute,
                               const WsfPM_Waypoint& aProxyWaypoint) const noexcept;


   //! Populate the wkf::WaypointInfo's altitude
   //! @param aInfo is the wkf::WaypointInfo object to populate
   //! @param aRoute is the wkf::RouteInfo object to which to add aInfo
   //! @param aProxyWaypoint is the proxy waypoint from which to get the altitude
   void GetAltitudeInfo(WaypointInfo& aInfo, const RouteInfo& aRoute, const WsfPM_Waypoint& aProxyWaypoint) const noexcept;

   //! Populate the wkf::WaypointInfo's altitude reference
   //! @param aInfo is the wkf::WaypointInfo object to populate
   //! @param aRoute is the wkf::RouteInfo object to which to add aInfo
   //! @param aProxyWaypoint is the proxy waypoint from which to get the altitude reference
   void GetAltitudeReferenceInfo(WaypointInfo&         aInfo,
                                 const RouteInfo&      aRoute,
                                 const WsfPM_Waypoint& aProxyWaypoint) const noexcept;

   //! Populate the wkf::WaypointInfo's turn direction
   //! @param aInfo is the wkf::WaypointInfo object to populate
   //! @param aRoute is the wkf::RouteInfo object to which to add aInfo
   //! @param aProxyWaypoint is the proxy waypoint from which to get the turn direction
   void GetTurnDirectionInfo(WaypointInfo& aInfo, const RouteInfo& aRoute, const WsfPM_Waypoint& aProxyWaypoint) const noexcept;

   //! Populate the wkf::WaypointInfo's switch type
   //! @param aInfo is the wkf::WaypointInfo object to populate
   //! @param aRoute is the wkf::RouteInfo object to which to add aInfo
   //! @param aProxyWaypoint is the proxy waypoint from which to get the switch type
   void GetSwitchTypeInfo(WaypointInfo& aInfo, const RouteInfo& aRoute, const WsfPM_Waypoint& aProxyWaypoint) const noexcept;

   //@}

   //! Gets the unitary property from the proxy waypoint to populate the wkf::WaypointInfo
   //! @tparam UNITARY_PROPERTY is the specific unitary property type
   //! @param aRoute is the wkf::RouteInfo object to which to add the wkf::WaypointInfo
   //! @param aProxyWaypoint is the proxy waypoint from which to get the unitary property
   //! @return the unitary property
   template<typename UNITARY_PROPERTY>
   double GetUnitaryInfo(const RouteInfo& aRoute, const UNITARY_PROPERTY& aUnitary) const noexcept
   {
      // To allow the compiler to give a concise error message when the developer uses IsUnset on aUnitary
      static_assert(std::is_base_of<WsfProxy::BasicValue, UNITARY_PROPERTY>::value,
                    "UNITARY_PROPERTY must derive from WsfProxy::BasicValue to use IsUnset");

      if (aUnitary.IsUnset())
      {
         if (aRoute.mWaypoints.isEmpty())
         {
            return wkf::WaypointValueType::cNOT_SET;
         }
         else
         {
            return wkf::WaypointValueType::cUSE_PREVIOUS;
         }
      }
      else
      {
         // To allow the compiler to give a concise error message when the developer uses IsUnset on aUnitary
         static_assert(
            std::is_base_of<WsfProxy::BasicValueT<typename UNITARY_PROPERTY::ContainedBasicType, UNITARY_PROPERTY::cPROXY_TYPE_ID>,
                            UNITARY_PROPERTY>::value,
            "UNITARY_PROPERTY must derive from WsfProxy::BasicValueT to use GetValue.");

         return static_cast<double>(aUnitary.GetValue());
      }
   }

   void ConnectP();
   void TempRouteEdited(const RouteInfo& aRouteInfo, wkf::AttachmentRoute* aRouteAttachmentPtr);
   void EditRoute(const RouteInfo& aRouteInfo);
   void LocalRouteEditDialogRequestedHandler(const QString& aPlatformName);
   void PlatformPropertiesChangedHandler(wkf::Platform* aPlatformPtr);
   void OnProxyAvailable(WsfPProxy* aProxyPtr);
   void OnGlobalRouteListChange(const wizard::ProxyChange& aProxyChange);
   bool PlatformHasRouteMover(const std::string& aPlatformName);

   PrefObject*      mPrefObject            = nullptr;
   wkf::Action*     mViewGlobalRouteAction = nullptr;
   UtCallbackHolder mProxyCallbacks;
   WsfPM_Root       mProxyRoot;

   static constexpr size_t cMAX_DISPLAYED_INDICES =
      300; // 'edit' route index labels are only shown if it has less than this many waypoints
};
} // namespace RouteBrowser

#endif
