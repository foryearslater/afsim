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

#include "Field.hpp"
#include "FuelType.hpp"

class FuelTypeParameterizedFixtureTestsValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(ValidFuelTypeTests,
                        FuelTypeParameterizedFixtureTestsValid,
                        ::testing::Values("JP4", "JP5", "JP7", "JP8", "F18", "F44", "F35", "F34", "F40"), );

TEST_P(FuelTypeParameterizedFixtureTestsValid, WorksForValidTypes)
{
   std::string     type = GetParam();
   usmtf::FuelType validator{};

   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{type}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ(type, validator.GetType());
}


class FuelTypeParameterizedFixtureTestsInValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(InValidAerialRefuelingSytemTests,
                        FuelTypeParameterizedFixtureTestsInValid,
                        ::testing::Values("not", "valid", "fuels"), );

TEST_P(FuelTypeParameterizedFixtureTestsInValid, FailsForInvalidFuelTypeSamples)
{
   std::string     type = GetParam();
   usmtf::FuelType validator{};

   EXPECT_FALSE(validator.ParseAndValidate(usmtf::Field{type}));
   EXPECT_FALSE(validator.IsValid());
   EXPECT_EQ("", validator.GetType());
   validator.LogErrors(std::cout);
}
