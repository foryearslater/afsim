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

#include "AstrolabeSimCommands.hpp"

#include "MissionVerifier.hpp"
#include "OrbitalModelToSequence.hpp"
#include "WkObserver.hpp"
#include "WsfDateTime.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfSpaceMoverBase.hpp"

void WkAstrolabe::CancelManeuversCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platformPtr = aSimulation.GetPlatformByName(mPlatformName);
   if (platformPtr && !platformPtr->IsExternallyControlled())
   {
      WsfSpaceMoverBase* moverPtr = dynamic_cast<WsfSpaceMoverBase*>(platformPtr->GetMover());
      if (moverPtr)
      {
         moverPtr->CancelMission();
      }
      WkObserver::SimulationUserAction(aSimulation)("Cancelled Current Maneuver", platformPtr);
   }
}

void WkAstrolabe::InjectMissionSequenceCommand::Process(WsfSimulation& aSimulation)
{
   WsfPlatform* platformPtr = aSimulation.GetPlatformByName(mPlatformName);
   if (platformPtr && !platformPtr->IsExternallyControlled())
   {
      WsfSpaceMoverBase* moverPtr = dynamic_cast<WsfSpaceMoverBase*>(platformPtr->GetMover());
      if (moverPtr)
      {
         wsfg::spaceg::OrbitalModelToSequence transform{aSimulation.GetScenario()};
         auto                                 sequencePtr = transform.Transform(mMission);

         UtCalendar now = aSimulation.GetDateTime().GetCurrentTime(aSimulation.GetSimTime());
         moverPtr->ScheduleMission(now, std::move(sequencePtr));
      }
      WkObserver::SimulationUserAction(aSimulation)("Assigned New Mission Sequence", platformPtr);
   }
}

void WkAstrolabe::VerifyMissionCommand::Process(WsfSimulation& aSimulation)
{
   MissionVerifier verifier{aSimulation};
   auto            results = verifier.Verify(mPlatformName, mData);
   AddSimEvent(ut::make_unique<MissionVerificationResultsEvent>(results));
}

void WkAstrolabe::CreateMissionCommand::Process(WsfSimulation& aSimulation)
{
   wsfg::spaceg::OrbitalModelToSequence transformer{aSimulation.GetScenario()};
   auto                                 sequencePtr = transformer.Transform(mMission);
   AddSimEvent(ut::make_unique<ContinueMissionSequenceEvent>(std::move(sequencePtr), mVerify));
}
