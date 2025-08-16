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

#include "Revision.hpp"

wizard::Revision::Revision(int aRevNo, int aParentRevNo, const std::string& aWorkingDir, int aTimestamp)
   : revNo(aRevNo)
   , parentRevNo(aParentRevNo)
   , workingDir(aWorkingDir)
   , time(QDateTime::fromTime_t((uint)aTimestamp))
{
}
