// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef SATELLITETETHERPLUGIN_HPP
#define SATELLITETETHERPLUGIN_HPP

#include "SatelliteTetherPropagationManager.hpp"
#include "SatelliteTetherSimInterface.hpp"
#include "WkfPlugin.hpp"
#include "satellite_tether/WkfSatelliteTetherPrefWidget.hpp"

namespace wkf
{
class Platform;
class SatelliteTetherDockWidget;
} // namespace wkf

namespace SatelliteTether
{
class DockWidget;

class Plugin : public wkf::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t& aUniqueId);
   ~Plugin() override = default;
   QList<wkf::Action*> GetActions() const override { return mActions; }

   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;
   void GuiUpdate() override;

   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

private:
   void Tether();
   void ConnectToPlatform(const QString& aPlatformName);

   PluginUiPointer<wkf::SatelliteTetherPrefWidget> mPrefWidget;
   QList<wkf::Action*>                             mActions;
   std::map<unsigned int, DockWidget*>             mWidgetList;
   unsigned int                                    mWidgetNamer;
   SatelliteTether::SimInterface*                  mInterfacePtr;
   PropagationManager                              mPropagationManager;
};
} // namespace SatelliteTether
#endif
