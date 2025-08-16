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

#ifndef CYBERENGAGEMENTCONTROLLERSIMEVENTS_HPP
#define CYBERENGAGEMENTCONTROLLERSIMEVENTS_HPP

#include <string>
#include <vector>

#include "CyberEngagementControllerDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkCyberEngagementController
{
class CyberEngagementControllerEvent : public warlock::SimEvent
{
public:
   virtual void Process(DataContainer& aDataContainer) = 0;
};

class SimulationInitializedEvent : public CyberEngagementControllerEvent
{
public:
   explicit SimulationInitializedEvent(std::vector<std::string>& aAttackTypes)
      : mAttackTypes(aAttackTypes)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.SetTypes(mAttackTypes); }

private:
   std::vector<std::string> mAttackTypes;
};

} // namespace WkCyberEngagementController

#endif
