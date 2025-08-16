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
#ifndef WKSIMINTERFACE_HPP
#define WKSIMINTERFACE_HPP

#include <atomic>
#include <list>
#include <memory>
#include <unordered_set>

#include <QMutex>
#include <QObject>
#include <QVector>

#include "UtConcurrentQueue.hpp"
class WsfPlatform;
class WsfSimulation;
#include "warlock_core_export.h"

namespace warlock
{

class WARLOCK_CORE_EXPORT SimCommand
{
public:
   explicit SimCommand(bool aUseWallClock = false)
      : mUseWallClock(aUseWallClock)
   {
   }

   virtual ~SimCommand()         = default;
   SimCommand(const SimCommand&) = delete;
   SimCommand(SimCommand&&)      = delete;
   SimCommand& operator=(const SimCommand&) = delete;
   SimCommand& operator=(SimCommand&&) = delete;

   virtual void Process(WsfSimulation& aSimulation) = 0;

   bool UseWallClock() const { return mUseWallClock; }

private:
   bool mUseWallClock;
};

class WARLOCK_CORE_EXPORT SimEvent
{
public:
   // The aRecurring flag indicates whether this is a recurring message such that
   //  only the most recent message is relevant. (Denotes that old messages can be
   //  safely ignored as the data is no longer relevant.)
   SimEvent(bool aRecurring = false)
      : mRecurring(aRecurring)
   {
   }
   virtual ~SimEvent()       = 0;
   SimEvent(const SimEvent&) = delete;
   SimEvent(SimEvent&&)      = delete;
   SimEvent& operator=(const SimEvent&) = delete;
   SimEvent& operator=(SimEvent&&) = delete;

   bool IsRecurring() const { return mRecurring; }

private:
   bool mRecurring;
};

class WARLOCK_CORE_EXPORT SimInterfaceBase : public QObject
{
public:
   explicit SimInterfaceBase(const QString& aName = "");

   ~SimInterfaceBase() override                   = 0;
   SimInterfaceBase(const SimInterfaceBase& aSrc) = delete;
   SimInterfaceBase& operator=(const SimInterfaceBase& aSrc) = delete;

   // Do NOT call SetEnabled or IsEnabled from the Sim thread.
   // Only call from the GUI thread.
   // When a SimInterface is not Enable, periodic slots will be disabled.
   void SetEnabled(bool aEnabled);
   bool IsEnabled() const { return mEnabled; }

   void AddSimCommand(std::unique_ptr<SimCommand> aCommand);

protected:
   // **************************************************************
   // Slots executed on simulation thread (Direct Connection)
   // **************************************************************
   // This slot is triggered when a platform is added to the scenario.
   // The initial platforms are added after SimulationInitializing but prior to SimulationStarting.
   // Note: A non-const reference to the platform is passed rather a const reference because many of the
   // state methods (GetLocationXXX, GetVelocityXXX, etc.) in UtEntity are non-const
   virtual void PlatformAdded(double aSimTime, WsfPlatform& aPlatform) {}

   // This slot is triggered when a platform is initialized. This occurs after the PlatformAdded notification.
   // Note: A non-const reference to the platform is passed rather a const reference because many of the
   // state methods (GetLocationXXX, GetVelocityXXX, etc.) in UtEntity are non-const
   virtual void PlatformInitialized(double aSimTime, WsfPlatform& aPlatform) {}

   // This slot is triggered when a platform is deleted from the scenario.
   // When the simulation completes all platforms are deleted, but when the simulation is terminated early platforms are not deleted.
   virtual void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) {}

   // This slot is triggered when the simulation initializes.
   virtual void SimulationInitializing(const WsfSimulation& aSimulation) {}

   // This slot is triggered when the simulation is starting.
   // SimulationStarting occurs after all initial platforms have been added to the simulation
   virtual void SimulationStarting(const WsfSimulation& aSimulation) {}

   // This slot is triggered on a wall clock event at a rate defined in warlock::SimEnvironment. They should be used
   // for simulation management or non-simulated events only.
   virtual void WallClockRead(const WsfSimulation& aSimulation) {}

   // This slot is triggered on a simulation clock event at a rate defined in warlock::SimEnvironment.
   virtual void SimulationClockRead(const WsfSimulation& aSimulation) {}

   // This slot is triggered when the simulation completes.
   // It is not triggered if the simulation terminates early.
   virtual void SimulationComplete(const WsfSimulation& aSimulation) {}

   // When SetEnabled changes the mEnabled flag, this function will called to let derived classes do any necessary processing.
   virtual void ProcessEnableFlagChanged(bool aEnabled) {}

   // QMutex is used to maintain thread safe communication between Sim and Gui threads.
   mutable QMutex mMutex;

private:
   void WallClockCommands(WsfSimulation& aSimulation);
   void SimulationClockCommands(WsfSimulation& aSimulation);

   void UpdatePeriodicConnections();

   QString                          mName;
   std::atomic<bool>                mEnabled;
   QVector<QMetaObject::Connection> mConnections;

   using SimCommandQueue = UtConcurrentQueue<std::unique_ptr<SimCommand>>;

   void ProcessCommands(WsfSimulation& aSimulation, SimCommandQueue& aCommands);

   SimCommandQueue mSimCommandsWallClock;
   SimCommandQueue mSimCommandsSimClock;
};

template<class EVENT_TYPE>
class SimInterfaceT : public SimInterfaceBase
{
   static_assert(std::is_base_of<SimEvent, EVENT_TYPE>::value,
                 "Template argument to warlock::SimInterfaceT must be a subclass of warlock::SimEvent");

public:
   explicit SimInterfaceT(const QString& aName = "")
      : SimInterfaceBase(aName)
   {
   }

   class Command : public SimCommand
   {
   public:
      Command(SimInterfaceT<EVENT_TYPE>* aSimInterface, bool aUseWallClock = false)
         : SimCommand(aUseWallClock)
         , mSimInterface(aSimInterface)
      {
      }

   protected:
      void AddSimEvent(std::unique_ptr<EVENT_TYPE> aEvent) { mSimInterface->AddSimEvent(std::move(aEvent)); }

   private:
      SimInterfaceT<EVENT_TYPE>* mSimInterface;
   };

   template<typename... Args>
   void ProcessEvents(Args&&... args)
   {
      while (!mSimEvents.Empty())
      {
         mSimEvents.Pop()->Process(std::forward<Args>(args)...);
      }
   }

   void AddSimEvent(std::unique_ptr<EVENT_TYPE> aEvent) { mSimEvents.Push(std::move(aEvent)); }

private:
   // Underlying container class template used for UtConcurrentQueue. Maintains a single occurrence of recurring event types.
   template<class T>
   class SimEventQueueContainer
   {
   public:
      using iterator        = typename std::list<T>::iterator;
      using size_type       = typename std::list<T>::size_type;
      using reference       = typename std::list<T>::reference;
      using const_reference = typename std::list<T>::const_reference;
      using value_type      = typename std::list<T>::value_type;

      bool      empty() const { return mStorage.empty(); }
      size_type size() const { return mStorage.size(); }
      void      pop_front() { mStorage.pop_front(); }

      void push_back(T&& val)
      {
         mStorage.push_back(std::move(val)); // Insert new element at end
         if (mStorage.back()->IsRecurring())
         {
            iterator last = std::prev(mStorage.end()); // Get iterator to last element (the one just inserted)
            auto     it   = mLookup.find(last);
            if (it != mLookup.end())
            {
               auto temp = *it;
               // If element is found in lookup, erase it from the storage and erase entry in lookup
               mLookup.erase(it);
               mStorage.erase(temp);
            }

            mLookup.insert(last); // Insert new element into lookup
         }
      }

      T& front()
      {
         // Front element is about to be popped. Can't put this in pop_front() because by the time pop_front() is called
         // the first element is invalidated from std::move.
         T& f = mStorage.front();
         if (f->IsRecurring())
         {
            // If front element is recurring, erase it from lookup
            mLookup.erase(mStorage.begin());
         }

         return f;
      }

   private:
      // These function objects are needed to use const_iterator as key for unordered_set
      class Hasher
      {
      public:
         size_t operator()(const iterator& val) const { return typeid(**val).hash_code(); }
      };

      class EqualTo
      {
      public:
         bool operator()(const iterator& val1, const iterator& val2) const { return typeid(**val1) == typeid(**val2); }
      };

      // Underlying storage. Use std::list rather than std::deque due to iterator validity concerns when erasing/inserting
      std::list<T> mStorage;
      // Lookup mechanism to enforce single occurrence of recurring events in sequence container
      std::unordered_set<iterator, Hasher, EqualTo> mLookup;
   };

   using EventPtr      = std::unique_ptr<EVENT_TYPE>;
   using SimEventQueue = UtConcurrentQueue<EventPtr, SimEventQueueContainer<EventPtr>>;

   SimEventQueue mSimEvents;
};

} // namespace warlock

#endif
