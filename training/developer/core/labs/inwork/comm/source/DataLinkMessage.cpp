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

#include "DataLinkMessage.hpp"

#include "DataLinkLocationMessage.hpp"
#include "GenI.hpp"
#include "GenO.hpp"

#include "UtCalendar.hpp"
#include "UtLog.hpp"
#include "WsfDateTime.hpp"

namespace DataLink
{

Message::Message(const UtCalendar& aCurrentTime,
                 unsigned short    aSourceTrackNumber)
   : mSourceTrackNumber(aSourceTrackNumber),
     mSize(GetSize()),
     mType(cUNDEFINED),
     mHour(0),
     mMin(0),
     mSec(0)
{
   mHour = static_cast<unsigned char>(aCurrentTime.GetHour());
   mMin  = static_cast<unsigned char>(aCurrentTime.GetMinute());
   mSec  = static_cast<unsigned char>(aCurrentTime.GetSecond());
}

Message::Message(GenI& aGenI)
{
   Message::Get(aGenI);
}

void Message::Get(GenI& aGenI)
{
   aGenI >> mSourceTrackNumber;
   aGenI >> mSize;
   aGenI >> mType;
   aGenI >> mHour;
   aGenI >> mMin;
   aGenI >> mSec;
}

void Message::Put(GenO& aGenO) const
{
   aGenO << mSourceTrackNumber;
   aGenO << mSize;
   aGenO << mType;
   aGenO << mHour;
   aGenO << mMin;
   aGenO << mSec;
}

//static
Message* Message::Create(GenI& aGenI)
{
   DataLink::Message Message(aGenI);

   if (aGenI.GetInputStatus() != GenBuf::NoError)
   {
      // No need to go any further
      // Buffer clean up will be done by GenIFactory
      return nullptr;
   }

   DataLink::Message* MessagePtr = nullptr;

   switch(Message.mType)
   {
      case DataLink::Message::cLOCATION :
         MessagePtr = new DataLink::LocationMessage(Message, aGenI);
         break;
      default :
      {
         auto out = ut::log::error() << "Invalid message type.";
         out.AddNote() << "Type: " << Message.GetType();
      }
   }

   return MessagePtr;
}

unsigned Message::GetSize() const
{
   return 8;
}

const std::string& Message::GetName() const
{
   static const std::string sName = "Message";
   return sName;
}

void Message::TestData() const
{
   if (mType >= cNUM_MESSAGE_TYPES)
   {
      auto out = ut::log::error() << "Invalid message type.";
      out.AddNote() << "Type: " << mType;
   }
   if (mSize != GetSize())
   {
      auto out = ut::log::error() << "Invalid message size.";
      out.AddNote() << "Type: " << mType;
      out.AddNote() << "Expected Size: " << GetSize();
      out.AddNote() << "Size: " << mSize;
   }
}

} // namespace DataLink
