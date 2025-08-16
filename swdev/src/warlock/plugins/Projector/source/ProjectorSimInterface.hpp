// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PROJECTORSIMINTERFACE_HPP
#define PROJECTORSIMINTERFACE_HPP

#include <memory>
#include <set>

#include <QObject>

#include "UtCallbackHolder.hpp"
#include "WkSimInterface.hpp"
#include "WsfEvent.hpp"
#include "WsfSensor.hpp"

namespace vespa
{
class VaEntity;
}

namespace wkf
{
class SensorVolumesPrefObject;
}

#include "ProjectorSimEvents.hpp"

namespace Projector
{
class SimInterface : public warlock::SimInterfaceT<ProjectorEvent>
{
   Q_OBJECT
public:
   SimInterface(const QString& aPluginName);

   void InstallUpdater(WsfSensor& aSensor);
   void UninstallUpdater(const std::string& aPlatformName, const std::string& aSensorName);

protected:
   void SimulationInitializing(const WsfSimulation& aSimulation) override;

   void OnSensorUpdate(double aSimTime, WsfSensor* aSensor);

private:
   class SensorUpdateEvent : public WsfEvent
   {
   public:
      SensorUpdateEvent(SimInterface& aInterface, const std::string& aPlatformName, const std::string& aSensorName)
         : mInterface(aInterface)
         , mPlatformName(aPlatformName)
         , mSensorName(aSensorName)
      {
      }
      ~SensorUpdateEvent() override = default;

      void                       Halt() { mHalt = true; }
      WsfEvent::EventDisposition Execute() override;

   private:
      SimInterface& mInterface;
      std::string   mPlatformName;
      std::string   mSensorName;
      bool          mHalt{false};
   };

   void CheckProjectability(const WsfSensor& aSensor);

   UtCallbackHolder                                                 mCallbacks;
   std::map<std::string, std::map<std::string, SensorUpdateEvent*>> mEventList;
};
} // namespace Projector

#endif // !PROJECTORSIMINTERFACE_HPP
