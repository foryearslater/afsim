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
#include "PlatformDataUpdaters.hpp"

#include <sstream>

#include "DisIO.hpp"
#include "UtAirspeed.hpp"
#include "UtLatPos.hpp"
#include "UtLonPos.hpp"
#include "UtMath.hpp"
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"
#include "WsfPlatform.hpp"
#include "dis/WsfDisInterface.hpp"
#include "dis/WsfDisPlatform.hpp"

double WkPlatformData::AltitudeUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkPlatformData::AltitudeUpdater::ReadData(WsfPlatform* aPlatform)
{
   mValue = aPlatform->GetAltitude();
   return true;
}

double WkPlatformData::HeadingUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return UtMath::NormalizeAngle0_TwoPi(mValue);
}

bool WkPlatformData::HeadingUpdater::ReadData(WsfPlatform* aPlatform)
{
   double pitch, roll;
   aPlatform->GetOrientationNED(mValue, pitch, roll);
   return true;
}

double WkPlatformData::PitchUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkPlatformData::PitchUpdater::ReadData(WsfPlatform* aPlatform)
{
   double heading, roll;
   aPlatform->GetOrientationNED(heading, mValue, roll);
   return true;
}

double WkPlatformData::RollUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkPlatformData::RollUpdater::ReadData(WsfPlatform* aPlatform)
{
   double heading, pitch;
   aPlatform->GetOrientationNED(heading, pitch, mValue);
   return true;
}

double WkPlatformData::SpeedUpdater::GetValueInDefaultUnits() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkPlatformData::SpeedUpdater::ReadData(WsfPlatform* aPlatform)
{
   mValue = aPlatform->GetSpeed();
   return true;
}

WkPlatformData::MachUpdater::MachUpdater(const std::string& aPlatformName)
   : warlock::PlatformUnitlessUpdater(cUPDATER_NAME, aPlatformName)
{
   SetPrecision(2);
}

double WkPlatformData::MachUpdater::GetValueDouble() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkPlatformData::MachUpdater::ReadData(WsfPlatform* aPlatform)
{
   WsfSpatialDomain spatialDomain = aPlatform->GetSpatialDomain();
   // Restrict mach updater to only calculate mach on air and space domain platforms
   if (spatialDomain == WsfSpatialDomain::WSF_SPATIAL_DOMAIN_AIR ||
       spatialDomain == WsfSpatialDomain::WSF_SPATIAL_DOMAIN_SPACE)
   {
      double alt_m   = aPlatform->GetAltitude();
      double speed_m = aPlatform->GetSpeed();
      mValue         = UtAirspeed::CalculateMachAtAltitude_m(alt_m, speed_m);
      return true;
   }
   return false;
}

double WkPlatformData::LatitudeUpdater::GetValueDouble() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkPlatformData::LatitudeUpdater::ReadData(WsfPlatform* aPlatform)
{
   double lon, alt;
   aPlatform->GetLocationLLA(mValue, lon, alt);
   return true;
}

double WkPlatformData::LongitudeUpdater::GetValueDouble() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkPlatformData::LongitudeUpdater::ReadData(WsfPlatform* aPlatform)
{
   double lat, alt;
   aPlatform->GetLocationLLA(lat, mValue, alt);
   return true;
}

double WkPlatformData::DamageFactorUpdater::GetValueDouble() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkPlatformData::DamageFactorUpdater::ReadData(WsfPlatform* aPlatform)
{
   mValue = aPlatform->GetDamageFactor();
   return true;
}

double WkPlatformData::FuelUpdater::GetValueDouble() const
{
   QMutexLocker locker(&mMutex);
   return mValue;
}

bool WkPlatformData::FuelUpdater::ReadData(WsfPlatform* aPlatform)
{
   mValue = 0;
   if (aPlatform->GetFuel() != nullptr)
   {
      mValue = aPlatform->GetFuelMass();
   }
   return true;
}

QString WkPlatformData::PlatformTypeUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);
   return QString::fromStdString(mPlatformType);
}

bool WkPlatformData::PlatformTypeUpdater::ReadData(WsfPlatform* aPlatform)
{
   mPlatformType = aPlatform->GetType();
   return true;
}

QString WkPlatformData::PositionUpdater::GetValueString() const
{
   QMutexLocker locker(&mMutex);

   UtLatPos latitude(mLatitude_deg, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting());
   UtLonPos longitude(mLongitude_deg, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting());

   std::ostringstream stream;
   stream << latitude << " " << longitude;
   return QString::fromStdString(stream.str());
}

bool WkPlatformData::PositionUpdater::ReadData(WsfPlatform* aPlatform)
{
   double alt;
   aPlatform->GetLocationLLA(mLatitude_deg, mLongitude_deg, alt);
   return true;
}

QString WkPlatformData::PlatformSideUpdater::GetValueString() const
{
   return mPlatformSide.c_str();
}

bool WkPlatformData::PlatformSideUpdater::ReadData(WsfPlatform* aPlatform)
{
   mPlatformSide = aPlatform->GetSide();
   return true;
}

QString WkPlatformData::PlatformDomainUpdater::GetValueString() const
{
   switch (mPlatformDomain)
   {
   case WSF_SPATIAL_DOMAIN_AIR:
      return "Air";
      break;
   case WSF_SPATIAL_DOMAIN_LAND:
      return "Land";
      break;
   case WSF_SPATIAL_DOMAIN_SPACE:
      return "Space";
      break;
   case WSF_SPATIAL_DOMAIN_SUBSURFACE:
      return "Subsurface";
      break;
   case WSF_SPATIAL_DOMAIN_SURFACE:
      return "Surface";
      break;
   case WSF_SPATIAL_DOMAIN_UNKNOWN:
   default:
      return "Unknown";
      break;
   }
}

bool WkPlatformData::PlatformDomainUpdater::ReadData(WsfPlatform* aPlatform)
{
   mPlatformDomain = aPlatform->GetSpatialDomain();
   return true;
}

QString WkPlatformData::PlatformIndexUpdater::GetValueString() const
{
   return QString::number(mIndex);
}

bool WkPlatformData::PlatformIndexUpdater::ReadData(WsfPlatform* aPlatform)
{
   mIndex = aPlatform->GetIndex();
   return true;
}

QString WkPlatformData::PlatformLocalityUpdater::GetValueString() const
{
   return mLocality;
}

bool WkPlatformData::PlatformLocalityUpdater::ReadData(WsfPlatform* aPlatform)
{
   WsfXIO_PlatformInfo* info = simEnv.GetXIO_Info().GetPlatformInfo(aPlatform);
   if (info != nullptr)
   {
      mLocality = "external";
   }
   else if (aPlatform->IsExternallyControlled())
   {
      mLocality = "external (DIS only)";
   }
   else if (aPlatform->IsExternallyMoved())
   {
      mLocality = "externally moved";
   }
   else
   {
      mLocality = "local";
   }
   return true;
}

QString WkPlatformData::PlatformDIS_IdUpdater::GetValueString() const
{
   std::ostringstream oss;
   oss << mDIS_Id;
   return oss.str().c_str();
}

bool WkPlatformData::PlatformDIS_IdUpdater::ReadData(WsfPlatform* aPlatform)
{
   mDIS_Id              = DisEntityId();
   WsfDisInterface* dis = WsfDisInterface::Find(*aPlatform->GetSimulation());
   if (dis != nullptr)
   {
      WsfDisPlatform* disPlatformPtr = dis->GetDisPlatform(aPlatform->GetIndex());
      if (disPlatformPtr != nullptr)
      {
         mDIS_Id = disPlatformPtr->GetEntityId();
      }
   }
   return true;
}

QString WkPlatformData::PlatformDIS_TypeUpdater::GetValueString() const
{
   std::ostringstream oss;
   oss << mDIS_Type;
   return oss.str().c_str();
}

bool WkPlatformData::PlatformDIS_TypeUpdater::ReadData(WsfPlatform* aPlatform)
{
   mDIS_Type            = DisEntityType();
   WsfDisInterface* dis = WsfDisInterface::Find(*aPlatform->GetSimulation());
   if (dis != nullptr)
   {
      WsfDisPlatform* disPlatformPtr = dis->GetDisPlatform(aPlatform->GetIndex());
      if (disPlatformPtr != nullptr)
      {
         mDIS_Type = disPlatformPtr->GetEntityType();
      }
   }
   return true;
}

QString WkPlatformData::PlatformDIS_MarkingUpdater::GetValueString() const
{
   return mDIS_Marking;
}

bool WkPlatformData::PlatformDIS_MarkingUpdater::ReadData(WsfPlatform* aPlatform)
{
   mDIS_Marking = aPlatform->GetMarking().c_str();
   return true;
}

QString WkPlatformData::PlatformAuxDataUpdater::GetValueString() const
{
   return mValue;
}

bool WkPlatformData::PlatformAuxDataUpdater::ReadData(WsfPlatform* aPlatform)
{
   if (aPlatform->GetAuxDataConst().AttributeExists(mDatumName))
   {
      const UtAttributeBase& attribute = aPlatform->GetAuxDataConst().GetAttribute(mDatumName);

      if (!attribute.IsContainerType())
      {
         // What kind?
         if (dynamic_cast<const UtIntAttribute*>(&attribute) != nullptr)
         {
            mValue = std::to_string(dynamic_cast<const UtIntAttribute*>(&attribute)->Get()).c_str();
         }
         else if (dynamic_cast<const UtDoubleAttribute*>(&attribute) != nullptr)
         {
            mValue = std::to_string(dynamic_cast<const UtDoubleAttribute*>(&attribute)->Get()).c_str();
         }
         else if (dynamic_cast<const UtStringAttribute*>(&attribute) != nullptr)
         {
            mValue = dynamic_cast<const UtStringAttribute*>(&attribute)->Get().c_str();
         }
         else if (dynamic_cast<const UtBoolAttribute*>(&attribute) != nullptr)
         {
            dynamic_cast<const UtBoolAttribute*>(&attribute)->Get() ? mValue = "true" : mValue = "false";
         }
      }
   }
   return true;
}
