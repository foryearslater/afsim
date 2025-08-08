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

#ifndef DATALINK_LOCATIONMESSAGE_HPP
#define DATALINK_LOCATIONMESSAGE_HPP

#include "DataLinkMessage.hpp"

namespace DataLink
{
   //! This class is used to serialize and deserialize
   //! LocationMessages with GenIO.
   class LocationMessage : public Message
   {
      public:
         LocationMessage() = delete;

         LocationMessage(const UtCalendar& aCurrentTime,
                         unsigned short    aSourceTrackNumber);

         LocationMessage(const Message& aMessage,
                         GenI&          aGenI);

         ~LocationMessage() noexcept override = default;

         double   mLatitude;
         double   mLongitude;
         double   mAltitude;
         double   mCourse;
         double   mSpeed;

         unsigned GetSize() const override;
         int GetType()      const override { return Message::cLOCATION; }
         const std::string& GetName() const override;

         void Get(GenI& aGenI) override;
         void Put(GenO& aGenO) const override;

      protected:

         virtual void GetMemberData(GenI& aGenI);
   };
};

#endif
