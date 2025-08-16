//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#ifndef ORBITALDATAUPDATERS_HPP
#define ORBITALDATAUPDATERS_HPP

#include "OrbitalDataSimInterface.hpp"
#include "WkPlatformUpdater.hpp"
#include "WkSimInterface.hpp"
#include "WsfSpaceMoverBase.hpp"
#include "dis/WsfDisMover.hpp"

namespace WkOrbitalData
{
template<class BASE_UPDATER>
class OrbitalUpdaterT : public warlock::SimInterfaceBase, public BASE_UPDATER
{
   static_assert(std::is_base_of<wkf::Updater, BASE_UPDATER>::value, "Template type must be subclass of wkf::Updater");

   using BaseUpdater = BASE_UPDATER;

public:
   OrbitalUpdaterT(const QString& aUpdaterName, const std::string& aPlatformName)
      : SimInterfaceBase(aUpdaterName)
      , BaseUpdater(aUpdaterName, aPlatformName)
      , mLastUpdateTime(-1.0)
   {
   }

   ~OrbitalUpdaterT() override
   {
      // Disconnect all of the (direct) connections to signals emitted from the simulation thread
      // so that the object isn't deleted from main thread while being accessed on other thread.
      if (warlock::SimEnvironment::Exists())
      {
         disconnect(&simEnv, nullptr, this, nullptr);
      }
      // Make sure the mutex is not locked prior to delete.
      QMutexLocker locker(&mMutex);
   }

   void WallClockRead(const WsfSimulation& aSimulation) override
   {
      if (mMutex.tryLock())
      {
         WsfPlatform* platform = aSimulation.GetPlatformByName(BaseUpdater::GetPlatformOfInterest());
         if (platform)
         {
            double updateTime = platform->GetLastUpdateTime();
            if (updateTime > mLastUpdateTime)
            {
               WsfSpaceMoverBase* spaceMover = dynamic_cast<WsfSpaceMoverBase*>(platform->GetMover());
               WsfDisMover*       disMover   = dynamic_cast<WsfDisMover*>(platform->GetMover());

               if (spaceMover)
               {
                  auto orbitalElements    = spaceMover->GetOrbitalState().GetOrbitalElementsTOD();
                  BaseUpdater::mDataValid = ReadData(orbitalElements);
               }
               else if (disMover && disMover->GetSpatialDomain() == WSF_SPATIAL_DOMAIN_SPACE)
               {
                  auto cal             = aSimulation.GetDateTime().GetCurrentTime(aSimulation.GetSimTime());
                  auto orbitalElements = std::move(WkOrbitalData::SimInterface::GenerateOrbitalElements(platform, cal));
                  BaseUpdater::mDataValid = ReadData(orbitalElements);
               }

               mLastUpdateTime = updateTime;
            }
         }
         mMutex.unlock();
      }
   }

protected:
   // This should be const WsfSpaceMoverBase* aMoverPtr, but some accessors in AFSIM are not const!
   virtual bool ReadData(const UtOrbitalElements& aElements) = 0;

   double mLastUpdateTime;
};

using OrbitalUnitlessUpdater = OrbitalUpdaterT<wkf::UnitlessValueUpdater>;
using OrbitalLengthUpdater   = OrbitalUpdaterT<wkf::LengthValueUpdater>;
using OrbitalAngleUpdater    = OrbitalUpdaterT<wkf::AngleValueUpdater>;

class EccentricityUpdater : public OrbitalUnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Eccentricity";

   EccentricityUpdater(const std::string& aPlatformName = "")
      : OrbitalUnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(const UtOrbitalElements& aElements) override;
};

class SemiMajorAxisUpdater : public OrbitalLengthUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Semi-major Axis";

   SemiMajorAxisUpdater(const std::string& aPlatformName = "")
      : OrbitalLengthUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(const UtOrbitalElements& aElements) override;
};

class InclinationUpdater : public OrbitalAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Inclination";

   InclinationUpdater(const std::string& aPlatformName = "")
      : OrbitalAngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(const UtOrbitalElements& aElements) override;
};

class RAAN_Updater : public OrbitalAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "RAAN";

   RAAN_Updater(const std::string& aPlatformName = "")
      : OrbitalAngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(const UtOrbitalElements& aElements) override;
};

class ArgOfPeriapsisUpdater : public OrbitalAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Argument of Periapsis";

   ArgOfPeriapsisUpdater(const std::string& aPlatformName = "")
      : OrbitalAngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(const UtOrbitalElements& aElements) override;
};

class TrueAnomalyUpdater : public OrbitalAngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "True Anomaly";

   TrueAnomalyUpdater(const std::string& aPlatformName = "")
      : OrbitalAngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(const UtOrbitalElements& aElements) override;
};
} // namespace WkOrbitalData

#endif
