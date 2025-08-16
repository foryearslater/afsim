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

#include "WkNetworkData.hpp"

#include "WkPlatform.hpp"
#include "WkScenario.hpp"

warlock::net::NetworkData::NetworkData(NetworkDataInit aInit) noexcept
   : mName(std::move(aInit.mName))
   , mOptional(aInit.mOptional)
{
}

void warlock::net::NetworkData::ReadFrom(QDataStream& aBuffer)
{
   if (mOptional)
   {
      aBuffer >> mIsValueSet;
      if (mIsValueSet)
      {
         Deserialize(aBuffer);
      }
   }
   else
   {
      mIsValueSet = true;
      Deserialize(aBuffer);
   }
}

void warlock::net::NetworkData::WriteTo(QDataStream& aBuffer) const
{
   if (mOptional)
   {
      aBuffer << mIsValueSet;
      if (mIsValueSet)
      {
         Serialize(aBuffer);
      }
   }
   else
   {
      Serialize(aBuffer);
   }
}

const QString& warlock::net::NetworkData::Name() const noexcept
{
   return mName;
}

bool warlock::net::NetworkData::IsOptional() const noexcept
{
   return mOptional;
}

bool warlock::net::NetworkData::IsValueSet() const noexcept
{
   return mIsValueSet || !mOptional;
}

void warlock::net::NetworkData::ValueSet() noexcept
{
   mIsValueSet = true;
}

void warlock::net::NetworkData::Reset() noexcept
{
   mIsValueSet = false;
}

//////////////////////////////////////////////////////////////////////////////////////

DisEntityId warlock::net::PlatformId::Get() const noexcept
{
   return mId;
}

QString warlock::net::PlatformId::GetDisIdString() const noexcept
{
   const QString form = "%1:%2:%3";
   return form.arg(GetSite()).arg(GetApplication()).arg(GetEntity());
}

quint16 warlock::net::PlatformId::GetSite() const noexcept
{
   return mId.GetSite();
}

quint16 warlock::net::PlatformId::GetApplication() const noexcept
{
   return mId.GetApplication();
}

quint16 warlock::net::PlatformId::GetEntity() const noexcept
{
   return mId.GetEntity();
}

warlock::Platform* warlock::net::PlatformId::GetPlatform() const
{
   auto scenario   = vaEnv.GetStandardScenario();
   auto wkScenario = dynamic_cast<warlock::Scenario*>(scenario);
   if (wkScenario)
   {
      return wkScenario->FindPlatformByDisId(mId);
   }
   return nullptr;
}

void warlock::net::PlatformId::Set(DisEntityId aId) noexcept
{
   mId = aId;
   ValueSet();
}

void warlock::net::PlatformId::Set(quint16 aSite, quint16 aApplication, quint16 aEntity) noexcept
{
   mId.Set(aSite, aApplication, aEntity);
   ValueSet();
}

void warlock::net::PlatformId::SetPlatform(const warlock::Platform* aPlatform) noexcept
{
   if (aPlatform)
   {
      Set(aPlatform->GetDisEntityId());
   }
   else
   {
      Set(DisEntityId());
   }
}

void warlock::net::PlatformId::Serialize(QDataStream& aBuffer) const
{
   aBuffer << GetSite() << GetApplication() << GetEntity();
}

void warlock::net::PlatformId::Deserialize(QDataStream& aBuffer)
{
   quint16 site, application, entity;
   aBuffer >> site >> application >> entity;
   Set(site, application, entity);
}

//////////////////////////////////////////////////////////////////////////////////////

bool warlock::net::ObjectData::IsValueSet() const noexcept
{
   if (NetworkData::IsValueSet())
   {
      return true;
   }
   const int count = FieldCount();
   for (int i = 0; i < count; i++)
   {
      if (GetField(i).IsValueSet())
      {
         return true;
      }
   }
   return false;
}

void warlock::net::ObjectData::Serialize(QDataStream& aBuffer) const
{
   const int count = FieldCount();
   for (int i = 0; i < count; i++)
   {
      GetField(i).WriteTo(aBuffer);
   }
}

void warlock::net::ObjectData::Deserialize(QDataStream& aBuffer)
{
   const int count = FieldCount();
   for (int i = 0; i < count; i++)
   {
      GetField(i).ReadFrom(aBuffer);
   }
}

//////////////////////////////////////////////////////////////////////////////////////

bool warlock::net::ListBase::Empty() const noexcept
{
   return Size() == 0;
}

void warlock::net::ListBase::Serialize(QDataStream& aBuffer) const
{
   const quint32 count = static_cast<quint32>(Size());
   aBuffer << count;
   for (quint32 i = 0; i < count; i++)
   {
      Get(i).WriteTo(aBuffer);
   }
}

void warlock::net::ListBase::Deserialize(QDataStream& aBuffer)
{
   Clear();

   quint32 count = 0;
   aBuffer >> count;
   for (quint32 i = 0; i < count; i++)
   {
      Append().ReadFrom(aBuffer);
   }
}

//////////////////////////////////////////////////////////////////////////////////////

warlock::net::Packet::Packet()
   : ReflectBase(NetworkDataInit{})
{
}

const QHostAddress& warlock::net::Packet::SenderAddress() const noexcept
{
   return mSender.mAddress;
}

int warlock::net::Packet::SenderPort() const noexcept
{
   return mSender.mPort;
}

qint64 warlock::net::Packet::SenderPID() const noexcept
{
   return mSender.mPID;
}

int warlock::net::Packet::ByteCount() const noexcept
{
   return mByteCount;
}

const warlock::net::SenderInfo& warlock::net::Packet::GetSender() const noexcept
{
   return mSender;
}

void warlock::net::Packet::SetMetadata(const SenderInfo& aSender, int aByteCount) const noexcept
{
   mSender    = aSender;
   mByteCount = aByteCount;
}
