// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef P6DOF_DATAUPDATERS_HPP
#define P6DOF_DATAUPDATERS_HPP

#include "WkPlatformUpdater.hpp"
#include "WkSimInterface.hpp"
#include "WsfP6DOF_Mover.hpp"

namespace WkP6DOF_Data
{
template<class BASE_UPDATER>
class P6DOF_UpdaterT : public warlock::SimInterfaceBase, public BASE_UPDATER
{
   static_assert(std::is_base_of<wkf::Updater, BASE_UPDATER>::value, "Template type must be subclass of wkf::Updater");

   using BaseUpdater = BASE_UPDATER;

public:
   P6DOF_UpdaterT(const QString& aUpdaterName, const std::string& aPlatformName)
      : SimInterfaceBase(aUpdaterName)
      , BaseUpdater(aUpdaterName, aPlatformName)
      , mLastUpdateTime(-1.0)
   {
   }

   ~P6DOF_UpdaterT() override
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
               WsfP6DOF_Mover* mover = dynamic_cast<WsfP6DOF_Mover*>(platform->GetMover());
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
   // This should be const WsfP6DOF_Mover* aMoverPtr, but some accessors in AFSIM are not const!
   virtual bool ReadData(WsfP6DOF_Mover* aMoverPtr) = 0;

   double mLastUpdateTime;
};


using P6DOF_AltitudeUpdater    = P6DOF_UpdaterT<wkf::AltitudeValueUpdater>;
using P6DOF_AngleUpdater       = P6DOF_UpdaterT<wkf::AngleValueUpdater>;
using P6DOF_AngularRateUpdater = P6DOF_UpdaterT<wkf::AngularRateValueUpdater>;
using P6DOF_ForceUpdater       = P6DOF_UpdaterT<wkf::ForceValueUpdater>;
using P6DOF_MassUpdater        = P6DOF_UpdaterT<wkf::MassValueUpdater>;
using P6DOF_PressureUpdater    = P6DOF_UpdaterT<wkf::PressureValueUpdater>;
using P6DOF_SpeedUpdater       = P6DOF_UpdaterT<wkf::SpeedValueUpdater>;
using P6DOF_UnitlessUpdater    = P6DOF_UpdaterT<wkf::UnitlessValueUpdater>;

class AltitudeUpdater : public P6DOF_AltitudeUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Altitude";

   AltitudeUpdater(const std::string& aPlatformName = "")
      : P6DOF_AltitudeUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class VerticalSpeedUpdater : public P6DOF_SpeedUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Vertical Speed";

   VerticalSpeedUpdater(const std::string& aPlatformName = "")
      : P6DOF_SpeedUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class KCAS_Updater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "KCAS";

   KCAS_Updater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class MachUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Mach";

   MachUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class Q_Updater : public P6DOF_PressureUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Q";

   Q_Updater(const std::string& aPlatformName = "")
      : P6DOF_PressureUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class HeadingUpdater : public P6DOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Heading";

   HeadingUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class PitchUpdater : public P6DOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Pitch";

   PitchUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class RollUpdater : public P6DOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Roll";

   RollUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class NxUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Nx";

   NxUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class NyUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Ny";

   NyUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class G_LoadUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "G-Load";

   G_LoadUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class AvailGsUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Avail Gs";

   AvailGsUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class AlphaUpdater : public P6DOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Alpha";

   AlphaUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class BetaUpdater : public P6DOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Beta";

   BetaUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class AlphaDotUpdater : public P6DOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Alpha-Dot";

   AlphaDotUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class BetaDotUpdater : public P6DOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Beta-Dot";

   BetaDotUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class YawRateUpdater : public P6DOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Yaw Rate";

   YawRateUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class PitchRateUpdater : public P6DOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Pitch Rate";

   PitchRateUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class RollRateUpdater : public P6DOF_AngularRateUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Roll Rate";

   RollRateUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngularRateUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class PilotModeUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Active Pilot";

   PilotModeUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
      , mPilotType("Unknown")
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;

private:
   QString mPilotType;
};

class LiftUpdater : public P6DOF_ForceUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Lift";

   LiftUpdater(const std::string& aPlatformName = "")
      : P6DOF_ForceUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class DragUpdater : public P6DOF_ForceUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Drag";

   DragUpdater(const std::string& aPlatformName = "")
      : P6DOF_ForceUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class ThrustUpdater : public P6DOF_ForceUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Thrust";

   ThrustUpdater(const std::string& aPlatformName = "")
      : P6DOF_ForceUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class FuelFlowUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Fuel Flow (PPH)";

   FuelFlowUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class FuelCapacityUpdater : public P6DOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Total Fuel Capacity";

   FuelCapacityUpdater(const std::string& aPlatformName = "")
      : P6DOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class FuelRemainingUpdater : public P6DOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Fuel Remaining";

   FuelRemainingUpdater(const std::string& aPlatformName = "")
      : P6DOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class FuelInternalUpdater : public P6DOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Fuel Internal";

   FuelInternalUpdater(const std::string& aPlatformName = "")
      : P6DOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class FuelExternalUpdater : public P6DOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Fuel External";

   FuelExternalUpdater(const std::string& aPlatformName = "")
      : P6DOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class StickBackUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Stick Back";

   StickBackUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class StickRightUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Stick Right";

   StickRightUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class RudderUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Rudder";

   RudderUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class ThrottleUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Throttle";

   ThrottleUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class AfterburnerUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Afterburner";

   AfterburnerUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
      , mOn(false)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;

private:
   bool mOn;
};

class SpeedBrakeCommandUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Speed Brake Lever";

   SpeedBrakeCommandUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class SpeedBrakeAngleUpdater : public P6DOF_AngleUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Speed Brake Angle";

   SpeedBrakeAngleUpdater(const std::string& aPlatformName = "")
      : P6DOF_AngleUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class WeightUpdater : public P6DOF_MassUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Total Mass";

   WeightUpdater(const std::string& aPlatformName = "")
      : P6DOF_MassUpdater(cUPDATER_NAME, aPlatformName)
   {
   }

protected:
   double GetValueInDefaultUnits() const override;
   bool   ReadData(WsfP6DOF_Mover* aMoverPtr) override;
};

class WeightOnWheelsUpdater : public P6DOF_UnitlessUpdater
{
public:
   static constexpr const char* cUPDATER_NAME = "Weight On Wheels";

   WeightOnWheelsUpdater(const std::string& aPlatformName = "")
      : P6DOF_UnitlessUpdater(cUPDATER_NAME, aPlatformName)
      , mOn(false)
   {
   }

protected:
   QString GetValueString() const override;
   bool    ReadData(WsfP6DOF_Mover* aMoverPtr) override;

private:
   bool mOn;
};
} // namespace WkP6DOF_Data

#endif
