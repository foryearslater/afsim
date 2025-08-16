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
#include "WkAppEnvironment.hpp"

#include <cassert>
#include <thread>

#include "UtMemory.hpp"
#include "WkNetwork.hpp"
#include "WkfQueueableMessageObject.hpp"

warlock::AppEnvironment* warlock::AppEnvironment::mInstancePtr = nullptr;

namespace
{
std::thread::id mainAppEnvironmentThreadId = std::this_thread::get_id();

void AssertCurrentAppThread(bool aIsMainThread)
{
   bool isMainThread = (mainAppEnvironmentThreadId == std::this_thread::get_id());
   if (isMainThread != aIsMainThread)
   {
      wkf::QueueableMessageObject::DisplayQueuedMessage(
         QMessageBox::Critical,
         "Thread Error",
         "Unsafe thread access of warlock::AppEnvironment.\nThis is most likely caused by a plugin accessing data in "
         "an unsafe way.\nConsider unloading the responsible plugin.");
   }
}
} // namespace

void warlock::AppEnvironment::Create()
{
   if (mInstancePtr == nullptr)
   {
      mInstancePtr = new AppEnvironment;
   }
}

warlock::AppEnvironment& warlock::AppEnvironment::GetInstance()
{
   assert(mInstancePtr != nullptr);
   return *mInstancePtr;
}

bool warlock::AppEnvironment::Exists()
{
   return (mInstancePtr != nullptr);
}

void warlock::AppEnvironment::Shutdown()
{
   if (mInstancePtr != nullptr)
   {
      delete mInstancePtr;
      mInstancePtr = nullptr;
   }
}

warlock::net::Network& warlock::AppEnvironment::GetNetwork() const
{
   // Error if this is called from the SIM thread.
   ::AssertCurrentAppThread(true);

   return *mNetworkPtr;
}

warlock::AppEnvironment::AppEnvironment()
   : QObject()
   , mNetworkPtr(ut::make_unique<warlock::net::Network>())
{
}

warlock::AppEnvironment::~AppEnvironment() = default;
