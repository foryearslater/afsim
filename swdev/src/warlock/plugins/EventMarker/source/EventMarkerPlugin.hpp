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
#ifndef WKEVENTMARKERPLUGIN_HPP
#define WKEVENTMARKERPLUGIN_HPP

#include "EventMarkerSimInterface.hpp"
#include "WkPlugin.hpp"
#include "event_marker/WkfEventMarkerPluginBase.hpp"

namespace WkEventMarker
{
class Plugin : public warlock::PluginT<SimInterface, wkf::EventMarkerPluginBase>
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

protected:
   void GuiUpdate() override;

   vespa::VaViewer* mViewerPtr;

private:
   bool mEnabled;
};
} // namespace WkEventMarker

#endif
