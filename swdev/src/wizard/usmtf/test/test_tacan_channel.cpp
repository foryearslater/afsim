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
#include "TacanChannel.hpp"

class TacanChannelParameterizedFixtureTestsValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(ValidTacanChannelTests,
                        TacanChannelParameterizedFixtureTestsValid,
                        ::testing::Values("1-1", "22-8", "4-700", "22-888", "16-72", "999-999"), );

TEST_P(TacanChannelParameterizedFixtureTestsValid, WorksForValidTacanChannels)
{
   std::string              channel = GetParam();
   usmtf::TacanChannel      validator{};
   std::vector<std::string> split           = validator.SplitOnDelimeter(channel, "-");
   std::string              recieverChannel = split[0];
   std::string              tankerChannel   = split[1];
   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{channel}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ(recieverChannel, validator.GetRecieverChannel());
   EXPECT_EQ(tankerChannel, validator.GetTankerChannel());
}


class TacanChannelParameterizedFixtureTestsInValid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(InValidTacanChannelTests,
                        TacanChannelParameterizedFixtureTestsInValid,
                        ::testing::Values("not", "valid", "channels", "0-11", "1-01", "0-9999"), );

TEST_P(TacanChannelParameterizedFixtureTestsInValid, FailsForInvalidTacanChannelSamples)
{
   std::string         channel = GetParam();
   usmtf::TacanChannel validator{};

   EXPECT_FALSE(validator.ParseAndValidate(usmtf::Field{channel}));
   EXPECT_FALSE(validator.IsValid());
   EXPECT_EQ("", validator.GetRecieverChannel());
   EXPECT_EQ("", validator.GetTankerChannel());
   validator.LogErrors(std::cout);
}
