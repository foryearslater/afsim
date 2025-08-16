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
#ifndef CRDIMPORTERPLUGIN_HPP
#define CRDIMPORTERPLUGIN_HPP

#include "CrdImporterDialog.hpp"
#include "Plugin.hpp"

namespace CrdImporter
{
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override = default;

private:
   Dialog* mDialogPtr;
};
} // namespace CrdImporter
#endif
