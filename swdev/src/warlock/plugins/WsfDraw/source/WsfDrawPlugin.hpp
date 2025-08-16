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
#ifndef WSFDRAWPLUGIN_HPP
#define WSFDRAWPLUGIN_HPP

#include "WkPlugin.hpp"
#include "WkWsfDrawDockWidget.hpp"
#include "WsfDrawObject.hpp"
#include "WsfDrawSimInterface.hpp"

namespace WkWsfDraw
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

private:
   void BuildViewerContextMenu(QMenu* aMenu, vespa::VaViewer* aViewerPtr) override;
   void GuiUpdate() override;

   // managed by Qt
   WsfDrawObject*          mDrawObjectPtr;
   wkf::WsfDrawDockWidget* mDockWidgetPtr;
};
} // namespace WkWsfDraw

#endif // !WSFDRAWPLUGIN_HPP
