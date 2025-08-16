//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#ifndef SimEventsPlatformHistory_HPP
#define SimEventsPlatformHistory_HPP

#include "WkSimInterface.hpp"

namespace WkPlatformHistory
{
class Plugin;

class PlatformHistoryEvent : public warlock::SimEvent
{
public:
   virtual void Process(Plugin& aPlugin) = 0;
};

class TraceLineEvent : public PlatformHistoryEvent
{
public:
   TraceLineEvent(size_t aSourcePlatformIndex, size_t aTargetPlatformIndex, bool aInitiated)
      : mSourcePlatformIndex(aSourcePlatformIndex)
      , mTargetPlatformIndex(aTargetPlatformIndex)
      , mInitiated(aInitiated)
   {
   }

protected:
   size_t mSourcePlatformIndex;
   size_t mTargetPlatformIndex;
   bool   mInitiated;
};

class DetectedEvent : public TraceLineEvent
{
public:
   using TraceLineEvent::TraceLineEvent;
   void Process(Plugin& aPlugin) override;
};

class TrackedEvent : public TraceLineEvent
{
public:
   using TraceLineEvent::TraceLineEvent;
   void Process(Plugin& aPlugin) override;
};

class AttackedEvent : public TraceLineEvent
{
public:
   using TraceLineEvent::TraceLineEvent;
   void Process(Plugin& aPlugin) override;
};

class PlatformDeletedEvent : public PlatformHistoryEvent
{
public:
   PlatformDeletedEvent(size_t aIndex)
      : mIndex(aIndex)
   {
   }

   void Process(Plugin& aPlugin) override;

private:
   size_t mIndex;
};
} // namespace WkPlatformHistory

#endif
