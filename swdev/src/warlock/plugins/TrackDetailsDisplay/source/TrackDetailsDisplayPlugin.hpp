// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TRACKDETAILSDISPLAYPLUGIN_HPP
#define TRACKDETAILSDISPLAYPLUGIN_HPP

#include <QPointer>

#include "TrackDetailsDisplayDialog.hpp"
#include "TrackDetailsSimInterface.hpp"
#include "WkPlugin.hpp"
#include "WkfObserver.hpp"

namespace WkTrackDetailsDisplay
{

class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() = default;

private:
   void                    GuiUpdate() override;
   DataContainer           mDataContainer;
   PluginUiPointer<Dialog> mDialog;
};
} // namespace WkTrackDetailsDisplay

#endif
