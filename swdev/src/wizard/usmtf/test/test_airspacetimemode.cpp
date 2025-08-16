// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <gtest/gtest.h>

#include "AirspaceTimeMode.hpp"
#include "Field.hpp"


TEST(AirspaceTimeModeValidatorTests, IsValidForValidTimeModeInterval)
{
   const usmtf::Field      f{"INTERVAL"};
   usmtf::AirspaceTimeMode atm{};
   atm.ParseAndValidate(f);
   EXPECT_TRUE(atm.IsValid());
   EXPECT_EQ(atm.GetTimeMode(), "INTERVAL");
   atm.LogErrors(std::cout);
}

TEST(AirspaceTimeModeValidatorTests, IsValidForValidTimeModeDiscrete)
{
   const usmtf::Field      f{"DISCRETE"};
   usmtf::AirspaceTimeMode atm{};
   atm.ParseAndValidate(f);
   EXPECT_TRUE(atm.IsValid());
   EXPECT_EQ(atm.GetTimeMode(), "DISCRETE");
   atm.LogErrors(std::cout);
}

TEST(AirspaceTimeModeValidatorTests, IsInValidForInValidTimeMode)
{
   const usmtf::Field      f{"INVALID"};
   usmtf::AirspaceTimeMode atm{};
   atm.ParseAndValidate(f);
   EXPECT_FALSE(atm.IsValid());
   EXPECT_EQ(atm.GetTimeMode(), "");
   atm.LogErrors(std::cout);
}
