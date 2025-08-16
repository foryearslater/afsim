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

#include "VisualEffectsSimEvents.hpp"

#include <osg/Vec4>

#include "VaCamera.hpp"
#include "VaCameraMotionTethered.hpp"
#include "VaEnvironment.hpp"
#include "VaModelDatabase.hpp"
#include "VaViewerManager.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfTeamVisibilityPrefObject.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

EffectPauseOrResumeEvent::EffectPauseOrResumeEvent(bool aPause)
   : mPause(aPause)
{
}

void EffectPauseOrResumeEvent::Process(wkf::VisualEffectsDisplayInterface* aDisplay)
{
   aDisplay->SetPaused(mPause);
}
