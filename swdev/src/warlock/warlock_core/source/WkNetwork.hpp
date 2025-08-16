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

#ifndef WK_NETWORK_HPP
#define WK_NETWORK_HPP

#include "warlock_core_export.h"

#include <functional>

#include <QCoreApplication>
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

#include "UtBinder.hpp"
#include "UtCallbackN.hpp"
#include "UtOptional.hpp"
#include "WkNetworkData.hpp"

namespace warlock
{
namespace net
{
namespace detail
{
//! Used when receiving to allocate the packet to fill with incoming data.
using PacketFactoryFn = std::unique_ptr<Packet> (*const)(const SenderInfo&, int);

//! The specific factory for each packet type.
template<typename T>
std::unique_ptr<Packet> PacketFactory(const SenderInfo& aSender, int aByteCount)
{
   auto ptr = ut::make_unique<T>();
   ptr->SetMetadata(aSender, aByteCount);
   return std::move(ptr); // std::move required for GCC
}

//! Helper alias.
using PacketRegistry = std::map<QString, PacketFactoryFn>;

//////////////////////////////////////////////////////////////////////////////

//! Adds a virtual method for performing downcasts when receiving packets on the network.
struct PacketCallbackList
{
   virtual ~PacketCallbackList() = default;
   //! Called when a packet is received.
   //! It is the caller's responsibility to ensure this only gets called on appropriate types.
   virtual void OnReceive(const Packet& aPacket) const = 0;
};

//! Callback list for a specific packet type.
template<typename T>
struct PacketCallbackListT final : UtCallbackListN<void(const T&)>, PacketCallbackList
{
   static_assert(std::is_base_of<Packet, T>::value, "T must derive from warlock::net::Packet.");

   void OnReceive(const Packet& aPacket) const override
   {
      // static_cast should be safe here.
      const T& pkt = dynamic_cast<const T&>(aPacket);
      (*this)(pkt);
   }
};

//! Helper alias.
using CallbackRegistry = std::map<std::type_index, std::unique_ptr<PacketCallbackList>>;
} // namespace detail

//! Network is the network interface that plugins may access to send/receive network packets.
class WARLOCK_CORE_EXPORT Network : public QObject
{
   Q_OBJECT

public:
   Network();

   //! Connects the Network to the socket.
   //! If already connected, disconnects and reconnects.
   //! Returns true on success.
   //! aAddress is the multicast address.
   //! aInteface is the local socket.
   //! aPort is the port number. [0, 65535]
   bool Reconnect(const QHostAddress& aAddress, const QHostAddress& aInterface, int aPort);

   //! Returns true if the network has successfully connected to the multicast group.
   bool IsBound() const noexcept;

   //! Resets the socket and disconnects all callbacks.
   //! Should be called during application shutdown to prevent dangling references.
   void Shutdown();

   //! Returns the values passed to Reconnect.
   //{
   const QHostAddress& GetAddress() const noexcept;   //!< The multicast address.
   const QHostAddress& GetInterface() const noexcept; //!< The local socket.
   int                 GetPort() const noexcept;      //!< The port number. [0, 65535]
   //}

   //! Returns the PID attached to all outgoing packets.
   qint64 GetPID() const noexcept;

   //! Returns the sender id for this application's network.
   SenderInfo GetSenderId() const noexcept;

   //! Attempts to write aPacket to a socket.
   //! Returns true on success.
   bool Publish(const Packet& aPacket);

   //! Registers the packet type T.
   //! Returns false if T is already registered.
   //! Throws std::logic_error if there is a name collision.
   //! Otherwise, returns true.
   //! @tparam T is the type being registered.
   template<typename T>
   bool RegisterPacket()
   {
      static_assert(!std::is_abstract<T>::value, "Cannot register abstract types.");
      static_assert(std::is_base_of<Packet, T>::value, "Registered type must be a warlock::net::Packet.");
      static_assert(std::is_default_constructible<T>::value, "Registered type must be default constructible.");
      static_assert(ut::reflect::is_derived<T>::value,
                    "Registered type does not properly use warlock::net::DerivePacket (ut::reflect::Derive).");
      return RegisterPacketImpl(T{}, &detail::PacketFactory<T>);
   }

   //! Creates a member function callback and subscribes it to type T.
   //! The returned value should be stored (@see UtCallbackHolder) to keep the subscription alive.
   //! Written this way to force users of the API to explicitly name type T.
   //! Also allows methods accepting a base type to subscribe to a derived type.
   //! @tparam T is the type being subscribed to.
   //! @tparam U is the argument type of the callback.
   //! @tparam C is the class type the method is called on.
   template<typename T, typename U, typename C>
   CPP17_NODISCARD std::unique_ptr<UtCallback> Subscribe(void (C::*aFn)(const U&), C* aObject)
   {
      static_assert(std::is_base_of<U, T>::value, "T must derive from the method argument.");
      return Subscribe<T>(UtStd::Bind(aFn, aObject));
   }

   //! Creates a non-member function callback and subscribes it to type T.
   //! The returned value should be stored (@see UtCallbackHolder) to keep the subscription alive.
   //! Written this way to force users of the API to explicitly name type T.
   //! Also allows methods accepting a base type to subscribe to a derived type.
   //! @tparam T is the type being subscribed to.
   //! @tparam FUNC is a type callable as `void (const T&)`.
   template<typename T, typename FUNC>
   CPP17_NODISCARD std::unique_ptr<UtCallback> Subscribe(FUNC&& aFn)
   {
      static_assert(std::is_base_of<Packet, T>::value, "T must be a warlock::net::Packet.");
      static_assert(std::is_constructible<std::function<void(const T&)>, FUNC>::value,
                    "Cannot create subscription callback.");
      static_assert(ut::reflect::is_derived<T>::value,
                    "T does not properly use warlock::net::DerivePacket (ut::reflect::Derive).");
      auto it = mCallbacks.find(typeid(T));
      if (it == mCallbacks.end())
      {
         it = mCallbacks.emplace(typeid(T), ut::make_unique<detail::PacketCallbackListT<T>>()).first;
      }
      auto& cb = dynamic_cast<detail::PacketCallbackListT<T>&>(*it->second);
      return cb.Connect(std::forward<FUNC>(aFn));
   }

signals:
   //! Called whenever a packet is successfully sent.
   void PacketSent(const Packet& aPacket);

   //! Called whenever a network error occurs.
   //! This may occur when the following actions happen.
   //!  * Failed to connect to socket.
   //!  * Failed to join multicast group.
   //!  * Failed to send message.
   //!  * Received message of unregistered type.
   //!  * Failed to read entire message.
   void NetworkError(const QString& aErrorMessage, const QString& aErrorDetails) const;

   //! Called after Reconnect is called.
   //! @param aSenderId contains the new network setup.
   //! @param aConnected contains whether Reconnect returned true or false.
   void Reconnected(const SenderInfo& aSenderId);

private:
   //! Called when the socket receives a datagram.
   void Read();

   //! Reads a single datagram from the socket.
   void ReadOne();

   //! Creates and fills a packet from the contents of aStream.
   void ProcessPacket(QDataStream& aStream, const SenderInfo& aSender, int aByteCount);

   //! Returns true if aSender refers to the current application.
   bool IsLoopback(const SenderInfo& aSender) const noexcept;

   //! Allocates a packet to fill with incoming data.
   //! Returns nullptr if aPacketType is not registered.
   std::unique_ptr<Packet> AllocatePacket(const QString& aPacketType, const SenderInfo& aSender, int aByteCount) const;

   //! Notifies callbacks of a received packet.
   //! Callbacks for every type in aPacket's type hierarchy is notified.
   void NotifyCallbacks(const Packet& aPrototype) const;

   //! Called from RegisterPacket.
   bool RegisterPacketImpl(const Packet& aPacket, detail::PacketFactoryFn aFactory);

   //! mSocket uses ut::optional so that it can be destructed and re-constructed several times without using heap allocation.
   ut::optional<QUdpSocket> mSocket;
   QHostAddress             mAddress;   //!< mAddress is the multicast address.
   QHostAddress             mInterface; //!< mInterface is the local socket.
   quint16                  mPort  = 0;
   const qint64             mAppId = 0x576b4e6574; // "WkNet"
   const qint64             mPID   = QCoreApplication::applicationPid();
   bool                     mBound = false;

   detail::PacketRegistry   mRegistry;
   detail::CallbackRegistry mCallbacks;
};
} // namespace net
} // namespace warlock

#endif
