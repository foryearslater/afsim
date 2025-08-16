// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WKPLUGININTERACTIONS_HPP
#define WKPLUGININTERACTIONS_HPP

#include "InteractionsSimInterface.hpp"
#include "WkPlugin.hpp"
#include "interaction/WkfInteractionPluginBase.hpp"

namespace WkInteractions
{
class Plugin : public warlock::PluginT<SimInterface, wkf::InteractionPluginBase>
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

protected:
   void GuiUpdate() override;

   vespa::VaViewer* mViewerPtr;
};
} // namespace WkInteractions
#endif
