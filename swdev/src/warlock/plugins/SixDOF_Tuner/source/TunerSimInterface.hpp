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

#ifndef TUNERSIMINTERFACE_HPP
#define TUNERSIMINTERFACE_HPP

#include <list>
#include <set>

#include <QMutex>
#include <QObject>

#include "TunerData.hpp"
#include "TunerPrefObject.hpp"
#include "WkSimInterface.hpp"
#include "WsfEvent.hpp"
#include "WsfSixDOF_TunerGAManager.hpp"

class WsfSimulation;
namespace wsf
{
namespace six_dof
{
class CommonController;
class PilotObject;
class Mover;
} // namespace six_dof
} // namespace wsf

namespace WkSixDOF_Tuner
{
class SimInterface : public warlock::SimInterfaceBase
{
   Q_OBJECT

public:
   explicit SimInterface(const QString& aPluginName);

   void GuiUpdate();

   // Call these from the AFSIM thread only! NOT the GUI thread!
   void SimulationClockRead(const WsfSimulation& aSimulation) override;
   void SimulationInitializing(const WsfSimulation& aSimulation) override;
   void PlatformDeleted(double aSimeTime, const WsfPlatform& aPlatform) override;

   void WriteData(WsfSimulation& aSimulation);

   double GetSimTime();
   bool   GetSimInitializing();

   enum FreezeEnum
   {
      FRZ_ALTITUDE,
      FRZ_SPEED,
      FRZ_POSITION,
      FRZ_FUEL,
      FRZ_YAW,
      FRZ_PITCH,
      FRZ_ROLL
   };

   enum VehicleType
   {
      Unknown,
      RigidBody,
      PointMass
   };

   void SetLogData(bool aVal);

   void RunAutoTuneCommand();
   void InitializeAutoTuneCommand(int                     aPlatformId,
                                  wsf::six_dof::Pid::Type aPid,
                                  double                  aAltitude_m,
                                  Data::CustomCommand     aSpeedType,
                                  double                  aSpeed);

   void CancelAutoTuneCommand();

   void SetAutoTunePopSize(double aVal);
   void SetAutoTuneMaxGenerations(double aVal);
   void SetAutoTuneCarryOverFrac(double aVal);
   void SetAutoTuneCrossoverProb(double aVal);
   void SetAutoTuneMutationProb(double aVal);

   void SetAutoTuneKpRange(double aMin, double aMax);
   void SetAutoTuneKiRange(double aMin, double aMax);
   void SetAutoTuneKdRange(double aMin, double aMax);
   void SetAutoTuneRunTime(double aVal);
   void SetAutoTuneErrorWeight(double aVal);
   void SetAutoTuneOvershootWeight(double aVal);

   void SendFreezeCommand(FreezeEnum aFreezeCommand, bool aEnabled);

   void SendSetPositionCommand(double aAltitude_m, Data::CustomCommand aSpeedType, double aSpeed);
   void SendTaxiCommand(bool aTaxiControl);

   void SendCommandBasedOnPID(wsf::six_dof::Pid::Type aPID, double aValue);
   void SendCustomCommand(Data::CustomCommand aCommandType, double aValue);

   void SendPValue(wsf::six_dof::Pid::Type aPID, double aControllingValue, double aValue);
   void SendIValue(wsf::six_dof::Pid::Type aPID, double aControllingValue, double aValue);
   void SendDValue(wsf::six_dof::Pid::Type aPID, double aControllingValue, double aValue);

   void SendAlphaValue(wsf::six_dof::Pid::Type aPID, double aControllingValue, double aValue);
   void SendMaxAccumulation(wsf::six_dof::Pid::Type aPID, bool aEnabled, double aControllingValue, double aValue);

   void SendRunCommand();
   void SendPauseCommand();
   void SendAdvanceToTimeCommand(double aSimTime, bool aRealTimeFlag = false);

   void   SendSimRateCommand(double aSimClockRate);
   double GetSimRate() { return mSimData.simRate; }

   bool GetFreezeFlags(bool& aAltitude, bool& aSpeed, bool& aPosition, bool& aFuel, bool& aRoll, bool& aPitch, bool& aYaw);

   std::list<double> GetControllingValueList(int aPlatformId, wsf::six_dof::Pid::Type aPid);

   double GetCurrentControllingValue(int aPlatformId);

   bool GetOutputValuesForPid(int aPlatformId, wsf::six_dof::Pid::Type aPid, double& aP, double& aI, double& aD, double& aFeedForward);

   void GetInputValuesForPid(int                     aPlatformId,
                             wsf::six_dof::Pid::Type aPid,
                             double                  aControllingValue,
                             double&                 aP,
                             double&                 aI,
                             double&                 aD,
                             double&                 aLowpassAlpha);

   bool GetStateValuesForPid(int aPlatformId, wsf::six_dof::Pid::Type aPid, double& aCurrentValue, double& aSetPoint);

   std::map<double, wsf::six_dof::PidGainData> GetInputValuesForPid(int aPlatformId, wsf::six_dof::Pid::Type aPid);

   double GetUpdateIntervalForPid(int aPlatformId, wsf::six_dof::Pid::Type aPid);

   void GetMaxAccumulationValues(int aPlatformId, wsf::six_dof::Pid::Type aPid, double aControllingValue, double& aMaxAccum);

   std::map<int, std::string> GetAndClearNewPlatformMap();
   std::map<int, std::string> GetAndClearDeletedPlatformMap();
   void                       GetRelevantLimits(int                     aPlatformId,
                                                wsf::six_dof::Pid::Type aPid,
                                                double&                 aLowerLimit,
                                                double&                 aUpperLimit,
                                                std::string&            aUnitName);

   wsf::six_dof::AutopilotLimitsAndSettings GetLimitsAndSettings(int aPlatformId);

   wsf::six_dof::Control::Method GetControlMethod(int aPlatformId);

   VehicleType GetType(int aPlatformId);

   bool IsDataValid();
   bool IsSimPaused();

   bool GetRollPitchYaw(int aPlatformId, double& aRoll_deg, double& aPitch_deg, double& aYaw_deg);
   bool GetPosition(int aPlatformId, double& aLatitude_deg, double& aLongitude_deg, double& aAltitude_ft);
   bool GetHeading(int aPlatformId, double& aHeading_deg);
   bool GetSpeed(int aPlatformId, double& aSpeed_fps);
   bool GetThrustVectorAngle(int aPlatformId, double& aThrustVectorPitch_deg);
   bool GetEngineThrottleLeverMilitary(int aPlatformId, double& aEngineThrottleLeverMilitary);
   bool GetEngineThrottleLeverAfterburner(int aPlatformId, double& aEngineThrottleLeverAfterburner);
   bool GetEngineThrust_lbs(int aPlatformId, double& aEngineThrust_lbs);
   bool GetAfterburnerOn(int aPlatformId, bool& aAfterburnerOn);
   bool GetPlatformCrashed(int aPlatformId, bool& aCrashed);
   bool GetRelevantPids(int aPlatformId, std::vector<wsf::six_dof::Pid::Type>& aPidVector);

   void GetOptimumSpecimen(wsf::six_dof::TunerGASpecimen& aOptimum);

   void                    SetPlatformOfInterest(int aPlatformId);
   void                    SetPidOfInterest(wsf::six_dof::Pid::Type aPid);
   wsf::six_dof::Pid::Type GetPidOfInterest();
   void                    GetPidLog(std::vector<wsf::six_dof::TunerGASpecimen::PidLogElement>& aPidDataVector);
   void                    ClearPidLog();

signals:
   void AdvanceToTimeAchieved();
   void NewPlatform(std::string aName, int aId);
   void DeletedPlatform(std::string aName, int aId);
   void AutoTuneComplete(int aPid);
   void AutoTuneProgress(int aProgress);
   void AutoTuneMaximum(int aMax);

private:
   const wsf::six_dof::SinglePidValueData& GetPID_Value(wsf::six_dof::Pid::Type                         aPid,
                                                        const wsf::six_dof::AutopilotPidGroupValueData& aPID_Values) const;

   void LogPidOfInterest(const wsf::six_dof::AutopilotPidGroupValueData& aPID_Values,
                         double                                          aSimTime,
                         const wsf::six_dof::Mover*                      aObject);

   void AFSIM_AutoTune(wsf::six_dof::PilotObject* aPilotController, WsfSimulation& aSimulation);
   void AFSIM_SendCommandBasedOnPid(wsf::six_dof::Pid::Type aPid, double aValue, wsf::six_dof::PilotObject* aPilotObject);

   void AFSIM_LoadPidGains(wsf::six_dof::CommonController*                                                 aPilot,
                           std::map<wsf::six_dof::Pid::Type, std::map<double, wsf::six_dof::PidGainData>>& aPidGains,
                           std::map<wsf::six_dof::Pid::Type, double>& aPidIntervals);

   void AFSIM_SendCustomCommand(Data::CustomCommand, double aValue, wsf::six_dof::PilotObject* aPilotObject);

   void AFSIM_SendPrePositionCommand(double                     aAltitude,
                                     Data::CustomCommand        aSpeedType,
                                     double                     aSpeed,
                                     wsf::six_dof::PilotObject* aPilotObject);

   struct GuiData
   {
      std::list<std::pair<FreezeEnum, bool>> mPendingFreezeCommands;

      bool                setPositionPending    = false;
      double              setPositionAltitude_m = 0.0;
      double              setPositionSpeed      = 0.0;
      Data::CustomCommand setPositionSpeedType  = Data::KTAS;

      struct Gui2SimData
      {
         bool                    pending          = false;
         wsf::six_dof::Pid::Type pid              = wsf::six_dof::Pid::Alpha;
         double                  controllingValue = 0.0;
         double                  value            = 0.0;
      };

      Gui2SimData command;
      Gui2SimData p;
      Gui2SimData i;
      Gui2SimData d;
      Gui2SimData alpha;
      Gui2SimData maxAccum;

      bool maxAccumEnable     = false;
      bool taxiControlPending = false;
      bool taxiControl        = false;

      struct GuiCustomCommand
      {
         bool                pending     = false;
         Data::CustomCommand commandType = Data::CustomCommand::KCAS;
         double              value       = 0.0;
      };

      GuiCustomCommand customCommand;

      struct GuiAutoTuneCommand
      {
         bool pending = false;
         bool running = false;

         double              setPositionAltitude_m = 0.0;
         double              setPositionSpeed      = 0.0;
         Data::CustomCommand setPositionSpeedType  = Data::KTAS;

         wsf::six_dof::Pid::Type tuningPid        = wsf::six_dof::Pid::Alpha;
         double                  commandVal       = 0.0;
         double                  runTime          = 0.0;
         double                  controllingValue = 0.0;

         wsf::six_dof::TunerGAManager  GAManager;
         wsf::six_dof::TunerGASpecimen parentSpecimen;
         wsf::six_dof::TunerGASpecimen optimum;
      };

      GuiAutoTuneCommand guiAutoTuneCommand;

      bool runCommandPending   = false;
      bool pauseCommandPending = false;

      bool   advanceToTimeCommandPending = false;
      bool   advanceRealTime             = false;
      double advanceTime                 = 0.0;

      bool   simRateCommandPending = false;
      double simRate               = 1.0;

      bool simRateAtTimeCommandPending = false;
   };
   GuiData mGui;

   struct Platform
   {
      double                        speed_fps                      = 0.0;
      double                        heading_deg                    = 0.0;
      double                        pitch_deg                      = 0.0;
      double                        roll_deg                       = 0.0;
      double                        latitude_deg                   = 0.0;
      double                        longitude_deg                  = 0.0;
      double                        altitude_ft                    = 0.0;
      double                        alpha_deg                      = 0.0;
      double                        beta_deg                       = 0.0;
      double                        thrustPitchVector_deg          = 0.0;
      double                        engineThrottleLeverMilitary    = 0.0;
      double                        engineThrottleLeverAfterburner = 0.0;
      double                        engineThrust_lbs               = 0.0;
      bool                          afterburnerOn                  = false;
      double                        airDensityAtCurrentAlt         = 0.0;
      bool                          crashed                        = false;
      wsf::six_dof::Control::Method controlMethod                  = wsf::six_dof::Control::Undefined;
      VehicleType                   type                           = VehicleType::Unknown;
      bool                          hasLandingGear                 = false;

      wsf::six_dof::AutopilotLimitsAndSettings                                       limits;
      wsf::six_dof::AutopilotPidGroupValueData                                       pidValues;
      std::map<wsf::six_dof::Pid::Type, std::map<double, wsf::six_dof::PidGainData>> pidGains;
      std::map<wsf::six_dof::Pid::Type, double>                                      pidIntervals;
   };

   struct SimData
   {
      bool                       simDataValid = false;
      std::map<int, Platform>    platformMap;
      std::map<FreezeEnum, bool> freezeFlags;

      double simTime      = 0.0;
      double simRate      = 0.0;
      bool   isPaused     = false;
      bool   initializing = false;
   };

   int                                                       mPlatformOfInterest = -1;
   wsf::six_dof::Pid::Type                                   mPidOfInterest      = wsf::six_dof::Pid::Unknown;
   std::vector<wsf::six_dof::TunerGASpecimen::PidLogElement> mPidLog;

   SimData mSimData;

   // All platforms that the Tuner plugin has received from the simulation
   std::set<int> mDiscoveredPlatforms;
   // Boolean to indicate if a mover has been found in the current scenario.
   bool mFirstMoverFound = false;

   // All platforms that have a relevant Mover attached to them that have not been added to the GUI.
   // This map is cleared out when the platform is added to the GUI.
   std::map<int, std::string> mNewPlatformMap;
   std::map<int, std::string> mDeletedPlatformMap;

   bool mLogData = false;

   // Used by advance to time command
   class AdvanceTimeEvent : public WsfEvent
   {
   public:
      AdvanceTimeEvent(double aSimTime, bool aPauseOnExecute, SimInterface* aSimInterface)
         : WsfEvent(aSimTime)
         , mPauseOnExecute(aPauseOnExecute)
      {
         mSimInterface = aSimInterface;
      }

      EventDisposition Execute() override;

   private:
      // Needed to restore simulation clock when advance time is reached
      bool          mPauseOnExecute = false;
      SimInterface* mSimInterface{nullptr};
   };
};

// This is a work-around to avoid refactoring the Tuner plugin to the
//  newer SimEvent/SimCommands architecture for Warlock Plugins.
class WriteDataEvent : public WsfEvent
{
public:
   WriteDataEvent(SimInterface* aSimInterface)
      : mSimInterface(aSimInterface)
   {
   }

   EventDisposition Execute() override;

private:
   SimInterface* mSimInterface{nullptr};
};
} // namespace WkSixDOF_Tuner

#endif
