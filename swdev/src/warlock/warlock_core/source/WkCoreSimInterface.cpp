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
#include "WkCoreSimInterface.hpp"

#include <set>

#include "UtMemory.hpp"
#include "WkCoreSimCommands.hpp"
#include "WkCoreSimEvents.hpp"
#include "WkPermissions.hpp"
#include "WkXIO_DataContainer.hpp"
#include "WsfClockSource.hpp"
#include "WsfComm.hpp"
#include "WsfDateTime.hpp"
#include "WsfPlatform.hpp"
#include "WsfScenario.hpp"
#include "WsfSensor.hpp"
#include "WsfSimulation.hpp"
#include "WsfTerrain.hpp"
#include "WsfVisualPart.hpp"
#include "WsfWeapon.hpp"
#include "dis/WsfDisInterface.hpp"
#include "dis/WsfDisPlatform.hpp"
#include "xio/WsfXIO_Connection.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio/WsfXIO_RemotePlatformList.hpp"
#include "xio_sim/WsfXIO_Extension.hpp"

warlock::CoreSimInterface::CoreSimInterface(XIO_DataContainer& aXIO_Data)
   : mXIO_Data(aXIO_Data)
{
}

void warlock::CoreSimInterface::PlatformBroken(double aSimTime, const WsfPlatform& aPlatform)
{
   AddSimEvent(ut::make_unique<PlatformBrokenEvent>(aPlatform.GetIndex(), aSimTime));
}

void warlock::CoreSimInterface::VisualPartTurnedOn(double aSimTime, const WsfVisualPart& aVisualPart)
{
   PlatformProxy::Subpart subpart;
   subpart.mIcon = aVisualPart.GetIcon();
   aVisualPart.GetLocation(subpart.mLocationECS);
   aVisualPart.GetOrientation(subpart.mOrientationECS[0], subpart.mOrientationECS[1], subpart.mOrientationECS[2]);

   AddSimEvent(ut::make_unique<VisualPartOnEvent>(aVisualPart.GetPlatform()->GetIndex(), aVisualPart.GetName(), subpart));
}

void warlock::CoreSimInterface::VisualPartTurnedOff(double aSimTime, const WsfVisualPart& aVisualPart)
{
   AddSimEvent(ut::make_unique<VisualPartOffEvent>(aVisualPart.GetPlatform()->GetIndex(), aVisualPart.GetName()));
}

void warlock::CoreSimInterface::VisualPartStateChanged(double aSimTime, const WsfVisualPart& aVisualPart)
{
   if (aVisualPart.IsTurnedOn())
   {
      VisualPartTurnedOn(aSimTime, aVisualPart);
   }
   else
   {
      VisualPartTurnedOff(aSimTime, aVisualPart);
   }
}

void warlock::CoreSimInterface::PlatformAdded(double aSimTime, WsfPlatform& aPlatform)
{
   PlatformProxy pp;
   // We assume the data saved here will not change over the course of the simulation.
   // If that assumption doesn't hold, we need to move this code to WallClockRead (PlatformUpdateEvent).
   pp.mName = aPlatform.GetNameId();
   pp.mIcon = aPlatform.GetIconId();
   pp.mSide = aPlatform.GetSideId();
   switch (aPlatform.GetSpatialDomain())
   {
   case (WSF_SPATIAL_DOMAIN_AIR):
      pp.mSpatialDomain = wkf::SPATIAL_DOMAIN_AIR;
      break;
   case (WSF_SPATIAL_DOMAIN_LAND):
      pp.mSpatialDomain = wkf::SPATIAL_DOMAIN_LAND;
      break;
   case (WSF_SPATIAL_DOMAIN_SPACE):
      pp.mSpatialDomain = wkf::SPATIAL_DOMAIN_SPACE;
      break;
   case (WSF_SPATIAL_DOMAIN_SURFACE):
      pp.mSpatialDomain = wkf::SPATIAL_DOMAIN_SURFACE;
      break;
   case (WSF_SPATIAL_DOMAIN_SUBSURFACE):
      pp.mSpatialDomain = wkf::SPATIAL_DOMAIN_SUBSURFACE;
      break;
   default:
      break;
   }

   // Must convert to string as it is not thread safe to look up WsfStringId in the WsfDictionary
   for (const auto& type : aPlatform.GetTypeList())
   {
      pp.mTypeList.emplace_back(type);
   }
   // Must convert to string as it is not thread safe to look up WsfStringId in the WsfDictionary
   for (const auto& type : aPlatform.GetCategories().GetCategoryList())
   {
      pp.mCategoryList.emplace_back(type);
   }

   pp.mExternallyControlled = aPlatform.IsExternallyControlled();

   // collect all of the visual subparts, as currently implemented these do not update position or orientation
   // they may be removed, but not added
   unsigned int vpCount = aPlatform.GetComponentCount<WsfVisualPart>();
   for (unsigned int vi = 0; vi < vpCount; ++vi)
   {
      WsfVisualPart* vp = aPlatform.GetComponentEntry<WsfVisualPart>(vi);
      if (!vp->GetIcon().empty())
      {
         pp.mSubpart[vp->GetName()].mIcon = vp->GetIcon();
         vp->GetLocation(pp.mSubpart[vp->GetName()].mLocationECS);
         double ypr[3];
         vp->GetOrientation(ypr[0], ypr[1], ypr[2]);
         pp.mSubpart[vp->GetName()].mOrientationECS[0] = ypr[0];
         pp.mSubpart[vp->GetName()].mOrientationECS[1] = ypr[1];
         pp.mSubpart[vp->GetName()].mOrientationECS[2] = ypr[2];
      }
   }

   mQueuedPlatformAddedData[aPlatform.GetIndex()] = pp;
}

void warlock::CoreSimInterface::PlatformInitialized(double aSimTime, WsfPlatform& aPlatform)
{
   double location[3], orientation[3], velocity[3];

   std::set<std::string> sensorNames;
   std::set<std::string> weaponNames;
   std::set<std::string> commNames;

   // Get initial state information
   aPlatform.GetLocationWCS(location);
   aPlatform.GetOrientationWCS(orientation[0], orientation[1], orientation[2]);
   aPlatform.GetVelocityWCS(velocity);

   DisEntityId disId;
   auto        disInterface = WsfDisInterface::Find(*aPlatform.GetSimulation());
   if (disInterface)
   {
      disInterface->GetEntityId(&aPlatform, disId);
   }

   auto sensorCount = aPlatform.GetComponentCount<WsfSensor>();
   auto weaponCount = aPlatform.GetComponentCount<WsfWeapon>();
   auto commCount   = aPlatform.GetComponentCount<wsf::comm::Comm>();

   for (unsigned int i = 0; i < sensorCount; i++)
   {
      sensorNames.insert(aPlatform.GetComponentEntry<WsfSensor>(i)->GetName());
   }

   for (unsigned int i = 0; i < weaponCount; i++)
   {
      weaponNames.insert(aPlatform.GetComponentEntry<WsfWeapon>(i)->GetName());
   }

   for (unsigned int i = 0; i < commCount; i++)
   {
      commNames.insert(aPlatform.GetComponentEntry<wsf::comm::Comm>(i)->GetName());
   }

   mQueuedPlatformInitializedEvents.emplace_back(ut::make_unique<PlatformInitializedEvent>(aPlatform.GetIndex(),
                                                                                           location,
                                                                                           orientation,
                                                                                           velocity,
                                                                                           disId,
                                                                                           sensorNames,
                                                                                           weaponNames,
                                                                                           commNames));
}

void warlock::CoreSimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   mQueuedPlatformDeletedEvents.emplace_back(new PlatformDeletedEvent(aPlatform.GetIndex(), aSimTime));

   WsfSimulation* simulationPtr = aPlatform.GetSimulation();
   simulationPtr->AddWallEvent(
      ut::make_unique<RemovePlatformWsfEvent>(this, simulationPtr->GetWallTime() + 5.0, aPlatform.GetIndex()));
}

void warlock::CoreSimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   mQueuedPlatformAddedData.clear();
   mQueuedPlatformInitializedEvents.clear();
   mQueuedPlatformDeletedEvents.clear();
   mXIO_Data.Clear();

   AddSimEvent(ut::make_unique<SimulationInitializingEvent>(aSimulation.GetTerrainInterface()->GetDTEDInputs(),
                                                            aSimulation.GetTerrainInterface()->GetGeotiffInputs(),
                                                            aSimulation.GetScenario().GetSimulationName(),
                                                            aSimulation.GetScenario().GetClassificationString(),
                                                            aSimulation.GetScenario().GetClassificationColor()));

   // Create an empty remote platform list to allow plugins to subscribe to callbacks prior to any remote simulations being added
   WsfDisInterface* disInterface = WsfDisInterface::Find(aSimulation);
   if (disInterface)
   {
      WsfXIO_Interface* xio = WsfXIO_Extension::Find(aSimulation);
      if (xio)
      {
         mXIO_Data.Initialize(xio);
      }
   }

   mCallbacks.Add(WsfObserver::AdvanceTime(&aSimulation).Connect([this](double aSimTime) { CheckForSendingBatchEvent(); }));
}

void warlock::CoreSimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   // Explicitly call the read function when the simulation is starting
   WallClockRead(aSimulation);

   CheckForSendingBatchEvent();

   AddSimEvent(ut::make_unique<SimulationStartingEvent>(aSimulation.GetDateTime().GetStartDateAndTime()));

   // Connect the callbacks for XIO
   WsfXIO_Interface* xio = WsfXIO_Extension::Find(aSimulation);
   if (xio)
   {
      mCallbacks.Add(xio->OnConnected.Connect([&](WsfXIO_Connection* connection)
                                              { mXIO_Data.AddConnection(connection, aSimulation.GetClockSource()); }));

      mCallbacks.Add(xio->OnDisconnected.Connect(&XIO_DataContainer::DropConnection, &mXIO_Data));
   }
}

void warlock::CoreSimInterface::SimulationComplete(const WsfSimulation& aSimulation)
{
   mCallbacks.Clear();
   mQueuedPlatformAddedData.clear();
   mQueuedPlatformInitializedEvents.clear();
   mQueuedPlatformDeletedEvents.clear();
   mXIO_Data.Clear();
   WsfAttributeContainer::ClearTypes();
}

void warlock::CoreSimInterface::WallClockRead(const WsfSimulation& aSimulation)
{
   PlatformMap platforms;

   WsfDisInterface* disInterface = WsfDisInterface::Find(aSimulation);

   for (size_t i = 0; i < aSimulation.GetPlatformCount(); ++i)
   {
      // platformPtr should always be valid, no need to check
      WsfPlatform* platformPtr = aSimulation.GetPlatformEntry(i);

      // Call Update so that the data is updated to the current time
      platformPtr->Update(aSimulation.GetSimTime());

      // Create the proxy entry
      PlatformProxy& proxy = platforms[platformPtr->GetIndex()];

      proxy.mUpdateTime = aSimulation.GetSimTime();
      platformPtr->GetLocationWCS(proxy.mLocationWCS);
      platformPtr->GetVelocityWCS(proxy.mVelocityWCS);
      platformPtr->GetAccelerationWCS(proxy.mAccelerationWCS);
      platformPtr->GetOrientationWCS(proxy.mOrientationWCS[0], proxy.mOrientationWCS[1], proxy.mOrientationWCS[2]);

      unsigned int vpCount = platformPtr->GetComponentCount<WsfVisualPart>();
      for (unsigned int vi = 0; vi < vpCount; ++vi)
      {
         WsfVisualPart* vp = platformPtr->GetComponentEntry<WsfVisualPart>(vi);
         if (vp != nullptr)
         {
            vp->GetLocation(proxy.mSubpart[vp->GetName()].mLocationECS);
            vp->GetOrientation(proxy.mSubpart[vp->GetName()].mOrientationECS[0],
                               proxy.mSubpart[vp->GetName()].mOrientationECS[1],
                               proxy.mSubpart[vp->GetName()].mOrientationECS[2]);
         }
      }

      // This following information shouldn't change once set.  The reason the information is set in WallClockRead() is
      //  we may receive the DIS platform first and then at some point in the future get the XIO information that the
      //  platform is controllable over XIO.  Thus the quick and dirty method is to check each read update.
      if (disInterface && platformPtr->IsExternallyControlled())
      {
         WsfDisPlatform* disPlatform = disInterface->GetDisPlatform(platformPtr->GetIndex());
         if (disPlatform != nullptr)
         {
            proxy.mExternallyControlled = platformPtr->IsExternallyControlled();
            proxy.mXIO_Controlled       = (mXIO_Data.GetPlatformInfo(platformPtr) != nullptr);
         }
      }
   }

   AddSimEvent(ut::make_unique<PlatformUpdateEvent>(aSimulation.GetSimTime(), platforms));
}

void warlock::CoreSimInterface::CheckForSendingBatchEvent()
{
   if (!mQueuedPlatformAddedData.empty() || !mQueuedPlatformInitializedEvents.empty() ||
       !mQueuedPlatformDeletedEvents.empty())
   {
      AddSimEvent(ut::make_unique<PlatformBatchEvent>(std::move(mQueuedPlatformAddedData),
                                                      std::move(mQueuedPlatformInitializedEvents),
                                                      std::move(mQueuedPlatformDeletedEvents)));
      mQueuedPlatformAddedData.clear();
      mQueuedPlatformInitializedEvents.clear();
      mQueuedPlatformDeletedEvents.clear();
   }
}

void warlock::CoreSimInterface::SendMoveCommand(const wkf::PlatformList& aSelectedPlatforms)
{
   for (const auto& platform : aSelectedPlatforms)
   {
      if (!warlock::HasPermissionToControlPlatform(*platform))
      {
         QMessageBox::warning(nullptr,
                              "Platform Not Moved",
                              "User does not have permissions to modify platform " +
                                 QString::fromStdString(platform->GetName()) + ".");
      }
      else
      {
         const vespa::VaPosition& pos = platform->GetPosition();
         AddSimCommand(
            ut::make_unique<PlatformMovementCommand>(platform->GetName(), pos.GetLat(), pos.GetLon(), pos.GetAlt()));
      }
   }
}

warlock::RemovePlatformWsfEvent::~RemovePlatformWsfEvent()
{
   mSimInterface->AddSimEvent(ut::make_unique<RemovePlatformEvent>(mIndex));
}

WsfEvent::EventDisposition warlock::RemovePlatformWsfEvent::Execute()
{
   // Don't remove platform if simulation is paused. Reschedule and wait until clock resumes.
   if (GetSimulation()->GetClockSource()->IsStopped())
   {
      SetTime(GetTime() + 1.0);
      return WsfEvent::cRESCHEDULE;
   }
   return WsfEvent::cDELETE;
}
