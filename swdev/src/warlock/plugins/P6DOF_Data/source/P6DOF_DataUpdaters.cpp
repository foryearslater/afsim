// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "P6DOF_DataUpdaters.hpp"

#include "WsfP6DOF_Mover.hpp"

double WkP6DOF_Data::AltitudeUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::AltitudeUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetAlt_m();
   return true;
}

double WkP6DOF_Data::VerticalSpeedUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::VerticalSpeedUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetVerticalSpeed();
   return true;
}

QString WkP6DOF_Data::KCAS_Updater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::KCAS_Updater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetSpeed_KCAS();
   return true;
}

QString WkP6DOF_Data::MachUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::MachUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetSpeed_Mach();
   return true;
}

double WkP6DOF_Data::Q_Updater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::Q_Updater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   UtPressureValue v(aMoverPtr->GetDynamicPressure_psf(), UtUnitPressure::cPSF);
   mValue = v;
   return true;
}

double WkP6DOF_Data::HeadingUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkP6DOF_Data::HeadingUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetP6DofVehicle()->GetHeading_deg();
   return true;
}

double WkP6DOF_Data::PitchUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkP6DOF_Data::PitchUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetP6DofVehicle()->GetPitch_deg();
   return true;
}

double WkP6DOF_Data::RollUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkP6DOF_Data::RollUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetP6DofVehicle()->GetRoll_deg();
   return true;
}

QString WkP6DOF_Data::NxUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::NxUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetNx_g();
   return true;
}

QString WkP6DOF_Data::NyUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::NyUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetNy_g();
   return true;
}

QString WkP6DOF_Data::G_LoadUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::G_LoadUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = -aMoverPtr->GetNz_g();
   return true;
}

QString WkP6DOF_Data::AvailGsUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::AvailGsUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetP6DofVehicle()->MaxPotentialManeuverGLoad();
   return true;
}

double WkP6DOF_Data::AlphaUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkP6DOF_Data::AlphaUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetAlpha_deg();
   return true;
}

double WkP6DOF_Data::BetaUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkP6DOF_Data::BetaUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetBeta_deg();
   return true;
}

double WkP6DOF_Data::AlphaDotUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::AlphaDotUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetP6DofVehicle()->GetKinematicState().GetAlphaDot_rps();
   return true;
}

double WkP6DOF_Data::BetaDotUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::BetaDotUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetP6DofVehicle()->GetKinematicState().GetBetaDot_rps();
   return true;
}

double WkP6DOF_Data::YawRateUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::YawRateUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetYawRate();
   return true;
}

double WkP6DOF_Data::PitchRateUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::PitchRateUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetPitchRate();
   return true;
}

double WkP6DOF_Data::RollRateUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::RollRateUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetRollRate();
   return true;
}

QString WkP6DOF_Data::PilotModeUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return mPilotType;
}

bool WkP6DOF_Data::PilotModeUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   P6DofPilotObject* pilotObj = aMoverPtr->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      mPilotType = pilotObj->GetPilotType().c_str();
   }
   return pilotObj != nullptr;
}

double WkP6DOF_Data::LiftUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cNT_PER_LB;
}

bool WkP6DOF_Data::LiftUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetLift_lbs();
   return true;
}

double WkP6DOF_Data::DragUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cNT_PER_LB;
}

bool WkP6DOF_Data::DragUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetDrag_lbs();
   return true;
}

double WkP6DOF_Data::ThrustUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::ThrustUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalForwardThrust();
   return true;
}

QString WkP6DOF_Data::FuelFlowUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue * UtMath::cLB_PER_KG * 3600.0);
}

bool WkP6DOF_Data::FuelFlowUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalFuelFlow();
   return true;
}

double WkP6DOF_Data::FuelCapacityUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::FuelCapacityUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalFuelCapacity();
   return true;
}

double WkP6DOF_Data::FuelRemainingUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::FuelRemainingUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalFuelRemaining();
   return true;
}

double WkP6DOF_Data::FuelInternalUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::FuelInternalUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetInternalFuelRemaining();
   return true;
}

double WkP6DOF_Data::FuelExternalUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::FuelExternalUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetExternalFuelRemaining();
   return true;
}

QString WkP6DOF_Data::StickBackUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::StickBackUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   P6DofPilotObject* pilotObj = aMoverPtr->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      mValue = pilotObj->GetStickBackControllerPosition();
   }
   return pilotObj != nullptr;
}

QString WkP6DOF_Data::StickRightUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::StickRightUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   P6DofPilotObject* pilotObj = aMoverPtr->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      mValue = pilotObj->GetStickRightControllerPosition();
   }
   return pilotObj != nullptr;
}

QString WkP6DOF_Data::RudderUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::RudderUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   P6DofPilotObject* pilotObj = aMoverPtr->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      mValue = pilotObj->GetRudderRightControllerPosition();
   }
   return pilotObj != nullptr;
}

QString WkP6DOF_Data::ThrottleUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::ThrottleUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   P6DofPilotObject* pilotObj = aMoverPtr->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      mValue = pilotObj->GetThrottleControllerPosition();
   }
   return pilotObj != nullptr;
}

QString WkP6DOF_Data::AfterburnerUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return mOn ? "on" : "off";
}

bool WkP6DOF_Data::AfterburnerUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mOn = aMoverPtr->GetAfterburnerOn();
   return true;
}

QString WkP6DOF_Data::SpeedBrakeCommandUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkP6DOF_Data::SpeedBrakeCommandUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   P6DofPilotObject* pilotObj = aMoverPtr->GetP6DofVehicle()->GetPilotObject();
   if (pilotObj != nullptr)
   {
      mValue = pilotObj->GetSpeedBrakesControllerPosition();
   }
   return pilotObj != nullptr;
}

double WkP6DOF_Data::SpeedBrakeAngleUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkP6DOF_Data::SpeedBrakeAngleUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   P6DofFlightControlSystem* fltControls = aMoverPtr->GetP6DofVehicle()->GetFlightControls();
   if (fltControls != nullptr)
   {
      size_t speedbrakeHandle = fltControls->GetControlSurfaceHandle("Speed_Brake_Extended");

      if (speedbrakeHandle > 0)
      {
         mValue = fltControls->GetControlSurfaceAngle_deg(speedbrakeHandle);
         return true;
      }
   }
   else
   {
      mValue = 0.0;
   }
   return false;
}

double WkP6DOF_Data::WeightUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkP6DOF_Data::WeightUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalWeight_kg();
   return true;
}

QString WkP6DOF_Data::WeightOnWheelsUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return mOn ? "true" : "false";
}

bool WkP6DOF_Data::WeightOnWheelsUpdater::ReadData(WsfP6DOF_Mover* aMoverPtr)
{
   mOn = aMoverPtr->GetWeightOnWheels();
   return true;
}
