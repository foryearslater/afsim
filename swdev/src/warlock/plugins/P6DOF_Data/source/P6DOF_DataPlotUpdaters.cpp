// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "P6DOF_DataPlotUpdaters.hpp"

#include "UtUnitTypes.hpp"
#include "WsfP6DOF_Mover.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"

void WkP6DOF_Data::PlotUpdater::ReadData(const WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformId);
   if (platform != nullptr)
   {
      WsfP6DOF_Mover* mover = dynamic_cast<WsfP6DOF_Mover*>(platform->GetMover());
      if (mover != nullptr)
      {
         double updateTime = mover->GetLastUpdateTime();
         if (mLastUpdate < updateTime)
         {
            mData.emplace_back(std::pair<double, double>(updateTime, ReadDataP(mover)));
            mLastUpdate = updateTime;
         }
      }
   }
}

WkP6DOF_Data::AltitudePlotUpdater::AltitudePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &AltitudePlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::AltitudePlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitLength::ConvertFromStandard(aMover->GetAlt_m(), mUnitType);
}

void WkP6DOF_Data::AltitudePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
}

double WkP6DOF_Data::VertSpeedPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return aMover->GetVerticalSpeed() * UtMath::cFT_PER_M * 60.0;
}

double WkP6DOF_Data::CasPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return aMover->GetSpeed_KCAS();
}

double WkP6DOF_Data::MachPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return aMover->GetSpeed_Mach();
}

double WkP6DOF_Data::DynamicPressurePlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return aMover->GetDynamicPressure_psf();
}

WkP6DOF_Data::HeadingPlotUpdater::HeadingPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &HeadingPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::HeadingPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetP6DofVehicle()->GetHeading_deg() * UtMath::cRAD_PER_DEG, mUnitType);
}

void WkP6DOF_Data::HeadingPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkP6DOF_Data::PitchPlotUpdater::PitchPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &PitchPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::PitchPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetP6DofVehicle()->GetPitch_deg() * UtMath::cRAD_PER_DEG, mUnitType);
}

void WkP6DOF_Data::PitchPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkP6DOF_Data::RollPlotUpdater::RollPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &RollPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::RollPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetP6DofVehicle()->GetRoll_deg() * UtMath::cRAD_PER_DEG, mUnitType);
}

void WkP6DOF_Data::RollPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

double WkP6DOF_Data::NxPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return aMover->GetNx_g();
}

double WkP6DOF_Data::NyPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return aMover->GetNy_g();
}

double WkP6DOF_Data::GLoadPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return -aMover->GetNz_g();
}

double WkP6DOF_Data::GAvailPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return aMover->GetP6DofVehicle()->MaxPotentialManeuverGLoad();
}

WkP6DOF_Data::AlphaPlotUpdater::AlphaPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &AlphaPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::AlphaPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetAlpha_deg() * UtMath::cRAD_PER_DEG, mUnitType);
}

void WkP6DOF_Data::AlphaPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkP6DOF_Data::BetaPlotUpdater::BetaPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &BetaPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::BetaPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetBeta_deg() * UtMath::cRAD_PER_DEG, mUnitType);
}

void WkP6DOF_Data::BetaPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkP6DOF_Data::AlphaDotPlotUpdater::AlphaDotPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &AlphaDotPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::AlphaDotPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetP6DofVehicle()->GetKinematicState().GetAlphaDot_rps(),
                                                 mUnitType);
}

void WkP6DOF_Data::AlphaDotPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkP6DOF_Data::BetaDotPlotUpdater::BetaDotPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &BetaDotPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::BetaDotPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetP6DofVehicle()->GetKinematicState().GetBetaDot_rps(),
                                                 mUnitType);
}

void WkP6DOF_Data::BetaDotPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkP6DOF_Data::YawRatePlotUpdater::YawRatePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &YawRatePlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::YawRatePlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetYawRate(), mUnitType);
}

void WkP6DOF_Data::YawRatePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkP6DOF_Data::PitchRatePlotUpdater::PitchRatePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &PitchRatePlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::PitchRatePlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetPitchRate(), mUnitType);
}

void WkP6DOF_Data::PitchRatePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkP6DOF_Data::RollRatePlotUpdater::RollRatePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &RollRatePlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::RollRatePlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetRollRate(), mUnitType);
}

void WkP6DOF_Data::RollRatePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkP6DOF_Data::LiftPlotUpdater::LiftPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &LiftPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::LiftPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitForce::ConvertFromStandard(aMover->GetLift_lbs() * UtMath::cNT_PER_LB, mUnitType);
}

void WkP6DOF_Data::LiftPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
}

WkP6DOF_Data::DragPlotUpdater::DragPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &DragPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::DragPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitForce::ConvertFromStandard(aMover->GetDrag_lbs() * UtMath::cNT_PER_LB, mUnitType);
}

void WkP6DOF_Data::DragPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
}

WkP6DOF_Data::ThrustPlotUpdater::ThrustPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &ThrustPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::ThrustPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitForce::ConvertFromStandard(aMover->GetTotalForwardThrust(), mUnitType);
}

void WkP6DOF_Data::ThrustPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
}

WkP6DOF_Data::FuelRemainingPlotUpdater::FuelRemainingPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &FuelRemainingPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::FuelRemainingPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetTotalFuelRemaining(), mUnitType);
}

void WkP6DOF_Data::FuelRemainingPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}

WkP6DOF_Data::FuelInternalPlotUpdater::FuelInternalPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &FuelInternalPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::FuelInternalPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetInternalFuelRemaining(), mUnitType);
}

void WkP6DOF_Data::FuelInternalPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}

WkP6DOF_Data::FuelExternalPlotUpdater::FuelExternalPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &FuelExternalPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::FuelExternalPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetExternalFuelRemaining(), mUnitType);
}

void WkP6DOF_Data::FuelExternalPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}

WkP6DOF_Data::FuelCapacityPlotUpdater::FuelCapacityPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &FuelCapacityPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::FuelCapacityPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetTotalFuelCapacity(), mUnitType);
}

void WkP6DOF_Data::FuelCapacityPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}

double WkP6DOF_Data::FuelFlowPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return aMover->GetTotalFuelFlow() * UtMath::cLB_PER_KG * 3600.0; // Convert from kg/sec to lbs/hr
}

double WkP6DOF_Data::StickBackPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   double            value    = 0.0;
   P6DofPilotObject* pilotObj = aMover->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      return pilotObj->GetStickBackControllerPosition();
   }
   return value;
}

double WkP6DOF_Data::StickRightPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   double            value    = 0.0;
   P6DofPilotObject* pilotObj = aMover->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      return pilotObj->GetStickRightControllerPosition();
   }
   return value;
}

double WkP6DOF_Data::RudderRightPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   double            value    = 0.0;
   P6DofPilotObject* pilotObj = aMover->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      return pilotObj->GetRudderRightControllerPosition();
   }
   return value;
}

double WkP6DOF_Data::ThrottlePlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   double            value    = 0.0;
   P6DofPilotObject* pilotObj = aMover->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      return pilotObj->GetThrottleControllerPosition();
   }
   return value;
}

double WkP6DOF_Data::SpeedbrakeCommandPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   double            value    = 0.0;
   P6DofPilotObject* pilotObj = aMover->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      return pilotObj->GetSpeedBrakesControllerPosition();
   }
   return value;
}

WkP6DOF_Data::SpeedbrakeAnglePlotUpdater::SpeedbrakeAnglePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &SpeedbrakeAnglePlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::SpeedbrakeAnglePlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   double                    value       = 0.0;
   P6DofFlightControlSystem* fltControls = aMover->GetP6DofVehicle()->GetFlightControls();
   if (fltControls != nullptr)
   {
      size_t speedbrakeHandle = fltControls->GetControlSurfaceHandle("Speed_Brake_Extended");

      if (speedbrakeHandle > 0)
      {
         return UtUnitAngle::ConvertFromStandard(fltControls->GetControlSurfaceAngle_deg(speedbrakeHandle) *
                                                    UtMath::cRAD_PER_DEG,
                                                 mUnitType);
      }
   }
   return value;
}

void WkP6DOF_Data::SpeedbrakeAnglePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkP6DOF_Data::TotalWeightPlotUpdater::TotalWeightPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &TotalWeightPlotUpdater::UnitChangedCB);
}

double WkP6DOF_Data::TotalWeightPlotUpdater::ReadDataP(WsfP6DOF_Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetTotalWeight_kg(), mUnitType);
}

void WkP6DOF_Data::TotalWeightPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}
