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


#ifndef THREADNOTIFYEVENT_HPP
#define THREADNOTIFYEVENT_HPP

#include <memory>

#include <QEvent>

#include "Project.hpp"
#include "WsfPProxySerialize.hpp"

namespace wizard
{
//! A notification event indicating that the thread has completed some work
class ThreadNotifyEvent : public QEvent
{
public:
   enum NotificationType
   {
      cFAILED,
      cPARSE_RESULTS_READY,
      cPROXY_READY,
      cPERFORMANCE_TEST_RESULTS_READY,
      cSERIALIZE_COMPLETED,
      cWORK_COMPLETE
   };
   static const int cTHREAD_NOTIFY_EVENT_ID = QEvent::User + 1;
   ThreadNotifyEvent(Project* aProjectPtr, NotificationType aNotificationType)
      : QEvent((QEvent::Type)cTHREAD_NOTIFY_EVENT_ID)
      , mNotificationType(aNotificationType)
      , mProjectPtr(aProjectPtr)
      , mTaskId(-1)
      , mModificationSequenceNumber(-1)
      , mTimeToCompleteParseMs(0)
      , mTimeToCompleteProxyMs(0)
      , mProxy(nullptr)
      , mProxyHash(nullptr)
      , mSerializeToComplete(nullptr)
   {
   }

   ThreadNotifyEvent(const ThreadNotifyEvent&) = delete;
   ThreadNotifyEvent(ThreadNotifyEvent&&)      = default;
   ThreadNotifyEvent& operator=(const ThreadNotifyEvent&) = delete;
   ThreadNotifyEvent& operator=(ThreadNotifyEvent&&) = default;

   ~ThreadNotifyEvent() override = default;

   NotificationType           mNotificationType;
   Project*                   mProjectPtr;
   int                        mTaskId;
   int                        mModificationSequenceNumber;
   int                        mTimeToCompleteParseMs;
   int                        mTimeToCompleteProxyMs;
   std::unique_ptr<WsfPProxy> mProxy;
   std::unique_ptr<ProxyHash> mProxyHash;
   // UtSharedPtr<WsfPProxyRegistry> mProxyRegistry;
   std::unique_ptr<WsfPProxySerialize> mSerializeToComplete;
};

} // namespace wizard

#endif
