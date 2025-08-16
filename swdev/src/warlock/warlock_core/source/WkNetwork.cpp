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

#include "WkNetwork.hpp"

#include <cassert>

#include <QMessageBox>

#include "UtCast.hpp"
#include "WkNetworkPrefObject.hpp"
#include "WkfEnvironment.hpp"

warlock::net::Network::Network()
{
   auto prefs = wkfEnv.GetPreferenceObject<PrefObject>();
   assert(prefs != nullptr);
   connect(prefs, &PrefObject::NetworkChanged, this, &Network::Reconnect);
}

bool warlock::net::Network::Reconnect(const QHostAddress& aAddress, const QHostAddress& aInterface, int aPort)
{
   mAddress   = aAddress;
   mInterface = aInterface;
   mPort      = ut::safe_cast<quint16>(aPort);

   // Replaces the existing socket with a default constructed one.
   mSocket.emplace();
   connect(&*mSocket, &QUdpSocket::readyRead, this, &Network::Read);

   mBound = mSocket->bind(mInterface, mPort, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
   if (mBound)
   {
      mSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 6);
      mBound = mSocket->joinMulticastGroup(mAddress);
      if (!mBound)
      {
         emit NetworkError("Failed to join multicast group", mSocket->errorString());
      }
   }
   else
   {
      emit NetworkError("Failed to bind to socket", mSocket->errorString());
   }

   emit Reconnected(GetSenderId());

   return mBound;
}

bool warlock::net::Network::IsBound() const noexcept
{
   return mBound;
}

void warlock::net::Network::Shutdown()
{
   mSocket.reset();
   mBound = false;
   mRegistry.clear();
   mCallbacks.clear();
}

const QHostAddress& warlock::net::Network::GetAddress() const noexcept
{
   return mAddress;
}

const QHostAddress& warlock::net::Network::GetInterface() const noexcept
{
   return mInterface;
}

int warlock::net::Network::GetPort() const noexcept
{
   return mPort;
}

qint64 warlock::net::Network::GetPID() const noexcept
{
   return mPID;
}

warlock::net::SenderInfo warlock::net::Network::GetSenderId() const noexcept
{
   SenderInfo retval;
   retval.mAddress = mInterface;
   retval.mPort    = mPort;
   retval.mPID     = mPID;
   return retval;
}

bool warlock::net::Network::Publish(const Packet& aPacket)
{
   const QString packetType = aPacket.PacketType();
   auto          it         = mRegistry.find(packetType);
   if (it == mRegistry.end())
   {
      const QString message = "Register with \"wkEnv.GetNetwork().Register<" + packetType + ">();\"";
      emit          NetworkError("Sending unregistered type", message);
      QMessageBox::critical(nullptr, "Warlock Network", "Sending unregistered type\n\n" + message);
      return false;
   }

   if (mBound)
   {
      QByteArray  ba;
      QDataStream ds{&ba, QIODevice::WriteOnly};
      ds << mAppId << mPID << packetType;
      aPacket.WriteTo(ds);

      const qint64 sentBytes = mSocket->writeDatagram(ba, mAddress, mPort);
      if (sentBytes > 0)
      {
         aPacket.SetMetadata(GetSenderId(), static_cast<int>(sentBytes));
         emit PacketSent(aPacket);
         return true;
      }
      emit NetworkError("Failed to send " + packetType, mSocket->errorString());
   }
   else
   {
      emit NetworkError("Failed to send " + packetType, "Bad network setup.");
   }
   return false;
}

// private
void warlock::net::Network::Read()
{
   while (mSocket->hasPendingDatagrams())
   {
      ReadOne();
   }
}

// private
void warlock::net::Network::ReadOne()
{
   QByteArray ba;
   SenderInfo sender;
   ba.resize(mSocket->pendingDatagramSize());
   const qint64 receivedBytes = mSocket->readDatagram(ba.data(), ba.size(), &sender.mAddress, &sender.mPort);

   QDataStream ds{ba};
   if (ds.atEnd())
   {
      // Empty packet.
      return;
   }

   qint64 id;
   ds >> id;
   if (id != mAppId)
   {
      // Packet did not originate from WkNet.
      return;
   }

   ds >> sender.mPID;
   if (IsLoopback(sender))
   {
      // Packet sent from current process.
      return;
   }

   ProcessPacket(ds, sender, static_cast<int>(receivedBytes));
}

// private
void warlock::net::Network::ProcessPacket(QDataStream& aStream, const SenderInfo& aSender, int aByteCount)
{
   QString packetType;
   aStream >> packetType;
   auto pkt = AllocatePacket(packetType, aSender, aByteCount);
   if (!pkt)
   {
      // Unrecognized packet type.
      emit NetworkError("Received unregistered type " + packetType,
                        "Register with \"wkEnv.GetNetwork().Register<" + packetType + ">();\"");
      return;
   }

   try
   {
      pkt->ReadFrom(aStream);
      if (aStream.status() == QDataStream::Status::ReadPastEnd)
      {
         emit NetworkError("Issue reading " + packetType, "Not enough data in stream.");
      }
      else if (!aStream.atEnd())
      {
         emit NetworkError("Issue reading " + packetType, "Data remaining in stream.");
      }
      NotifyCallbacks(*pkt);
   }
   catch (std::exception& e)
   {
      emit NetworkError("Failed to read " + packetType, e.what());
   }
}

// private
bool warlock::net::Network::IsLoopback(const SenderInfo& aSender) const noexcept
{
   return mInterface == aSender.mAddress && mPort == aSender.mPort && mPID == aSender.mPID;
}

// private
std::unique_ptr<warlock::net::Packet> warlock::net::Network::AllocatePacket(const QString&    aPacketType,
                                                                            const SenderInfo& aSender,
                                                                            int               aByteCount) const
{
   auto it = mRegistry.find(aPacketType);
   if (it != mRegistry.end())
   {
      return it->second(aSender, aByteCount);
   }
   return nullptr;
}

// private
void warlock::net::Network::NotifyCallbacks(const Packet& aPacket) const
{
   try
   {
      const int typeCount = aPacket.NodeTypeCount();
      for (int i = typeCount - 1; i >= 0; i--)
      {
         auto it = mCallbacks.find(aPacket.GetNodeType(i));
         if (it != mCallbacks.end())
         {
            it->second->OnReceive(aPacket);
         }
      }
   }
   catch (std::exception& e)
   {
      const QString form  = "Error encountered in packet handler for %1.\n\n%2";
      const QString error = form.arg(aPacket.PacketType(), e.what());
      emit          NetworkError("Error processing " + aPacket.PacketType(), e.what());
      QMessageBox::critical(nullptr, "Warlock Network", error);
   }
}

// private
bool warlock::net::Network::RegisterPacketImpl(const Packet& aPrototype, detail::PacketFactoryFn aFactory)
{
   QString packetType = aPrototype.PacketType();
   auto    it         = mRegistry.find(packetType);
   if (it != mRegistry.end())
   {
      if (it->second != aFactory)
      {
         throw std::logic_error("Another packet is already registered with this name: " + packetType.toStdString());
      }
      return false;
   }
   mRegistry.emplace(packetType, aFactory);
   return true;
}
