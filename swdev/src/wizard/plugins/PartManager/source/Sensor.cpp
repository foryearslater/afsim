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

#include "Sensor.hpp"

PartManager::Sensor::Sensor(const WsfPProxyNode& proxyNode, QWidget* parent /*= nullptr*/)
   : ArticulatedPart(proxyNode, parent)
{
}
