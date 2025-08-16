// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "TaskAssignerSimEvents.hpp"

// ============================================================================
void WkTaskAssigner::TaskUpdateEvent::Process(DataContainer& aData)
{
   aData.SetData(mPlatformOfInterest,
                 mCommandChainData,
                 mCommunicationDevices,
                 mJammers,
                 mProcessors,
                 mTaskProcessors,
                 mSensors,
                 mWeapons);
}

// ============================================================================
void WkTaskAssigner::DeletePlatformEvent::Process(DataContainer& aData)
{
   aData.RemovePlatform(mPlatformName);
}
