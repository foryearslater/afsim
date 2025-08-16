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

#ifndef WkAnnotationSimInterface_HPP
#define WkAnnotationSimInterface_HPP

#include "AnnotationSimEvents.hpp"
#include "WkSimInterface.hpp"

class WsfSimulation;

namespace WkAnnotation
{
class SimInterface : public warlock::SimInterfaceT<AnnotationSimEvent>
{
public:
   explicit SimInterface(const QString& aPluginName);

private:
   void SimulationStarting(const WsfSimulation& aSimulation) override;
};
} // namespace WkAnnotation
#endif
