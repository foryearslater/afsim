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

#include "AstrolabeEventInterface.hpp"

#include "AstrolabeDockWidget.hpp"
#include "WsfOrbitalMissionSequence.hpp"

void WkAstrolabe::EventInterface::SetPlatformAvailability(const QString& aPlatformName, bool aAvailability)
{
   mDockWidget.SetPlatformAvailability(aPlatformName, aAvailability);
}

void WkAstrolabe::EventInterface::SetMissionVerificationResults(const wsfg::spaceg::MissionVerificationResults& aResults)
{
   mDockWidget.SetMissionVerificationResults(aResults);
}

void WkAstrolabe::EventInterface::FinishVerify(std::unique_ptr<WsfOrbitalMissionSequence> aSequencePtr)
{
   mDockWidget.FinishVerify(std::move(aSequencePtr));
}

void WkAstrolabe::EventInterface::FinishApply(std::unique_ptr<WsfOrbitalMissionSequence> aSequencePtr)
{
   mDockWidget.FinishApply(std::move(aSequencePtr));
}
