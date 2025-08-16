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

#ifndef P6DOF_CONTROLLERSIMEVENTS_HPP
#define P6DOF_CONTROLLERSIMEVENTS_HPP

#include <string>

#include "P6DOF_ControllerDataContainer.hpp"
#include "WkSimInterface.hpp"

// This is a collection of SimEvents that are used in the P6DofController

namespace WkP6DOF_Controller
{
// This defines a base class for SimEvents for the P6DofController
class P6DOF_ControllerEvent : public warlock::SimEvent
{
public:
   P6DOF_ControllerEvent(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }
   virtual void Process(P6DOF_ControllerDataContainer& aData) = 0;
};

// This is the SimEvent for sim initialization
class SimulationInitializingEvent : public P6DOF_ControllerEvent
{
public:
   SimulationInitializingEvent()
      : P6DOF_ControllerEvent(false)
   {
   }

   void Process(P6DOF_ControllerDataContainer& aData) override;
};

// This is the SimEvent for adding a platform
class PlatformAddedEvent : public P6DOF_ControllerEvent
{
public:
   PlatformAddedEvent(const std::string aPlatform)
      : P6DOF_ControllerEvent(false)
      , mPlatform(aPlatform)
   {
   }

   void Process(P6DOF_ControllerDataContainer& aData) override;

private:
   std::string mPlatform;
};

// This is the SimEvent for deleting a platform
class PlatformDeletedEvent : public P6DOF_ControllerEvent
{
public:
   PlatformDeletedEvent(const std::string& aPlatform)
      : P6DOF_ControllerEvent(false)
      , mPlatform(aPlatform)
   {
   }

   void Process(P6DOF_ControllerDataContainer& aData) override;

private:
   std::string mPlatform;
};

// This is the SimEvent for a wall clock read event
class UpdateEvent : public P6DOF_ControllerEvent
{
public:
   UpdateEvent(const P6DOF_ControllerDataContainer& aData)
      : P6DOF_ControllerEvent(true)
      , mData(aData)
   {
   }

   void Process(P6DOF_ControllerDataContainer& aData) override;

private:
   P6DOF_ControllerDataContainer mData;
};
} // namespace WkP6DOF_Controller

#endif
