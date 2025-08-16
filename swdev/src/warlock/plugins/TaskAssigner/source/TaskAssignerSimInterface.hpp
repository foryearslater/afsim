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
#ifndef TASKASSIGNERSIMINTERFACE_HPP
#define TASKASSIGNERSIMINTERFACE_HPP

#include <QObject>

#include "TaskAssignerSimEvents.hpp"
#include "WkSimInterface.hpp"

namespace WkTaskAssigner
{
class SimInterface : public warlock::SimInterfaceT<TaskEvent>
{
   Q_OBJECT

public:
   explicit SimInterface(const QString& aPluginName);

   void SetPlatformOfInterest(const std::string& aPlatformName);

protected:
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override;
   void WallClockRead(const WsfSimulation& aSimulation) override;

private:
   void GetComponentData(const WsfPlatform*                    aPlatformPtr,
                         std::map<std::string, JammerList>&    aJammersData,
                         std::map<std::string, ProcessorList>& aProcessorsData,
                         std::map<std::string, ProcessorList>& aTaskProcessorsData,
                         std::map<std::string, SensorList>&    aSensorsData,
                         std::map<std::string, WeaponList>&    aWeaponsData);

   void GetComponentData(const WsfPlatform* aPlatformPtr,
                         JammerList&        aJammerData,
                         ProcessorList&     aProcessorData,
                         ProcessorList&     aTaskProcessorData,
                         SensorList&        aSensorData,
                         WeaponList&        aWeaponData);

   bool Related(const WsfPlatform& aPlatform);

   std::string mPlatformOfInterest;
};
} // namespace WkTaskAssigner
#endif