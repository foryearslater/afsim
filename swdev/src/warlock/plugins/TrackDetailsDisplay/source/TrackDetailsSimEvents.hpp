// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TRACKDETAILSSIMEVENTS_HPP
#define TRACKDETAILSSIMEVENTS_HPP


#include "DataContainer.hpp"
#include "DataTypes.hpp"
#include "WkSimInterface.hpp"

namespace WkTrackDetailsDisplay
{
class EventBase : public warlock::SimEvent
{
public:
   EventBase(bool aRecurring)
      : warlock::SimEvent(aRecurring)
   {
   }

   virtual void Process(DataContainer& aDataContainer) = 0;
};

class AddPlatformEvent : public EventBase
{
public:
   AddPlatformEvent(const std::string& aName);
   void Process(DataContainer& aDataContainer) override;

private:
   std::string mPlatformName;
};

class DeletePlatformEvent : public EventBase
{
public:
   DeletePlatformEvent(const std::string& aName);
   void Process(DataContainer& aDataContainer) override;

private:
   std::string mPlatformName;
};

class UpdateEvent : public EventBase
{
public:
   UpdateEvent(const PlatformsTrackData& aPlatform);
   void Process(DataContainer& aDataContainer) override;

private:
   PlatformsTrackData mPlatformData;
};


} // namespace WkTrackDetailsDisplay


#endif
