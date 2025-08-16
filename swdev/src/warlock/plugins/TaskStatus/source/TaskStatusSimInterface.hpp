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
#ifndef TASKSTATUSSIMINTERFACE_HPP
#define TASKSTATUSSIMINTERFACE_HPP

#include <QObject>

#include "TaskStatusSimEvents.hpp"
#include "WkSimInterface.hpp"

namespace WkTaskStatus
{
class SimInterface : public warlock::SimInterfaceT<TaskEvent>
{
   Q_OBJECT

public:
   explicit SimInterface(const QString& aPluginName);

   void SetPlatformOfInterest(const std::string& aPlatformName);

protected:
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void WallClockRead(const WsfSimulation& aSimulation) override;

private:
   void GetComponentData(const WsfPlatform*                                 aPlatformPtr,
                         std::map<std::string, std::vector<ProcessorData>>& aTaskProcessorsData);

   void GetComponentData(const WsfPlatform* aPlatformPtr, std::vector<ProcessorData>& aTaskProcessorData);

   std::string mPlatformOfInterest;
};
} // namespace WkTaskStatus
#endif