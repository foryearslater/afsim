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

#ifndef WKCORESIMEVENTS_HPP
#define WKCORESIMEVENTS_HPP

#include <set>
#include <string>

#include "DisEntityId.hpp"
#include "UtCalendar.hpp"
#include "UtColor.hpp"
#include "WkSimInterface.hpp"
#include "WkfDefines.hpp"
#include "WsfTerrain.hpp"

namespace warlock
{

//*******************************************
// Data Structure
struct PlatformProxy
{
   PlatformProxy()
      : mName()
      , mIcon()
      , mSide()
      , mUpdateTime(0.0)
      , mLocationWCS{0, 0, 0}
      , mVelocityWCS{0, 0, 0}
      , mAccelerationWCS{0, 0, 0}
      , mOrientationWCS{0, 0, 0}
      , mExternallyControlled(false)
      , mXIO_Controlled(false)
      , mBrokenTime(-1.0)
      , mDeletionTime(-1.0)
   {
   }

   struct Subpart
   {
      Subpart()
         : mIcon()
         , mLocationECS{0.0, 0.0, 0.0}
         , mOrientationECS{0.0, 0.0, 0.0}
      {
      }

      std::string mIcon;
      double      mLocationECS[3];
      double      mOrientationECS[3];
   };
   std::string              mName;
   std::string              mIcon;
   std::string              mSide;
   wkf::SpatialDomain       mSpatialDomain;
   std::vector<std::string> mTypeList;
   std::vector<std::string> mCategoryList;
   double                   mUpdateTime;
   double                   mLocationWCS[3];
   double                   mVelocityWCS[3];
   double                   mAccelerationWCS[3];
   double                   mOrientationWCS[3];
   bool                     mExternallyControlled;
   bool                     mXIO_Controlled;
   double mBrokenTime;   // Platform is broken (i.e. damage factor is 1.0) but not removed from simulation
   double mDeletionTime; // Platform is scheduled for deletion
   std::map<std::string, Subpart> mSubpart;
};

using PlatformMap = std::map<size_t, PlatformProxy>; // Key is platform index

//*******************************************
// SimEvents
class CoreSimEvent : public SimEvent
{
public:
   CoreSimEvent(bool aRecurring = false)
      : SimEvent(aRecurring)
   {
   }
   virtual void Process() = 0;
};

class PlatformInitializedEvent : public CoreSimEvent
{
public:
   PlatformInitializedEvent(size_t                       aPlatformIndex,
                            const double                 aLocationWCS[3],
                            const double                 aOrientationWCS[3],
                            const double                 aVelocityWCS[3],
                            const DisEntityId            aDisEntityId,
                            const std::set<std::string>& aSensorNames,
                            const std::set<std::string>& aWeaponNames,
                            const std::set<std::string>& aCommNames)
      : CoreSimEvent()
      , mPlatformIndex(aPlatformIndex)
      , mLocationWCS{aLocationWCS[0], aLocationWCS[1], aLocationWCS[2]}
      , mOrientationWCS{aOrientationWCS[0], aOrientationWCS[1], aOrientationWCS[2]}
      , mVelocityWCS{aVelocityWCS[0], aVelocityWCS[1], aVelocityWCS[2]}
      , mDisEntityId{aDisEntityId}
      , mSensorNames{aSensorNames}
      , mWeaponNames{aWeaponNames}
      , mCommNames{aCommNames}
   {
   }

   void Process() override;

private:
   size_t mPlatformIndex;
   double mLocationWCS[3];
   double mOrientationWCS[3];
   double mVelocityWCS[3];

   DisEntityId mDisEntityId;

   std::set<std::string> mSensorNames;
   std::set<std::string> mWeaponNames;
   std::set<std::string> mCommNames;
};

// This occurs when a platform is deleted in the simulation
//  and it informs the GUI at what time the platform was delete.
// It does not remove the platform, RemovePlatformEvent
//  is responsible for removing the platform.
class PlatformDeletedEvent : public CoreSimEvent
{
public:
   PlatformDeletedEvent(size_t aPlatformIndex, double aSimTime)
      : CoreSimEvent()
      , mPlatformIndex(aPlatformIndex)
      , mSimTime(aSimTime)
   {
   }

   void Process() override;

private:
   size_t mPlatformIndex;
   double mSimTime;
};

class PlatformBatchEvent : public CoreSimEvent
{
public:
   PlatformBatchEvent(PlatformMap                                            aQueuedPlatformAddedEvents,
                      std::vector<std::unique_ptr<PlatformInitializedEvent>> aQueuedPlatformInitializedEvents,
                      std::vector<std::unique_ptr<PlatformDeletedEvent>>     aQueuedPlatformDeletedEvents)
      : CoreSimEvent()
      , mQueuedPlatformAddedEvents(std::move(aQueuedPlatformAddedEvents))
      , mQueuedPlatformInitializedEvents(std::move(aQueuedPlatformInitializedEvents))
      , mQueuedPlatformDeletedEvents(std::move(aQueuedPlatformDeletedEvents))
   {
   }

   void Process() override;

private:
   PlatformMap                                            mQueuedPlatformAddedEvents;
   std::vector<std::unique_ptr<PlatformInitializedEvent>> mQueuedPlatformInitializedEvents;
   std::vector<std::unique_ptr<PlatformDeletedEvent>>     mQueuedPlatformDeletedEvents;
};

// This event will remove a platform from the scenario.
//  It occurs five seconds of after the PlatformDeletedEvent
//  and will only trigger if the simulation is running.
//  If paused the event is rescheduled and will check once
//  a second until the simulation is running.
class RemovePlatformEvent : public CoreSimEvent
{
public:
   RemovePlatformEvent(size_t aPlatformIndex)
      : CoreSimEvent()
      , mPlatformIndex(aPlatformIndex)
   {
   }

   void Process() override;

private:
   size_t mPlatformIndex;
};

class PlatformBrokenEvent : public CoreSimEvent
{
public:
   PlatformBrokenEvent(size_t aPlatformIndex, double aSimTime)
      : CoreSimEvent()
      , mPlatformIndex(aPlatformIndex)
      , mSimTime(aSimTime)
   {
   }

   void Process() override;

private:
   size_t mPlatformIndex;
   double mSimTime;
};

class PlatformUpdateEvent : public CoreSimEvent
{
public:
   PlatformUpdateEvent(double aSimTime, const PlatformMap& aPlatformMap)
      : CoreSimEvent(true)
      , mSimTime(aSimTime)
      , mPlatformMap(aPlatformMap)
   {
   }

   void Process() override;

private:
   double      mSimTime;
   PlatformMap mPlatformMap;
};

class VisualPartOnEvent : public CoreSimEvent
{
public:
   VisualPartOnEvent(size_t aPlatformIndex, const std::string& aSubPartName, const PlatformProxy::Subpart& aSubpart)
      : CoreSimEvent()
      , mPlatformIndex(aPlatformIndex)
      , mSubPartName(aSubPartName)
      , mSubpart(aSubpart)
   {
   }

   void Process() override;

private:
   size_t                 mPlatformIndex;
   std::string            mSubPartName;
   PlatformProxy::Subpart mSubpart;
};

class VisualPartOffEvent : public CoreSimEvent
{
public:
   VisualPartOffEvent(size_t aPlatformIndex, const std::string& aSubPartName)
      : CoreSimEvent()
      , mPlatformIndex(aPlatformIndex)
      , mSubPartName(aSubPartName)
   {
   }

   void Process() override;

private:
   size_t      mPlatformIndex;
   std::string mSubPartName;
};

class SimulationInitializingEvent : public CoreSimEvent
{
public:
   SimulationInitializingEvent(std::vector<wsf::TerrainInterface::DtedInput>    aDTED,
                               std::vector<wsf::TerrainInterface::GeotiffInput> aGeotiff,
                               std::string                                      aSimulationName,
                               std::string                                      aClassification,
                               const UtColor&                                   aClassificationColor)
      : CoreSimEvent()
      , mDTED(std::move(aDTED))
      , mGeotiff(std::move(aGeotiff))
      , mSimulationName(std::move(aSimulationName))
      , mClassificationString(std::move(aClassification))
      , mClassificationColor(aClassificationColor)
   {
   }

   void Process() override;

private:
   std::vector<wsf::TerrainInterface::DtedInput>    mDTED;
   std::vector<wsf::TerrainInterface::GeotiffInput> mGeotiff;
   std::string                                      mSimulationName;
   std::string                                      mClassificationString;
   UtColor                                          mClassificationColor;
};

class SimulationStartingEvent : public CoreSimEvent
{
public:
   SimulationStartingEvent(const UtCalendar& aStartTime)
      : CoreSimEvent()
      , mStartTime(aStartTime)
   {
   }

   void Process() override;

private:
   UtCalendar mStartTime;
};

} // namespace warlock

#endif
