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

#include "LogPlugin.hpp"

#include <QDial>
#include <QDir>
#include <QProcess>

#include "UtQtLabeledInput.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(WkLog::Plugin,
                          "Log",
                          "Provides a flexible display for viewing log messages from the simulation.",
                          "warlock")

WkLog::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wkf::log::PluginBase(aPluginName, aUniqueId)
   , mSubscriber(ut::log::Message::PresetDeveloper(), *this)
   , mConsoleSubscriber("raw", *this)
{
   CreateConsoleWidget();
}

void WkLog::Plugin::GuiUpdate()
{
   mSubscriber.ProcessMessages();
   mConsoleSubscriber.ProcessMessages();
   ApplyChanges();
}
