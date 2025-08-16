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
#include "LatLon.hpp"


// LatLon has been tested several times over in the sets its used in, however, a dedicated suite provides more insight
// into potential errors.

TEST(LatLonValidatorTests, IsValidForValidLatS)
{
   const usmtf::Field f{"LATS:203632N0594256E"};
   usmtf::LatLon      ll{};
   ll.ParseAndValidate(f);
   EXPECT_TRUE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "20");
   EXPECT_EQ(ll.GetLatMinutes(), "36");
   EXPECT_EQ(ll.GetLatSeconds(), "32");
   EXPECT_EQ(ll.GetLatHemisphere(), "N");
   EXPECT_EQ(ll.GetLonDegree(), "059");
   EXPECT_EQ(ll.GetLonMinutes(), "42");
   EXPECT_EQ(ll.GetLonSeconds(), "56");
   EXPECT_EQ(ll.GetLonHemisphere(), "E");
   ll.LogErrors(std::cout);
}

TEST(LatLonValidatorTests, IsValidForValidLatM)
{
   const usmtf::Field f{"LATM:2037N05934E"};
   usmtf::LatLon      ll{};
   ll.ParseAndValidate(f);
   EXPECT_TRUE(ll.IsValid());
   EXPECT_EQ(ll.GetLatDegree(), "20");
   EXPECT_EQ(ll.GetLatMinutes(), "37");
   EXPECT_EQ(ll.GetLatSeconds(), "0");
   EXPECT_EQ(ll.GetLatHemisphere(), "N");
   EXPECT_EQ(ll.GetLonDegree(), "059");
   EXPECT_EQ(ll.GetLonMinutes(), "34");
   EXPECT_EQ(ll.GetLonSeconds(), "0");
   EXPECT_EQ(ll.GetLonHemisphere(), "E");
   ll.LogErrors(std::cout);
}

TEST(LatLonValidatorTests, IsinValidForinValidLatMLength)
{
   const usmtf::Field f{"LATM:2037N05934EASDFA"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatMLatDegree)
{
   const usmtf::Field f{"LATM:9137N05934E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatMLatMin)
{
   const usmtf::Field f{"LATM:2071N05934E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatMLatHem)
{
   const usmtf::Field f{"LATM:2037E05934E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatMLonDeg)
{
   const usmtf::Field f{"LATM:2037N18134E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatMLonMin)
{
   const usmtf::Field f{"LATM:2037N05962E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatMLonHem)
{
   const usmtf::Field f{"LATM:2037N05934S"};
   usmtf::LatLon      ll{};
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


TEST(LatLonValidatorTests, IsinValidForinValidLatSLength)
{
   const usmtf::Field f{"LATS:203632N0594256EASDF"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatSLatDegree)
{
   const usmtf::Field f{"LATS:913632N0594256E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatSLatMin)
{
   const usmtf::Field f{"LATS:206532N0594256E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatSLatSec)
{
   const usmtf::Field f{"LATS:203666N0594256E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatSLatHem)
{
   const usmtf::Field f{"LATS:203632W0594256E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatSLonDeg)
{
   const usmtf::Field f{"LATS:203632N1814256E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatSLonMin)
{
   const usmtf::Field f{"LATS:203632N0596556E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatSLonSec)
{
   const usmtf::Field f{"LATS:203632N0594265E"};
   usmtf::LatLon      ll{};
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

TEST(LatLonValidatorTests, IsinValidForinValidLatSLonHem)
{
   const usmtf::Field f{"LATS:203632N0594256S"};
   usmtf::LatLon      ll{};
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
