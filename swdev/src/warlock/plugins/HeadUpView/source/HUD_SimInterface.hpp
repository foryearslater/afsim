// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef HUD_SIMINTERFACE_HPP
#define HUD_SIMINTERFACE_HPP

#include <set>

#include <QString>

#include "HUD_SimEvents.hpp"

class WsfSensor;
class WsfTrack;

namespace HeadsUpDisplay
{
class SimInterface : public warlock::SimInterfaceT<WkHeadsUpDisplayEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);
   ~SimInterface();

   /** Called when a platform is initialized.
    * @param aSimTime sim time at time of initialization.
    * @param aPlatform platform being initialized.
    */
   void PlatformInitialized(double aSimTime, WsfPlatform& aPlatform) override;

   /** Add a platform to the connected platform map.
    * @param aPlatformName name of the platform being added.
    */
   void AddConnectedPlatform(const std::string& aPlatformName);

   /** Remove a platform from the connected platform map.
    * @param aPlatformName name of the platform being removed.
    */
   void ReleaseConnectedPlatform(const std::string& aPlatformName);

   /** Return if the connection for a platform is destroyed.
    * @param aPlatformName name of the platform to check.
    */
   bool IsConnectionDestroyed(const std::string& aPlatformName);

   /**
    * Does the given platform index reference a platform that has a SituationAwarenessProcessor?
    * @param aIndex  the index of the platform to check for a processor.
    * @return        true if the platform has a SA processor, false otherwise.
    */
   bool HasSituationAwarenessProcessor(size_t aIndex);

protected:
   /** This slot is triggered when the simulation initializes.
    * @param aSimulation reference to the simulation initializing.
    */
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   /** This slot is triggered when a platform is deleted from the scenario.
    * @param aSimTime time the platform was deleted.
    * @param aPlatform reference to the platform being deleted.
    */
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;

   /** This slot is triggered on a wall clock event at a rate defined in warlock::SimEnvironment.
    * @param aSimulation being run.
    */
   void WallClockRead(const WsfSimulation& aSimulation) override;

private:
   /** This is a helper function that gets state data for a PlatformData.
    * @param aSimulation simulation being run.
    * @param aPlatform platform to read from.
    * @param aSimTime current simulation time.
    * @param aPlatData container to fill.
    */
   void ReadPlatformData(const WsfSimulation&                  aSimulation,
                         WsfPlatform*                          aPlatform,
                         double                                aSimTime,
                         wkf::HUD_DataContainer::PlatformData& aPlatformData);

   std::map<std::string, bool> mConnectedPlatforms; ///< Map of platforms names to their connection status (connected/destroyed)
   std::set<size_t>            mPlatformsWithSAP; ///< Set of platforms with SituationAwarenessProcessors.
};
} // namespace HeadsUpDisplay

#endif // HUD_SIMINTERFACE_HPP
