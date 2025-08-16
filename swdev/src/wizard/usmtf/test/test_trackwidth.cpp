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
#include "TrackWidth.hpp"


TEST(TrackWidthValidatorTests, IsValidForValidTrackWidth)
{
   const usmtf::Field f{"30.5NML-60.9NMR"};
   usmtf::TrackWidth  tw{};
   tw.ParseAndValidate(f);
   EXPECT_TRUE(tw.IsValid());
   EXPECT_EQ(tw.GetLeftRadius()->GetRadiusValue(), "30.500000");
   EXPECT_EQ(tw.GetLeftRadius()->GetUnitOfMeasure(), "NM");
   EXPECT_EQ(tw.GetRightRadius()->GetRadiusValue(), "60.900000");
   EXPECT_EQ(tw.GetRightRadius()->GetUnitOfMeasure(), "NM");
   tw.LogErrors(std::cout);
}

TEST(TrackWidthValidatorTests, IsInValidForInValidTrackWidth)
{
   const usmtf::Field f{"NOTVALID"};
   usmtf::TrackWidth  tw{};
   tw.ParseAndValidate(f);
   EXPECT_FALSE(tw.IsValid());
   EXPECT_EQ(tw.GetLeftRadius(), nullptr);
   EXPECT_EQ(tw.GetRightRadius(), nullptr);
   tw.LogErrors(std::cout);
}
