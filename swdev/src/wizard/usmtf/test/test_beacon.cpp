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

#include "Beacon.hpp"
#include "Field.hpp"

class BeaconParameterizedFixtureTestsValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(ValidBeaconTests,
                        BeaconParameterizedFixtureTestsValid,
                        ::testing::Values("1-2-6", "3-4-5", "6-6-6", "1-1-1", "4-5-3", "0-0-0"), );

TEST_P(BeaconParameterizedFixtureTestsValid, WorksForValidBeacon)
{
   std::string              signal = GetParam();
   usmtf::Beacon            validator{};
   std::vector<std::string> split  = validator.SplitOnDelimeter(signal, "-");
   std::string              first  = split[0];
   std::string              second = split[1];
   std::string              third  = split[2];
   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{signal}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ(first, validator.GetFirst());
   EXPECT_EQ(second, validator.GetSecond());
   EXPECT_EQ(third, validator.GetThird());
}


class BeaconParameterizedFixtureTestsInValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(InValidBeaconTests,
                        BeaconParameterizedFixtureTestsInValid,
                        ::testing::Values("not", "valid", "beacons", "7-3-1", "4-7-1", "4-3-7", "11-23-22"), );

TEST_P(BeaconParameterizedFixtureTestsInValid, FailsForInvalidBeaconSamples)
{
   std::string   signal = GetParam();
   usmtf::Beacon validator{};

   EXPECT_FALSE(validator.ParseAndValidate(usmtf::Field{signal}));
   EXPECT_FALSE(validator.IsValid());
   EXPECT_EQ("", validator.GetFirst());
   EXPECT_EQ("", validator.GetSecond());
   EXPECT_EQ("", validator.GetThird());
   validator.LogErrors(std::cout);
}
