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

#ifndef HDD_SIMINTERFACE_HPP
#define HDD_SIMINTERFACE_HPP

#include <set>

#include <QString>
#include <QVector3D>

#include "WkSimInterface.hpp"

class WsfMover;

#include "HDD_SimEvents.hpp"
#include "UtCallbackHolder.hpp"
#include "hdd/WkfHDD_DataContainer.hpp"

namespace HDD
{
class SimInterface : public warlock::SimInterfaceT<HDDEvent>
{
   Q_OBJECT

public:
   SimInterface(const QString& aPluginName);
   ~SimInterface();

   /** Called when a platform is initialized.
    * @param aSimTime sim time at time of initialization.
    * @param aPlatform platform being initialized.
    */
   void PlatformInitialized(double aSimTime, WsfPlatform& aPlatform) override;

   /** Add a platform to the connected platform set.
    * @param aPlatformName name of the platform to connect.
    */
   void AddConnectedPlatform(const QString& aPlatformName);

   /** Release a platform from the connected platform set.
    * @param aPlatformName name of the platform to release.
    */
   void ReleaseConnectedPlatform(const QString& aPlatformName);

   /**
    * Does the given platform index reference a platform that has a SituationAwarenessProcessor?
    * @param aIndex  the index of the platform to check for a processor.
    * @return        true if the platform has a SA processor, false otherwise.
    */
   bool HasSituationAwarenessProcessor(size_t aIndex);

   /** Return if the connection for a platform is destroyed.
    * @param aPlatformName name of the platform to check.
    */
   bool IsConnectionDestroyed(const QString& aPlatformName);

private:
   /** Triggers when a platform is deleted.
    * @param aSimTime sim time when the platform was deleted.
    * @param aPlatform reference to the platform.
    */
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;

   /** Triggers when the simulation initializes or restarts.
    * @param aSimulation pointer to the simulation.
    */
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   /** Update the interface.
    * @param aSimulation pointer to the simulation.
    */
   void WallClockRead(const WsfSimulation& aSimulation) override;

   std::set<QString> mConnectedPlatforms;
   std::set<size_t>  mPlatformsWithSAP; ///< Set of platforms with SituationAwarenessProcessors.
   UtCallbackHolder  mCallbacks;
};
} // namespace HDD
#endif // HDD_SIMINTERFACE_HPP
