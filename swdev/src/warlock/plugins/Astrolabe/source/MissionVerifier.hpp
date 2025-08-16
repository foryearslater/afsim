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

#ifndef MISSIONVERIFIER_HPP
#define MISSIONVERIFIER_HPP

#include <memory>
#include <utility>

#include "UtCalendar.hpp"

class WsfScenario;
#include "WsfSimulation.hpp"
class WsfSpaceMoverBase;

#include "MissionVerifierBase.hpp"

namespace WkAstrolabe
{

//! Class to provide mission verification for the Astrolabe plugin.
//!
//! Please note that this class should only be used from the sim thread.
//! To do otherwise is a usage error.
class MissionVerifier : public wsfg::spaceg::MissionVerifierBase
{
public:
   explicit MissionVerifier(const WsfSimulation& aSimulation);
   ~MissionVerifier() override = default;

protected:
   WsfSpaceMoverBase*                GetSpaceMover(const std::string& aPlatformName) override;
   std::pair<UtCalendar, UtCalendar> GetStartEpoch(const std::string& aPlatformName) override;
   const WsfScenario&                GetScenario() const override { return mSimulation.GetScenario(); }

private:
   const WsfSimulation& mSimulation;
};

} // namespace WkAstrolabe

#endif // MISSIONVERIFIER_HPP
