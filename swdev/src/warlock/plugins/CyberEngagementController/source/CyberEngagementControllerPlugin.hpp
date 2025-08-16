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

#ifndef CYBERENGAGEMENTCONTROLLERPLUGIN_HPP
#define CYBERENGAGEMENTCONTROLLERPLUGIN_HPP

#include <QMenu>

#include "CyberEngagementControllerDataContainer.hpp"
#include "CyberEngagementControllerSimInterface.hpp"
#include "WkPlugin.hpp"

namespace WkCyberEngagementController
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT
public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);

protected:
   void BuildEntityContextMenu(QMenu* aMenu, wkf::Entity* aEntityPtr) override;

   void GuiUpdate() override;

private:
   DataContainer mCyberEngagementControllerData;
};
} // namespace WkCyberEngagementController
#endif
