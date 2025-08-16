// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SixDOF_DataUpdaters.hpp"

#include "WsfSixDOF_Mover.hpp"
#include "WsfSixDOF_PilotObject.hpp"

double WkSixDOF_Data::AltitudeUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::AltitudeUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetAlt_m();
   return true;
}

double WkSixDOF_Data::VerticalSpeedUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::VerticalSpeedUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetVerticalSpeed();
   return true;
}

QString WkSixDOF_Data::KCAS_Updater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::KCAS_Updater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetSpeed_KCAS();
   return true;
}

QString WkSixDOF_Data::MachUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::MachUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetSpeed_Mach();
   return true;
}

double WkSixDOF_Data::Q_Updater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::Q_Updater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   UtPressureValue v(aMoverPtr->GetDynamicPressure_psf(), UtUnitPressure::cPSF);
   mValue = v;
   return true;
}

double WkSixDOF_Data::HeadingUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkSixDOF_Data::HeadingUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetHeading_deg();
   return true;
}

double WkSixDOF_Data::PitchUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkSixDOF_Data::PitchUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetPitch_deg();
   return true;
}

double WkSixDOF_Data::RollUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkSixDOF_Data::RollUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetRoll_deg();
   return true;
}

QString WkSixDOF_Data::NxUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::NxUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetNx_g();
   return true;
}

QString WkSixDOF_Data::NyUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::NyUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetNy_g();
   return true;
}

QString WkSixDOF_Data::G_LoadUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::G_LoadUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = -aMoverPtr->GetNz_g();
   return true;
}

QString WkSixDOF_Data::AvailGsUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::AvailGsUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->MaxPotentialManeuverGLoad();
   return true;
}

double WkSixDOF_Data::AlphaUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkSixDOF_Data::AlphaUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetAlpha_deg();
   return true;
}

double WkSixDOF_Data::BetaUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkSixDOF_Data::BetaUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetBeta_deg();
   return true;
}

double WkSixDOF_Data::AlphaDotUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::AlphaDotUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetKinematicState()->GetAlphaDot_rps();
   return true;
}

double WkSixDOF_Data::BetaDotUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::BetaDotUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetKinematicState()->GetBetaDot_rps();
   return true;
}

double WkSixDOF_Data::YawRateUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::YawRateUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetYawRate();
   return true;
}

double WkSixDOF_Data::PitchRateUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::PitchRateUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetPitchRate();
   return true;
}

double WkSixDOF_Data::RollRateUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::RollRateUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetRollRate();
   return true;
}

QString WkSixDOF_Data::PilotModeUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return mPilotType;
}

bool WkSixDOF_Data::PilotModeUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mPilotType = QString::fromStdString(aMoverPtr->GetActivePilot());
   return true;
}

double WkSixDOF_Data::LiftUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cNT_PER_LB;
}

bool WkSixDOF_Data::LiftUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetLift_lbs();
   return true;
}

double WkSixDOF_Data::DragUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cNT_PER_LB;
}

bool WkSixDOF_Data::DragUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetDrag_lbs();
   return true;
}

double WkSixDOF_Data::ThrustUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::ThrustUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalForwardThrust();
   return true;
}

QString WkSixDOF_Data::FuelFlowUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue * UtMath::cLB_PER_KG * 3600.0);
}

bool WkSixDOF_Data::FuelFlowUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalFuelFlow();
   return true;
}

double WkSixDOF_Data::FuelCapacityUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::FuelCapacityUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalFuelCapacity();
   return true;
}

double WkSixDOF_Data::FuelRemainingUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::FuelRemainingUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalFuelRemaining();
   return true;
}

double WkSixDOF_Data::FuelInternalUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::FuelInternalUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetInternalFuelRemaining();
   return true;
}

double WkSixDOF_Data::FuelExternalUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::FuelExternalUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetExternalFuelRemaining();
   return true;
}

QString WkSixDOF_Data::StickBackUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::StickBackUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetStickBackPosition();
   return true;
}

QString WkSixDOF_Data::StickRightUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::StickRightUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetStickRightPosition();
   return true;
}

QString WkSixDOF_Data::RudderUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::RudderUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetRudderRightPosition();
   return true;
}

QString WkSixDOF_Data::ThrottleUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::ThrottleUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   wsf::six_dof::PilotObject* pilotObj = aMoverPtr->GetActivePilotObject();
   if (pilotObj != nullptr)
   {
      mValue = pilotObj->GetThrottleControllerPosition();
   }
   return pilotObj != nullptr;
}

QString WkSixDOF_Data::AfterburnerUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return mOn ? "on" : "off";
}

bool WkSixDOF_Data::AfterburnerUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mOn = aMoverPtr->GetAfterburnerOn();
   return true;
}

QString WkSixDOF_Data::SpeedBrakeCommandUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::number(mValue);
}

bool WkSixDOF_Data::SpeedBrakeCommandUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetSpeedBrakeControlPosition();
   return true;
}

double WkSixDOF_Data::SpeedBrakeAngleUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue * UtMath::cRAD_PER_DEG;
}

bool WkSixDOF_Data::SpeedBrakeAngleUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetSpeedBrakePosition();
   return true;
}

double WkSixDOF_Data::WeightUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkSixDOF_Data::WeightUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mValue = aMoverPtr->GetTotalWeight_kg();
   return true;
}

QString WkSixDOF_Data::WeightOnWheelsUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return mOn ? "true" : "false";
}

bool WkSixDOF_Data::WeightOnWheelsUpdater::ReadData(wsf::six_dof::Mover* aMoverPtr)
{
   mOn = aMoverPtr->GetWeightOnWheels();
   return true;
}
