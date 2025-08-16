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

#ifndef WSFWKSIMENVIRONMENT_HPP
#define WSFWKSIMENVIRONMENT_HPP

#include <memory>

#include <QObject>

#include "UtCallbackHolder.hpp"
#include "WsfEvent.hpp"
class WsfPlatform;
class WsfSimulation;
#include "warlock_core_export.h"

#include "WkCoreSimInterface.hpp"
#include "WkScriptSimInterface.hpp"
#include "WkXIO_DataContainer.hpp"

#if defined(simEnv)
#undef simEnv
#endif
#define simEnv warlock::SimEnvironment::GetInstance()

namespace warlock
{
class WARLOCK_CORE_EXPORT SimEnvironment : public QObject
{
   // These two classes are marked as friend to restrict the access to signal the environment implements.
   // These classes handle the thread affinity to ensure safe use, and it is intended that user will derive
   //  from warlock::SimInterfaceBase or warlock::Plugin to gain access to the connected slots.
   friend class SimInterfaceBase;
   template<class P>
   friend class PluginBaseT;

   Q_OBJECT

public:
   // The following 4 static function should be called from the GUI Thread.
   //  However due to mInstance being valid for the the entire lifetime of the
   //  simulation thread and that it can't change, it should be okay to call from
   //  SIM thread even though there is no mutex locking.
   static void            Create();
   static SimEnvironment& GetInstance();
   static bool            Exists();
   static void            Shutdown();

   // The following four accessors are thread safe
   void   SetSimClockInterval(double aInterval);
   void   SetWallClockInterval(double aInterval);
   double GetSimClockInterval() const;
   double GetWallClockInterval() const;

   //*******************************************************
   // Note: The following functions are NOT Thread Safe and should only be called by the thread for which they
   // are intended to be called by.  If called from the wrong thread a message will appear warning the user
   // and a dialog will be presented to close the application.

   // This should be called from the Simulation Thread only!
   void Attach(WsfSimulation& aSimulation);

   // This should be called from the SIM thread only!
   const XIO_DataContainer& GetXIO_Info() const;

   // This is a common SimInterface that can be used by anyone who needs access to scripts on the simulation
   // This should be called from the GUI thread only!
   std::shared_ptr<ScriptSimInterface> GetScriptSimInterface() const;
   //*******************************************************

protected:
   SimEnvironment();
   // destructor defaulted in cpp file.
   ~SimEnvironment() override;

signals:
#ifndef Q_MOC_RUN
protected:
#endif
   // All signals are protected and emitted from the simulation thread.
   // They are connected to corresponding virtual methods in friend class warlock::SimInterface (direct connections
   // executed on simulation thread)
   void SimulationInitializing(const WsfSimulation& aSimulation);
   void SimulationInitializing();
   void SimulationStarting(const WsfSimulation& aSimulation);
   void SimulationComplete(const WsfSimulation& aSimulation);
   void PlatformAdded(double aSimTime, WsfPlatform& aPlatform);
   void PlatformInitialized(double aSimTime, WsfPlatform& aPlatform);
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform);

   // These signals are emitted on simulation clock events
   void SimulationClockRead(const WsfSimulation& aSimulation);
   void SimulationClockWrite(WsfSimulation& aSimulation);
   // These signals are emitted on wall clock events. They should be used for simulation management or
   // non-simulated events only.
   void WallClockRead(const WsfSimulation& aSimulation);
   void WallClockWrite(WsfSimulation& aSimulation);

private:
   // ClockEvent is intended to be executed on the Simulation Thread as it is a WsfEvent
   class ClockEvent : public WsfEvent
   {
   public:
      ClockEvent(bool aSimClock)
         : mSimClock(aSimClock)
      {
      }

      EventDisposition Execute() override;

   private:
      const bool mSimClock; // If true, event is triggered on simulation clock.
                            // If false, event is triggered on wall clock.
   };

   void Initialize();
   void InitializeCallbacks();

   void GuiUpdate();

   static SimEnvironment* mInstancePtr;

   WsfSimulation*    mSimulationPtr{nullptr};
   UtCallbackHolder  mCallbacks{};
   double            mSimClockInterval{1. / 30.};  // in sec, 30 Hz default
   double            mWallClockInterval{1. / 30.}; // in sec, 30 Hz default
   XIO_DataContainer mXIO_DataContainer{};

   std::unique_ptr<CoreSimInterface>         mCoreSimInterfacePtr{nullptr};
   mutable std::weak_ptr<ScriptSimInterface> mScriptSimInterfacePtr{};

   // QMutex is used to maintain thread safe communication between Sim and Gui threads.
   mutable QMutex mMutex;
};
} // namespace warlock
#endif // WKENVIRONMENT_HPP
