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
#ifndef TASKASSIGNERCOMMAND_HPP
#define TASKASSIGNERCOMMAND_HPP

#include "WkSimInterface.hpp"
#include "WsfTaskResource.hpp"
#include "WsfWeaponTaskResource.hpp"

namespace WkTaskAssigner
{
class TaskCommand : public warlock::SimCommand
{
public:
   TaskCommand(const std::string& aTaskProcessorName,
               const std::string& aAssignerName,
               int                aTrackIndex,
               const std::string& aTaskType,
               const std::string& aAssigneeName,
               const std::string& aCommName)
      : warlock::SimCommand()
      , mTaskProcessorName(aTaskProcessorName)
      , mAssignerName(aAssignerName)
      , mTrackIndex(aTrackIndex)
      , mTaskType(aTaskType)
      , mAssigneeName(aAssigneeName)
      , mCommName(aCommName)
   {
   }

protected:
   void AssignTask(WsfSimulation& aSimulation, const WsfTaskResource& aResource);

   std::string mTaskProcessorName;
   std::string mAssignerName;
   int         mTrackIndex;
   std::string mTaskType;
   std::string mAssigneeName;
   std::string mCommName;
};

class AssignTaskCommand : public TaskCommand
{
public:
   AssignTaskCommand(const std::string&     aTaskProcessorName,
                     const std::string&     aAssignerName,
                     int                    aTrackIndex,
                     const std::string&     aTaskType,
                     const WsfTaskResource& aResource,
                     const std::string&     aAssigneeName,
                     const std::string&     aCommName)
      : TaskCommand(aTaskProcessorName, aAssignerName, aTrackIndex, aTaskType, aAssigneeName, aCommName)
      , mResource(aResource)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

protected:
   WsfTaskResource mResource;
};

class MilAssignJammerTaskCommand : public TaskCommand
{
public:
   MilAssignJammerTaskCommand(const std::string&           aTaskProcessorName,
                              const std::string&           aAssignerName,
                              int                          aTrackIndex,
                              const std::string&           aTaskType,
                              const WsfJammerTaskResource& aResource,
                              const std::string&           aAssigneeName,
                              const std::string&           aCommName)
      : TaskCommand(aTaskProcessorName, aAssignerName, aTrackIndex, aTaskType, aAssigneeName, aCommName)
      , mResource(aResource)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

protected:
   WsfJammerTaskResource mResource;
};

class MilAssignWeaponTaskCommand : public TaskCommand
{
public:
   MilAssignWeaponTaskCommand(const std::string&           aTaskProcessorName,
                              const std::string&           aAssignerName,
                              int                          aTrackIndex,
                              const std::string&           aTaskType,
                              const WsfWeaponTaskResource& aResource,
                              const std::string&           aAssigneeName,
                              const std::string&           aCommName)
      : TaskCommand(aTaskProcessorName, aAssignerName, aTrackIndex, aTaskType, aAssigneeName, aCommName)
      , mResource(aResource)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

protected:
   WsfWeaponTaskResource mResource;
};

} // namespace WkTaskAssigner

#endif // TASKASSIGNERCOMMAND_HPP
