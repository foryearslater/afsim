// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "WkSimEnvironment.hpp"

#include <cassert>

#include "UtMemory.hpp"
#include "WkNetwork.hpp"
#include "WkfEnvironment.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfPlatformObserver.hpp"
#include "WsfPlatformPartObserver.hpp"
#include "WsfSimulation.hpp"
#include "WsfSimulationObserver.hpp"

warlock::SimEnvironment* warlock::SimEnvironment::mInstancePtr = nullptr;

namespace
{
std::thread::id mainSimEnvironmentThreadId = std::this_thread::get_id();

void AssertCurrentSimThread(bool aIsMainThread)
{
   bool isMainThread = (mainSimEnvironmentThreadId == std::this_thread::get_id());
   if (isMainThread != aIsMainThread)
   {
      wkf::QueueableMessageObject::DisplayQueuedMessage(
         QMessageBox::Critical,
         "Thread Error",
         "Unsafe thread access of warlock::SimEnvironment.\nThis is most likely caused by a plugin accessing data in "
         "an unsafe way.\nConsider unloading the responsible plugin.");
   }
}
} // namespace

warlock::SimEnvironment::SimEnvironment()
   : QObject()
{
   // Queued connection because SimulationInitializing is emitted from simulation thread.
   connect(
      this,
      QOverload<>::of(&SimEnvironment::SimulationInitializing),
      &wkfEnv,
      []()
      {
         wkfEnv.SetFrameUpdateEnabled(true);
         emit wkfEnv.CloseDialogs();
      },
      Qt::QueuedConnection);
   connect(&wkfEnv, &wkf::Environment::UpdateFrame, this, &SimEnvironment::GuiUpdate, Qt::DirectConnection);
}

warlock::SimEnvironment::~SimEnvironment() = default;

warlock::SimEnvironment& warlock::SimEnvironment::GetInstance()
{
   assert(mInstancePtr != nullptr);
   return *mInstancePtr;
}

bool warlock::SimEnvironment::Exists()
{
   return (mInstancePtr != nullptr);
}

void warlock::SimEnvironment::Create()
{
   if (mInstancePtr == nullptr)
   {
      mInstancePtr = new SimEnvironment;
      mInstancePtr->Initialize();
   }
}

void warlock::SimEnvironment::Shutdown()
{
   if (mInstancePtr != nullptr)
   {
      delete mInstancePtr;
      mInstancePtr = nullptr;
   }
}

void warlock::SimEnvironment::Attach(WsfSimulation& aSimulation)
{
   // Error if this is called from the GUI thread.
   ::AssertCurrentSimThread(false);

   mSimulationPtr = &aSimulation;
   mCallbacks.Clear();
   InitializeCallbacks();
}

const warlock::XIO_DataContainer& warlock::SimEnvironment::GetXIO_Info() const
{
   // Error if this is called from the GUI thread.
   ::AssertCurrentSimThread(false);

   return mXIO_DataContainer;
}

void warlock::SimEnvironment::SetSimClockInterval(double aInterval)
{
   QMutexLocker locker(&mMutex);
   mSimClockInterval = aInterval;
}

void warlock::SimEnvironment::SetWallClockInterval(double aInterval)
{
   QMutexLocker locker(&mMutex);
   mWallClockInterval = aInterval;
}

double warlock::SimEnvironment::GetSimClockInterval() const
{
   QMutexLocker locker(&mMutex);
   return mSimClockInterval;
}

double warlock::SimEnvironment::GetWallClockInterval() const
{
   QMutexLocker locker(&mMutex);
   return mWallClockInterval;
}

std::shared_ptr<warlock::ScriptSimInterface> warlock::SimEnvironment::GetScriptSimInterface() const
{
   // Error if this is called from the SIM thread.
   ::AssertCurrentSimThread(true);

   if (!mScriptSimInterfacePtr.expired())
   {
      return mScriptSimInterfacePtr.lock();
   }

   auto ssiPtr            = std::make_shared<ScriptSimInterface>("script");
   mScriptSimInterfacePtr = ssiPtr;
   return ssiPtr;
}

void warlock::SimEnvironment::Initialize()
{
   // Construction of Sim Interface require the Warlock Environment to be constructed first,
   //  therefore we cant do this in constructor of this class.
   mCoreSimInterfacePtr = ut::make_unique<CoreSimInterface>(mXIO_DataContainer);
   connect(&wkfEnv,
           &wkf::Environment::MoveSelectionComplete,
           mCoreSimInterfacePtr.get(),
           &CoreSimInterface::SendMoveCommand,
           Qt::DirectConnection);
}

void warlock::SimEnvironment::InitializeCallbacks()
{
   if (mSimulationPtr != nullptr)
   {
      mCallbacks.Add(
         WsfObserver::PlatformAdded(mSimulationPtr)
            .Connect([this](double aSimTime, WsfPlatform* aPlatform) { emit PlatformAdded(aSimTime, *aPlatform); }));

      mCallbacks.Add(WsfObserver::PlatformInitialized(mSimulationPtr)
                        .Connect([this](double aSimTime, WsfPlatform* aPlatform)
                                 { emit PlatformInitialized(aSimTime, *aPlatform); }));

      mCallbacks.Add(
         WsfObserver::PlatformDeleted(mSimulationPtr)
            .Connect([this](double aSimTime, WsfPlatform* aPlatform) { emit PlatformDeleted(aSimTime, *aPlatform); }));

      mCallbacks.Add(WsfObserver::PlatformBroken(mSimulationPtr)
                        .Connect([this](double aSimTime, WsfPlatform* aPlatform)
                                 { mCoreSimInterfacePtr->PlatformBroken(aSimTime, *aPlatform); }));

      mCallbacks.Add(WsfObserver::VisualPartTurnedOn(mSimulationPtr)
                        .Connect([this](double aSimTime, WsfVisualPart* aPart)
                                 { mCoreSimInterfacePtr->VisualPartTurnedOn(aSimTime, *aPart); }));

      mCallbacks.Add(WsfObserver::VisualPartTurnedOff(mSimulationPtr)
                        .Connect([this](double aSimTime, WsfVisualPart* aPart)
                                 { mCoreSimInterfacePtr->VisualPartTurnedOff(aSimTime, *aPart); }));

      mCallbacks.Add(WsfObserver::VisualPartNonOperational(mSimulationPtr)
                        .Connect([this](double aSimTime, WsfVisualPart* aPart)
                                 { mCoreSimInterfacePtr->VisualPartStateChanged(aSimTime, *aPart); }));

      mCallbacks.Add(WsfObserver::VisualPartOperational(mSimulationPtr)
                        .Connect([this](double aSimTime, WsfVisualPart* aPart)
                                 { mCoreSimInterfacePtr->VisualPartStateChanged(aSimTime, *aPart); }));

      mCallbacks.Add(WsfObserver::VisualPartBroken(mSimulationPtr)
                        .Connect([this](double aSimTime, WsfVisualPart* aPart)
                                 { mCoreSimInterfacePtr->VisualPartStateChanged(aSimTime, *aPart); }));

      mCallbacks.Add(WsfObserver::SimulationInitializing(mSimulationPtr)
                        .Connect(
                           [this]
                           {
                              emit SimulationInitializing(*mSimulationPtr);
                              emit SimulationInitializing();
                           }));

      mCallbacks.Add(
         WsfObserver::SimulationStarting(mSimulationPtr).Connect([this] { emit SimulationStarting(*mSimulationPtr); }));

      mCallbacks.Add(WsfObserver::SimulationComplete(mSimulationPtr)
                        .Connect([this](double aTime) { emit SimulationComplete(*mSimulationPtr); }));

      mSimulationPtr->AddWallEvent(ut::make_unique<ClockEvent>(false));
      mSimulationPtr->AddEvent(ut::make_unique<ClockEvent>(true));
   }
}

void warlock::SimEnvironment::GuiUpdate()
{
   mCoreSimInterfacePtr->ProcessEvents();
}

WsfEvent::EventDisposition warlock::SimEnvironment::ClockEvent::Execute()
{
   if (SimEnvironment::Exists())
   {
      double rescheduleTime = GetTime();
      if (mSimClock)
      {
         emit simEnv.SimulationClockRead(*GetSimulation());
         emit simEnv.SimulationClockWrite(*GetSimulation());
         rescheduleTime += simEnv.GetSimClockInterval();
      }
      else
      {
         emit simEnv.WallClockRead(*GetSimulation());
         emit simEnv.WallClockWrite(*GetSimulation());
         rescheduleTime += simEnv.GetWallClockInterval();
      }

      SetTime(rescheduleTime);
      return WsfEvent::cRESCHEDULE;
   }
   else
   {
      return WsfEvent::cDELETE;
   }
}
