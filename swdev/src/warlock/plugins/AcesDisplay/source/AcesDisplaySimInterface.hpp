// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef ACESDISPLAY_SIM_INTERFACE_HPP
#define ACESDISPLAY_SIM_INTERFACE_HPP

#include <set>
#include <vector>

#include <QString>

#include "AcesDisplayEvents.hpp"
#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WkfPlatform.hpp"
#include "WsfSA_EntityPerception.hpp"
#include "aces/WkfAcesDisplayDataContainer.hpp"

namespace WkAcesDisplay
{
class SimInterface : public warlock::SimInterfaceT<AcesEvent>
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

   /** Set the target platform list for the ACES Display.
    * @param aPlatforms platforms to set.
    */
   void SetTargetPlatforms(const std::map<size_t, std::vector<size_t>>& aPlatforms);

protected:
   /** This slot is triggered when the simulation initializes.
    * @param aSimulation reference to the simulation initializing.
    */
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

private:
   /** Called when a platform is added.
    * @param aSimTime sim time at time of add.
    * @param aPlatform platform being added.
    */
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;

   /** Triggers when a platform is deleted.
    * @param aSimTime sim time when the platform was deleted.
    * @param aPlatform reference to the platform.
    */
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;

   /** Populate an EntityPerception container using the wsf equivalent.
    * @param aEntityPerception perception container to fill.
    * @param aData data to fill with.
    */
   void PopulateEntityPerception(wkf::AcesDisplay::EntityPerception& aEntityPerception,
                                 const WsfSA_EntityPerception&       aData);

   /** Populate engagement data for a platform.
    * @param aTempData data container to fill.
    * @param aPlatform platform to populate data for.
    */
   void PopulateEngagementData(const WsfSimulation&              aSimulation,
                               wkf::AcesDisplay::EngagementData& aData,
                               WsfPlatform&                      aPlatform);

   std::set<size_t>                      mPlatformsOfInterest; ///< Set of indices of the platforms of interest
   std::map<size_t, std::vector<size_t>> mTargetPlatforms;     ///< Map of platforms to their target platforms to gather
                                                               ///< engagement data for.
   UtCallbackHolder mCallbacks;                                ///< Callback list
};
} // namespace WkAcesDisplay
#endif // ACESDISPLAY_SIM_INTERFACE_HPP
