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
#include "PlatformDataUpdater.hpp"

#include "Platform.hpp"
#include "UtLatPos.hpp"
#include "UtLonPos.hpp"
#include "UtMath.hpp"
#include "WkfScenario.hpp"
#include "WkfVtkEnvironment.hpp"

namespace
{
// The reason the WsfPM_Platform is used instead of the VaEntity/UtEntity data from wkf::Platform is to avoid displaying
// negative floating point zero values that sometimes occur for default values (e.g. heading) Additionally, the concept
// of a altitude reference does not exist on the VaEntity/UtEntity, making it more complicated to use agl altitude values
wizard::Platform* GetWizardPlatform(const std::string& aPlatformName)
{
   wkf::Scenario* scenarioPtr = vaEnv.GetStandardScenario();
   if (scenarioPtr)
   {
      wizard::Platform* curPlat = dynamic_cast<wizard::Platform*>(scenarioPtr->FindPlatformByName(aPlatformName));
      return (curPlat && WsfPM_Root(curPlat->GetPlatform().GetProxy()).platforms().Find(aPlatformName).IsValid()) ?
                curPlat :
                nullptr;
   }
   return nullptr;
}
} // namespace

PlatformData::AltitudeUpdater::AltitudeUpdater(const std::string& aPlatformName)
   : wizard::PlatformAltitudeUpdater(cUPDATER_NAME, aPlatformName)
{
   wizard::Platform* plat = GetWizardPlatform(aPlatformName);
   if (plat)
   {
      float           elevation = 0.0;
      WsfPM_Platform& proxyPlat = plat->GetPlatform();
      // if a route is defined on the platform via a mover, the mover is responsible for the altitude reference instead
      // of the platform itself
      WsfPM_Route                       proxyRoute = proxyPlat.Mover().InitialRoute();
      wsf::proxy::AltitudeReferenceEnum altRef{proxyPlat.EffectiveAGL()};
      if (altRef == wsf::proxy::AltitudeReferenceEnum::AGL)
      {
         wkfEnv.GetResourceManager().QueryElevation(wkf::ResourceManager::cDEFAULT_REQUEST_ID,
                                                    proxyPlat.InitialLocation().GetLatitude(),
                                                    proxyPlat.InitialLocation().GetLongitude(),
                                                    elevation);
      }
      mValue     = UtLengthValue(proxyPlat.EffectiveAltitude()) + elevation;
      mDataValid = true;
   }
}

PlatformData::HeadingUpdater::HeadingUpdater(const std::string& aPlatformName)
   : wizard::PlatformAngleUpdater(cUPDATER_NAME, aPlatformName)
{
   wizard::Platform* plat = GetWizardPlatform(aPlatformName);
   if (plat)
   {
      SetValue(UtAngleValue(plat->GetPlatform().Heading()));
   }
}

PlatformData::LatitudeUpdater::LatitudeUpdater(const std::string& aPlatformName)
   : wizard::PlatformLatitudeUpdater(cUPDATER_NAME, aPlatformName)
{
   wizard::Platform* plat = GetWizardPlatform(aPlatformName);
   if (plat)
   {
      SetValue(plat->GetPlatform().InitialLocation().GetLatitude());
   }
}

PlatformData::LongitudeUpdater::LongitudeUpdater(const std::string& aPlatformName)
   : wizard::PlatformLongitudeUpdater(cUPDATER_NAME, aPlatformName)
{
   wizard::Platform* plat = GetWizardPlatform(aPlatformName);
   if (plat)
   {
      SetValue(plat->GetPlatform().InitialLocation().GetLongitude());
   }
}

PlatformData::PitchUpdater::PitchUpdater(const std::string& aPlatformName)
   : wizard::PlatformAngleUpdater(cUPDATER_NAME, aPlatformName)
{
   wizard::Platform* plat = GetWizardPlatform(aPlatformName);
   if (plat)
   {
      SetValue(UtAngleValue(plat->GetPlatform().Pitch()));
   }
}

PlatformData::RollUpdater::RollUpdater(const std::string& aPlatformName)
   : wizard::PlatformAngleUpdater(cUPDATER_NAME, aPlatformName)
{
   wizard::Platform* plat = GetWizardPlatform(aPlatformName);
   if (plat)
   {
      SetValue(UtAngleValue(plat->GetPlatform().Roll()));
   }
}

PlatformData::SpeedUpdater::SpeedUpdater(const std::string& aPlatformName)
   : wizard::PlatformSpeedUpdater(cUPDATER_NAME, aPlatformName)
{
   wizard::Platform* plat = GetWizardPlatform(aPlatformName);
   if (plat)
   {
      // Do nothing. SpeedUpdater is not used.
   }
}

PlatformData::PlatformTypeUpdater::PlatformTypeUpdater(const std::string& aPlatformName)
   : wizard::PlatformUpdater(cUPDATER_NAME, aPlatformName)
{
   wizard::Platform* plat = GetWizardPlatform(aPlatformName);
   if (plat)
   {
      mPlatformType = plat->GetPlatform().GetTypeName();
      mDataValid    = true; // If the updater uses data other than 'double mValue', this flag MUST be set manually.
   }
}

PlatformData::PositionUpdater::PositionUpdater(const std::string& aPlatformName)
   : wizard::PlatformUpdater(cUPDATER_NAME, aPlatformName)
{
   wizard::Platform* plat = GetWizardPlatform(aPlatformName);
   if (plat)
   {
      mLatitude_deg  = plat->GetPlatform().InitialLocation().GetLatitude();
      mLongitude_deg = plat->GetPlatform().InitialLocation().GetLongitude();
      mDataValid     = true; // If the updater uses data other than 'double mValue', this flag MUST be set manually.
   }
}

double PlatformData::HeadingUpdater::GetValueInDefaultUnits() const
{
   return UtMath::NormalizeAngle0_TwoPi(mValue);
}

QString PlatformData::PlatformTypeUpdater::GetValueString() const
{
   return QString::fromStdString(mPlatformType);
}

QString PlatformData::PositionUpdater::GetValueString() const
{
   UtLatPos latitude(mLatitude_deg, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting());
   UtLonPos longitude(mLongitude_deg, wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting());

   std::ostringstream stream;
   stream << latitude << " " << longitude;
   return QString::fromStdString(stream.str());
}
