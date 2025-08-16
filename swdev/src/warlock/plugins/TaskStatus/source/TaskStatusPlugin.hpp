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

#ifndef TASKSTATUSPLUGIN_HPP
#define TASKSTATUSPLUGIN_HPP

#include <QObject>

#include "TaskStatusDataContainer.hpp"
#include "TaskStatusDockWidget.hpp"
#include "TaskStatusSimInterface.hpp"
#include "WkPlugin.hpp"

namespace wkf
{
class Action;
}

namespace WkTaskStatus
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
} // namespace WkTaskStatus

#endif // TASKSTATUSPLUGIN_HPP
