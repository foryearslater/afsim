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

#include "LogSubscriber.hpp"

WkLog::MessageProcessor::MessageProcessor(wkf::log::PluginBase& aPlugin) noexcept
   : mPlugin(aPlugin)
{
}

void WkLog::MessageProcessor::Process(const ut::log::Message& aMessage)
{
   mPlugin.QueueMessage(aMessage);
}

WkLog::ConsoleProcessor::ConsoleProcessor(wkf::log::PluginBase& aPlugin) noexcept
   : mPlugin(aPlugin)
{
}

void WkLog::ConsoleProcessor::Process(const ut::log::Message& aMessage)
{
   mPlugin.QueueForConsole(QString::fromStdString(aMessage.mData));
}
