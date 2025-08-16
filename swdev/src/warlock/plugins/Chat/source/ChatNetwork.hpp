// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef WKFCHATNETWORK_HPP
#define WKFCHATNETWORK_HPP

#include <set>

#include <QCoreApplication>
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

#include "UtCallbackHolder.hpp"
#include "UtOptional.hpp"
#include "WkNetwork.hpp"

class QDataStream;
class QUdpSocket;

namespace Chat
{
using warlock::net::String;
struct MessagePacket : warlock::net::DerivePacket<MessagePacket>
{
   QString PacketType() const override { return "ChatMessage"; }

   Field<String> mUser{this, "user"};
   Field<String> mChannel{this, "channel"};
   Field<String> mText{this, "text"};
};

struct UserJoinPacket : warlock::net::DerivePacket<UserJoinPacket>
{
   QString PacketType() const override { return "ChatJoin"; }

   Field<String> mUser{this, "user"};
   Field<String> mChannel{this, "channel"};
};

struct UserLeavePacket : warlock::net::DerivePacket<UserLeavePacket>
{
   QString PacketType() const override { return "ChatLeave"; }

   Field<String> mUser{this, "user"};
   Field<String> mChannel{this, "channel"};
};

struct UserRenamePacket : warlock::net::DerivePacket<UserRenamePacket>
{
   QString PacketType() const override { return "ChatRename"; }

   Field<String> mUser{this, "user"};
   Field<String> mNewName{this, "new_name"};
};

struct RollCallPacket : warlock::net::DerivePacket<RollCallPacket>
{
   QString PacketType() const override { return "ChatRollCall"; }

   Field<String> mUser{this, "user"};
   Field<String> mChannel{this, "channel"};
   Field<String> mRecipient{this, "recipient"};
};

//! A wrapper around a QUdpSocket to handle sending and receiving Chat-specific packets.
class Network : public QObject
{
   Q_OBJECT

public:
   //! Registers packet types and sets up subscriptions.
   Network();

   //! Emits ConnectionError with the appropriate arguments.
   void EmitConnectionError();

   //! Called when the user sends a message.
   //! @param aUserName is the name of the user.
   //! @param aChannel is the channel the message is sent to.
   //! @param aMessage is the text of the message.
   bool WriteMessage(const QString& aUserName, const QString& aChannel, const QString& aMessage);
   //! Called when the user joins a message.
   //! @param aUserName is the name of the user.
   //! @param aChannel is the channel being joined.
   bool WriteJoinChannel(const QString& aUserName, const QString& aChannel);
   //! Called when the user leaves a channel.
   //! @param aUserName is the name of the user.
   //! @param aChannel is the channel being left.
   bool WriteLeaveChannel(const QString& aUserName, const QString& aChannel);
   //! Called when the user changes their name.
   //! @param aUserName is the user's old name.
   //! @param aNewName is the user's new name.
   bool WriteRename(const QString& aUserName, const QString& aNewName);
   //! Called after receiving a join channel message if the user is on the joined channel.
   //! The idea is when a user joins a channel, they should expect a roll call so they can know who is on the channel.
   //! @param aUserName is the name of the user.
   //! @param aChannel is the channel the other user joined.
   //! @param aName is the name of the other user that joined the channel.
   bool WriteRollCall(const QString& aUserName, const QString& aChannel, const QString& aName);

signals:
   void ReceivedMessage(const QString& aUser, const QString& aChannel, const QString& aMessage);
   void ReceivedJoinChannel(const QString& aUser, const QString& aChannel);
   void ReceivedLeaveChannel(const QString& aUser, const QString& aChannel);
   void ReceivedRename(const QString& aUser, const QString& aNewName);
   void ReceivedRollCall(const QString& aUser, const QString& aChannel, const QString& aRecipient);

   void ConnectionError(const QString& aInterface, int aPort, const QString& aErrorMessage);

private:
   static bool Write(const warlock::net::Packet& aPacket);

   void OnMessagePacket(const MessagePacket& aPacket);
   void OnJoinPacket(const UserJoinPacket& aPacket);
   void OnLeavePacket(const UserLeavePacket& aPacket);
   void OnRenamePacket(const UserRenamePacket& aPacket);
   void OnRollCallPacket(const RollCallPacket& aPacket);

   UtCallbackHolder mCallbacks;
};
} // namespace Chat

#endif
