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

#ifndef ORBITSIMEVENTS_HPP
#define ORBITSIMEVENTS_HPP

#include "UtOrbitalElements.hpp"
#include "WkSimInterface.hpp"
#include "orbit/WkfOrbitInterface.hpp"

namespace WkOrbit
{

class OrbitEvent : public warlock::SimEvent
{
public:
   explicit OrbitEvent(bool aRecurring)
      : warlock::SimEvent{aRecurring}
   {
   }
   virtual ~OrbitEvent() = default;

   virtual void Process(wkf::OrbitInterface* aInterface) = 0;
};

class SimStartingEvent : public OrbitEvent
{
public:
   explicit SimStartingEvent(const UtCalendar& aStartDateTime)
      : OrbitEvent{false}
      , mStartDateTime{aStartDateTime}
   {
   }
   ~SimStartingEvent() override = default;

   void Process(wkf::OrbitInterface* aInterface) override;

private:
   UtCalendar mStartDateTime;
};

class UpdateOrbitAngles : public OrbitEvent
{
public:
   explicit UpdateOrbitAngles(double aSimTime)
      : OrbitEvent{true}
      , mSimTime{aSimTime}
   {
   }
   ~UpdateOrbitAngles() override = default;

   void Process(wkf::OrbitInterface* aInterface) override;

private:
   double mSimTime;
};

class MoonOrbitUpdateEvent : public OrbitEvent
{
public:
   explicit MoonOrbitUpdateEvent(double aSimTime)
      : OrbitEvent{true}
      , mSimTime{aSimTime}
   {
   }
   ~MoonOrbitUpdateEvent() override = default;

   void Process(wkf::OrbitInterface* aInterface) override;

private:
   double mSimTime;
};

class OrbitalElementsUpdateEvent : public OrbitEvent
{
public:
   OrbitalElementsUpdateEvent(size_t                       aPlatformId,
                              double                       aSimTime,
                              const UtOrbitalElements&     aElements,
                              bool                         aManeuver,
                              const ut::optional<UtColor>& aColor)
      : OrbitEvent{false}
      , mPlatformId{aPlatformId}
      , mElements{aElements}
      , mSimTime{aSimTime}
      , mManeuver{aManeuver}
      , mDefinedColor{aColor}
   {
   }

   ~OrbitalElementsUpdateEvent() override = default;

   void Process(wkf::OrbitInterface* aInterfacePtr) override;

private:
   size_t                mPlatformId;
   UtOrbitalElements     mElements;
   double                mSimTime;
   bool                  mManeuver;
   ut::optional<UtColor> mDefinedColor;
};

class RemoveOrbitEvent : public OrbitEvent
{
public:
   explicit RemoveOrbitEvent(size_t aPlatformId)
      : OrbitEvent{false}
      , mPlatformId{aPlatformId}
   {
   }
   ~RemoveOrbitEvent() override = default;

   void Process(wkf::OrbitInterface* aInterface) override;

private:
   size_t mPlatformId;
};

} // namespace WkOrbit

#endif // ORBITSIMEVENTS_HPP
