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
#include "VerticalDimension.hpp"

TEST(VerticalDimensionValidatorTests, IsValidForValidVerticalDimensionRelativeAltToFlightLevel)
{
   const usmtf::Field       f{"RAFL:100AGL-FL230"};
   usmtf::VerticalDimension vd{};
   vd.ParseAndValidate(f);
   EXPECT_TRUE(vd.IsValid());
   EXPECT_EQ(vd.GetAltitudeUnitOfMeasure(), "AGL");
   EXPECT_EQ(vd.GetFlightIndicator(), "FL");
   EXPECT_EQ(vd.GetMinAltitude(), "100");
   EXPECT_EQ(vd.GetMaxAltitude(), "230");
   vd.LogErrors(std::cout);
}

TEST(VerticalDimensionValidatorTests, IsValidForValidRelativeAltToRelativeAlt)
{
   const usmtf::Field       f{"RARA:000AGL-020AGL"};
   usmtf::VerticalDimension vd{};
   vd.ParseAndValidate(f);
   EXPECT_TRUE(vd.IsValid());
   EXPECT_EQ(vd.GetAltitudeUnitOfMeasure(), "AGL");
   EXPECT_EQ(vd.GetFlightIndicator(), "");
   EXPECT_EQ(vd.GetMinAltitude(), "000");
   EXPECT_EQ(vd.GetMaxAltitude(), "020");
   vd.LogErrors(std::cout);
}

TEST(VerticalDimensionValidatorTests, IsValidForValidVerticalDimensionFlightLevelToFlightLevel)
{
   const usmtf::Field       f{"FLFL:FL250-FL290"};
   usmtf::VerticalDimension vd{};
   vd.ParseAndValidate(f);
   EXPECT_TRUE(vd.IsValid());
   EXPECT_EQ(vd.GetAltitudeUnitOfMeasure(), "");
   EXPECT_EQ(vd.GetFlightIndicator(), "FLFL");
   EXPECT_EQ(vd.GetMinAltitude(), "250");
   EXPECT_EQ(vd.GetMaxAltitude(), "290");
   vd.LogErrors(std::cout);
}
