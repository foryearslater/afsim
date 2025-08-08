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

#ifndef WARLOCK_TRAINING_SIMCOMMANDS_HPP
#define WARLOCK_TRAINING_SIMCOMMANDS_HPP

#include "WkSimInterface.hpp"

namespace WarlockTraining
{
   //! Represents a command to turn a platform to a specified direction.
   class TurnCommand : public warlock::SimCommand
   {
   public:
      TurnCommand(const std::string& aPlatformName,
                  double             aHeading);
      ~TurnCommand() override = default;

      //! Processes the command by changing the heading of the platform in question.
      //! @param aSimulation This is the simulation to modify.
      void Process(WsfSimulation& aSimulation) override;

   private:
      //! This is the direction to turn to.
      double mHeading;
      //! This is the name of the platform to be modified.
      std::string mPlatformName;
   };
}
#endif
