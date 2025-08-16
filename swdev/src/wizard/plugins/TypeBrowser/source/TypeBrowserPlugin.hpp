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
#ifndef TYPEBROWSERPLUGIN_HPP
#define TYPEBROWSERPLUGIN_HPP

#include "DockWidget.hpp"
#include "Plugin.hpp"

namespace TypeBrowser
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

protected:
private:
   void FocusedCopy() override;

   DockWidget* mDockWidget;
};
} // namespace TypeBrowser
#endif
