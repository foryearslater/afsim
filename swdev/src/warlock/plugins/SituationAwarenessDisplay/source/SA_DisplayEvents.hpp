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

#ifndef SA_DISPLAY_EVENTS_HPP
#define SA_DISPLAY_EVENTS_HPP

#include "WkSimInterface.hpp"
#include "sa_display/WkfSA_DisplayDataContainer.hpp"

namespace WkSA_Display
{
class Plugin;

class SA_Event : public warlock::SimEvent
{
public:
   SA_Event(bool aRecurring = false)
      : warlock::SimEvent(aRecurring)
   {
   }
   virtual void Process(wkf::SA_Display::DataContainer& aState) = 0;
};

class SimulationInitializingEvent : public SA_Event
{
public:
   SimulationInitializingEvent()
      : SA_Event(false)
   {
   }

   void Process(wkf::SA_Display::DataContainer& aData) override;
};

class UpdateSA_DataEvent : public SA_Event
{
public:
   UpdateSA_DataEvent(const wkf::SA_Display::DataContainer& aData)
      : SA_Event(true)
      , mData(aData)
   {
   }
   void Process(wkf::SA_Display::DataContainer& aData) override;

private:
   wkf::SA_Display::DataContainer mData;
};
} // namespace WkSA_Display

#endif // SA_DISPLAY_EVENTS_HPP
