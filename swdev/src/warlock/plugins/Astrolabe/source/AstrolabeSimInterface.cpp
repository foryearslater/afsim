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

#include "AstrolabeSimInterface.hpp"

#include "AstrolabeSimEvents.hpp"
#include "UtMemory.hpp"
#include "WsfPlatform.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"
#include "WsfSpaceMoverBase.hpp"

WkAstrolabe::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<AstrolabeEvent>{aPluginName}
{
}

//! Notify the tool if a relevant platform is initialized.
//!
//! Astrolabe is interested in this simulation event so that the tool can keep a list
//! of relevant platforms. This allows Astrolabe to filter platforms
//! which are not relevant, and in so doing, preclude the user from attempting to
//! set a mission sequence on a platform that cannot support one.
//!
//! \param aSimTime  - the time at which the platform was initialized.
//! \param aPlatform - the platform that was initialized.
void WkAstrolabe::SimInterface::PlatformInitialized(double aSimTime, WsfPlatform& aPlatform)
{
   if (!aPlatform.IsExternallyControlled())
   {
      WsfSpaceMoverBase* moverPtr = dynamic_cast<WsfSpaceMoverBase*>(aPlatform.GetMover());
      if (moverPtr)
      {
         AddSimEvent(ut::make_unique<PlatformAvailabilityEvent>(QString::fromStdString(aPlatform.GetName()), true));
      }
   }
}

//! Notify the tool if a relevant platform is deleted.
//!
//! Astrolabe is interested in this simulation event so that the tool can keep a list of
//! relevant platforms. In addition to maintaining the platform filter, this allows the
//! tool to notify the user that a platform that they may have selected to edit was removed.
//!
//! \param aSimTime  - the time at which the platform was deleted.
//! \param aPlatform - the platform that was deleted.
void WkAstrolabe::SimInterface::PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform)
{
   if (!aPlatform.IsExternallyControlled())
   {
      WsfSpaceMoverBase* moverPtr = dynamic_cast<WsfSpaceMoverBase*>(aPlatform.GetMover());
      if (moverPtr)
      {
         AddSimEvent(ut::make_unique<PlatformAvailabilityEvent>(QString::fromStdString(aPlatform.GetName()), false));
      }
   }
}
