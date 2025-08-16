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

#ifndef CREATEPLATFORMPLUGIN_HPP
#define CREATEPLATFORMPLUGIN_HPP

#include "CreatePlatformDockWidget.hpp"
#include "CreatePlatformSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkCreatePlatform
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

protected:
   void GuiUpdate() override;

private:
   QPointer<DockWidget> mDockWidget;
   DataContainer        mCreatePlatformData;
};
} // namespace WkCreatePlatform
#endif
