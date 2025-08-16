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
#ifndef TASKSTATUSCOMMAND_HPP
#define TASKSTATUSCOMMAND_HPP

#include "WkSimInterface.hpp"
#include "WsfTaskResource.hpp"
#include "WsfWeaponTaskResource.hpp"

namespace WkTaskStatus
{
class CancelTaskCommand : public warlock::SimCommand
{
public:
   CancelTaskCommand(const std::string& aPlatformName, const std::string& aTaskProcessorName, unsigned int aTaskId)
      : warlock::SimCommand()
      , mAssignerName(aPlatformName)
      , mTaskProcessorName(aTaskProcessorName)
      , mTaskId(aTaskId)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

protected:
   std::string  mAssignerName;
   std::string  mTaskProcessorName;
   unsigned int mTaskId;
};
} // namespace WkTaskStatus

#endif
