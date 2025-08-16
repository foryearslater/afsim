//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************

#include "PlatformHistorySimEvents.hpp"

#include "PlatformHistoryPlugin.hpp"

void WkPlatformHistory::DetectedEvent::Process(Plugin& aPlugin)
{
   aPlugin.UpdateTraceLineDetectionData(mTargetPlatformIndex, mSourcePlatformIndex, mInitiated);
}

void WkPlatformHistory::TrackedEvent::Process(Plugin& aPlugin)
{
   aPlugin.UpdateTraceLineTrackData(mTargetPlatformIndex, mSourcePlatformIndex, mInitiated);
}

void WkPlatformHistory::AttackedEvent::Process(Plugin& aPlugin)
{
   aPlugin.UpdateTraceLineAttackData(mTargetPlatformIndex, mSourcePlatformIndex, mInitiated);
}

void WkPlatformHistory::PlatformDeletedEvent::Process(Plugin& aPlugin)
{
   aPlugin.PlatformDeletedHandler(mIndex);
}
