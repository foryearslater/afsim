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

#include "ProjectorSimInterface.hpp"

#include "UtMemory.hpp"
#include "WkfPlatform.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfEM_Antenna.hpp"
#include "WsfEM_Rcvr.hpp"
#include "WsfPlatform.hpp"
#include "WsfRectangularFieldOfView.hpp"
#include "WsfSensor.hpp"
#include "WsfSensorBeam.hpp"
#include "WsfSensorMode.hpp"
#include "WsfSensorModeList.hpp"
#include "WsfSimulation.hpp"
#include "sensor_volume/WkfSensorVolumesPrefObject.hpp"

Projector::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<ProjectorEvent>(aPluginName)
{
}

void Projector::SimInterface::InstallUpdater(WsfSensor& aSensor)
{
   auto eventPtr = ut::make_unique<SensorUpdateEvent>(*this, aSensor.GetPlatform()->GetName(), aSensor.GetName());
   mEventList[aSensor.GetPlatform()->GetName()][aSensor.GetName()] = eventPtr.get();
   eventPtr->SetTime(aSensor.GetSimulation()->GetSimTime() + 0.1);
   aSensor.GetSimulation()->AddEvent(std::move(eventPtr));
}

void Projector::SimInterface::UninstallUpdater(const std::string& aPlatformName, const std::string& aSensorName)
{
   auto slist = mEventList.find(aPlatformName);
   if (slist != mEventList.end())
   {
      auto s = slist->second.find(aSensorName);
      if (s != slist->second.end())
      {
         s->second->Halt();
      }
   }
}

void Projector::SimInterface::SimulationInitializing(const WsfSimulation& aSimulation)
{
   // add an event on sensor turned on
   // add an event on sensor mode changed
   mCallbacks.Add(WsfObserver::SensorTurnedOn(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor) { CheckProjectability(*aSensor); }));
   mCallbacks.Add(WsfObserver::SensorTurnedOff(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor) { CheckProjectability(*aSensor); }));
   mCallbacks.Add(WsfObserver::SensorModeActivated(&aSimulation)
                     .Connect([this](double aSimTime, WsfSensor* aSensor, WsfSensorMode* aMode)
                              { CheckProjectability(*aSensor); }));
}

void Projector::SimInterface::OnSensorUpdate(double aSimTime, WsfSensor* aSensor)
{
   if ((aSensor != nullptr) && (aSensor->IsTurnedOn()) && (!aSensor->IsBroken())) // is the sensor live?
   {
      SensorState sensorData;
      sensorData.mUsable  = false;
      WsfSensorMode* mode = aSensor->GetCurrentMode();
      WsfSensorBeam* beam = mode->GetBeamEntry(0);
      if (beam != nullptr)
      {
         if (beam->GetEM_Rcvr() != nullptr)
         {
            WsfEM_Antenna* antenna = beam->GetEM_Rcvr()->GetAntenna();
            if (antenna != nullptr)
            {
               const WsfFieldOfView* fovPtr = antenna->GetFieldOfView();
               if (fovPtr != nullptr)
               {
                  const WsfRectangularFieldOfView* rfov = dynamic_cast<const WsfRectangularFieldOfView*>(fovPtr);
                  if (rfov != nullptr)
                  {
                     double amin;
                     double amax;
                     double emin;
                     double emax;
                     rfov->GetAzimuthFieldOfView(amin, amax);
                     rfov->GetAzimuthFieldOfView(emin, emax);
                     sensorData.mAzWidth = amax - amin;
                     sensorData.mElWidth = emax - emin;
                     if ((-amin == amax) && (-emin == emax) && (sensorData.mAzWidth < UtMath::cPI * 0.75) &&
                         (sensorData.mElWidth < UtMath::cPI * 0.75))
                     {
                        sensorData.mTilt   = beam->GetEM_Rcvr()->GetBeamTilt();
                        sensorData.mPitch  = aSensor->GetPitch();
                        sensorData.mYaw    = aSensor->GetYaw();
                        sensorData.mRoll   = aSensor->GetRoll();
                        sensorData.mUsable = true;
                     }
                  }
               }
            }
         }
      }
      else if (aSensor->GetEM_RcvrCount() > 0)
      {
         WsfEM_Antenna* antenna = aSensor->GetEM_Rcvr(0).GetAntenna();
         if (antenna != nullptr)
         {
            const WsfFieldOfView* fovPtr = antenna->GetFieldOfView();
            if (fovPtr != nullptr)
            {
               const WsfRectangularFieldOfView* rfov = dynamic_cast<const WsfRectangularFieldOfView*>(fovPtr);
               if (rfov != nullptr)
               {
                  double amin;
                  double amax;
                  double emin;
                  double emax;
                  rfov->GetAzimuthFieldOfView(amin, amax);
                  rfov->GetAzimuthFieldOfView(emin, emax);
                  sensorData.mAzWidth = amax - amin;
                  sensorData.mElWidth = emax - emin;
                  if ((-amin == amax) && (-emin == emax) && (sensorData.mAzWidth < UtMath::cPI * 0.75) &&
                      (sensorData.mElWidth < UtMath::cPI * 0.75))
                  {
                     sensorData.mTilt   = aSensor->GetEM_Rcvr(0).GetBeamTilt();
                     sensorData.mPitch  = aSensor->GetPitch();
                     sensorData.mYaw    = aSensor->GetYaw();
                     sensorData.mRoll   = aSensor->GetRoll();
                     sensorData.mUsable = true;
                  }
               }
            }
         }
      }
      AddSimEvent(ut::make_unique<UpdateEvent>(aSensor->GetPlatform()->GetName(), aSensor->GetName(), sensorData));
   }
}

void Projector::SimInterface::CheckProjectability(const WsfSensor& aSensor)
{
   bool good = false;
   if ((aSensor.IsTurnedOn()) && (!aSensor.IsBroken())) // is the sensor live?
   {
      WsfSensorMode* mode = aSensor.GetCurrentMode();
      WsfSensorBeam* beam = mode->GetBeamEntry(0);
      if (beam != nullptr)
      {
         if (beam->GetEM_Rcvr() != nullptr)
         {
            WsfEM_Antenna* antenna = beam->GetEM_Rcvr()->GetAntenna();
            if (antenna != nullptr)
            {
               const WsfFieldOfView* fovPtr = antenna->GetFieldOfView();
               if (fovPtr != nullptr)
               {
                  const WsfRectangularFieldOfView* rfov = dynamic_cast<const WsfRectangularFieldOfView*>(fovPtr);
                  if (rfov != nullptr)
                  {
                     double amin;
                     double amax;
                     double emin;
                     double emax;
                     rfov->GetAzimuthFieldOfView(amin, amax);
                     rfov->GetAzimuthFieldOfView(emin, emax);
                     if ((-amin == amax) && (-emin == emax) && (amax - amin < UtMath::cPI * 0.75) &&
                         (emax - emin < UtMath::cPI * 0.75))
                     {
                        good = true;
                     }
                  }
               }
            }
         }
      }
      else if (aSensor.GetEM_RcvrCount() > 0)
      {
         WsfEM_Antenna* antenna = aSensor.GetEM_Rcvr(0).GetAntenna();
         if (antenna != nullptr)
         {
            const WsfFieldOfView* fovPtr = antenna->GetFieldOfView();
            if (fovPtr != nullptr)
            {
               const WsfRectangularFieldOfView* rfov = dynamic_cast<const WsfRectangularFieldOfView*>(fovPtr);
               if (rfov != nullptr)
               {
                  double amin;
                  double amax;
                  double emin;
                  double emax;
                  rfov->GetAzimuthFieldOfView(amin, amax);
                  rfov->GetAzimuthFieldOfView(emin, emax);
                  if ((-amin == amax) && (-emin == emax) && (amax - amin < UtMath::cPI * 0.75) &&
                      (emax - emin < UtMath::cPI * 0.75))
                  {
                     good = true;
                  }
               }
            }
         }
      }
   }
   AddSimEvent(ut::make_unique<SensorProjectableEvent>(aSensor.GetPlatform()->GetName(), aSensor.GetName(), good));
}

WsfEvent::EventDisposition Projector::SimInterface::SensorUpdateEvent::Execute()
{
   WsfEvent::EventDisposition retval{WsfEvent::EventDisposition::cDELETE};

   if (!mHalt)
   {
      WsfPlatform* platformPtr = GetSimulation()->GetPlatformByName(mPlatformName);
      if (platformPtr != nullptr)
      {
         auto sensor = platformPtr->GetComponent<WsfSensor>(mSensorName);

         if (sensor != nullptr)
         {
            double eventTime = GetTime();

            mInterface.OnSensorUpdate(eventTime, sensor);
            SetTime(eventTime + 1.0);
            retval = WsfEvent::EventDisposition::cRESCHEDULE;
         }
      }
   }

   return retval;
}
