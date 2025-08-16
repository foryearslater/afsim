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

#include "WkCoreSimEvents.hpp"

#include <QDebug>

#include "VaTimeManager.hpp"
#include "VaViewer.hpp"
#include "WkPlatform.hpp"
#include "WkfEnvironment.hpp"
#include "WkfGeneralPrefObject.hpp"
#include "WkfObserver.hpp"
#include "WkfScenario.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

void warlock::PlatformInitializedEvent::Process()
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      Platform* platformPtr = dynamic_cast<Platform*>(scenarioPtr->FindPlatformByIndex(mPlatformIndex));
      if (platformPtr)
      {
         platformPtr->SetPositionOrientation(mLocationWCS, mOrientationWCS[0], mOrientationWCS[1], mOrientationWCS[2]);
         platformPtr->SetVelocityWCS(mVelocityWCS);

         platformPtr->SetDisEntityId(mDisEntityId);

         platformPtr->SetSensorNames(mSensorNames);
         platformPtr->SetWeaponNames(mWeaponNames);
         platformPtr->SetCommNames(mCommNames);
      }
      else
      {
         qDebug() << "Failed to find platform in PlatformInitializedEvent";
      }
   }
}

void warlock::PlatformDeletedEvent::Process()
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      Platform* platformPtr = dynamic_cast<Platform*>(scenarioPtr->FindPlatformByIndex(mPlatformIndex));
      if (platformPtr)
      {
         platformPtr->SetDeleted(mSimTime);
      }
      else
      {
         qDebug() << "Failed to find platform in PlatformDeletedEvent";
      }
   }
}

void warlock::PlatformBrokenEvent::Process()
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      Platform* platformPtr = dynamic_cast<Platform*>(scenarioPtr->FindPlatformByIndex(mPlatformIndex));
      if (platformPtr)
      {
         platformPtr->SetBroken(mSimTime);
      }
   }
}

void warlock::PlatformUpdateEvent::Process()
{
   vaEnv.GetTime()->SetMaxTime(mSimTime);
   vaEnv.GetTime()->SetTime(mSimTime);

   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      for (auto& pps : mPlatformMap)
      {
         unsigned int index       = pps.first;
         Platform*    platformPtr = dynamic_cast<Platform*>(scenarioPtr->FindPlatformByIndex(index));
         if (platformPtr)
         {
            PlatformProxy& pp = pps.second;

            if (!platformPtr->IsBeingDragged())
            {
               platformPtr->SetPositionOrientation(pp.mLocationWCS,
                                                   pp.mOrientationWCS[0],
                                                   pp.mOrientationWCS[1],
                                                   pp.mOrientationWCS[2]);
               platformPtr->SetVelocityWCS(pp.mVelocityWCS);
               for (auto&& subpart : pp.mSubpart)
               {
                  platformPtr->UpdateSubobject(subpart.first, subpart.second.mLocationECS, subpart.second.mOrientationECS);
                  wkf::Observer::SubpartUpdated(platformPtr, subpart.first);
               } // update parts
               platformPtr->SetUpdateTime(pp.mUpdateTime);
            }

            platformPtr->SetXIO_Controlled(pp.mXIO_Controlled);
            if (pp.mBrokenTime >= 0.0)
            {
               platformPtr->SetBroken(pp.mBrokenTime);
            }
            if (pp.mDeletionTime >= 0.0)
            {
               platformPtr->SetDeleted(pp.mDeletionTime);
            }
         }
      }
   }
}

void warlock::SimulationInitializingEvent::Process()
{
   vaEnv.ResetStandardScenario();
   for (auto&& dtedsrc : mDTED)
   {
      if (dtedsrc.mRaw)
      {
         wkfEnv.GetResourceManager().AddRawDTED_Source(QString::fromStdString(dtedsrc.mDirName),
                                                       dtedsrc.mLevel,
                                                       dtedsrc.mSWLat,
                                                       dtedsrc.mSWLon,
                                                       dtedsrc.mNELat,
                                                       dtedsrc.mNELon);
      }
      else
      {
         wkfEnv.GetResourceManager().AddDTED_Source(QString::fromStdString(dtedsrc.mDirName), dtedsrc.mLevel);
      }
   }
   for (auto&& geotiffSrc : mGeotiff)
   {
      wkfEnv.GetResourceManager().AddGeotiffSource(QString::fromStdString(geotiffSrc.mDirName));
   }

   auto* generalPrefs = wkfEnv.GetPreferenceObject<wkf::GeneralPrefObject>();
   if (generalPrefs)
   {
      generalPrefs->SetOverlayBannerText(mClassificationString, mClassificationColor);
      generalPrefs->SetSimulationName(mSimulationName);
   }
}

void warlock::SimulationStartingEvent::Process()
{
   vaEnv.SetStartTime(mStartTime);

   wkf::Viewer* viewerPtr = vaEnv.GetStandardViewer();
   if (viewerPtr)
   {
      // If CenterOnStart is not specified (default to true) OR
      // If CenterOnStart is specified and it is true then
      // Center the camera on the scenario.
      if (!viewerPtr->DataExists("CenterOnStart") || viewerPtr->GetData("CenterOnStart").ToBool())
      {
         viewerPtr->CenterCamera(false);
      }
   }
}

void warlock::VisualPartOnEvent::Process()
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      auto platformPtr = scenarioPtr->FindPlatformByIndex(mPlatformIndex);
      if (platformPtr)
      {
         platformPtr->AddSubobject(mSubPartName, mSubpart.mIcon, mSubpart.mLocationECS, mSubpart.mOrientationECS);
         wkf::Observer::SubpartAdded(platformPtr, mSubPartName);
      }
   }
}

void warlock::VisualPartOffEvent::Process()
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      auto platformPtr = scenarioPtr->FindPlatformByIndex(mPlatformIndex);
      if (platformPtr)
      {
         platformPtr->RemoveSubobject(mSubPartName);
         wkf::Observer::SubpartRemoved(platformPtr, mSubPartName);
      }
   }
}

void warlock::RemovePlatformEvent::Process()
{
   vaEnv.DeletePlatformByIndex(mPlatformIndex);
}

void warlock::PlatformBatchEvent::Process()
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      std::map<wkf::Platform*, std::vector<std::string>> platforms;
      for (const auto& platformInfo : mQueuedPlatformAddedEvents)
      {
         size_t               index         = platformInfo.first;
         const PlatformProxy& platformProxy = platformInfo.second;

         Platform* platformPtr = new Platform(platformProxy.mName);

         platformPtr->SetIcon(platformProxy.mIcon);
         platformPtr->SetSide(platformProxy.mSide);
         platformPtr->SetIndex(index);
         platformPtr->SetSpatialDomain(platformProxy.mSpatialDomain);
         platformPtr->SetTypeList(platformProxy.mTypeList);
         platformPtr->SetExternallyControlled(platformProxy.mExternallyControlled);

         platformPtr->SetUpdateTime(platformProxy.mUpdateTime);

         if (!scenarioPtr->AddPlatformWithIndex(platformPtr, index))
         {
            qDebug() << "Failed to Add Platform in PlatformAddedEvent";
         }
         platformPtr->Initialize();

         // Set platform categories for option filtering
         wkfEnv.ClearPlatformGroupings(index);
         wkfEnv.SetPlatformGrouping(index, wkf::PlatformGrouping(wkf::PlatformGrouping::eSide, platformProxy.mSide));
         for (const std::string& type : platformProxy.mTypeList)
         {
            wkfEnv.SetPlatformGrouping(index, wkf::PlatformGrouping(wkf::PlatformGrouping::eType, type));
         }
         for (const std::string& cat : platformProxy.mCategoryList)
         {
            wkfEnv.SetPlatformGrouping(index, wkf::PlatformGrouping(wkf::PlatformGrouping::eCategory, cat));
         }

         platforms[platformPtr] = std::vector<std::string>();

         // add the sub-objects
         for (auto& so : platformProxy.mSubpart)
         {
            platformPtr->AddSubobject(so.first, so.second.mIcon, so.second.mLocationECS, so.second.mOrientationECS);
            platforms[platformPtr].push_back(so.first);
         }
      }

      wkf::PlatformList platformList;
      for (auto& platform : platforms)
      {
         platformList.push_back(platform.first);
      }
      if (!platformList.empty())
      {
         wkf::Observer::PlatformsAdded(platformList);
      }

      for (auto& platform : platforms)
      {
         wkf::Observer::PlatformConfigured(platform.first);

         for (auto& subObject : platform.second)
         {
            wkf::Observer::SubpartAdded(platform.first, subObject);
         }
      }

      for (const auto& simEvent : mQueuedPlatformInitializedEvents)
      {
         simEvent->Process();
      }

      for (const auto& simEvent : mQueuedPlatformDeletedEvents)
      {
         simEvent->Process();
      }
   }
}
