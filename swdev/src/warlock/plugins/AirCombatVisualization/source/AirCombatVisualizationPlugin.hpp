// ****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef AIRCOMBATVISUALIZATIONPLUGIN_HPP
#define AIRCOMBATVISUALIZATIONPLUGIN_HPP

#include "AirCombatVisualizationSimInterface.hpp"
#include "VaCallbackHolder.hpp"
#include "WkPlugin.hpp"
#include "WkfPlatform.hpp"
#include "air_combat/WkfAirCombatDataContainer.hpp"
#include "air_combat/WkfAirCombatDisplayInterface.hpp"
#include "air_combat/WkfAirCombatOverlay.hpp"
#include "air_combat/WkfAirCombatVisualizationPrefWidget.hpp"

namespace WkAirCombat
{
class Plugin : public warlock::PluginT<SimInterface>
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

   void                    BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr) override;
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

protected:
   void GuiUpdate() override;

private:
   std::unique_ptr<wkf::AirCombatDisplayInterface>        mDisplayInterfacePtr;
   PluginUiPointer<wkf::AirCombatVisualizationPrefWidget> mPrefWidgetPtr;
   vespa::VaCallbackHolder                                mCallbacks;
   wkf::DataContainer                                     mDataContainer;
   bool                                                   mEnabled;
};
} // namespace WkAirCombat

#endif
