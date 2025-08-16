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
#ifndef WK_LOG_SUBSCRIBER_HPP
#define WK_LOG_SUBSCRIBER_HPP

#include "UtLogSubscriberT.hpp"
#include "logging/WkfLogPluginBase.hpp"

namespace WkLog
{
class MessageProcessor final : public ut::log::MessageProcessor
{
public:
   MessageProcessor(wkf::log::PluginBase& aPlugin) noexcept;

   void Process(const ut::log::Message& aMessage) override;

private:
   wkf::log::PluginBase& mPlugin;
};

using Subscriber = ut::log::SubscriberT<MessageProcessor>;

class ConsoleProcessor final : public ut::log::MessageProcessor
{
public:
   ConsoleProcessor(wkf::log::PluginBase& aPlugin) noexcept;

   void Process(const ut::log::Message& aMessage) override;

private:
   wkf::log::PluginBase& mPlugin;
};

using ConsoleSubscriber = ut::log::SubscriberT<ConsoleProcessor>;
} // namespace WkLog

#endif
