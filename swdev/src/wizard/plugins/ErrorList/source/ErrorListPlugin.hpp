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
#ifndef ERRORLISTPLUGIN_HPP
#define ERRORLISTPLUGIN_HPP

#include "ErrorDock.hpp"
#include "Plugin.hpp"

namespace ErrorList
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;
   // QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

   // QList<wkf::Action*> GetActions() const override;

protected:
private:
   ErrorDock* mErrorDock;
};
} // namespace ErrorList
#endif
