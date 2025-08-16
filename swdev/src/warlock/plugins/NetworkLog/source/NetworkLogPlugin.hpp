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
#ifndef NETWORK_LOG_PLUGIN_HPP
#define NETWORK_LOG_PLUGIN_HPP

#include <set>

#include "NetworkLogListDialog.hpp"
#include "NetworkLogModel.hpp"
#include "WkNetworkData.hpp"
#include "WkfPlugin.hpp"
#include "filter_widget/WkfFilterWidget.hpp"

namespace NetworkLog
{
struct PingPacket;
struct AckPacket;

class Plugin final : public wkf::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);

private:
   //! Registers types and sets up callbacks in the warlock Network.
   void ConnectToNetwork();
   //! Adds "Network Log" to the Developer menu.
   void AddMenuItem();

   //! Adds the sent/received packet to mModel.
   //@{
   void OnPacketSent(const warlock::net::Packet& aPacket);
   void OnPacketReceived(const warlock::net::Packet& aPacket);
   //@}

   //! Called when a PingPacket is received.
   //! Sends an AckPacket.
   void OnPingReceived(const PingPacket& aPacket);

   //! Called when the user clicks the "Ping" button in mDialog.
   //! Sends a PingPacket.
   void OnPingClicked();
   void ShowDialog();

   PluginUiPointer<ListDialog> mDialog;
   Model                       mModel;

   UtCallbackHolder mCallbacks;
};
} // namespace NetworkLog

#endif
