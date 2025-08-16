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

#ifndef WKCORESIMINTERFACE_HPP
#define WKCORESIMINTERFACE_HPP

#include <memory>
#include <string>

#include "UtCallback.hpp"
#include "WkCoreSimEvents.hpp"
#include "WkSimInterface.hpp"
#include "WkXIO_DataContainer.hpp"
#include "WkfPlatform.hpp"
#include "WsfEvent.hpp"
#include "WsfVisualPart.hpp"

namespace warlock
{
//*******************************************
// SimInterface
class CoreSimInterface : public SimInterfaceT<CoreSimEvent>
{
public:
   explicit CoreSimInterface(XIO_DataContainer& aXIO_Data);
   ~CoreSimInterface() override = default;

   void PlatformBroken(double aSimTime, const WsfPlatform& aPlatform);
   void VisualPartTurnedOn(double aSimTime, const WsfVisualPart& aVisualPart);
   void VisualPartTurnedOff(double aSimTime, const WsfVisualPart& aVisualPart);
   void VisualPartStateChanged(double aSimTime, const WsfVisualPart& aVisualPart);
   // Executed on GUI thread
   void SendMoveCommand(const wkf::PlatformList& aSelectedPlatforms);

protected:
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) override;
   void PlatformInitialized(double aSimTime, WsfPlatform& aPlatform) override;
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void SimulationStarting(const WsfSimulation& aSimulation) override;
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void WallClockRead(const WsfSimulation& aSimulation) override;

private:
   void CheckForSendingBatchEvent();

   UtCallbackHolder mCallbacks;

   // These should only be accessed on the SIM Thread.
   PlatformMap                                            mQueuedPlatformAddedData;
   std::vector<std::unique_ptr<PlatformInitializedEvent>> mQueuedPlatformInitializedEvents;
   std::vector<std::unique_ptr<PlatformDeletedEvent>>     mQueuedPlatformDeletedEvents;
   XIO_DataContainer&                                     mXIO_Data;
};

class RemovePlatformWsfEvent : public WsfEvent
{
public:
   RemovePlatformWsfEvent(CoreSimInterface* aSimInterface, double aSimTime, size_t aIndex)
      : WsfEvent(aSimTime)
      , mSimInterface(aSimInterface)
      , mIndex(aIndex)
   {
   }

   ~RemovePlatformWsfEvent() override;

   EventDisposition Execute() override;

private:
   CoreSimInterface* mSimInterface;
   size_t            mIndex;
};
} // namespace warlock
#endif
