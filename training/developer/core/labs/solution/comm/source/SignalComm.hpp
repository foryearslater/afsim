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

#ifndef COMM_LAB_COMM_HPP
#define COMM_LAB_COMM_HPP

// Base class
#include "WsfComm.hpp"
#include "WsfCommandChain.hpp"
#include "WsfScenario.hpp"

// Utilities
#include "UtCallbackHolder.hpp"
#include "UtScriptClassDefine.hpp"

// Specific to this exercise
namespace CommLab
{
   class Interface;
}

namespace DataLink
{
   class LocationMessage;
}

// Forward declarations
struct UtPluginObjectParameters;


namespace wsf
{
namespace comm
{
//! A comm device that triggers the sending of DIS signal PDUs.
class SignalComm : public wsf::comm::Comm
{
   public:

      //! Constructor
      explicit SignalComm(WsfScenario& aScenario);

      SignalComm& operator=(const SignalComm&) = delete;

      //! Virtual destructor
      ~SignalComm() noexcept override = default;

      //! Get the class ID associated with the object (poor man's RTTI).
      //! @return Returns the associated class ID.
      static WsfStringId GetSignalCommClassId();

      //! @name Framework methods
      //@{
      wsf::comm::Comm* Clone() const override;
      bool             ProcessInput(UtInput& aInput) override;
      bool             Initialize(double aSimTime) override;
      //@}

      unsigned int GetSourceTrackNumber() const { return mSourceTrackNumber; }

      CommLab::Interface* GetInterface() const { return mInterfacePtr; }

      bool Receive(double aSimTime, wsf::comm::Comm* aCommPtr, wsf::comm::Message& aMessage) override;

      bool Send(double aSimTime, std::unique_ptr<WsfMessage> aMessagePtr, const Address& aAddress) override;

      void LocationMessageReceived(double                        aSimTime,
                                    DataLink::LocationMessage*   aLocMsgPtr);

      const char* GetScriptClassName() const override;

   protected:

      //! Copy Constructor; used by clone
      SignalComm(const SignalComm& aSrc) = default;

   private:

      unsigned int        mSourceTrackNumber;  //! Like a link-16 source identifier
      CommLab::Interface* mInterfacePtr;
      UtCallbackHolder    mCallbacks;

};

}
}


#endif
