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

#ifndef ARTICULATEDPART_HPP
#define ARTICULATEDPART_HPP

#include "PlatformPart.hpp"

class WsfPProxyNode;

namespace PartManager
{
class ArticulatedPart : public PlatformPart
{
public:
   explicit ArticulatedPart(const WsfPProxyNode& proxyNode, QWidget* parent = nullptr);
};
} // namespace PartManager

#endif
