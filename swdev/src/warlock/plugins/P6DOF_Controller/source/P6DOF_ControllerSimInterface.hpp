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

#ifndef P6DOF_CONTROLLERPLUGININTERFACE_HPP
#define P6DOF_CONTROLLERPLUGININTERFACE_HPP

#include <set>

#include <QString>

#include "P6DOF_ControllerDataContainer.hpp"
#include "P6DOF_ControllerSimEvents.hpp"
#include "WsfSensor.hpp"

class WsfLocalTrack;
class WsfTrack;

namespace WkP6DOF_Controller
{
// The WkP6DOF_Controller::SimInterface provides the conduit between the
// Warlock plugin and AFSIM simulation data. It provides thread-safe
// communications using SimEvents and SimCommands.
class SimInterface : public warlock::SimInterfaceT<P6DOF_ControllerEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);

   // This is called to set the local value of the "connected platform"
   void SetConnectedPlatform(const std::string& aPlatformName);

   // This is called to clear the local value of the "connected platform"
   void ReleaseConnectedPlatform();

protected:
   // These slots are executed on the simulation thread to handle input/output

   // This slot is triggered when the simulation initializes
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   // This slot is triggered when a platform is added to the scenario
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;

   // This slot is triggered when a platform is deleted from the scenario
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;

   // This slot is triggered on a wall clock event at a rate defined
   // in warlock::SimEnvironment
   void WallClockRead(const WsfSimulation& aSimulation) override;

private:
   // This is a helper function that gets state data for a radar, if present
   void GetRadarData(const WsfSimulation&                         aSimulation,
                     WsfPlatform*                                 aPlatform,
                     P6DOF_ControllerDataContainer::PlatformData& aPlatData);

   // This is a helper function that gets state data for a RWR, if present
   void GetRWR_Data(const WsfSimulation&                         aSimulation,
                    WsfPlatform*                                 aPlatform,
                    P6DOF_ControllerDataContainer::PlatformData& aPlatData);

   // This is a helper function that gets state data for a IRST, if present
   void GetIRST_Data(const WsfSimulation&                         aSimulation,
                     WsfPlatform*                                 aPlatform,
                     P6DOF_ControllerDataContainer::PlatformData& aPlatData);

   // This is a helper function that gets state data for a FLIR, if present
   void GetFLIR_Data(const WsfSimulation&                         aSimulation,
                     WsfPlatform*                                 aPlatform,
                     P6DOF_ControllerDataContainer::PlatformData& aPlatData);

   // This is a helper function that gets track data for the track manager
   void GetTrackDataTrackManager(const WsfSimulation&                         aSimulation,
                                 WsfPlatform*                                 aPlatform,
                                 P6DOF_ControllerDataContainer::PlatformData& aPlatData);

   // This is a helper function that calculates additional data for tracks
   void CalcTrackDataForTrack(const WsfSimulation&                      aSimulation,
                              WsfPlatform*                              aPlatform,
                              const WsfTrack*                           aTrack,
                              P6DOF_ControllerDataContainer::TrackData& aTrackData);

   // This is a helper function that calculates additional data for tracks
   void CalcTrackDataForTrack(const WsfSimulation&                      aSimulation,
                              WsfPlatform*                              aPlatform,
                              const WsfTrack*                           aTrack,
                              P6DOF_ControllerDataContainer::TrackData& aTrackData,
                              const std::string&                        aAuxData,
                              bool                                      aSupportsAz  = false,
                              bool                                      aSupportsEl  = false,
                              bool                                      aSupportsAlt = false,
                              bool                                      aSupportsHdg = false,
                              bool                                      aSupportsIFF = false);

   // This is a helper function that gets the scan limits for a specified sensor
   void GetScanLimits(WsfSensor* aSensorPtr, double& aMinAz, double& aMaxAz, double& aMinEl, double& aMaxEl);

   // This is a helper function that gets the scan mode number for a specified sensor
   int GetScanMode(WsfSensor* aSensorPtr);

   // This is a helper function that gets the FOV limits for a specified sensor
   void GetFOV_Limits(WsfSensor* aSensorPtr, double& aMinAz, double& aMaxAz, double& aMinEl, double& aMaxEl);

   // This is a helper function that gets the range limits for a specified sensor
   void GetRangeLimits(WsfSensor* aSensorPtr, double& aMinRange, double& aMaxRange);

   // Although these "mirrored" parameters are passed to the DataContainer via SimEvents,
   // they are also required locally within SimInterface, since they are used in
   // conditional data output to SimEvents for any ClockRead (either SimulationClockRead
   // or WallClockRead) function.

   // This is the name of the connected platform that is being controlled/monitored
   std::string mConnectedPlatform;

   // This is a flag that denotes when the connection to a platform has been
   // broken/destroyed
   bool mConnectionDestroyed;
};
} // namespace WkP6DOF_Controller

#endif
