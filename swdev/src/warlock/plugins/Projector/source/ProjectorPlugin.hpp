// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PROJECTORPLUGIN_HPP
#define PROJECTORPLUGIN_HPP

#include "ProjectorSimInterface.hpp"
#include "WkPlugin.hpp"

namespace Projector
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;


   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

   void UpdateSensor(const std::string& aPlatformName, const std::string& aSensorName, const SensorState& aSensorState);
   void SensorProjectableChanged(const std::string& aPlatformName, const std::string& aSensorName, bool aGood);

private:
   void CreateProjector(wkf::Platform& aPlatform, const QString& aSensorName);
   void RemoveProjector(const QString& aPlatformName, const QString& aSensorName);
   void RemoveProjector(const wkf::Platform& aPlatform, const QString& aSensorName);
   void PlatformDeletedHandler(vespa::VaEntity* aPlatformPtr);
   void GuiUpdate() override;

   void UpdateProjector(const wkf::Platform& aPlatform, unsigned int aProjectorId, const SensorState& aSensorState);
   bool ProjectorExists(const QString& aPlatformName, const QString& aSensorName) const;

   std::map<QString, std::map<QString, SensorState>>  mSensors;
   std::map<QString, std::map<QString, unsigned int>> mProjectorMap;
   std::map<QString, std::set<QString>>               mProjectableSensors;
   vespa::VaCallbackHolder                            mCallbacks;
};
} // namespace Projector
#endif
