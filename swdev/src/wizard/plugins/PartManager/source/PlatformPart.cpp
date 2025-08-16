// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PlatformPart.hpp"

PartManager::PlatformPart::PlatformPart(const WsfPProxyNode& proxyNode, QWidget* parent /*= nullptr*/)
   : Widget(proxyNode, parent)
{
   AddCheckBoxAttr("On", GetNode() + "on");
   AddCheckBoxAttr("Operational", GetNode() + "operational");
   AddCheckBoxAttr("Resurrectible", GetNode() + "resurrectible");
}
