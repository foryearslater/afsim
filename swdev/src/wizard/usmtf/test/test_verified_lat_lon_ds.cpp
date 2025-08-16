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
#include "VerifiedLatLonDS.hpp"

TEST(ValidVerifiedLatLonDSValid, WorksOnValidVerifiedLatLonDS)
{
   std::string             validLocation = "351025.3N9-0790125.7W1";
   usmtf::VerifiedLatLonDS validator{};

   EXPECT_TRUE(validator.ParseAndValidate(usmtf::Field{validLocation}));
   EXPECT_TRUE(validator.IsValid());
   EXPECT_EQ(validator.GetLatDegree(), "35");
   EXPECT_EQ(validator.GetLatMinutes(), "10");
   EXPECT_EQ(validator.GetLatSeconds(), "25.3");
   EXPECT_EQ(validator.GetLatHemisphere(), "N");
   EXPECT_EQ(validator.GetLonDegree(), "079");
   EXPECT_EQ(validator.GetLonMinutes(), "01");
   EXPECT_EQ(validator.GetLonSeconds(), "25.7");
   EXPECT_EQ(validator.GetLonHemisphere(), "W");
   EXPECT_EQ(validator.GetFormattedLatLon(), "35:10:25.3n 79:01:24.3w ");
   validator.LogErrors(std::cout);
}

class VerifiedLatLonDSParameterizedFixtureTestsInvalid : public ::testing::TestWithParam<std::string>
{
};

INSTANTIATE_TEST_CASE_P(UVerifiedLatLonDSTestsInvalid,
                        VerifiedLatLonDSParameterizedFixtureTestsInvalid,
                        ::testing::Values("911025.3N9-0790125.7W1",  // bad lat
                                          "356025.3N9-0790125.7W1",  // bad minute lat
                                          "351060.3N9-0790125.7W1",  // bad second lat
                                          "351025.90N9-0790125.7W1", // bad second lat decimal
                                          "351025.3N9-1810125.7W1",  // bad lon
                                          "351025.3N9-0796025.7W1",  // bad lon minute
                                          "351025.3N9-0790160.7W1",  // bad second lon
                                          "351025.3N9-0790125.90W1", // bad second lon decimal
                                          "351025.3W9-0790125.7W1",  // bad hemi
                                          "351025.3N9-0790125.7S1"   // bad hemi
                                          ), );

TEST_P(VerifiedLatLonDSParameterizedFixtureTestsInvalid, FailsForInvalidVerifiedLatLonDS)
{
   std::string             invalidLocation = GetParam();
   usmtf::VerifiedLatLonDS validator{};

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
