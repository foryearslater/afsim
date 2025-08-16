// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PLATFORMPARTBROWSERPLUGIN_HPP
#define PLATFORMPARTBROWSERPLUGIN_HPP

#include "PlatformPartBrowserDataContainer.hpp"
#include "PlatformPartBrowserSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkPlatformPartBrowser
{
class DockWidget;

class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

   void GuiUpdate() override;

private:
   WkPlatformPartBrowser::DockWidget* mDockWidget;

   DataContainer mData;
};
} // namespace WkPlatformPartBrowser

#endif // !PLATFORMPARTBROWSERPLUGIN_HPP
