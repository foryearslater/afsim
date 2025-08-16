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
#ifndef WKEVENTMARKERSIMEVENTS_HPP
#define WKEVENTMARKERSIMEVENTS_HPP

#include <string>

#include "VaPosition.hpp"
#include "WkSimInterface.hpp"
#include "WsfWeaponEngagement.hpp"
#include "event_marker/WkfEventMarkerDisplayInterface.hpp"
#include "event_marker/WkfEventMarkerPrefObject.hpp"

class EventMarkSimEvent : public warlock::SimEvent
{
public:
   virtual void Process(wkf::EventMarkerDisplayInterface* aDisplayInterface,
                        const wkf::EventMarkerPrefObject* aPrefObject) = 0;
};

class SimCompleteEvent : public EventMarkSimEvent
{
public:
   void Process(wkf::EventMarkerDisplayInterface* aDisplayInterface, const wkf::EventMarkerPrefObject* aPrefObject) override;
};

class MarkerEvent : public EventMarkSimEvent
{
public:
   //! aTime = the current sim time
   //! aReferencePlatform = where the event occurred
   //! aType = the type of event
   //! aDescription = summary
   MarkerEvent(double aTime, const std::string& aType, std::string aDescription, const WsfPlatform* aReferencePlatform);

protected:
   double            mTime;
   vespa::VaPosition mPosition;
   std::string       mType;
   std::string       mSummary;
};

class PlatformMarkerEvent : public MarkerEvent
{
public:
   PlatformMarkerEvent(double aTime, const std::string& aType, std::string aDescription, const WsfPlatform* aReferencePlatform);

   void Process(wkf::EventMarkerDisplayInterface* aDisplayInterface, const wkf::EventMarkerPrefObject* aPrefObject) override;

private:
   wkf::PlatformEventInfo mInfo;
};

class WeaponMarkerEvent : public MarkerEvent
{
public:
   WeaponMarkerEvent(double aTime, const std::string& aType, std::string aDescription, const WsfWeaponEngagement* aEngagement);

   void Process(wkf::EventMarkerDisplayInterface* aDisplayInterface, const wkf::EventMarkerPrefObject* aPrefObject) override;

private:
   wkf::WeaponEventInfo mInfo;
};

#endif
