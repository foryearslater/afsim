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

#ifndef PLATFORMCONTROLSERVICE_HPP
#define PLATFORMCONTROLSERVICE_HPP

class PakPacket;

// Base Class
#include "WsfSimulationExtension.hpp"

// Utilities
#include "UtCallbackHolder.hpp"

// AFSIM
class WsfPlatform;
class WsfXIO_Interface;

//! A rudimentary service that accepts Flight Control packets
//! and uses them to control entities equipped with 6-dof movers.
class PlatformControlService : public WsfSimulationExtension
{
   public:

      //! Constructor
      PlatformControlService();

      //! Virtual destructor
      ~PlatformControlService() noexcept override = default;

      //! Callback Function
      void ControlPlatform(PakPacket& aPacket);

      void Start() override;

   private:

      void SelectPlatform(const WsfPlatform* aPlatformPtr);

      void DeselectPlatform(const WsfPlatform* aPlatformPtr);

      //! The callback holder to maintain list of subscriptions made by this class
      UtCallbackHolder     mCallbacks;

      //! Maintain a pointer to the xio interface.
      //! This is guaranteed to be valid throughout the simulation.
      WsfXIO_Interface*    mXIO_Ptr;

      //! The platform we are currently controlling
      unsigned int         mControlledPlatformIndex;
};

#endif
