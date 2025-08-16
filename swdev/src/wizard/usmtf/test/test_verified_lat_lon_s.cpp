// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <gtest/gtest.h>

#include "Field.hpp"
#include "VerifiedLatLonS.hpp"


// LatLon has been tested several times over in the sets its used in, however, a dedicated suite provides more insight
// into potential errors.

TEST(VerifiedLatLonSTests, IsValidForValidLatS)
{
   const usmtf::Field     f{"VLATS:351025N6-0790125W4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_TRUE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "35");
   EXPECT_EQ(ll.GetLatMinutes(), "10");
   EXPECT_EQ(ll.GetLatSeconds(), "25");
   EXPECT_EQ(ll.GetLatHemisphere(), "N");
   EXPECT_EQ(ll.GetLonDegree(), "079");
   EXPECT_EQ(ll.GetLonMinutes(), "01");
   EXPECT_EQ(ll.GetLonSeconds(), "25");
   EXPECT_EQ(ll.GetLonHemisphere(), "W");
   ll.LogErrors(std::cout);
}

TEST(VerifiedLatLonSTests, InvalidForToLargeLatDeg)
{
   const usmtf::Field     f{"VLATS:911025N6-0790125W4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}

TEST(VerifiedLatLonSTests, InvalidForToLargeLatMin)
{
   const usmtf::Field     f{"VLATS:016025N6-0790125W4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}

TEST(VerifiedLatLonSTests, InvalidForToLargeLatSec)
{
   const usmtf::Field     f{"VLATS:351060N6-0790125W4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}

TEST(VerifiedLatLonSTests, InvalidForWrongLatHemi)
{
   const usmtf::Field     f{"VLATS:351025E6-0790125W4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}

TEST(VerifiedLatLonSTests, InvalidForBadContextValue)
{
   const usmtf::Field     f{"VLATS:351025N10-0790125W4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}


TEST(VerifiedLatLonSTests, InvalidForToLargeLonDeg)
{
   const usmtf::Field     f{"VLATS:351025N6-1810125W4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}

TEST(VerifiedLatLonSTests, InvalidForToLargeLonMin)
{
   const usmtf::Field     f{"VLATS:351025N6-0796025W4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}

TEST(VerifiedLatLonSTests, InvalidForToLargeLonSec)
{
   const usmtf::Field     f{"VLATS:351025N6-0790160W4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}

TEST(VerifiedLatLonSTests, InvalidForWrongLonHemi)
{
   const usmtf::Field     f{"VLATS:351025N6-0790125S4"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}

TEST(VerifiedLatLonSTests, InvalidForBadLonContextValue)
{
   const usmtf::Field     f{"VLATS:351025N6-0790125W10"};
   usmtf::VerifiedLatLonS ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "");
   EXPECT_EQ(ll.GetLatMinutes(), "");
   EXPECT_EQ(ll.GetLatSeconds(), "");
   EXPECT_EQ(ll.GetLatHemisphere(), "");
   EXPECT_EQ(ll.GetLonDegree(), "");
   EXPECT_EQ(ll.GetLonMinutes(), "");
   EXPECT_EQ(ll.GetLonSeconds(), "");
   EXPECT_EQ(ll.GetLonHemisphere(), "");
   ll.LogErrors(std::cout);
}
