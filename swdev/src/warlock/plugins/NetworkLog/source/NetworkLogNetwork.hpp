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

#ifndef NETWORK_LOG_NETWORK_HPP
#define NETWORK_LOG_NETWORK_HPP

#include "WkNetwork.hpp"

namespace NetworkLog
{
//! A PingPacket can be sent to determine who is on the network.
//! When a PingPacket is received, an AckPacket is published.
struct PingPacket final : warlock::net::DerivePacket<PingPacket>
{
   QString PacketType() const override { return "NetworkLogPing"; }
};

//! An AckPacket is part of the mechanism to determine who all is on the network.
//! When received, the sender is added to a list.
struct AckPacket final : warlock::net::DerivePacket<AckPacket>
{
   QString PacketType() const override { return "NetworkLogAck"; }
};
} // namespace NetworkLog

#endif
