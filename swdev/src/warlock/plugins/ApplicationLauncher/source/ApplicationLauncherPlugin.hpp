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
#ifndef ApplicationLauncherPlugin_HPP
#define ApplicationLauncherPlugin_HPP

#include <QPointer>

#include "ApplicationLauncherToolbar.hpp"
#include "WkPlugin.hpp"


namespace WkApplicationLauncher
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;
};
} // namespace WkApplicationLauncher
#endif
