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

#ifndef SPACETOOLSSCENARIO_HPP
#define SPACETOOLSSCENARIO_HPP

#include <memory>

#include "WsfScenario.hpp"

namespace SpaceTools
{

//! This utility class provides the WsfScenario for the SpaceTools plugin.
//!
//! Instances of this class should not be kept beyond the bare minimum in which it is needed.
class Scenario
{
public:
   Scenario();
   ~Scenario() = default;

   // Disallowing copy and move limits the lifetime of these objects.
   Scenario(const Scenario& aOther) = delete;
   Scenario(Scenario&& aOther)      = delete;
   Scenario& operator=(const Scenario& aOther) = delete;
   Scenario& operator=(Scenario&& aOther) = delete;

   WsfScenario& GetScenario() const { return *mScenarioPtr; }

private:
   std::unique_ptr<WsfScenario> mScenarioPtr;
};

} // namespace SpaceTools

#endif // SPACETOOLSSCENARIO_HPP
