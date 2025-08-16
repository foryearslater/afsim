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

#ifndef CREATEPLATFORMSIMEVENTS_HPP
#define CREATEPLATFORMSIMEVENTS_HPP

#include <string>
#include <vector>

#include "CreatePlatformDataContainer.hpp"
#include "WkSimInterface.hpp"

namespace WkCreatePlatform
{
class CreatePlatformEvent : public warlock::SimEvent
{
public:
   virtual void Process(DataContainer& aDataContainer) = 0;
};

class SimulationInitializingEvent : public CreatePlatformEvent
{
public:
   SimulationInitializingEvent(const std::vector<std::string>& aPlatformTypesList)
      : mPlatformTypesList(aPlatformTypesList)
   {
   }

   void Process(DataContainer& aDataContainer) override { aDataContainer.SetTypeList(mPlatformTypesList); }

private:
   std::vector<std::string> mPlatformTypesList;
};

} // namespace WkCreatePlatform

#endif
