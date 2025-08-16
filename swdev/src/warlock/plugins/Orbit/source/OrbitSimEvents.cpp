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

#include "OrbitSimEvents.hpp"

#include "UtCast.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "orbit/WkfAttachmentOrbit.hpp"

void WkOrbit::SimStartingEvent::Process(wkf::OrbitInterface* aInterface)
{
   aInterface->SetStartDateTime(mStartDateTime);
   aInterface->UpdateOrbitAngles(0.0);
   aInterface->AddMoonOrbit();
}

void WkOrbit::UpdateOrbitAngles::Process(wkf::OrbitInterface* aInterface)
{
   aInterface->UpdateOrbitAngles(mSimTime);
}

void WkOrbit::OrbitalElementsUpdateEvent::Process(wkf::OrbitInterface* aInterfacePtr)
{
   wkf::AttachmentOrbit* orbitPtr = aInterfacePtr->GetAttachment(ut::safe_cast<unsigned int, size_t>(mPlatformId));
   if (orbitPtr)
   {
      if (aInterfacePtr->IsScenarioColorMode())
      {
         UtColor color(mDefinedColor.value_or(aInterfacePtr->GetTeamColor(mPlatformId)));
         orbitPtr->SetColor(color);
      }

      orbitPtr->Add(mSimTime,
                    mElements.GetSemiMajorAxis(),
                    mElements.GetEccentricity(),
                    mElements.GetRAAN(),
                    mElements.GetInclination(),
                    mElements.GetArgumentOfPeriapsis(),
                    mElements.GetTrueAnomaly(),
                    mManeuver);
      double angle = aInterfacePtr->GetAngleForTime(mSimTime);
      orbitPtr->UpdateTimeAngle(angle, mSimTime);
   }
}

void WkOrbit::RemoveOrbitEvent::Process(wkf::OrbitInterface* aInterface)
{
   aInterface->RemoveAttachment(ut::safe_cast<unsigned int, size_t>(mPlatformId));
}

void WkOrbit::MoonOrbitUpdateEvent::Process(wkf::OrbitInterface* aInterface)
{
   aInterface->UpdateMoonOrbit(mSimTime);
}
