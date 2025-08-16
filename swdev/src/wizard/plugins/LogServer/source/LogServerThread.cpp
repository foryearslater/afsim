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

#include "LogServerThread.hpp"

#include "PakLogPacket.hpp"
#include "UtException.hpp"

LogServerThread::LogServerThread(QObject* aParent)
   : QThread(aParent)
{
   mProcessor = ut::make_unique<PakProcessor>();
   mProcessor->RegisterPacket<log_server::LogPacket>(log_server::LogPacket::cPACKET_ID,
                                                     "Log Packet",
                                                     new log_server::LogPacket{});
   mServer = ut::make_unique<PakTCP_Connector>(mProcessor.get());
}

void LogServerThread::SetConnectionParameters(int aPort)
{
   mPort = aPort;
}

void LogServerThread::run()
{
   ut::SetupThreadErrorHandling();

   StartServer();
   for (;;)
   {
      if (isInterruptionRequested())
      {
         return;
      }
      HandleIncomingConnections();
      HandleIncomingMessages();
   }
}

void LogServerThread::AddConnection(std::unique_ptr<PakTCP_IO> aConnection)
{
   mConnections.push_back(std::move(aConnection));
}

void LogServerThread::HandleIncomingConnections()
{
   PakTCP_IO* ioPtr = mServer->Accept(0);
   if (ioPtr != nullptr)
   {
      std::unique_ptr<PakTCP_IO> tcpconn{ioPtr};
      AddConnection(std::move(tcpconn));
   }
}

void LogServerThread::StartServer()
{
   mServer->Listen(mPort);
}

void LogServerThread::HandleIncomingMessages()
{
   for (auto& con : mConnections)
   {
      if (con->IsConnected())
      {
         PakPacket* message = con->ReceiveNew();
         if (message)
         {
            log_server::LogPacket* log = dynamic_cast<log_server::LogPacket*>(message);
            if (log)
            {
               emit LogMessageReceived(log->message);
            }
         }
      }
   }
}
