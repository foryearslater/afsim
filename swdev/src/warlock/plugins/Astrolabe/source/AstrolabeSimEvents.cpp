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

#include "AstrolabeSimEvents.hpp"

#include "AstrolabeEventInterface.hpp"

void WkAstrolabe::PlatformAvailabilityEvent::Process(EventInterface& aInterface)
{
   aInterface.SetPlatformAvailability(mPlatformName, mAvailable);
}

void WkAstrolabe::MissionVerificationResultsEvent::Process(EventInterface& aInterface)
{
   aInterface.SetMissionVerificationResults(mResults);
}

void WkAstrolabe::ContinueMissionSequenceEvent::Process(EventInterface& aInterface)
{
   if (mVerify)
   {
      aInterface.FinishVerify(std::move(mMissionPtr));
   }
   else
   {
      aInterface.FinishApply(std::move(mMissionPtr));
   }
}
