// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WsfConnector.hpp"

#include <cstdlib>

#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

#include "GenSocket.hpp"
#include "GenUDP_Connection.hpp"
#include "PakUDP_IO.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "UtDictionary.hpp"
#include "UtQtMemory.hpp"
#include "UtScriptTypes.hpp"
#include "UtSleep.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "xio/WsfXIO_Interface.hpp"
#include "xio/WsfXIO_SimTimeRequest.hpp"

namespace
{
class TimeRequest : public WsfXIO_SimTimeRequest
{
public:
   explicit TimeRequest(WsfXIO_Connection* aConnectionPtr)
      : WsfXIO_SimTimeRequest(aConnectionPtr, WsfXIO_RequestSimTimePkt::cREAL_TIME_INTERVAL, 1.0 / 20, true)
      , mConnectorPtr(nullptr)
   {
   }
   void HandleResponse(WsfXIO_ResponsePkt& aPkt) override
   {
      if (WsfXIO_SimTimePkt::cPACKET_ID == aPkt.ID())
      {
         WsfXIO_SimTimePkt* pkt = (WsfXIO_SimTimePkt*)&aPkt;
         mConnectorPtr->UpdateTimeInfo(pkt->mSimTime, pkt->mEndTime, pkt->mCurrentRunNumber, pkt->mFinalRunNumber);
      }
   }
   wizard::WsfConnector* mConnectorPtr;
};
} // namespace

wizard::WsfConnector::WsfConnector()
   : mSimTime(0.0)
   , mEndTime(0.0)
   , mInterfacePtr(ut::make_unique<WsfXIO_Interface>())
   , mConnectionPtr(nullptr)
   , mDebuggerPtr(nullptr)
   , mDebugEnabled(false)
   , mXIO_RecvPort(0)
{
   WsfXIO_Interface::UDP_Target udp;
   udp.mAddress  = "127.0.0.1";
   udp.mRecvPort = -1;
   udp.mType     = WsfXIO_Interface::cUNICAST;
   mInterfacePtr->AddUDP_Target(udp);

   mInterfacePtr->SetApplicationName("Wizard");
   mInterfacePtr->mApplicationTypeFilter.insert(wsf::xio::cAPP_SIMULATION);
   mInterfacePtr->Initialize();
   mCallbacks += mInterfacePtr->OnConnected.Connect(&WsfConnector::HandleConnection, this);
   mCallbacks += mInterfacePtr->OnDisconnected.Connect(&WsfConnector::HandleDisconnect, this);

   const WsfXIO_Interface::ConnectionList& connections = mInterfacePtr->GetConnections();
   if (!connections.empty() && connections[0]->GetUDP_IO())
   {
      GenSockets::GenSocket* socketPtr = connections[0]->GetUDP_IO()->GetConnection().GetRecvSocket();
      if (socketPtr)
      {
         mXIO_RecvPort = socketPtr->GetBoundPort();
      }
   }
   mCallbacks += wizSignals->DebugSessionActive.Connect(&WsfConnector::DebugSessionActive, this);
}

void wizard::WsfConnector::Update()
{
   mInterfacePtr->AdvanceTime(0);
}

void wizard::WsfConnector::HandleDisconnect(WsfXIO_Connection* c)
{
   if (mConnectionPtr == c)
   {
      emit DisconnectedFromWsf();
      mConnectionPtr = nullptr;
   }
}

void wizard::WsfConnector::HandleConnection(WsfXIO_Connection* aConnectionPtr)
{
   if (aConnectionPtr->IsReliable())
   {
      mConnectionPtr             = aConnectionPtr;
      TimeRequest* timeRequest   = new TimeRequest(aConnectionPtr);
      timeRequest->mConnectorPtr = this;
      mInterfacePtr->GetRequestManager().AddRequest(timeRequest);
      emit ConnectedToWsf();
      // wait a little bit before we tell WSF executable to start
      // otherwise we won't have a chance to stop in early scripts
      // MAGIC NUMBER: 1/2 second  --  This lets us keep this code decoupled from debugging logic
      QTimer::singleShot(500, this, &WsfConnector::DeferredResume);
   }
}
void wizard::WsfConnector::DeferredResume()
{
   if (mConnectionPtr)
   {
      WsfXIO_SimTimeCommandPkt pkt;
      pkt.mCommandType = WsfXIO_SimTimeCommandPkt::cRESUME;
      mConnectionPtr->Send(pkt);
   }
}
void wizard::WsfConnector::UpdateTimeInfo(double aSimTime, double aEndTime, int32_t aCurrentRunNumber, int32_t aFinalRunNumber)
{
   mSimTime = aSimTime;
   mEndTime = aEndTime;
   if (aCurrentRunNumber >= 0 && aFinalRunNumber >= 0)
   {
      mCurrentRunNumber = aCurrentRunNumber;
      mFinalRunNumber   = aFinalRunNumber;
   }

   emit TimeUpdated();
}

void wizard::WsfConnector::ResumeConstructive()
{
   if (mConnectionPtr)
   {
      if (mDebuggerPtr)
      {
         mDebuggerPtr->Resume();
      }

      WsfXIO_SimTimeCommandPkt pkt;
      pkt.mCommandType = WsfXIO_SimTimeCommandPkt::cSET_NON_REALTIME;
      mConnectionPtr->Send(pkt);
   }
}

void wizard::WsfConnector::ResumeRealtime()
{
   if (mConnectionPtr)
   {
      // If debugger is stopped at a breakpoint, the sim will not quit.
      if (mDebuggerPtr)
      {
         mDebuggerPtr->Resume();
      }

      // First send a packet that sets to realtime, then one that resumes the sim.
      WsfXIO_SimTimeCommandPkt pkt;
      pkt.mCommandType = WsfXIO_SimTimeCommandPkt::cSET_REALTIME;
      mConnectionPtr->Send(pkt);

      pkt.mCommandType = WsfXIO_SimTimeCommandPkt::cRESUME;
      mConnectionPtr->Send(pkt);
   }
}

void wizard::WsfConnector::Pause()
{
   if (mConnectionPtr)
   {
      WsfXIO_SimTimeCommandPkt pkt;
      pkt.mCommandType = WsfXIO_SimTimeCommandPkt::cPAUSE;
      mConnectionPtr->Send(pkt);
   }
}

void wizard::WsfConnector::Stop()
{
   if (mConnectionPtr)
   {
      if (mFinalRunNumber - mCurrentRunNumber > 0)
      {
         Pause();
         auto    msgBox      = ut::qt::make_qt_ptr<QMessageBox>(wkfEnv.GetMainWindow());
         QString messageText = "Do you want to stop all runs or the current run? " + QString::number(mCurrentRunNumber) +
                               "/" + QString::number(mFinalRunNumber) + " Runs Completed";
         msgBox->setIcon(QMessageBox::Question);
         msgBox->setWindowTitle("Stop Multiple Runs");
         msgBox->setText(messageText);
         auto* yesBtn = msgBox->addButton("Stop All Runs", QMessageBox::YesRole);
         msgBox->addButton("Stop Current Run", QMessageBox::NoRole);
         msgBox->exec();
         if (msgBox->clickedButton() == yesBtn)
         {
            ProjectWorkspace::Instance()->StopAllExecutions();
            return;
         }
      }
      if (mDebuggerPtr)
      {
         // If debugger is stopped at a breakpoint, the sim will not quit.
         mDebuggerPtr->ForceDetatch();
      }
      WsfXIO_SimTimeCommandPkt pkt;
      pkt.mCommandType = WsfXIO_SimTimeCommandPkt::cSET_END_TIME;
      pkt.mSimTime     = .1;
      mConnectionPtr->Send(pkt);
      ResumeConstructive();
   }
}

void wizard::WsfConnector::DebugSessionActive(ScriptDebugger* aDebugger, bool aIsActive)
{
   if (aIsActive)
   {
      mDebuggerPtr = aDebugger;
   }
   else
   {
      mDebuggerPtr = nullptr;
   }
}
