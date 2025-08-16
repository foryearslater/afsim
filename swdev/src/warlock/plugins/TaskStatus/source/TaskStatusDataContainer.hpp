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

#ifndef TASKSTATUSDATACONTAINER_HPP
#define TASKSTATUSDATACONTAINER_HPP

#include <map>
#include <string>
#include <vector>

#include <QObject>

#include "WsfTaskManager.hpp"

namespace WkTaskStatus
{
class ObjectData
{
public:
   ObjectData()
      : mName()
      , mType()
   {
   }

   ObjectData(const std::string& aName, const std::string& aType)
      : mName(aName)
      , mType(aType)
   {
   }

   const std::string& Name() const { return mName; }
   const std::string& Type() const { return mType; }

private:
   std::string mName;
   std::string mType;
};

class ProcessorData : public ObjectData
{
public:
   using TaskList = std::map<WsfTrackId, WsfTaskManager::TaskList>;

   ProcessorData(const std::string& aName,
                 const std::string& aType,
                 const TaskList&    aAssignedTaskList,
                 const TaskList&    aReceivedTaskList)
      : ObjectData(aName, aType)
      , mAssignedTaskList(aAssignedTaskList)
      , mReceivedTaskList(aReceivedTaskList)
   {
   }

   // This function will be necessary once we implement a way to select a track and have the task status dialog
   //  update to show all tasks relating to the selected track. Until then, comment it out.
   //  WsfTaskManager::TaskList GetAssignedData(const WsfTrackId& aTrackId);

   bool operator==(const ProcessorData& aRhs) const;

   TaskList mAssignedTaskList;
   TaskList mReceivedTaskList;
};

class DataContainer : public QObject
{
   Q_OBJECT

public:
   using TaskProcessors = std::map<std::string, std::vector<ProcessorData>>;

   DataContainer()
      : mValid(false)
   {
   }

   bool IsValid() const { return mValid; }

   const std::string& GetPlatformOfInterest() const { return mPlatformOfInterest; }
   void               SetPlatformOfInterest(const std::string& aPlatformName);

   const std::vector<ProcessorData>* GetData(const std::string& aPlatformName) const;
   void SetData(const std::string& aPlatformName, const TaskProcessors& aTaskProcessorData);

signals:
   void TaskChanged();

private:
   bool           mValid;
   std::string    mPlatformOfInterest;
   TaskProcessors mTaskProcessors;
};
} // namespace WkTaskStatus
#endif