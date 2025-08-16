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

#include "LogServer.hpp"

#include "ProjectWorkspace.hpp"
#include "UtLogPublisher.hpp"
#include "UtMemory.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WsfExecution.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(
   LogServer::Plugin,
   "Log Server",
   "Provides a logging server for aggregating simulation messages and a flexible display for viewing them.",
   "wizard")

LogServer::Plugin::Plugin(const QString& aPluginName, const size_t aId)
   : wizard::PluginT<wkf::log::PluginBase>(aPluginName, aId)
{
   mPrefWidgetPtr = new PrefWidget();
   qRegisterMetaType<ut::log::Message>("ut::log::Message");
   mLogServerThread = ut::make_unique<LogServerThread>(this);
   connect(mLogServerThread.get(), &LogServerThread::LogMessageReceived, this, &Plugin::HandleLogMessageReceived);
   connect(mPrefWidgetPtr->GetPreferenceObject(), &PrefObject::PreferencesChanged, this, &Plugin::PreferencesChanged);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::StartingExecution, this, &Plugin::StartingExecution);
   mLogServerThread->start();
}

LogServer::Plugin::~Plugin()
{
   mLogServerThread->quit();
   mLogServerThread->requestInterruption();
   LetThreadTeardown();
}

void LogServer::Plugin::GuiUpdate()
{
   ApplyChanges();
}

void LogServer::Plugin::HandleLogMessageReceived(const ut::log::Message& aMessage)
{
   QueueMessage(aMessage);
}

QList<wkf::PrefWidget*> LogServer::Plugin::GetPreferencesWidgets() const
{
   QList<wkf::PrefWidget*> prefs = wkf::log::PluginBase::GetPreferencesWidgets();
   prefs << mPrefWidgetPtr;
   return prefs;
}

void LogServer::Plugin::PreferencesChanged()
{
   mLogServerThread->quit();
   mLogServerThread->requestInterruption();
   LetThreadTeardown();
   auto* prefs     = wkfEnv.GetPreferenceObject<LogServer::PrefObject>();
   mLogsServerPort = prefs->GetLogServerPort();
   mLogServerThread->SetConnectionParameters(mLogsServerPort);
   mLogServerThread->start();
}

void LogServer::Plugin::LetThreadTeardown()
{
   mLogServerThread->wait();
}

void LogServer::Plugin::StartingExecution(wizard::WsfExecution* aExecutionPtr)
{
   Clear();
}
