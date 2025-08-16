// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "RouteTerrainQuery.hpp"

#include <QTimer>

#include "Environment.hpp"
#include "ProxyWatcher.hpp"
#include "WkfEnvironment.hpp"

namespace
{
const int cINTERVAL = 100;
}

double MapRoute::RouteTerrainQuery::GetElevationData(double aLat, double aLon)
{
   // Make sure the clean-up method is attached to the wizard::ProxyWatcher
   if (mCallbacks.Empty())
   {
      WsfPM_Root    proxyRoot(wizard::ProxyWatcher::GetActiveProxy());
      WsfPProxyNode terrainNode = proxyRoot + "terrain";
      auto          pathStr     = terrainNode.GetPathString();
      mCallbacks.Add(
         wizard::WatchProxy(terrainNode.GetPath()).AnyUpdate.Connect(&RouteTerrainQuery::TerrainProxyModified, this));
   }
   // Check for the elevation/ground level in the cache
   std::pair<double, double> latLonPair{aLat, aLon};
   auto                      cacheIter = mWaypointCache.find(latLonPair);
   if (cacheIter != mWaypointCache.end())
   {
      return cacheIter->second;
   }

   // Check for a current request for this lat/lon
   size_t requestId;
   auto   requestIter = mWaypointRequestCache.find(latLonPair);
   if (requestIter != mWaypointRequestCache.end())
   {
      requestId = requestIter->second;
   }
   else
   {
      requestId                         = wkfEnv.GetResourceManager().Register();
      mWaypointRequestCache[latLonPair] = requestId;
   }

   // Kick off the acquisition of the elevation/ground level from the wkf::ResourceManager.
   // This will periodically poll the wkf::ResourceManager for the data.
   // Once the wkf::ResourceManger has the data, store the data in the cache.
   // Future requests will get the data from the cache, rather than the wkf::ResourceManager.
   float elevationResult = 0;
   if (!wkfEnv.GetResourceManager().QueryElevation(requestId, aLat, aLon, elevationResult))
   {
      wkfEnv.GetResourceManager().ElevationRequest(requestId, latLonPair);
      QTimer::singleShot(cINTERVAL, [this, requestId]() { DelayedRouteUpdate(requestId); });
      elevationResult = 0;
   }
   else
   {
      mWaypointCache[latLonPair] = elevationResult;
   }

   return elevationResult;
}

void MapRoute::RouteTerrainQuery::DelayedRouteUpdate(size_t aId)
{
   std::pair<float, bool> elevationResult;
   if (wkfEnv.GetResourceManager().ElevationResult(aId, elevationResult))
   {
      // The result is still being processed.
      if (!elevationResult.second)
      {
         QTimer::singleShot(cINTERVAL, [this, aId]() { DelayedRouteUpdate(aId); });
      }
      // The result is successfully calculated and stored off.
      else
      {
         // Search the mWaypointRequestCache to find the request and its lat/lon.
         // Using that lat/lon, add the found elevation/ground level to mWaypointCache.
         for (const auto& curRequest : mWaypointRequestCache)
         {
            if (curRequest.second == aId)
            {
               std::pair<double, double> latLonPair = curRequest.first;
               mWaypointCache[latLonPair]           = elevationResult.first;
               break;
            }
         }
         wizEnv.PlatformUpdated(mPlatformName);
      }
   }
}

double MapRoute::RouteTerrainQuery::GetWaypointAltitude(const WsfPM_Waypoint& aWaypoint)
{
   double altitudeVal{aWaypoint.Altitude().GetValue()};
   auto   altRef = aWaypoint.AltitudeReferenceValue();
   // Accumulate the elevation/ground level
   if (altRef == wsf::proxy::AltitudeReferenceEnum::AGL)
   {
      altitudeVal += GetElevationData(aWaypoint.Position().GetLatitude(), aWaypoint.Position().GetLongitude());
   }
   return altitudeVal;
}

void MapRoute::RouteTerrainQuery::TerrainProxyModified(const wizard::ProxyChange& aProxyChange)
{
   // The terrain block changed, so clear the caches
   mWaypointCache.clear();
   mWaypointRequestCache.clear();
}
