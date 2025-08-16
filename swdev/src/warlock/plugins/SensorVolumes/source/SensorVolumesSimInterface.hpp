// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef SENSORVOLUMESSIMINTERFACE_HPP
#define SENSORVOLUMESSIMINTERFACE_HPP

#include <memory>
#include <set>

#include <QObject>

#include "SensorVolumesPlatform.hpp"
#include "WkSimInterface.hpp"
#include "WsfEvent.hpp"
#include "WsfPlatform.hpp"
#include "WsfSensor.hpp"
#include "WsfSimulation.hpp"
#include "misc/WkfImmersiveViewDockWidget.hpp"

namespace vespa
{
class VaEntity;
}

namespace wkf
{
class SensorVolumesPrefObject;
}

#include "SensorVolumesSimEvents.hpp"

namespace WkSensorVolumes
{
class SimInterface : public warlock::SimInterfaceT<SensorVolumeEvent>
{
   Q_OBJECT
public:
   SimInterface(const QString& aPluginName);

   void EnableBoresightUpdates(wkf::ImmersiveViewDockWidget* aWidget, const std::string& aSensorName, bool aEnabled = true);

   WsfPlatform* GetPlatform(const std::string& aPlatName) { return mPlatforms[aPlatName]; }

signals:
   void ShowBoresight(wkf::ImmersiveViewDockWidget* aWidget, const std::string& aSensorName, double aAzimuth, double aElevation);
   void UpdateBoresight(wkf::ImmersiveViewDockWidget* aWidget,
                        const std::string&            aSensorName,
                        double                        aAzimuth,
                        double                        aElevation);
   void HideBoresight(wkf::ImmersiveViewDockWidget* aWidget, const std::string& aSensorName);

protected:
   // Executed on the simulation thread to read and write data from/to the simulation
   void WallClockRead(const WsfSimulation& aSimulation) override;
   void SimulationComplete(const WsfSimulation& aSimulation) override;
   void PlatformInitialized(double aSimTime, WsfPlatform& aPlatform) override
   {
      mPlatforms[aPlatform.GetName()] = &aPlatform;
   }
   void PlatformDeleted(double aSimTime, const WsfPlatform& aPlatform) override
   {
      mPlatforms.erase(aPlatform.GetName());
   }
   void SimulationClockRead(const WsfSimulation& aSimulation) override;

   void ReadData(const WsfSimulation&                                      aSimulation,
                 unsigned int                                              aPlatformIndex,
                 char                                                      aComponents,
                 std::map<unsigned int, VolumeUpdateEvent::PlatformEntry>& aData) const;

private:
   enum
   {
      eSENSOR = 1,
      eWEAPON = 2
   };

   //! Used by ReadData() to fill aEntry with aPlatform's sensor data.
   void ReadSensorData(const WsfSimulation&              aSimulation,
                       const WsfPlatform&                aPlatform,
                       VolumeUpdateEvent::PlatformEntry& aEntry) const;
   //! Used by ReadData() to fill aEntry with aPlatform's weapon data.
   void ReadWeaponData(const WsfSimulation&              aSimulation,
                       const WsfPlatform&                aPlatformIndex,
                       VolumeUpdateEvent::PlatformEntry& aEntry) const;

   std::pair<double, double> GetOrientation(WsfSensor* aSensorPtr);

   std::map<wkf::ImmersiveViewDockWidget*, std::vector<std::string>> mUpdateWidgets;
   std::map<std::string, WsfPlatform*>                               mPlatforms;
};
} // namespace WkSensorVolumes

#endif // !SENSORVOLUMESSIMINTERFACE_HPP
