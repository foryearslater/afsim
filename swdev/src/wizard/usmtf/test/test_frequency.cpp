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
#include "Frequency.hpp"

class FrequencyParameterizedFixtureTestsValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(ValidFrequencyTests,
                        FrequencyParameterizedFixtureTestsValid,
                        ::testing::Values(".0000001", "1.1111", "34.2", "999999.9"), );

TEST_P(FrequencyParameterizedFixtureTestsValid, WorksForValidFrequencySamples)
{
   std::string      frequency = GetParam();
   usmtf::Frequency validator{};

   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{frequency}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ(frequency, validator.GetFrequency());
   EXPECT_EQ(validator.GetCastedValue(), validator.GetNumericalFrequency());
}


class FrequencyParameterizedFixtureTestsInvalid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(InValidFrequencyTests,
                        FrequencyParameterizedFixtureTestsInvalid,
                        ::testing::Values(".00000001", "999999.99", "Frequency", "1000000000"), );

TEST_P(FrequencyParameterizedFixtureTestsInvalid, FailsForInvalidFrequencySamples)
{
   std::string      frequency = GetParam();
   usmtf::Frequency validator{};

   EXPECT_FALSE(validator.ParseAndValidate(usmtf::Field{frequency}));
   EXPECT_FALSE(validator.IsValid());
   EXPECT_EQ("", validator.GetFrequency());
   EXPECT_EQ(0.0, validator.GetNumericalFrequency());
   validator.LogErrors(std::cout);
}
