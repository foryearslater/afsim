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

#ifndef MISSIONVERIFIER_HPP
#define MISSIONVERIFIER_HPP

#include <memory>
#include <utility>

#include "UtCalendar.hpp"

class WsfScenario;
#include "MissionVerifierBase.hpp"
#include "SpaceToolsScenario.hpp"
#include "WsfSpaceMoverBase.hpp"

namespace SpaceTools
{

class MissionVerifier : public wsfg::spaceg::MissionVerifierBase
{
public:
   MissionVerifier();
   ~MissionVerifier() override = default;

protected:
   WsfSpaceMoverBase*                GetSpaceMover(const std::string& aPlatformName) override;
   std::pair<UtCalendar, UtCalendar> GetStartEpoch(const std::string& aPlatformName) override;
   const WsfScenario&                GetScenario() const override { return mScenario.GetScenario(); }

private:
   Scenario                           mScenario;
   std::unique_ptr<WsfSpaceMoverBase> mMoverPtr;
};

} // namespace SpaceTools

#endif // MISSIONVERIFIER_HPP
