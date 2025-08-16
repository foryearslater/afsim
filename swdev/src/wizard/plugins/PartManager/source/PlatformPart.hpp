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

#ifndef PLATFORMPART_HPP
#define PLATFORMPART_HPP

#include "Widget.hpp"

namespace PartManager
{
class PlatformPart : public Widget
{
public:
   explicit PlatformPart(const WsfPProxyNode& proxyNode, QWidget* parent = nullptr);
};
} // namespace PartManager

#endif
