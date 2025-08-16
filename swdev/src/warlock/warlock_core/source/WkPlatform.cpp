// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "WkPlatform.hpp"

warlock::Platform::Platform(const std::string& aName)
   : wkf::Platform(aName)
   , mUpdateTime(0.0)
   , mIsExternallyControlled(false)
   , mIsXIO_Controlled(false)
{
   SetType<Platform>();
}
