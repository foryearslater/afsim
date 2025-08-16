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

#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include "ParentWidget.hpp"

namespace PartManager
{
class Platform : public ParentWidget
{
public:
   explicit Platform(const WsfPProxyNode& proxyNode, QWidget* parent = nullptr);

protected:
   void HandleAddPartButtonPressed() override;
   bool IsAddPartSupported() const override;
   void InitAllChildren() override;
};
} // namespace PartManager

#endif
