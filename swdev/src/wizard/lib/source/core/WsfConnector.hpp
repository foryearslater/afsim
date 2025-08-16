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
#ifndef WSFCONNECTOR_HPP
#define WSFCONNECTOR_HPP

#include <QObject>

#include "ScriptDebugger.hpp"
#include "UtCallbackHolder.hpp"
#include "UtMemory.hpp"
#include "ViExport.hpp"
#include "xio/WsfXIO_Interface.hpp"

class WsfXIO_Connection;

namespace wizard
{
class VI_EXPORT WsfConnector : public QObject
{
   Q_OBJECT

public:
   WsfConnector();
   WsfConnector(const WsfConnector&) = delete;
   WsfConnector(WsfConnector&&)      = default;
   WsfConnector& operator=(const WsfConnector&) = delete;
   WsfConnector& operator=(WsfConnector&&) = default;
   ~WsfConnector() override                = default;
   bool Initialize();

   bool IsConnected();
   void Update();

   //! Updates the information about simulation time
   //! @param aSimTime is the current simulation time
   //! @param aEndTime is the simulation end time
   //! @param aCurrentRunNumber is the current run number
   //! @param aFinalRunNumber is the final run number
   void UpdateTimeInfo(double aSimTime, double aEndTime, int32_t aCurrentRunNumber = -1, int32_t aFinalRunNumber = -1);

   WsfXIO_Interface*  Xio() { return mInterfacePtr.get(); }
   WsfXIO_Connection* Connection() { return mConnectionPtr; }
   ScriptDebugger*    Debugger() { return mDebuggerPtr; }
   int                XioRecvPort() { return mXIO_RecvPort; }

   double  mSimTime;
   double  mEndTime;
   int32_t mCurrentRunNumber;
   int32_t mFinalRunNumber;

   void Pause();
   void ResumeRealtime();
   void ResumeConstructive();
   void Stop();
   void DebugSessionActive(ScriptDebugger* aDebugger, bool aIsActive);

signals:
   void ConnectedToWsf();
   void DisconnectedFromWsf();
   void TimeUpdated();

private:
   void DeferredResume();
   void HandleDisconnect(WsfXIO_Connection* aC);
   void HandleConnection(WsfXIO_Connection* aConnectionPtr);

   UtCallbackHolder                  mCallbacks;
   std::unique_ptr<WsfXIO_Interface> mInterfacePtr;
   WsfXIO_Connection*                mConnectionPtr;
   ScriptDebugger*                   mDebuggerPtr;
   bool                              mDebugEnabled;
   int                               mXIO_RecvPort;
};
} // namespace wizard
#endif
