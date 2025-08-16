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

#ifndef OSMCONVERTERPLUGIN_HPP
#define OSMCONVERTERPLUGIN_HPP

#include "OSMConverterDialog.hpp"
#include "Plugin.hpp"

namespace OSMConverter
{
class Plugin : public wizard::Plugin
{
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

private:
   OSMConverterDialog* mDialog;
};
} // namespace OSMConverter

#endif
