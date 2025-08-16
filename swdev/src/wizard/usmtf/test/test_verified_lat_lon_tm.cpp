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
#include "VerifiedLatLonTM.hpp"

TEST(ValidVerifiedLatLonTMValid, WorksOnValidVerifiedLatLonTM)
{
   std::string             validLocation = "VGEOK:3510.234N8-07901.123W3";
   usmtf::VerifiedLatLonTM validator{};

   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{validLocation}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ(validator.GetLatDegree(), "35");
   EXPECT_EQ(validator.GetLatMinutes(), "10");
   EXPECT_EQ(validator.GetLatSeconds(), "14.040000");
   EXPECT_EQ(validator.GetLatHemisphere(), "N");
   EXPECT_EQ(validator.GetLonDegree(), "079");
   EXPECT_EQ(validator.GetLonMinutes(), "01");
   EXPECT_EQ(validator.GetLonSeconds(), "7.380000");
   EXPECT_EQ(validator.GetLonHemisphere(), "W");
   EXPECT_EQ(validator.GetFormattedLatLon(), "35:10:14.0n 79:01:06.6w ");
   validator.LogErrors(std::cout);
}

class VerifiedLatLonTMParameterizedFixtureTestsInvalid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(UVerifiedLatLonTMTestsInvalid,
                        VerifiedLatLonTMParameterizedFixtureTestsInvalid,
                        ::testing::Values("VGEOK:9110.234N8-07901.123W3",  // bad lat
                                          "VGEOK:3560.234N8-07901.123W3",  // bad minute lat
                                          "VGEOK:3510.234N8-18101.123W3",  // bad lon
                                          "VGEOK:3510.234N8-07960.123W3",  // bad lon minute
                                          "VGEOK:3510.234N8-07901.9999W3", // bad fraction
                                          "VGEOK:3510.234W8-07901.123W3",  // bad hemi
                                          "VGEOK:3510.234W8-07901.123S3"   // bad hemi
                                          ), );

TEST_P(VerifiedLatLonTMParameterizedFixtureTestsInvalid, FailsForInvalidVerifiedLatLonTM)
{
   std::string             invalidLocation = GetParam();
   usmtf::VerifiedLatLonTM validator{};

   EXPECT_FALSE(validator.ParseAndValidate(usmtf::Field{invalidLocation}));
   EXPECT_FALSE(validator.IsValid());
   EXPECT_EQ(validator.GetLatDegree(), "");
   EXPECT_EQ(validator.GetLatMinutes(), "");
   EXPECT_EQ(validator.GetLatSeconds(), "");
   EXPECT_EQ(validator.GetLatHemisphere(), "");
   EXPECT_EQ(validator.GetLonDegree(), "");
   EXPECT_EQ(validator.GetLonMinutes(), "");
   EXPECT_EQ(validator.GetLonSeconds(), "");
   EXPECT_EQ(validator.GetLonHemisphere(), "");
   EXPECT_EQ(validator.GetFormattedLatLon(), "00:00:00.0n 00:00:00.0e ");
   validator.LogErrors(std::cout);
   ;
}
