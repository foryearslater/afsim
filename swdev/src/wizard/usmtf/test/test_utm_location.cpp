// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <string>
#include <tuple>

#include <gtest/gtest.h>

#include "Field.hpp"
#include "UTM_Location.hpp"

TEST(UTM_LocationValid, WorksSeveralValidUTM_Locations)
{
   std::string         validLocation = "UTM:32WDL123123";
   usmtf::UTM_Location validator{};

   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{validLocation}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ("32", validator.GetZoneColumn());
   EXPECT_EQ("W", validator.GetZoneRow());
   EXPECT_EQ("D", validator.GetSquareColumn());
   EXPECT_EQ("L", validator.GetSquareRow());
   EXPECT_EQ("123", validator.GetEasting());
   EXPECT_EQ("123", validator.GetNorthing());
   EXPECT_EQ("55:30:36.3n 35:12:13.2e ", validator.GetFormattedLocation());
}

class UTM_LocationParameterizedFixtureTestsInvalid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(UTM_LocationTestsInvalid,
                        UTM_LocationParameterizedFixtureTestsInvalid,
                        ::testing::Values("UTM:61WDL1234512345",
                                          "UTM:32ADL1234512345",
                                          "UTM:32WIL1234512345",
                                          "UTM:32WDZ1234512345",
                                          "UTM:32WDL99999990000000"), );

TEST_P(UTM_LocationParameterizedFixtureTestsInvalid, FailsForInvalidUTM_Locations)
{
   std::string         invalidLocation = GetParam();
   usmtf::UTM_Location validator{};

   EXPECT_FALSE(validator.ParseAndValidate(usmtf::Field{invalidLocation}));
   EXPECT_FALSE(validator.IsValid());
   EXPECT_EQ("", validator.GetZoneColumn());
   EXPECT_EQ("", validator.GetZoneRow());
   EXPECT_EQ("", validator.GetSquareColumn());
   EXPECT_EQ("", validator.GetSquareRow());
   EXPECT_EQ("", validator.GetEasting());
   EXPECT_EQ("", validator.GetNorthing());
   EXPECT_EQ("00:00:00.0n 00:00:00.0e ", validator.GetFormattedLocation());

   validator.LogErrors(std::cout);
}
