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

#ifndef LOGSERVERTHREAD_HPP
#define LOGSERVERTHREAD_HPP

#include <QThread>

#include "PakProcessor.hpp"
#include "PakTCP_Connector.hpp"
#include "PakTCP_IO.hpp"
#include "UtLogMessage.hpp"


//! The LogServerThread is responsible for running the wizard log server.
//! The Wizard log server is responsible for capturing log messages
//! coming from mission instances.
class LogServerThread : public QThread
{
   Q_OBJECT
public:
   explicit LogServerThread(QObject* aParent);
   ~LogServerThread() override = default;
   //! For handling changes to server configuration
   void SetConnectionParameters(int aPort);
signals:
   //! Emitted when a log message is sent from server client,
   //! this is used to communicate messages across the thread
   //! boundry.
   void LogMessageReceived(const ut::log::Message& aMessage);

private:
   // Runs when QT kicks off thread. Starts log server.
   void run() override;
   // Stores connection for log message processing
   void AddConnection(std::unique_ptr<PakTCP_IO> aConnection);
   // Queries the tcp server for incoming connections requests.
   void HandleIncomingConnections();
   // Starts the server by binding to configured port
   void StartServer();
   // Queries current connections for new log messagess
   void HandleIncomingMessages();
   // The TCP Server abstraction
   std::unique_ptr<PakTCP_Connector> mServer;
   // The TCP Connections abstraction
   std::vector<std::unique_ptr<PakTCP_IO>> mConnections;
   // Serializes incoming log messages
   std::unique_ptr<PakProcessor> mProcessor;
   // TCP Server port
   int mPort = 18888;
   // No need for TCP Host as it will bind to 0.0.0.0:<port> covering all nics.
};
#endif
