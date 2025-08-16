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

#ifndef CYBERENGAGEMENTCONTROLLERSIMCOMMAND_HPP
#define CYBERENGAGEMENTCONTROLLERSIMCOMMAND_HPP

#include "WkSimInterface.hpp"

class WsfSimulation;

namespace WkCyberEngagementController
{
class CyberEngagementControllerCommand : public warlock::SimCommand
{
public:
   explicit CyberEngagementControllerCommand(const std::string& aAttackType,
                                             const std::string& aAttacker,
                                             const std::string& aVictim)
      : SimCommand(false)
      , mAttackType(aAttackType)
      , mAttacker(aAttacker)
      , mVictim(aVictim)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mAttackType;
   std::string mAttacker;
   std::string mVictim;
};

class CyberScanCommand : public warlock::SimCommand
{
public:
   explicit CyberScanCommand(const std::string& aAttackType, const std::string& aAttacker, const std::string& aVictim)
      : SimCommand(false)
      , mAttackType(aAttackType)
      , mAttacker(aAttacker)
      , mVictim(aVictim)
   {
   }

   void Process(WsfSimulation& aSimulation) override;

private:
   std::string mAttackType;
   std::string mAttacker;
   std::string mVictim;
};
} // namespace WkCyberEngagementController

#endif
