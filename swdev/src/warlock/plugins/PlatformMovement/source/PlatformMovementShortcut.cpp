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

#include "PlatformMovementShortcut.hpp"

#include <functional>

#include "PlatformMovementSimCommands.hpp"
#include "UtSphericalEarth.hpp"
#include "VaEnvironment.hpp"
#include "VaPosition.hpp"
#include "WkfAttachmentRubberband.hpp"
#include "WkfEnvironment.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"

WkPlatformMovement::Shortcut::Shortcut(SimInterface&   aSimInterface,
                                       QObject*        aParent /* = nullptr */,
                                       Qt::WindowFlags aFlags /* = Qt::WindowFlags() */)
   : mSimInterface(aSimInterface)
{
}

void WkPlatformMovement::Shortcut::StartHeading()
{
   auto rb = Start(&Shortcut::FinishHeading);
   if (rb != nullptr)
   {
      rb->SetLineColor(UtoColor(0, 255, 0));
   }
}

void WkPlatformMovement::Shortcut::StartGoto()
{
   auto rb = Start(&Shortcut::FinishGoto);
   if (rb != nullptr)
   {
      rb->SetLineColor(UtoColor(255, 0, 0));
   }
}

wkf::AttachmentRubberband* WkPlatformMovement::Shortcut::Start(void (Shortcut::*aFinishCB)(const wkf::AttachmentRubberband&))
{
   using namespace std::placeholders;

   auto source = wkfEnv.GetPlatformOfInterest();

   auto* viewerPtr = vaEnv.GetStandardViewer();
   if (viewerPtr != nullptr && source != nullptr)
   {
      mSourceName = source->GetName();
      return viewerPtr->BeginRubberband(*source,
                                        std::bind(&Shortcut::RubberbandUpdate, this, _1),
                                        std::bind(aFinishCB, this, _1),
                                        "Select location");
   }
   else
   {
      mSourceName.clear();
   }

   return nullptr;
}

void WkPlatformMovement::Shortcut::RubberbandUpdate(wkf::AttachmentRubberband& aRubberband)
{
   // Do nothing for now.
}

void WkPlatformMovement::Shortcut::FinishHeading(const wkf::AttachmentRubberband& aRubberband)
{
   auto* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      auto source = scenarioPtr->FindPlatformByName(mSourceName);
      if (source != nullptr)
      {
         const vespa::VaPosition& beginPosition = source->GetPosition();
         const vespa::VaPosition& endPosition   = aRubberband.GetHoverLocation();
         double                   heading;
         double                   distance;

         UtSphericalEarth::GreatCircleHeadingAndDistance(beginPosition.GetLat(),
                                                         beginPosition.GetLon(), // Begin
                                                         endPosition.GetLat(),
                                                         endPosition.GetLon(), // End
                                                         heading,
                                                         distance); // Result

         TurnToHeading(heading);
      }
   }
}

void WkPlatformMovement::Shortcut::FinishGoto(const wkf::AttachmentRubberband& aRubberband)
{
   auto* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      auto source = vaEnv.GetStandardScenario()->FindPlatformByName(mSourceName);
      if (source != nullptr)
      {
         GoToLocation(aRubberband.GetHoverLocation());
      }
   }
}

void WkPlatformMovement::Shortcut::TurnToHeading(double aHeadingDegrees) const
{
   const wkf::PlatformList& selectedPlatforms = wkfEnv.GetSelectedPlatforms();

   double headingRadians = aHeadingDegrees * UtMath::cRAD_PER_DEG;

   for (const wkf::Platform* platform : selectedPlatforms)
   {
      mSimInterface.AddSimCommand(ut::make_unique<HeadingCommand>(platform->GetName(),
                                                                  headingRadians,
                                                                  0,      // Use default acceleration
                                                                  true)); // Absolute direction
   }
}

void WkPlatformMovement::Shortcut::GoToLocation(const vespa::VaPosition& aPosition) const
{
   const wkf::PlatformList& selectedPlatforms = wkfEnv.GetSelectedPlatforms();

   for (const wkf::Platform* platform : selectedPlatforms)
   {
      mSimInterface.AddSimCommand(ut::make_unique<LocationCommand>(platform->GetName(),
                                                                   aPosition.GetLat(),
                                                                   aPosition.GetLon(),
                                                                   false,   // Do not change altitude
                                                                   0,       // Dummy altitude
                                                                   false)); // Not instant
   }
}
