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

#ifndef TARGETMOVER_HPP
#define TARGETMOVER_HPP

#include "WeaponToolsExport.hpp"
#include "WsfMover.hpp"

//! A specialized mover that will move in a straight line as initialized,
//! or, upon having a missile platform index set, will turn away and run
//! from that platform at some specified maximum G setting.  Used in
//! weapon_tools generation.

class WT_EXPORT TargetMover : public WsfMover
{
public:
   TargetMover(WsfScenario& aScenario);
   TargetMover(const TargetMover& aSrc);
   TargetMover& operator=(const TargetMover&) = delete;
   ~TargetMover() override;

   WsfMover* Clone() const override;

   bool Initialize(double aSimTime) override;
   bool ProcessInput(UtInput& aInput) override;
   void Update(double aSimTime) override;

   // Just leave these attributes public.  It's simpler.
   size_t mMissileIndex;
   double mLateralAccelLimit;
   bool   mBreakRight;
   // bool   mTurnDetermined;
};

#endif
