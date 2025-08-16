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

#ifndef TASKASSIGNERPLUGIN_HPP
#define TASKASSIGNERPLUGIN_HPP

#include <QObject>

#include "TaskAssignerDataContainer.hpp"
#include "TaskAssignerDockWidget.hpp"
#include "TaskAssignerSimInterface.hpp"
#include "WkPlugin.hpp"

namespace wkf
{
class Action;
}

namespace WkTaskAssigner
{
class Plugin : public warlock::PluginT<SimInterface>
{
   Q_OBJECT

public:
   explicit Plugin(const QString& aPluginName, const size_t aUniqueId);

protected:
   void GuiUpdate() override;

private:
   DataContainer mData;
   DockWidget*   mDockWidgetPtr;
};
} // namespace WkTaskAssigner

#endif // TASKASSIGNERPLUGIN_HPP
