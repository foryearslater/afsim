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

#ifndef SA_DISPLAY_SIM_INTERFACE_HPP
#define SA_DISPLAY_SIM_INTERFACE_HPP

#include <set>
#include <vector>

#include <QString>

#include "SA_DisplayEvents.hpp"
#include "WkSimInterface.hpp"
#include "WsfSA_EntityPerception.hpp"
#include "sa_display/WkfSA_DisplayDataContainer.hpp"

namespace WkSA_Display
{
class SimInterface : public warlock::SimInterfaceT<SA_Event>
{
   Q_OBJECT
public:
   SimInterface(const QString& aPluginName);

   /**
    * Called repeatedly as time continues forward, regardless of if the simulation is running.
    * It collects all of the perceptions from the platforms referenced in mPlatformsOfInterest,
    * and adds the sim event containing the perceptions and groupings to the event queue for
    * retrieval by the main plugin.
    * @param aSimulation  A reference to the simulation.
    */
   void WallClockRead(const WsfSimulation& aSimulation) override;

   /** Called when a platform is initialized.
    * @param aSimTime sim time at time of initialization.
    * @param aPlatform platform being initialized.
    */
   void PlatformInitialized(double aSimTime, WsfPlatform& aPlatform) override;

   /**
    * Does the given platform index reference a platform that has a SituationAwarenessProcessor?
    * @param aIndex  the index of the platform to check for a processor.
    * @return        true if the platform has a perception processor, false otherwise.
    */
   bool HasSituationAwarenessProcessor(size_t aIndex);

   /**
    * Add the platform referenced by the given index to the set of platforms of interest.
    * @param aIndex  The index of the new platform of interest.
    */
   void AddPlatformOfInterest(size_t aIndex);

   /**
    * Removes the platform referenced by the given index from the set of platforms of interest.
    * @param aIndex  The index of the old platform of interest.
    */
   void RemovePlatformOfInterest(size_t aIndex);

   /** Return if the connection for a platform is destroyed.
    * @param aPlatformIndex index of the platform to check.
    */
   bool IsConnectionDestroyed(const size_t aPlatformIndex);

protected:
   /** This slot is triggered when the simulation initializes.
    * @param aSimulation reference to the simulation initializing.
    */
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

private:
   /** Triggers when a platform is deleted.
    * @param aSimTime sim time when the platform was deleted.
    * @param aPlatform reference to the platform.
    */
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;

   /** Populate an EntityPerception container using the wsf equivalent.
    * @param aEntityPerception perception container to fill.
    * @param aData data to fill with.
    */
   void PopulateEntityPerception(wkf::SA_Display::EntityPerception& aEntityPerception, const WsfSA_EntityPerception& aData);

   std::set<size_t> mPlatformsOfInterest; ///< Set of indices of the platforms of interest
   std::set<size_t> mPlatformsWithSAP;    ///< Set of platforms with SituationAwarenessProcessors.
};
} // namespace WkSA_Display
#endif // SA_DISPLAY_SIM_INTERFACE_HPP
