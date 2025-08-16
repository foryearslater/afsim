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
#ifndef DEMOBROWSERPLUGIN_HPP
#define DEMOBROWSERPLUGIN_HPP

#include "DemoBrowserDialog.hpp"
#include "Plugin.hpp"

namespace DemoBrowser
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

   QList<wkf::StartupWidgetWrapper*> GetStartupWidgets() const override;
   void                              Show();

private:
   DemoBrowserDialog* mDialog = nullptr;
};
} // namespace DemoBrowser
#endif
