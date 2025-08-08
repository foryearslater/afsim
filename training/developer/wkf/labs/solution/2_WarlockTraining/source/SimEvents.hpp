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

#ifndef WARLOCK_TRAINING_SIMEVENTS_HPP
#define WARLOCK_TRAINING_SIMEVENTS_HPP

#include <map>
#include <string>

#include "DataContainer.hpp"
#include "Types.hpp"

#include "WkSimInterface.hpp"

namespace WarlockTraining
{
   //! This is a base for events being processed by our plugin.
   //! A base class is used so that multiple event types can be processed if necessary.
   class EventBase : public warlock::SimEvent
   {
   public:
      EventBase(bool aRecurring)
         : warlock::SimEvent(aRecurring) {}

      //! All event types should have a Process function. The arguments can vary depending on need.
      virtual void Process(DataContainer& aDataContainer) = 0;
   };

   //! This is an event that represents the clock updating.
   class UpdateEvent : public EventBase
   {
   public:
      UpdateEvent(const std::map<std::string, PlatformData>& aData);

      //! Processes the event by updating the DockWidget's display.
      void Process(DataContainer& aDataContainer) override;

   private:
      //! Data about all platforms' position and heading.
      std::map<std::string, PlatformData> mData;
   };
}

#endif
