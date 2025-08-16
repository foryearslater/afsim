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

#include "ChatNetwork.hpp"

#include <QDataStream>
#include <QNetworkInterface>

#include "WkAppEnvironment.hpp"

Chat::Network::Network()
{
   auto& wkNet = wkEnv.GetNetwork();

   wkNet.RegisterPacket<MessagePacket>();
   wkNet.RegisterPacket<UserJoinPacket>();
   wkNet.RegisterPacket<UserLeavePacket>();
   wkNet.RegisterPacket<UserRenamePacket>();
   wkNet.RegisterPacket<RollCallPacket>();

   mCallbacks += wkNet.Subscribe<MessagePacket>(&Network::OnMessagePacket, this);
   mCallbacks += wkNet.Subscribe<UserJoinPacket>(&Network::OnJoinPacket, this);
   mCallbacks += wkNet.Subscribe<UserLeavePacket>(&Network::OnLeavePacket, this);
   mCallbacks += wkNet.Subscribe<UserRenamePacket>(&Network::OnRenamePacket, this);
   mCallbacks += wkNet.Subscribe<RollCallPacket>(&Network::OnRollCallPacket, this);
}

void Chat::Network::EmitConnectionError()
{
   const auto& wkNet = wkEnv.GetNetwork();
   emit        ConnectionError(wkNet.GetInterface().toString(), wkNet.GetPort(), "connection error");
}

bool Chat::Network::WriteMessage(const QString& aUserName, const QString& aChannel, const QString& aMessage)
{
   MessagePacket pkt;
   pkt.mUser->Set(aUserName);
   pkt.mChannel->Set(aChannel);
   pkt.mText->Set(aMessage);
   return Write(pkt);
}

bool Chat::Network::WriteJoinChannel(const QString& aUserName, const QString& aChannel)
{
   UserJoinPacket pkt;
   pkt.mUser->Set(aUserName);
   pkt.mChannel->Set(aChannel);
   return Write(pkt);
}

bool Chat::Network::WriteLeaveChannel(const QString& aUserName, const QString& aChannel)
{
   UserLeavePacket pkt;
   pkt.mUser->Set(aUserName);
   pkt.mChannel->Set(aChannel);
   return Write(pkt);
}

bool Chat::Network::WriteRename(const QString& aUserName, const QString& aNewName)
{
   UserRenamePacket pkt;
   pkt.mUser->Set(aUserName);
   pkt.mNewName->Set(aNewName);
   return Write(pkt);
}

bool Chat::Network::WriteRollCall(const QString& aUserName, const QString& aChannel, const QString& aName)
{
   RollCallPacket pkt;
   pkt.mUser->Set(aUserName);
   pkt.mChannel->Set(aUserName);
   pkt.mRecipient->Set(aName);
   return Write(pkt);
}

// static
bool Chat::Network::Write(const warlock::net::Packet& aPacket)
{
   return wkEnv.GetNetwork().Publish(aPacket);
}

void Chat::Network::OnMessagePacket(const MessagePacket& aPacket)
{
   emit ReceivedMessage(aPacket.mUser->Get(), aPacket.mChannel->Get(), aPacket.mText->Get());
}

void Chat::Network::OnJoinPacket(const UserJoinPacket& aPacket)
{
   emit ReceivedJoinChannel(aPacket.mUser->Get(), aPacket.mChannel->Get());
}

void Chat::Network::OnLeavePacket(const UserLeavePacket& aPacket)
{
   emit ReceivedLeaveChannel(aPacket.mUser->Get(), aPacket.mChannel->Get());
}

void Chat::Network::OnRenamePacket(const UserRenamePacket& aPacket)
{
   emit ReceivedRename(aPacket.mUser->Get(), aPacket.mNewName->Get());
}

void Chat::Network::OnRollCallPacket(const RollCallPacket& aPacket)
{
   emit ReceivedMessage(aPacket.mUser->Get(), aPacket.mChannel->Get(), aPacket.mRecipient->Get());
}
