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
#ifndef PLUGINPLATFORMMOVEMENT_HPP
#define PLUGINPLATFORMMOVEMENT_HPP

#include "PlatformMovementDockWidget.hpp"
#include "PlatformMovementShortcut.hpp"
#include "PlatformMovementSimInterface.hpp"
#include "RouteBrowserInterface.hpp"
#include "WkPlugin.hpp"

namespace WkPlatformMovement
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   void ResetOptionStates() override;

   void           SetPlatformOptionState(int aOptionId, bool aState, wkf::Platform* aPlatformPtr) override;
   Qt::CheckState GetPlatformOptionState(int aOptionId, const wkf::Platform* aPlatformPtr) const override;

   QList<wkf::Action*> GetActions() const override;

protected:
   void GuiUpdate() override;

private:
   void PlatformsAddedHandler(const wkf::PlatformList& aPlatforms);

   // managed by Qt, parented at base class
   RouteBrowserInterface* mRouteBrowserInterface; // not a sim interface
   DockWidget*            mDockWidgetPtr;
   Shortcut*              mShortcutPtr;

   vespa::VaCallbackHolder mPlatformCallbacks;
   QList<wkf::Action*>     mActions;
};
} // namespace WkPlatformMovement
#endif
