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

#include "DataLinkLocationMessage.hpp"

#include "GenI.hpp"
#include "GenO.hpp"

namespace DataLink
{

LocationMessage::LocationMessage(const UtCalendar& aCurrentTime,
                                 unsigned short    aSourceTrackNumber)
   : Message(aCurrentTime, aSourceTrackNumber),
     mLatitude(0.0),
     mLongitude(0.0),
     mAltitude(0.0),
     mCourse(0.0),
     mSpeed(0.0)
{
   mType = GetType();
   mSize = GetSize();
}

LocationMessage::LocationMessage(const Message& aMessage,
                                 GenI&         aGenI)
   : Message(aMessage)
{
   //explicitly scope virtual calls in constructor
   LocationMessage::GetMemberData(aGenI);
}

void LocationMessage::Get(GenI& aGenI)
{
   Message::Get(aGenI);
   GetMemberData(aGenI);
}

void LocationMessage::Put(GenO& aGenO) const
{
   Message::Put(aGenO);
   aGenO << mLatitude;
   aGenO << mLongitude;
   aGenO << mAltitude;
   aGenO << mCourse;
   aGenO << mSpeed;
}

//virtual
void LocationMessage::GetMemberData(GenI& aGenI)
{
   aGenI >> mLatitude;
   aGenI >> mLongitude;
   aGenI >> mAltitude;
   aGenI >> mCourse;
   aGenI >> mSpeed;
}

unsigned LocationMessage::GetSize() const
{
   return Message::GetSize() + 40;
}

const std::string& LocationMessage::GetName() const
{
   static const std::string sName = "LocationMessage";
   return sName;
}

} // namespace DataLink
