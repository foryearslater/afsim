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

#ifndef ASTROLABEPLUGIN_HPP
#define ASTROLABEPLUGIN_HPP

#include "AstrolabeDockWidget.hpp"
#include "AstrolabeSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkAstrolabe
{

class EventInterface;

class Plugin : public warlock::PluginT<SimInterface>
{
public:
   explicit Plugin(const QString& aPluginName, size_t aUniqueId);
   ~Plugin() override = default;

   void BuildEntityContextMenu(QMenu* aMenuPtr, wkf::Entity* aEntityPtr) override;

protected:
   void GuiUpdate() override;

private:
   DockWidget*     mDockWidgetPtr;
   EventInterface* mEventInterfacePtr;
};

} // namespace WkAstrolabe

#endif // ASTROLABEPLUGIN_HPP
