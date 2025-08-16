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

#include "AnnotationSimInterface.hpp"

#include "WsfAnnotation.hpp"
#include "WsfAnnotationDataTypes.hpp"
#include "WsfEventPipe.hpp"
#include "WsfScenario.hpp"
#include "WsfSimulation.hpp"

WkAnnotation::SimInterface::SimInterface(const QString& aPluginName)
   : warlock::SimInterfaceT<AnnotationSimEvent>(aPluginName)
{
}

void WkAnnotation::SimInterface::SimulationStarting(const WsfSimulation& aSimulation)
{
   WsfAnnotationExtension* annotationExtension =
      dynamic_cast<WsfAnnotationExtension*>(aSimulation.GetScenario().FindExtension("wsf_annotation"));
   if (annotationExtension)
   {
      AddSimEvent(ut::make_unique<SimStartingEvent>(annotationExtension->GetAnnotationInfo()));
   }
}
