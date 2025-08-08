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

#ifndef FLIGHTCONTROLPKT_HPP
#define FLIGHTCONTROLPKT_HPP

#include "xio/WsfXIO_Packet.hpp"
#include "xio/WsfXIO_PacketRegistry.hpp"

// Keep as last include
#include "PakSerializeImpl.hpp"

//! Packet sent from flight controller to the controlling simulation.
class FlightControlPkt : public WsfXIO_Packet
{
   public:

      ~FlightControlPkt() noexcept override = default;

      //! Use the predefined macros to define constructor, serialization, and
      //! de-serialization methods.
      XIO_DEFINE_PACKET(FlightControlPkt, WsfXIO_Packet, 350)
      {
         aBuff & mPlatformIndex & mPitchRate & mRollRate & mYawRate & mThrottle;
      }

      unsigned mPlatformIndex = 0;
      double   mPitchRate     = 0; // -1 to 1
      double   mRollRate      = 0; // -1 to 1
      double   mYawRate       = 0; // -1 to 1
      double   mThrottle      = 0; //  0 to 2
};

#endif
