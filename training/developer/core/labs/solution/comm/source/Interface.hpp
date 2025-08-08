// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2013 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
// ****************************************************************************
// Updated by Infoscitex, a DCS Company.
// ****************************************************************************

#ifndef COMM_LAB_INTERFACE_HPP
#define COMM_LAB_INTERFACE_HPP

// Utilities
#include "UtCallback.hpp"
#include "UtCallbackHolder.hpp"
class     WsfDisInterface;
#include "WsfSimulationExtension.hpp"
class     WsfSimulation;

// Forward declarations
namespace DataLink
{
   class LocationMessage;
}

class UtInput;
class WsfDisSignal;
class WsfMessage;
class WsfPlatform;

namespace CommLab
{
   //! CommLab::Interface is primarily an interface to DIS using
   //! the DisSignal PDU.
   class Interface : public WsfSimulationExtension
   {
      public:

         //! Constructor
         Interface();

         //! Virtual destructor
         ~Interface() noexcept override = default;

         bool Initialize() override;
         bool         ProcessInput(UtInput& aInput);
         void Start() override;

         void SendMessage(double       aSimTime,
                          WsfPlatform* aSenderPlatformPtr,
                          int          aSourceTrackNumberOffset,
                          const WsfMessage*  aMessagePtr);

         //! Determine if debugging is enabled.
         bool DebugEnabled() const { return mDebugEnabled; }

         using LocationMessageReceivedCallback = UtCallbackListN<void(double, DataLink::LocationMessage*)>;

         static LocationMessageReceivedCallback LocationMessageReceived;

         bool  GetDebugEnabled() { return mDebugEnabled; }
         bool  GetPrintMessages() { return mPrintMessages; }
         void  SetDebugEnabled(bool aDebugEnabled) { mDebugEnabled = aDebugEnabled; }
         void  SetPrintMessages(bool aPrintMessages) { mPrintMessages = aPrintMessages; }

      private:

         void HandleSignalPDU(WsfDisInterface* aInterfacePtr, const WsfDisSignal& aPdu);

         UtCallbackHolder             mCallbacks;
         WsfDisInterface*             mDisPtr;
         bool                         mDebugEnabled;
         bool                         mPrintMessages;

   };
}

#endif
