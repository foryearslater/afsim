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

#ifndef UDP_OBSERVER_HPP
#define UDP_OBSERVER_HPP

#include <string>

// Base Class
#include "WsfScenarioExtension.hpp"

// Utilities
#include "UtCallbackHolder.hpp"

// Forward declarations listed to satisfy prototypes suggested for exercise
class  GenUDP_Connection;
class  UtInput;
struct UtPluginObjectParameters;
class  WsfPlatform;
#include "WsfScenarioExtension.hpp"
#include "WsfSimulationExtension.hpp"
class  WsfSensor;
class  WsfTrack;

//! Registers for simulation events using WsfObserver, and outputs
//! related information over a UDP socket.
class UDP_Observer : public WsfSimulationExtension
{
   public:

      //! Constructor
      UDP_Observer();
      UDP_Observer(const UDP_Observer& aSrc);
      UDP_Observer& operator=(const UDP_Observer& aSrc);

      //! Virtual destructor
      ~UDP_Observer() noexcept override;

      bool Initialize() override;

      bool ProcessInput(UtInput& aInput);

   private:

      void PlatformAdded(double       aSimTime,
                         WsfPlatform* aPlatformPtr);

      //!  PlatformDeleted prototype is provided as a sample for the exercise
      void PlatformDeleted(double       aSimTime,
                           WsfPlatform* aPlatformPtr);

      //!  SensorTrackUpdated prototype is provided as a sample for the exercise
      void SensorTrackUpdated(double          aSimTime,
                              WsfSensor*      aSensorPtr,
                              const WsfTrack* aTrackPtr);

      void Disconnect();
      void SendPacket(const std::string& aMessage);

      //! The port to send data on, default is 14421
      int                  mPort;

      //! The address to send to.  No default address.
      //  Check mAddress at Initialize(); Disconnect if not loaded by ProcessInput()
      std::string          mAddress;

      //! The callback holder to maintain list of subscriptions made by this class
      UtCallbackHolder     mCallbacks;

      //! Provides and configures the UDP socket
      GenUDP_Connection*   mConnectionPtr;
};

#endif
