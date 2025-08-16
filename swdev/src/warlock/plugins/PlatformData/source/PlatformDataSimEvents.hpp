// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PLATFORMDATASIMEVENTS_HPP
#define PLATFORMDATASIMEVENTS_HPP

#include "PlatformDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkPlatformData
{

class PlatformDataEvent : public warlock::SimEvent
{
public:
   PlatformDataEvent(bool aRecurring = false)
      : SimEvent(aRecurring)
   {
   }
   virtual void Process(PlatformData& aData) = 0;
};

class UpdateEvent : public PlatformDataEvent
{
public:
   UpdateEvent(const PlatformData& aData)
      : PlatformDataEvent(true)
      , mData(aData)
   {
   }

   void Process(PlatformData& aData) override { aData = mData; }

private:
   PlatformData mData;
};

} // namespace WkPlatformData

#endif
