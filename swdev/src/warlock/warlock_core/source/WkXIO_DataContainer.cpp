// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WkXIO_DataContainer.hpp"

#include "UtMemory.hpp"
#include "WsfPlatform.hpp"
#include "dis/WsfDisInterface.hpp"
#include "dis/WsfDisPlatform.hpp"

void warlock::XIO_DataContainer::Initialize(WsfXIO_Interface* aXIO_Interface)
{
   mXIO_PlatformListPtr = ut::make_unique<WsfXIO_RemotePlatformList>(aXIO_Interface);
}

WsfXIO_PlatformInfo* warlock::XIO_DataContainer::GetPlatformInfo(const WsfPlatform* aPlatform) const
{
   WsfXIO_PlatformInfo* retVal = nullptr;
   if (mXIO_PlatformListPtr && aPlatform->IsExternallyControlled())
   {
      WsfDisInterface* disInterface = WsfDisInterface::Find(*aPlatform->GetSimulation());
      if (disInterface)
      {
         WsfDisPlatform* disPlatformPtr = disInterface->GetDisPlatform(aPlatform->GetIndex());
         if (disPlatformPtr)
         {
            retVal = mXIO_PlatformListPtr->GetPlatformByEntityId(disPlatformPtr->GetEntityId());
         }
      }
   }
   return retVal;
}

WsfXIO_PlatformInfo* warlock::XIO_DataContainer::GetPlatformInfo(const WsfXIO_Connection* aConnectionPtr,
                                                                 const unsigned int       aHostPlatformIndex) const
{
   WsfXIO_PlatformInfo* retVal = nullptr;
   if (mXIO_PlatformListPtr)
   {
      retVal = mXIO_PlatformListPtr->GetPlatformByIndex(aConnectionPtr, aHostPlatformIndex);
   }
   return retVal;
}

void warlock::XIO_DataContainer::Clear()
{
   mXIO_PlatformListPtr.reset();
}

bool warlock::XIO_DataContainer::AddConnection(WsfXIO_Connection* aConnectionPtr, const WsfClockSource* aClockPtr)
{
   return mXIO_PlatformListPtr->AddConnection(aConnectionPtr, aClockPtr);
}

void warlock::XIO_DataContainer::DropConnection(WsfXIO_Connection* aConnectionPtr)
{
   mXIO_PlatformListPtr->DropConnection(aConnectionPtr);
}
