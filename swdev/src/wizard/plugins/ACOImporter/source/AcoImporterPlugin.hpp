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

#ifndef ACOIMPORTERPLUGIN_HPP
#define ACOIMPORTERPLUGIN_HPP

#include "ImporterDialog.hpp"
#include "Plugin.hpp"


namespace AcoImporter
{

class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aId);
   ~Plugin() override = default;

private:
   PluginUiPointer<Dialog> mDialog;
};

} // namespace AcoImporter

#endif
