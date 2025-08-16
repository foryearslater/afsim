// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TunerSimInterface_HPP
#define TunerSimInterface_HPP

#include <list>
#include <set>

#include <QMutex>
#include <QObject>

#include "P6DofTunerGAManager.hpp"
#include "TunerData.hpp"
#include "TunerPrefObject.hpp"
#include "WkSimInterface.hpp"
#include "WsfEvent.hpp"

class WsfSimulation;
class P6DofAtmosphere;
class P6DofCommonController;
class P6DofPilotObject;
class P6DofVehicle;

namespace WkTuner
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

   struct PID_Flags
   {
      bool useAlpha;
      bool limitMax;
      bool zeroGtMax;
      bool zeroLtMin;
      bool useKt;
   };

   void SetLogData(bool aVal);

   void RunAutoTuneCommand();
   void InitializeAutoTuneCommand(int                 aPlatformId,
                                  P6DOF::Pid::Type    aPid,
                                  double              aAltitude_m,
                                  Data::CustomCommand aSpeedType,
                                  double              aSpeed);

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

   void SendCommandBasedOnPID(P6DOF::Pid::Type aPID, double aValue);
   void SendCustomCommand(Data::CustomCommand aCommandType, double aValue);

   void SendPValue(P6DOF::Pid::Type aPID, double aControllingValue, double aValue);
   void SendIValue(P6DOF::Pid::Type aPID, double aControllingValue, double aValue);
   void SendDValue(P6DOF::Pid::Type aPID, double aControllingValue, double aValue);

   void SendMaxAccumulation(P6DOF::Pid::Type aPID, bool aEnabled, double aControllingValue, double aValue);

   void SendRunCommand();
   void SendPauseCommand();
   void SendAdvanceToTimeCommand(double aSimTime, bool aRealTimeFlag = false);

   void   SendSimRateCommand(double aSimClockRate);
   double GetSimRate() { return mSimData.simRate; }

   bool GetFreezeFlags(bool& aAltitude, bool& aSpeed, bool& aPosition, bool& aFuel, bool& aRoll, bool& aPitch, bool& aYaw);

   std::list<double> GetControllingValueList(int aPlatformId, P6DOF::Pid::Type aPid);

   double GetCurrentControllingValue(int aPlatformId);

   bool GetOutputValuesForPid(int aPlatformId, P6DOF::Pid::Type aPid, double& aP, double& aI, double& aD, double& aFeedForward);

   void GetInputValuesForPid(int aPlatformId, P6DOF::Pid::Type aPid, double aControllingValue, double& aP, double& aI, double& aD);

   bool GetStateValuesForPid(int aPlatformId, P6DOF::Pid::Type aPid, double& aCurrentValue, double& aSetPoint);

   std::map<double, P6DOF::PidGainData> GetInputValuesForPid(int aPlatformId, P6DOF::Pid::Type aPid);

   PID_Flags GetFlagsForPid(int aPlatformId, P6DOF::Pid::Type aPid);

   void GetMaxAccumulationValues(int              aPlatformId,
                                 P6DOF::Pid::Type aPid,
                                 double           aControllingValue,
                                 bool&            aUseMaxAccum,
                                 double&          aMaxAccum);

   std::map<int, std::string> GetAndClearNewPlatformMap();
   std::map<int, std::string> GetAndClearDeletedPlatformMap();
   void                       GetRelevantLimits(int aPlatformId, P6DOF::Pid::Type aPid, double& aLowerLimit, double& aUpperLimit, std::string& aUnitName);

   P6DOF::AutopilotLimitsAndSettings GetLimitsAndSettings(int aPlatformId);

   P6DOF::Control::Method GetControlMethod(int aPlatformId);

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
   bool GetRelevantPids(int aPlatformId, std::vector<P6DOF::Pid::Type>& aPidVector);

   void GetOptimumSpecimen(P6DofTunerGASpecimen& aOptimum);

   void             SetPlatformOfInterest(int aPlatformId);
   void             SetPidOfInterest(P6DOF::Pid::Type aPid);
   P6DOF::Pid::Type GetPidOfInterest();
   void             GetPidLog(std::vector<P6DofTunerGASpecimen::PidLogElement>& aPidDataVector);
   void             ClearPidLog();

signals:
   void AdvanceToTimeAchieved();
   void NewP6DofPlatform(std::string aName, int aId);
   void DeletedP6DofPlatform(std::string aName, int aId);
   void AutoTuneComplete(int aPid);
   void AutoTuneProgress(int aProgress);
   void AutoTuneMaximum(int aMax);

private:
   const P6DOF::SinglePidValueData& GetPID_Value(P6DOF::Pid::Type                         aPid,
                                                 const P6DOF::AutopilotPidGroupValueData& aPID_Values) const;

   void LogPidOfInterest(const P6DOF::AutopilotPidGroupValueData& aPID_Values, double aSimTime, const P6DofVehicle* aObject);

   void AFSIM_AutoTune(P6DofPilotObject* aPilotController, WsfSimulation& aSimulation);
   void AFSIM_SendCommandBasedOnPid(P6DOF::Pid::Type aPid, double aValue, P6DofPilotObject* aPilotObject);

   void AFSIM_LoadPidGains(P6DofCommonController*                                            aPilot,
                           std::map<P6DOF::Pid::Type, std::map<double, P6DOF::PidGainData>>& aPidGains,
                           std::map<P6DOF::Pid::Type, PID_Flags>&                            aPidFlags);

   void AFSIM_SendCustomCommand(Data::CustomCommand, double aValue, P6DofPilotObject* aPilotObject);

   void AFSIM_SendPrePositionCommand(double              aAltitude,
                                     Data::CustomCommand aSpeedType,
                                     double              aSpeed,
                                     P6DofPilotObject*   aPilotObject);

   struct GuiData
   {
      GuiData()
         : setPositionPending(false)
         , setPositionAltitude_m(0)
         , setPositionSpeedType(Data::KTAS)
         , setPositionSpeed(0)
         , maxAccumEnable(false)
         , taxiControlPending(false)
         , taxiControl(false)
         , runCommandPending(false)
         , pauseCommandPending(false)
         , advanceToTimeCommandPending(false)
         , advanceRealTime(false)
         , advanceTime(0)
         , simRateCommandPending(false)
         , simRate(1.0)
         , simRateAtTimeCommandPending(false)
      {
      }

      std::list<std::pair<FreezeEnum, bool>> mPendingFreezeCommands;

      bool                setPositionPending;
      double              setPositionAltitude_m;
      Data::CustomCommand setPositionSpeedType;
      double              setPositionSpeed;

      struct Gui2SimData
      {
         Gui2SimData()
            : pending(false)
            , pid(P6DOF::Pid::Alpha)
            , controllingValue(0)
            , value(0)
         {
         }

         bool             pending;
         P6DOF::Pid::Type pid;
         double           controllingValue;
         double           value;
      };

      Gui2SimData command;
      Gui2SimData p;
      Gui2SimData i;
      Gui2SimData d;
      Gui2SimData maxAccum;

      bool maxAccumEnable;
      bool taxiControlPending;
      bool taxiControl;

      struct GuiCustomCommand
      {
         GuiCustomCommand()
            : pending(false)
            , commandType(Data::CustomCommand::KCAS)
            , value(0)
         {
         }

         bool                pending;
         Data::CustomCommand commandType;
         double              value;
      };

      GuiCustomCommand customCommand;

      struct GuiAutoTuneCommand
      {
         GuiAutoTuneCommand()
            : pending(false)
            , running(false)
            , setPositionAltitude_m(0)
            , setPositionSpeedType(Data::KTAS)
            , setPositionSpeed(0)
            , tuningPid(P6DOF::Pid::Alpha)
            , commandVal(0)
            , runTime(0)
            , controllingValue(0)
         {
         }

         bool pending;
         bool running;

         double              setPositionAltitude_m;
         Data::CustomCommand setPositionSpeedType;
         double              setPositionSpeed;

         P6DOF::Pid::Type tuningPid;
         double           commandVal;
         double           runTime;
         double           controllingValue;

         P6dofTunerGAManager  GAManager;
         P6DofTunerGASpecimen parentSpecimen;
         P6DofTunerGASpecimen optimum;
      };

      GuiAutoTuneCommand guiAutoTuneCommand;

      bool runCommandPending;
      bool pauseCommandPending;

      bool   advanceToTimeCommandPending;
      bool   advanceRealTime;
      double advanceTime;

      bool   simRateCommandPending;
      double simRate;

      bool simRateAtTimeCommandPending;
   };
   GuiData mGui;

   struct Platform
   {
      double                 speed_fps                      = 0.0;
      double                 heading_deg                    = 0.0;
      double                 pitch_deg                      = 0.0;
      double                 roll_deg                       = 0.0;
      double                 latitude_deg                   = 0.0;
      double                 longitude_deg                  = 0.0;
      double                 altitude_ft                    = 0.0;
      double                 alpha_deg                      = 0.0;
      double                 beta_deg                       = 0.0;
      double                 thrustPitchVector_deg          = 0.0;
      double                 engineThrottleLeverMilitary    = 0.0;
      double                 engineThrottleLeverAfterburner = 0.0;
      double                 engineThrust_lbs               = 0.0;
      bool                   afterburnerOn                  = false;
      double                 airDensityAtCurrentAlt         = 0.0;
      bool                   crashed                        = false;
      P6DOF::Control::Method controlMethod                  = P6DOF::Control::Undefined;
      bool                   hasLandingGear                 = false;

      P6DOF::AutopilotLimitsAndSettings                                limits;
      P6DOF::AutopilotPidGroupValueData                                pidValues;
      std::map<P6DOF::Pid::Type, std::map<double, P6DOF::PidGainData>> pidGains;
      std::map<P6DOF::Pid::Type, PID_Flags>                            pidFlags;
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

   int                                              mPlatformOfInterest;
   P6DOF::Pid::Type                                 mPidOfInterest;
   std::vector<P6DofTunerGASpecimen::PidLogElement> mPidLog;

   SimData mSimData;

   // All platforms that the P6Dof Tuner plugin has received from the simulation
   std::set<int> mDiscoveredPlatforms;
   // Boolean to indicate if a P6DOF mover has been found in the current scenario.
   bool mFirstP6DofMoverFound;

   // All platforms that have a P6Dof Mover attached to them that have not been added to the GUI.
   // This map is cleared out when the platform is added to the GUI.
   std::map<int, std::string> mNewPlatformMap;
   std::map<int, std::string> mDeletedPlatformMap;

   bool mLogData;

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
      bool          mPauseOnExecute;
      SimInterface* mSimInterface;
   };
};

// This is a work-around to avoid refactoring the P6DOF Tuner plugin to the
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
   SimInterface* mSimInterface;
};
} // namespace WkTuner

#endif
