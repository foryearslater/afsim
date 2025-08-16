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
#ifndef Wk_LOG_PLUGIN_HPP
#define Wk_LOG_PLUGIN_HPP

#include "LogSubscriber.hpp"
#include "logging/WkfLogPluginBase.hpp"

namespace WkLog
{
class Plugin final : public wkf::log::PluginBase
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

private:
   void GuiUpdate() override;

   Subscriber        mSubscriber;
   ConsoleSubscriber mConsoleSubscriber;
};
} // namespace WkLog

#endif
