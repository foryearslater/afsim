// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef NETWORK_LOG_VISITOR_HPP
#define NETWORK_LOG_VISITOR_HPP

#include "UtOptional.hpp"
#include "UtReflectConstVisitor.hpp"
#include "WkNetworkData.hpp"

namespace NetworkLog
{
struct DataEntry
{
   QString                mName;
   QVariant               mData;
   QList<DataEntry>       mListData;
   ut::optional<unsigned> mPlatformIndex;
};

//! Visits packet contents to create a DataEntry.
//! Sub-objects and list elements are stored in mListData.
class PacketVisitor : public ut::reflect::ConstVisitor<PacketVisitor, const warlock::net::NetworkData, DataEntry()>
{
public:
   PacketVisitor();

   //! Function called if an object does not match any of the registered visitable types.
   DataEntry Fallback(const warlock::net::NetworkData& aData) const override;

private:
   static DataEntry MakeDataEntry(QString aName, bool aDataValid, QVariant aData);

   //! Visits individual types.
   //@{
   DataEntry VisitBasicData(const warlock::net::BasicDataBase& aData) const;
   DataEntry VisitPlatform(const warlock::net::PlatformId& aData) const;
   DataEntry VisitList(const warlock::net::ListBase& aData) const;
   DataEntry VisitObject(const warlock::net::ObjectData& aData) const;
   //@}
};
} // namespace NetworkLog

#endif
