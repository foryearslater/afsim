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

#include "TerrainMonitor.hpp"

#include <QTimer>

#include "Environment.hpp"
#include "Platform.hpp"
#include "ProxyWatcher.hpp"
#include "WkfEnvironment.hpp"
#include "WkfObserver.hpp"
#include "WkfResourceManager.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPM_ObjectList.hpp"
#include "WsfPProxyNode.hpp"

namespace
{
const int cINTERVAL = 100;

void DelayedPlatformUpdate(const QString& aPlatformName, size_t aDtedId)
{
   std::pair<float, bool> elevationResult;
   if (wkfEnv.GetResourceManager().ElevationResult(aDtedId, elevationResult))
   {
      // The result is still being processed
      if (!elevationResult.second)
      {
         QTimer::singleShot(cINTERVAL, [aPlatformName, aDtedId]() { DelayedPlatformUpdate(aPlatformName, aDtedId); });
      }
      // The result is successfully calculated and stored off
      else
      {
         wizEnv.PlatformUpdated(aPlatformName);
      }
   }
   // There is no terrain at the relevant position (but platform position still needs to be updated accordingly)
   else
   {
      wizEnv.PlatformUpdated(aPlatformName);
   }
}

void HandlePlatformElevation(const vespa::VaEntity* aPlatformPtr)
{
   if (aPlatformPtr)
   {
      double  lat          = aPlatformPtr->GetPosition().GetLat();
      double  lon          = aPlatformPtr->GetPosition().GetLon();
      QString platformName = QString::fromStdString(aPlatformPtr->GetName());
      // Acquire the appropriate DTED identifier.
      // Use the cDEFAULT_REQUEST_ID for vespa::VaEntities.
      size_t dtedId = wkf::ResourceManager::cDEFAULT_REQUEST_ID;
      if (aPlatformPtr->IsA_TypeOf<wizard::Platform>())
      {
         auto wizPlatPtr = dynamic_cast<const wizard::Platform*>(aPlatformPtr);
         {
            if (wizPlatPtr)
            {
               // This is a wizard::Platform, so use its DTED identifier
               dtedId = wizPlatPtr->GetDtedId();
            }
         }
      }
      wkfEnv.GetResourceManager().ElevationRequest(dtedId, std::pair<double, double>(lat, lon));
      QTimer::singleShot(cINTERVAL, [dtedId, platformName]() { DelayedPlatformUpdate(platformName, dtedId); });
   }
}

void HandleDelayedElevation()
{
   if (vaEnv.GetStandardScenario())
   {
      vespa::VaEntityList platformList;
      vaEnv.GetStandardScenario()->GetEntityList(platformList);
      for (auto platformIt : platformList)
      {
         HandlePlatformElevation(platformIt);
      }
   }
}
} // namespace

wizard::TerrainMonitor::TerrainMonitor()
{
   mCallbacks.Add(ProxyWatcher::ProxyAvailable.Connect(&TerrainMonitor::OnProxyAvailable, this));
   mVaCallbacks.Add(wkf::Observer::PlatformsAdded.Connect(&TerrainMonitor::PlatformsAddedHandler, this));
   mVaCallbacks.Add(wkf::Observer::PlatformPropertiesChanged.Connect(&TerrainMonitor::PropertiesChangedHandler, this));
}

void wizard::TerrainMonitor::OnProxyAvailable(WsfPProxy* aProxyPtr)
{
   mProxyRoot = aProxyPtr;

   WsfPProxyPath dted = (mProxyRoot + "terrain").GetPath();
   mCallbacks.Add(WatchProxy(dted).ChildUpdate.Connect(&TerrainMonitor::TerrainChange, this));

   PopulateResources();
   HandleDelayedElevation();
}

void wizard::TerrainMonitor::PlatformsAddedHandler(const wkf::PlatformList& aPlatforms)
{
   for (const auto& platform : aPlatforms)
   {
      HandlePlatformElevation(dynamic_cast<wizard::Platform*>(platform));
   }
}

void wizard::TerrainMonitor::PropertiesChangedHandler(wkf::Platform* aPlatformPtr)
{
   HandlePlatformElevation(dynamic_cast<wizard::Platform*>(aPlatformPtr));
}

void wizard::TerrainMonitor::TerrainChange(const ProxyChange& aProxyChange)
{
   wkfEnv.GetResourceManager().Reset(nullptr);
   PopulateResources();
   HandleDelayedElevation();
}

void wizard::TerrainMonitor::PopulateResources()
{
   WsfPProxyNode terrainNode = mProxyRoot + "terrain";
   if (terrainNode.IsValid())
   {
      WsfPM_List dtedList(terrainNode + "dted");
      if (dtedList.IsValid())
      {
         size_t listSize = dtedList.size();
         for (size_t i = 0; i < listSize; ++i)
         {
            WsfPProxyNode p = dtedList[i];
            if (p.IsValid())
            {
               WsfPProxyValue l = (p + "level").GetValue();
               int            level;
               l.GetIntegerValue(level);
               WsfPProxyValue v   = (p + "path").GetValue();
               std::string    str = v.ValueToString();
               wkfEnv.GetResourceManager().AddDTED_Source(QString::fromStdString(str), level);
            }
         }
      }
      WsfPM_List rDtedList(terrainNode + "rawDted");
      if (rDtedList.IsValid())
      {
         size_t listSize = rDtedList.size();
         for (size_t i = 0; i < listSize; ++i)
         {
            WsfPProxyNode p = rDtedList[i];
            if (p.IsValid())
            {
               WsfPProxyValue l = (p + "level").GetValue();
               int            level;
               l.GetIntegerValue(level);
               WsfPProxyValue w = (p + "west").GetValue();
               int            west;
               w.GetIntegerValue(west);
               WsfPProxyValue e = (p + "east").GetValue();
               int            east;
               e.GetIntegerValue(east);
               WsfPProxyValue n = (p + "north").GetValue();
               int            north;
               n.GetIntegerValue(north);
               WsfPProxyValue s = (p + "south").GetValue();
               int            south;
               s.GetIntegerValue(south);
               WsfPProxyValue v   = (p + "path").GetValue();
               std::string    str = v.ValueToString();
               wkfEnv.GetResourceManager().AddRawDTED_Source(QString::fromStdString(str), level, south, west, north, east);
            }
         }
      }
      WsfPM_List rGeotiffList(terrainNode + "geotiff");
      if (rGeotiffList.IsValid())
      {
         size_t listSize = rGeotiffList.size();
         for (size_t i = 0; i < listSize; ++i)
         {
            WsfPProxyNode p = rGeotiffList[i];
            if (p.IsValid())
            {
               WsfPProxyValue v   = (p + "path").GetValue();
               std::string    str = v.ValueToString();
               wkfEnv.GetResourceManager().AddGeotiffSource(QString::fromStdString(str));
            }
         }
      }
   }
}
