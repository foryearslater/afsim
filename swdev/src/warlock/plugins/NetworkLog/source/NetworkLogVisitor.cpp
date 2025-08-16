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

#include "NetworkLogVisitor.hpp"

#include "WkPlatform.hpp"

NetworkLog::PacketVisitor::PacketVisitor()
{
   Register<warlock::net::BasicDataBase, &PacketVisitor::VisitBasicData>();
   Register<warlock::net::PlatformId, &PacketVisitor::VisitPlatform>();
   Register<warlock::net::ListBase, &PacketVisitor::VisitList>();
   Register<warlock::net::ObjectData, &PacketVisitor::VisitObject>();
}

NetworkLog::DataEntry NetworkLog::PacketVisitor::Fallback(const warlock::net::NetworkData& aData) const
{
   return MakeDataEntry(aData.Name(), aData.IsValueSet(), "(unknown type)");
}

// static
NetworkLog::DataEntry NetworkLog::PacketVisitor::MakeDataEntry(QString aName, bool aIsDataValid, QVariant aData)
{
   if (aIsDataValid)
   {
      return DataEntry{aName, std::move(aData)};
   }
   return DataEntry{aName, "(no data)"};
}

NetworkLog::DataEntry NetworkLog::PacketVisitor::VisitBasicData(const warlock::net::BasicDataBase& aData) const
{
   return MakeDataEntry(aData.Name(), aData.IsValueSet(), aData.GetVariant());
}

NetworkLog::DataEntry NetworkLog::PacketVisitor::VisitPlatform(const warlock::net::PlatformId& aData) const
{
   if (aData.IsValueSet())
   {
      const QString            form         = "%1 (%2)";
      const warlock::Platform* platform     = aData.GetPlatform();
      const QString            platformName = platform ? QString::fromStdString(platform->GetName()) : "none";
      const unsigned           index        = platform ? platform->GetIndex() : 0;

      return DataEntry{aData.Name(), form.arg(aData.GetDisIdString(), platformName), {}, index};
   }

   return DataEntry{aData.Name(), "(no data)"};
}

NetworkLog::DataEntry NetworkLog::PacketVisitor::VisitList(const warlock::net::ListBase& aData) const
{
   const int     size   = aData.Size();
   const QString form   = "{size: %1}";
   DataEntry     retval = MakeDataEntry(aData.Name(), aData.IsValueSet(), form.arg(size));

   for (int i = 0; i < size; i++)
   {
      auto data  = Visit(aData.Get(i));
      data.mName = "[" + QString::number(i) + "]";
      retval.mListData += std::move(data);
   }
   return retval;
}

NetworkLog::DataEntry NetworkLog::PacketVisitor::VisitObject(const warlock::net::ObjectData& aData) const
{
   const int count   = aData.FieldCount();
   DataEntry retval  = MakeDataEntry(aData.Name(), aData.IsValueSet(), QString{});
   QString   display = "(";

   for (int i = 0; i < count; i++)
   {
      display += ((i > 0) ? ", " : ""); // Append comma after first element.
      display += aData.GetField(i).Name();
      retval.mListData += Visit(aData.GetField(i));
   }

   display += ")";
   if (aData.IsValueSet())
   {
      retval.mData = std::move(display);
   }

   return retval;
}
