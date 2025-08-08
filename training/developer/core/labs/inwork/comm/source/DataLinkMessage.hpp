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

// Message - Base class for Datalink messages

#ifndef DATALINK_MESSAGE_HPP
#define DATALINK_MESSAGE_HPP

#include <iosfwd>
#include <memory>
#include <string>

class GenI;
class GenO;
class UtCalendar;

namespace DataLink
{
   //! This is an example base class representing a header for any number of
   //! different possible messages.  A factory method, Create(..) is provided
   //! to automatically create derived message types from a GenIO (generic) input.
   class Message
   {
      public:

         // This method reads enough of the input stream to
         // determine the type of Message being read.  It then creates the proper Message
         // type from a factory class.  Once the Message is generated,
         // it is populated with the data from the input stream.
         //
         // The caller owns the returned Message and is responsible for its destruction.
         static Message* Create(GenI& aGenI);

         Message() = delete;

         Message(const UtCalendar& aCurrentTime,
                 unsigned short    aSourceTrackNumber);

         explicit Message(GenI& aGenI);

         virtual ~Message() noexcept = default;

         enum Type
         {
            cUNDEFINED                    = 0,
            cLOCATION                     = 1,
            cNUM_MESSAGE_TYPES            = 2
         };

         // Input/output

         virtual void Get(GenI& aGenI);
         virtual void Put(GenO& aGenO) const;

         virtual unsigned GetSize() const;
         virtual int      GetType() const { return cUNDEFINED; }

         virtual const std::string& GetName() const;

         virtual void TestData() const; // Test Message for bad or questionable data

         // The following are simply the data and so are public

         // Message header
         unsigned short mSourceTrackNumber; // The originator of the message
         unsigned short mSize;              // total number of bytes in the Message
         unsigned char  mType;              // Message type enumeration
         unsigned char  mHour;              // 0-24 UT
         unsigned char  mMin;               // 0-60
         unsigned char  mSec;               // 0-60
   };
};

#endif
