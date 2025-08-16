// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef CRDTRANSITION_HPP
#define CRDTRANSITION_HPP

#include <map>

#include "ElementBase.hpp"
#include "Intent.hpp"

namespace CrdImporter
{
/*
CrdTransition, CrdState, are prepended with Crd to try to avoid confusion with state machine classes in Wizard, even
though they are not named Transition or State.  CrdMission is used instead of Mission since the main program file is in
mission.cpp. C++ would allow Mission (uppercase M) as a class, but can't have two mission.cpp files.
*/

class CrdTransition : public ElementBase
{
public:
   CrdTransition();
   ~CrdTransition() override;

   CrdTransition(const CrdTransition& aRhs) = delete;
   CrdTransition& operator=(const CrdTransition& aRhs) = delete;

   void SetIntentReference(long aId) { mIntentReference = aId; }
   void SetStartStateReference(long aId) { mStartStateReference = aId; }
   void SetEndStateReference(long aId) { mEndStateReference = aId; }
   void SetMinimumSafeAltitude(double aAlt) { mMinimumSafeAlt = aAlt; }

   long   GetIntentReference() const { return mIntentReference; }
   long   GetStartStateReference() const { return mStartStateReference; }
   long   GetEndStateReference() const { return mEndStateReference; }
   double GetMinimumSafeAltitude() const { return mMinimumSafeAlt; }

private:
   bool   mCalculationFlag;
   long   mIntentReference;
   long   mStartStateReference;
   long   mEndStateReference;
   double mMinimumSafeAlt;
};
} // namespace CrdImporter
#endif
