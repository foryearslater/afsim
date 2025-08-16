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
#include "ProxyChange.hpp"


wizard::ProxyChange::ProxyChange(Reason aReason, const WsfPProxyNode& aNode)
   : mReason(aReason)
   , mChanged(aNode)
{
}

wizard::ProxyChange::ProxyChange(Reason aReason, const WsfPProxyNode& aNode, const std::string& aOldName)
   : mReason(aReason)
   , mChanged(aNode)
   , mOldName(aOldName)
{
}
