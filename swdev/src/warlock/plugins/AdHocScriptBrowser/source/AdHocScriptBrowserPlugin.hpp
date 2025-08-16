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
#ifndef WKPLUGINADHOCSCRIPTBROWSER_HPP
#define WKPLUGINADHOCSCRIPTBROWSER_HPP

#include <QSharedPointer>

#include "AdHocScriptBrowserDockWidget.hpp"
#include "WkfPlugin.hpp"

namespace WkAdHocScriptBrowser
{
class Plugin : public wkf::Plugin
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

private:
   void DialogActionTriggered();

   PluginUiPointer<DockWidget> mDockWidgetPtr;
};
} // namespace WkAdHocScriptBrowser
#endif
