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

#ifndef VISUALEFFECTSPLUGIN_HPP
#define VISUALEFFECTSPLUGIN_HPP

#include "VisualEffectsSimInterface.hpp"
#include "WkPlugin.hpp"
#include "WkfPlugin.hpp"
#include "WkfPrefWidget.hpp"
#include "visual_effect/WkfVisualEffectsPrefWidget.hpp"

namespace WkVisualEffects
{

class Plugin : public warlock::PluginT<SimInterface>
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

protected:
   void                    GuiUpdate() override;
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

private:
   bool                                          mEnabled;
   PluginUiPointer<wkf::VisualEffectsPrefWidget> mPrefWidgetPtr;
   wkf::VisualEffectsPrefObject*                 mPrefObjectPtr;

   // managed by Qt
   wkf::VisualEffectsDisplayInterface* mDisplayInterfacePtr;
};

} // namespace WkVisualEffects

#endif
