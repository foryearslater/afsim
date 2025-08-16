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

#ifndef SATELLITETETHERSIMEVENTS_HPP
#define SATELLITETETHERSIMEVENTS_HPP

#include <QColor>

#include "UtCalendar.hpp"
#include "UtOrbitalPropagatorBase.hpp"
#include "WkSimInterface.hpp"

namespace SatelliteTether
{
class DockWidget;
class PropagationManager;

class SatelliteTetherEvent : public warlock::SimEvent
{
public:
   explicit SatelliteTetherEvent(bool aRecurring)
      : warlock::SimEvent{aRecurring}
   {
   }
   virtual ~SatelliteTetherEvent() = default;

   virtual void Process(const std::map<unsigned int, DockWidget*>& aWidgetList, PropagationManager& aPropagationManager) = 0;
};


class OrbitColorUpdateEvent : public SatelliteTetherEvent
{
public:
   OrbitColorUpdateEvent(size_t aPlatformId, const QColor& aColor)
      : SatelliteTetherEvent(false)
      , mPlatformId(aPlatformId)
      , mDefinedColor(aColor)
   {
   }

   ~OrbitColorUpdateEvent() override = default;

   void Process(const std::map<unsigned int, DockWidget*>& aWidgetList, PropagationManager& aPropagationManager) override;

private:
   size_t mPlatformId;
   QColor mDefinedColor;
};

class AddTrackEvent : public SatelliteTetherEvent
{
public:
   AddTrackEvent(size_t aPlatformId, size_t aTetherId, const QColor& aColor, std::unique_ptr<UtOrbitalPropagatorBase> aPropagatorPtr)
      : SatelliteTetherEvent(false)
      , mPlatformId(aPlatformId)
      , mTetherId(aTetherId)
      , mColor(aColor)
      , mPropagatorPtr(std::move(aPropagatorPtr))
   {
   }

   ~AddTrackEvent() override = default;

   void Process(const std::map<unsigned int, DockWidget*>& aWidgetList, PropagationManager& aPropagationManager) override;

private:
   size_t                                   mPlatformId;
   size_t                                   mTetherId;
   QColor                                   mColor;
   std::unique_ptr<UtOrbitalPropagatorBase> mPropagatorPtr;
};

class ReportPropagatorEvent : public SatelliteTetherEvent
{
public:
   ReportPropagatorEvent(std::size_t aPlatformId, std::unique_ptr<UtOrbitalPropagatorBase> aPropagatorPtr, double aSimTime)
      : SatelliteTetherEvent{false}
      , mPlatformId{aPlatformId}
      , mPropagatorPtr{std::move(aPropagatorPtr)}
      , mSimTime{aSimTime}
   {
   }
   ~ReportPropagatorEvent() override = default;

   void Process(const std::map<unsigned int, DockWidget*>& aWidgetList, PropagationManager& aPropagationManager) override;

private:
   std::size_t                              mPlatformId;
   std::unique_ptr<UtOrbitalPropagatorBase> mPropagatorPtr;
   double                                   mSimTime;
};

class ReportInitialEpochEvent : public SatelliteTetherEvent
{
public:
   explicit ReportInitialEpochEvent(const UtCalendar& aInitialEpoch)
      : SatelliteTetherEvent{false}
      , mInitialEpoch{aInitialEpoch}
   {
   }
   ~ReportInitialEpochEvent() override = default;

   void Process(const std::map<unsigned int, DockWidget*>& aWidgetList, PropagationManager& aPropagationManager) override;

private:
   UtCalendar mInitialEpoch;
};
} // namespace SatelliteTether

#endif // ORBITSIMEVENTS_HPP
