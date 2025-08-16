// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SixDOF_DataPlotUpdaters.hpp"

#include "UtUnitTypes.hpp"
#include "WsfPlatform.hpp"
#include "WsfSimulation.hpp"
#include "WsfSixDOF_Mover.hpp"
#include "WsfSixDOF_PilotObject.hpp"

void WkSixDOF_Data::PlotUpdater::ReadData(const WsfSimulation& aSimulation)
{
   WsfPlatform* platform = aSimulation.GetPlatformByName(mPlatformId);
   if (platform != nullptr)
   {
      auto mover = dynamic_cast<wsf::six_dof::Mover*>(platform->GetMover());
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

WkSixDOF_Data::AltitudePlotUpdater::AltitudePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &AltitudePlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::AltitudePlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitLength::ConvertFromStandard(aMover->GetAlt_m(), mUnitType);
}

void WkSixDOF_Data::AltitudePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLowAltitudeUnit();
   SetUnit(unit, QString::fromStdString(UtUnitLength::FindUnitName(unit)));
}

double WkSixDOF_Data::VertSpeedPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetVerticalSpeed() * UtMath::cFT_PER_M * 60.0;
}

double WkSixDOF_Data::CasPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetSpeed_KCAS();
}

double WkSixDOF_Data::MachPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetSpeed_Mach();
}

double WkSixDOF_Data::DynamicPressurePlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetDynamicPressure_psf();
}

WkSixDOF_Data::HeadingPlotUpdater::HeadingPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &HeadingPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::HeadingPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetHeading_rad(), mUnitType);
}

void WkSixDOF_Data::HeadingPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkSixDOF_Data::PitchPlotUpdater::PitchPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &PitchPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::PitchPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetPitch_rad(), mUnitType);
}

void WkSixDOF_Data::PitchPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkSixDOF_Data::RollPlotUpdater::RollPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &RollPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::RollPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetRoll_rad(), mUnitType);
}

void WkSixDOF_Data::RollPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

double WkSixDOF_Data::NxPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetNx_g();
}

double WkSixDOF_Data::NyPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetNy_g();
}

double WkSixDOF_Data::GLoadPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return -aMover->GetNz_g();
}

double WkSixDOF_Data::GAvailPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->MaxPotentialManeuverGLoad();
}

WkSixDOF_Data::AlphaPlotUpdater::AlphaPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &AlphaPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::AlphaPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetAlpha_deg() * UtMath::cRAD_PER_DEG, mUnitType);
}

void WkSixDOF_Data::AlphaPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkSixDOF_Data::BetaPlotUpdater::BetaPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &BetaPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::BetaPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetBeta_rad(), mUnitType);
}

void WkSixDOF_Data::BetaPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkSixDOF_Data::AlphaDotPlotUpdater::AlphaDotPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &AlphaDotPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::AlphaDotPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetAlphaDot_rps(), mUnitType);
}

void WkSixDOF_Data::AlphaDotPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkSixDOF_Data::BetaDotPlotUpdater::BetaDotPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &BetaDotPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::BetaDotPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetBetaDot_rps(), mUnitType);
}

void WkSixDOF_Data::BetaDotPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkSixDOF_Data::YawRatePlotUpdater::YawRatePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &YawRatePlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::YawRatePlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetYawRate(), mUnitType);
}

void WkSixDOF_Data::YawRatePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkSixDOF_Data::PitchRatePlotUpdater::PitchRatePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &PitchRatePlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::PitchRatePlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetPitchRate(), mUnitType);
}

void WkSixDOF_Data::PitchRatePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkSixDOF_Data::RollRatePlotUpdater::RollRatePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &RollRatePlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::RollRatePlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngularRate::ConvertFromStandard(aMover->GetRollRate(), mUnitType);
}

void WkSixDOF_Data::RollRatePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngularRateUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngularRate::FindUnitName(unit)));
}

WkSixDOF_Data::LiftPlotUpdater::LiftPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &LiftPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::LiftPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitForce::ConvertFromStandard(aMover->GetLift_nt(), mUnitType);
}

void WkSixDOF_Data::LiftPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
}

WkSixDOF_Data::DragPlotUpdater::DragPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &DragPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::DragPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitForce::ConvertFromStandard(aMover->GetDrag_nt(), mUnitType);
}

void WkSixDOF_Data::DragPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
}

WkSixDOF_Data::ThrustPlotUpdater::ThrustPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &ThrustPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::ThrustPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitForce::ConvertFromStandard(aMover->GetTotalForwardThrust(), mUnitType);
}

void WkSixDOF_Data::ThrustPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetForceUnit();
   SetUnit(unit, QString::fromStdString(UtUnitForce::FindUnitName(unit)));
}

WkSixDOF_Data::FuelRemainingPlotUpdater::FuelRemainingPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &FuelRemainingPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::FuelRemainingPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetTotalFuelRemaining(), mUnitType);
}

void WkSixDOF_Data::FuelRemainingPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}

WkSixDOF_Data::FuelInternalPlotUpdater::FuelInternalPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &FuelInternalPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::FuelInternalPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetInternalFuelRemaining(), mUnitType);
}

void WkSixDOF_Data::FuelInternalPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}

WkSixDOF_Data::FuelExternalPlotUpdater::FuelExternalPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &FuelExternalPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::FuelExternalPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetExternalFuelRemaining(), mUnitType);
}

void WkSixDOF_Data::FuelExternalPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}

WkSixDOF_Data::FuelCapacityPlotUpdater::FuelCapacityPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &FuelCapacityPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::FuelCapacityPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetTotalFuelCapacity(), mUnitType);
}

void WkSixDOF_Data::FuelCapacityPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}

double WkSixDOF_Data::FuelFlowPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetTotalFuelFlow() * UtMath::cLB_PER_KG * 3600.0; // Convert from kg/sec to lbs/hr
}

double WkSixDOF_Data::StickBackPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetStickBackPosition();
}

double WkSixDOF_Data::StickRightPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetStickRightPosition();
}

double WkSixDOF_Data::RudderRightPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetRudderRightPosition();
}

double WkSixDOF_Data::ThrottlePlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   double                     value    = 0.0;
   wsf::six_dof::PilotObject* pilotObj = aMover->GetActivePilotObject();
   if (pilotObj != nullptr)
   {
      return pilotObj->GetThrottleControllerPosition();
   }
   return value;
}

double WkSixDOF_Data::SpeedbrakeCommandPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return aMover->GetSpeedBrakeControlPosition();
}

WkSixDOF_Data::SpeedbrakeAnglePlotUpdater::SpeedbrakeAnglePlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &SpeedbrakeAnglePlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::SpeedbrakeAnglePlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitAngle::ConvertFromStandard(aMover->GetSpeedBrakePosition(), mUnitType);
}

void WkSixDOF_Data::SpeedbrakeAnglePlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetAngleUnit();
   SetUnit(unit, QString::fromStdString(UtUnitAngle::FindUnitName(unit)));
}

WkSixDOF_Data::TotalWeightPlotUpdater::TotalWeightPlotUpdater(const std::string& aId, unsigned int aSeriesNum)
   : PlotUpdater(aId, aSeriesNum)
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::UnitsChanged,
           this,
           &TotalWeightPlotUpdater::UnitChangedCB);
}

double WkSixDOF_Data::TotalWeightPlotUpdater::ReadDataP(wsf::six_dof::Mover* aMover)
{
   return UtUnitMass::ConvertFromStandard(aMover->GetTotalWeight_kg(), mUnitType);
}

void WkSixDOF_Data::TotalWeightPlotUpdater::UnitChangedCB()
{
   int unit = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetMassUnit();
   SetUnit(unit, QString::fromStdString(UtUnitMass::FindUnitName(unit)));
}
