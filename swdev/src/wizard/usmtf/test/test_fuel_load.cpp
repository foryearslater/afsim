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
#include "FuelLoad.hpp"

class FuelLoadParameterizedFixtureTestsValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(ValidFuelLoadTests,
                        FuelLoadParameterizedFixtureTestsValid,
                        ::testing::Values(".1", "5.1", "34.2", "999.9"), );

TEST_P(FuelLoadParameterizedFixtureTestsValid, WorksForValidFuelLoadSamples)
{
   std::string     frequency = GetParam();
   usmtf::FuelLoad validator{};

   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{frequency}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ(frequency, validator.GetFuelAmount());
   EXPECT_EQ(validator.GetCastedValue(), validator.GetNumericalFuelAmount());
}


class FuelLoadParameterizedFixtureTestsInvalid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(InValidFuelLoadTests,
                        FuelLoadParameterizedFixtureTestsInvalid,
                        ::testing::Values(".00000001", "999999.99", "FuelLoad", "1000000000"), );

TEST_P(FuelLoadParameterizedFixtureTestsInvalid, FailsForInvalidFuelLoadSamples)
{
   std::string     frequency = GetParam();
   usmtf::FuelLoad validator{};

   EXPECT_FALSE(validator.ParseAndValidate(usmtf::Field{frequency}));
   EXPECT_FALSE(validator.IsValid());
   EXPECT_EQ("", validator.GetFuelAmount());
   EXPECT_EQ(0.0, validator.GetNumericalFuelAmount());
   validator.LogErrors(std::cout);
}
