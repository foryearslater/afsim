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

#ifndef ASTROLABESIMEVENTS_HPP
#define ASTROLABESIMEVENTS_HPP

class WsfScenario;
#include "MissionVerificationResults.hpp"
#include "WkSimInterface.hpp"
#include "WsfOrbitalMissionSequence.hpp"

namespace WkAstrolabe
{

class EventInterface;

class AstrolabeEvent : public warlock::SimEvent
{
public:
   explicit AstrolabeEvent(bool aRecurring)
      : warlock::SimEvent{aRecurring}
   {
   }
   ~AstrolabeEvent() override = default;

   virtual void Process(EventInterface& aInterface) = 0;
};

//! A SimEvent that notifies Astrolabe about the availability of a platform.
class PlatformAvailabilityEvent : public AstrolabeEvent
{
public:
   explicit PlatformAvailabilityEvent(const QString& aPlatformName, bool aAvailable = true)
      : AstrolabeEvent{false}
      , mPlatformName{aPlatformName}
      , mAvailable{aAvailable}
   {
   }

   void Process(EventInterface& aInterface) override;

private:
   QString mPlatformName;
   bool    mAvailable;
};

//! A SimEvent that provides the results of mission verification to Astrolabe.
class MissionVerificationResultsEvent : public AstrolabeEvent
{
public:
   explicit MissionVerificationResultsEvent(const wsfg::spaceg::MissionVerificationResults& aResults)
      : AstrolabeEvent{false}
      , mResults{aResults}
   {
   }

   void Process(EventInterface& aInterface) override;

private:
   wsfg::spaceg::MissionVerificationResults mResults;
};

//! A SimEvent that returns a created WsfOrbitalMissionSequence to the GUI.
class ContinueMissionSequenceEvent : public AstrolabeEvent
{
public:
   ContinueMissionSequenceEvent(std::unique_ptr<WsfOrbitalMissionSequence> aMissionPtr, bool aVerify)
      : AstrolabeEvent{false}
      , mMissionPtr{std::move(aMissionPtr)}
      , mVerify{aVerify}
   {
   }

   void Process(EventInterface& aInterface) override;

private:
   std::unique_ptr<WsfOrbitalMissionSequence> mMissionPtr;
   bool                                       mVerify;
};

} // namespace WkAstrolabe

#endif // ASTROLABESIMEVENTS_HPP
