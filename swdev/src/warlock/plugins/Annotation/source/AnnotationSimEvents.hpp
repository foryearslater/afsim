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

#ifndef WKANNOTATIONSIMEVENTS_HPP
#define WKANNOTATIONSIMEVENTS_HPP

#include "WkSimInterface.hpp"
#include "WkfEnvironment.hpp"
#include "WkfVtkEnvironment.hpp"
#include "WsfAnnotationDataTypes.hpp"
#include "annotation/WkfAnnotationDisplayInterface.hpp"

class AnnotationSimEvent : public warlock::SimEvent
{
public:
   virtual void Process(Annotation::AnnotationDisplayInterface* aDisplayInterface) = 0;
};

class SimStartingEvent : public AnnotationSimEvent
{
public:
   explicit SimStartingEvent(const AnnotationData::AnnotationInfo& aAnnotationInfo)
      : AnnotationSimEvent()
      , mAnnotationInfo(aAnnotationInfo)
   {
   }

   void Process(Annotation::AnnotationDisplayInterface* aDisplayInterface) override;

private:
   AnnotationData::AnnotationInfo mAnnotationInfo;
};

#endif
