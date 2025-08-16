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

#include "NetworkLogPlugin.hpp"

#include <QListWidget>

#include "NetworkLogListDialog.hpp"
#include "NetworkLogNetwork.hpp"
#include "WkAppEnvironment.hpp"
#include "WkNetwork.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WKF_PLUGIN_DEFINE_SYMBOLS(NetworkLog::Plugin,
                          "Network Log",
                          "Provides a developer dialog for viewing packets sent and received over the Warlock Network.",
                          "warlock")

NetworkLog::Plugin::Plugin(const QString& aPluginName, const size_t aUniqueId)
   : wkf::Plugin(aPluginName, aUniqueId)
{
   ConnectToNetwork();
   AddMenuItem();
}

void NetworkLog::Plugin::ConnectToNetwork()
{
   auto& network = wkEnv.GetNetwork();

   connect(&network, &warlock::net::Network::NetworkError, &mModel, &Model::OnNetworkError);
   connect(&network, &warlock::net::Network::Reconnected, &mModel, &Model::OnReconnect);

   network.RegisterPacket<PingPacket>();
   network.RegisterPacket<AckPacket>();

   mCallbacks += network.Subscribe<warlock::net::Packet>(&Plugin::OnPacketReceived, this);
   connect(&network, &warlock::net::Network::PacketSent, this, &Plugin::OnPacketSent);

   mCallbacks += network.Subscribe<PingPacket>(&Plugin::OnPingReceived, this);
}

void NetworkLog::Plugin::AddMenuItem()
{
   auto* mainWindow = wkfEnv.GetMainWindow();
   if (mainWindow)
   {
      auto* menu = mainWindow->GetMenuByName("Developer");
      if (menu)
      {
         auto* action = menu->addAction("Network Log");
         connect(action, &QAction::triggered, this, &Plugin::ShowDialog);
      }
   }
}

void NetworkLog::Plugin::OnPacketSent(const warlock::net::Packet& aPacket)
{
   mModel.SentPacket(aPacket);
}

void NetworkLog::Plugin::OnPacketReceived(const warlock::net::Packet& aPacket)
{
   mModel.ReceivedPacket(aPacket);
}

void NetworkLog::Plugin::OnPingReceived(const PingPacket& aPacket)
{
   auto& network = wkEnv.GetNetwork();
   network.Publish(AckPacket{});
}

void NetworkLog::Plugin::OnPingClicked()
{
   auto& network = wkEnv.GetNetwork();
   network.Publish(PingPacket{});
}

void NetworkLog::Plugin::ShowDialog()
{
   if (!mDialog)
   {
      mDialog = new ListDialog(wkfEnv.GetMainWindow(), mModel);
      connect(mDialog.data(), &ListDialog::PingClicked, this, &Plugin::OnPingClicked);
   }
   mDialog->show();
}
