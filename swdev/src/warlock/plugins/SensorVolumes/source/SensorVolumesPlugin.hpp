// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef SENSORVOLUMESPLUGIN_HPP
#define SENSORVOLUMESPLUGIN_HPP

#include "SensorVolumesSimInterface.hpp"
#include "WkNetwork.hpp"
#include "WkPlatform.hpp"
#include "WkPlugin.hpp"
#include "sensor_volume/WkfSensorVolumesPrefWidget.hpp"

namespace wkf
{
class SensorVolumesPrefObject;
}

class WsfSensorMode;

#include "SensorVolumesPlatform.hpp"

namespace WkSensorVolumes
{
struct SensorVolumePacket;

class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

   void BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr) override;
   void ResetOptionStates() override;

   //! Called automatically when a check box in Platform Options is clicked.
   void SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatformPtr) override;
   //! Returns the expected value of a check box in Platform Options.
   Qt::CheckState GetPlatformOptionState(int aOptionId, const wkf::Platform* aPlatformPtr) const override;

   //! Adds a checkbox to platform options for a sensor or weapon.
   //@{
   void ProcessSensor(unsigned int aEntityId, const std::string& aSensorName);
   void ProcessWeapon(unsigned int aEntityId, const std::string& aWeaponName);
   //@}

   //! Gets an entry to mSensorVolumeMap for the given platform.
   //! Makes a new entry if necessary.
   Platform* GetOrAddSensorVolumeEntry(unsigned int aPlatform, bool aWeapon);

   //! Creates and sends a SensorVolumePacket with all the passive sensors on the platform.
   //! If aPlatform is null or there are no passive sensors, does not send a packet.
   //! Sensor info is not sent if it has been sent within the past cNETWORK_UPDATE_INTERVAL seconds (see cpp file).
   void SendSensorVolumePacket(const warlock::Platform* aPlatform, const VolumeUpdateEvent::PlatformEntry& aEntry);

protected:
   void GuiUpdate() override;
   void ClearIndividualOptions() override;

private:
   //! Called automatically on the GUI thread when a SensorVolumePacket is received.
   //! Prepares the received volumes for display.
   void OnSensorVolumePacketReceived(const SensorVolumePacket& aPacket);

   void RegisterIndividualOptions(const std::string& aStringId) override;
   void AddMode(WsfSensorMode& aSensorMode);
   void PlatformDeletedHandler(vespa::VaEntity* aPlatformPtr);

   void ModesReset();
   void DrawModeChanged(const wkf::SensorVolumesPrefData::DrawMode& aState);

   //! Calls RemoveAttachmentsPastTimeout(...) on all platforms in mSensorVolumeMap.
   //! Removes any attachments that have not had an update in too long. (see cSENSOR_TIMEOUT in cpp file)
   //! Called on GuiUpdate().
   void RemoveAttachmentsPastTimeout();

   PlatformSensorMap mSensorVolumeMap; // platform and weapon/sensor to volume map

   const wkf::SensorVolumesPrefObject* mPrefObjectPtr;
   // Use guarded pointers because objects will be owned by main window
   PluginUiPointer<wkf::SensorVolumesPrefWidget> mPrefWidgetPtr;
   QTreeWidgetItem*                              mTopSensorItem;
   QTreeWidgetItem*                              mTopWeaponItem;
   unsigned int                                  mIndividualIndex;
   std::set<std::string>                         mIndividualsSensors;
   std::set<std::string>                         mIndividualsWeapons;

   vespa::VaCallbackHolder mCallbacks;
   UtCallbackHolder        mNetworkCallbacks;
};
} // namespace WkSensorVolumes
#endif // !SENSORVOLUMESPLUGIN_HPP
