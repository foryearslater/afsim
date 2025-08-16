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

#include "ArticulatedPart.hpp"

PartManager::ArticulatedPart::ArticulatedPart(const WsfPProxyNode& proxyNode, QWidget* parent /*= nullptr*/)
   : PlatformPart(proxyNode, parent)
{
   AddAngleAttr("Pitch", GetNode() + "pitch");
   AddAngleAttr("Roll", GetNode() + "roll");
   AddAngleAttr("Yaw", GetNode() + "yaw");
   AddAngleAttr("Tilt", GetNode() + "tilt");
   AddAngleAttr("Azimuth min", GetNode() + "azimuthMin");
   AddAngleAttr("Azimuth max", GetNode() + "azimuthMax");
   AddAngleAttr("Elevation min", GetNode() + "elevationMin");
   AddAngleAttr("Elevation max", GetNode() + "elevationMax");
}
