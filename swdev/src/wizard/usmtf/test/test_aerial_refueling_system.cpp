// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <string>

#include <gtest/gtest.h>

#include "AerialRefuelingSystem.hpp"
#include "Field.hpp"

class AerialRefuelingSytemParameterizedFixtureTestsValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(ValidAerialRefuelingTests,
                        AerialRefuelingSytemParameterizedFixtureTestsValid,
                        ::testing::Values("BOM", "CBD", "CWT", "BWD", "CLD", "CDT", "WTD"), );

TEST_P(AerialRefuelingSytemParameterizedFixtureTestsValid, WorksForValidSystems)
{
   std::string                  system = GetParam();
   usmtf::AerialRefuelingSystem validator{};

   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{system}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ(system, validator.GetSystem());
}


class AerialRefuelingSytemParameterizedFixtureTestsInValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(InValidAerialRefuelingSytemTests,
                        AerialRefuelingSytemParameterizedFixtureTestsInValid,
                        ::testing::Values("not", "valid", "systems"), );

TEST_P(AerialRefuelingSytemParameterizedFixtureTestsInValid, FailsForInvalidSystemSamples)
{
   std::string                  system = GetParam();
   usmtf::AerialRefuelingSystem validator{};

   EXPECT_FALSE(validator.ParseAndValidate(usmtf::Field{system}));
   EXPECT_FALSE(validator.IsValid());
   EXPECT_EQ("", validator.GetSystem());
   validator.LogErrors(std::cout);
}
