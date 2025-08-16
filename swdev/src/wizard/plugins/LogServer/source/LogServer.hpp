// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef LOGSERVERPLUGIN_HPP
#define LOGSERVERPLUGIN_HPP

#include <QThread>

#include "LogServerPrefWidget.hpp"
#include "LogServerThread.hpp"
#include "Plugin.hpp"
#include "UtLogMessage.hpp"
#include "logging/WkfLogPluginBase.hpp"

namespace wizard
{
class WsfExecution;
}

namespace LogServer
{
class Plugin : public wizard::PluginT<wkf::log::PluginBase>
{
   Q_OBJECT
public:
   Plugin(const QString& aPluginName, const size_t aId);
   ~Plugin();

   void GuiUpdate() override;

public slots:
   //! Receives log messages across the thread boundry
   //! and pushes it to the interactive logging widget
   //! for user use.
   void                    HandleLogMessageReceived(const ut::log::Message& aMessage);
   QList<wkf::PrefWidget*> GetPreferencesWidgets() const override;

private:
   //! Handle when server changes port
   void PreferencesChanged();
   //! Let Thread gracefully end.
   void LetThreadTeardown();
   //! Subscribes to wizard sim execution so we can clear out interactive output between runs.
   void StartingExecution(wizard::WsfExecution* aExecutionPtr);
   // TCP server thread to process log messages
   std::unique_ptr<LogServerThread> mLogServerThread;
   PluginUiPointer<PrefWidget>      mPrefWidgetPtr;
   int                              mLogsServerPort = 18888;
};
} // namespace LogServer

Q_DECLARE_METATYPE(ut::log::Message)
#endif
