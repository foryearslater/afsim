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

#ifndef FLIGHTCONTROLLERPLATFORMLISTREQUEST_HPP
#define FLIGHTCONTROLLERPLATFORMLISTREQUEST_HPP

#include <vector>

#include "xio/WsfXIO_PacketRegistry.hpp"
#include "xio/WsfXIO_PlatformListRequest.hpp"
#include "xio/WsfXIO_Connection.hpp"
#include "xio/WsfXIO_Interface.hpp"

//! Requests for an application to send its platform list information.
class FlightControllerPlatformListRequest : public WsfXIO_PlatformListRequest
{
   public:

      explicit FlightControllerPlatformListRequest(WsfXIO_Connection* aConnectionPtr);
      ~FlightControllerPlatformListRequest() noexcept override = default;

      void HandlePlatformList(WsfXIO_PlatformListUpdatePkt& aPkt) override;

      //! Keep information about platform data here
      using Platforms = std::vector<WsfXIO_PlatformListUpdatePkt::PlatformData>;

      //! Provide an interface to get the platform data.
      static const Platforms& GetPlatforms() { return sPlatforms; }

      //! Reset platform list (after a disconnect)
      static void ResetPlatforms() { sPlatforms.clear(); }
   
   private:

      static Platforms sPlatforms;
};

#endif
