// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "CreatePlatformSimCommand.hpp"

#include <memory>

#include "WkObserver.hpp"
#include "WkfQueueableMessageObject.hpp"
#include "WsfPlatformTypes.hpp"
#include "WsfSimulation.hpp"

void WkCreatePlatform::CreatePlatformCommand::Process(WsfSimulation& aSimulation)
{
   auto newPlatform = std::unique_ptr<WsfPlatform>(WsfPlatformTypes::Get(aSimulation.GetScenario()).Clone(mType));
   newPlatform->SetName(mName);
   newPlatform->SetSide(mSide);
   newPlatform->SetLocationLLA(mLat, mLon, mAlt);
   newPlatform->SetOrientationNED(mHeading, 0, 0);
   double velocity[3] = {mSpeed * cos(mHeading), mSpeed * sin(mHeading), 0};
   newPlatform->SetVelocityNED(velocity);

   if (!aSimulation.AddPlatform(newPlatform.get()))
   {
      wkf::QueueableMessageObject::DisplayQueuedMessage(QMessageBox::Information,
                                                        "Platform not created",
                                                        "The platform \"" + QString::fromStdString(mName) +
                                                           "\" could not be created.");
   }
   else
   {
      WkObserver::SimulationUserAction(aSimulation)("New platform created", newPlatform.get());
      newPlatform.release(); // The simulation has taken ownership of the platform.
   }
}
