// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SIXDOFDATAUPDATERS_HPP
#define SIXDOFDATAUPDATERS_HPP

#include "WkPlatformUpdater.hpp"
#include "WkSimInterface.hpp"
#include "WsfSixDOF_Mover.hpp"

namespace wsf
{
namespace six_dof
{
class Mover;
}
} // namespace wsf

namespace WkSixDOF_Data
{
template<class BASE_UPDATER>
class SixDOF_UpdaterT : public warlock::SimInterfaceBase, public BASE_UPDATER
{
   static_assert(std::is_base_of<wkf::Updater, BASE_UPDATER>::value, "Template type must be subclass of wkf::Updater");

   using BaseUpdater = BASE_UPDATER;

public:
   SixDOF_UpdaterT(const QString& aUpdaterName, const std::string& aPlatformName)
      : SimInterfaceBase(aUpdaterName)
      , BaseUpdater(aUpdaterName, aPlatformName)
      , mLastUpdateTime(-1.0)
   {
   }

   ~SixDOF_UpdaterT() override
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
               auto mover = dynamic_cast<wsf::six_dof::Mover*>(platform->GetMover());
               if (mover)
               {
                  BaseUpdater::mDataValid = ReadData(mover);
               }
               mLastUpdateTime = updateTime;
            }
         }
         mMutex.unlock();
      }
   }

protected:
   // This should be const WsfSixDOF_Mover* aMoverPtr, but some accessors in AFSIM are not const!
   virtual bool ReadData(wsf::six_dof::Mover* aMoverPtr) = 0;

   double mLastUpdateTime;
};


using SixDOF_AltitudeUpdater    = SixDOF_UpdaterT<wkf::AltitudeValueUpdater>;
using SixDOF_AngleUpdater       = SixDOF_UpdaterT<wkf::AngleValueUpdater>;
using SixDOF_AngularRateUpdater = SixDOF_UpdaterT<wkf::AngularRateValueUpdater>;
using SixDOF_ForceUpdater       = SixDOF_UpdaterT<wkf::ForceValueUpdater>;
using SixDOF_MassUpdater        = SixDOF_UpdaterT<wkf::MassValueUpdater>;
using SixDOF_PressureUpdater    = SixDOF_UpdaterT<wkf::PressureValueUpdater>;
using SixDOF_SpeedUpdater       = SixDOF_UpdaterT<wkf::SpeedValueUpdater>;
using SixDOF_UnitlessUpdater    = SixDOF_UpdaterT<wkf::UnitlessValueUpdater>;

class AltitudeUpdater : public SixDOF_AltitudeUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Altitude";

   AltitudeUpdater(const std::string& aPlatformName = "")
      : SixDOF_AltitudeUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class VerticalSpeedUpdater : public SixDOF_SpeedUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Vertical Speed";

   VerticalSpeedUpdater(const std::string& aPlatformName = "")
      : SixDOF_SpeedUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class KCAS_Updater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "KCAS";

   KCAS_Updater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class MachUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Mach";

   MachUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class Q_Updater : public SixDOF_PressureUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Q";

   Q_Updater(const std::string& aPlatformName = "")
      : SixDOF_PressureUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class HeadingUpdater : public SixDOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Heading";

   HeadingUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class PitchUpdater : public SixDOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Pitch";

   PitchUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class RollUpdater : public SixDOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Roll";

   RollUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class NxUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Nx";

   NxUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class NyUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Ny";

   NyUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class G_LoadUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "G-Load";

   G_LoadUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class AvailGsUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Avail Gs";

   AvailGsUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class AlphaUpdater : public SixDOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Alpha";

   AlphaUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class BetaUpdater : public SixDOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Beta";

   BetaUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class AlphaDotUpdater : public SixDOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Alpha-Dot";

   AlphaDotUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class BetaDotUpdater : public SixDOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Beta-Dot";

   BetaDotUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class YawRateUpdater : public SixDOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Yaw Rate";

   YawRateUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class PitchRateUpdater : public SixDOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Pitch Rate";

   PitchRateUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class RollRateUpdater : public SixDOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Roll Rate";

   RollRateUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class PilotModeUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Active Pilot";

   PilotModeUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
      , mPilotType("Unknown")
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;

private:
   QString mPilotType;
};

class LiftUpdater : public SixDOF_ForceUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Lift";

   LiftUpdater(const std::string& aPlatformName = "")
      : SixDOF_ForceUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class DragUpdater : public SixDOF_ForceUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Drag";

   DragUpdater(const std::string& aPlatformName = "")
      : SixDOF_ForceUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class ThrustUpdater : public SixDOF_ForceUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Thrust";

   ThrustUpdater(const std::string& aPlatformName = "")
      : SixDOF_ForceUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class FuelFlowUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Fuel Flow (PPH)";

   FuelFlowUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class FuelCapacityUpdater : public SixDOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Total Fuel Capacity";

   FuelCapacityUpdater(const std::string& aPlatformName = "")
      : SixDOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class FuelRemainingUpdater : public SixDOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Fuel Remaining";

   FuelRemainingUpdater(const std::string& aPlatformName = "")
      : SixDOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class FuelInternalUpdater : public SixDOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Fuel Internal";

   FuelInternalUpdater(const std::string& aPlatformName = "")
      : SixDOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class FuelExternalUpdater : public SixDOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Fuel External";

   FuelExternalUpdater(const std::string& aPlatformName = "")
      : SixDOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class StickBackUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Stick Back";

   StickBackUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class StickRightUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Stick Right";

   StickRightUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class RudderUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Rudder";

   RudderUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class ThrottleUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Throttle";

   ThrottleUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class AfterburnerUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Afterburner";

   AfterburnerUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
      , mOn(false)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;

private:
   bool mOn;
};

class SpeedBrakeCommandUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Speed Brake Lever";

   SpeedBrakeCommandUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class SpeedBrakeAngleUpdater : public SixDOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Speed Brake Angle";

   SpeedBrakeAngleUpdater(const std::string& aPlatformName = "")
      : SixDOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class WeightUpdater : public SixDOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Total Mass";

   WeightUpdater(const std::string& aPlatformName = "")
      : SixDOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(wsf::six_dof::Mover* aMoverPtr) override;
};

class WeightOnWheelsUpdater : public SixDOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Weight On Wheels";

   WeightOnWheelsUpdater(const std::string& aPlatformName = "")
      : SixDOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
      , mOn(false)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(wsf::six_dof::Mover* aMoverPtr) override;

private:
   bool mOn;
};
} // namespace WkSixDOF_Data

#endif
