// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <gtest/gtest.h>

#include "Altitude.hpp"
#include "Field.hpp"


TEST(AltitudeTestCase, IsValidForValidAltitude)
{
   const usmtf::Field f{"210"};
   usmtf::Altitude    av{};
   av.ParseAndValidate(f);
   EXPECT_TRUE(av.IsValid());
   EXPECT_EQ(av.GetVerticalDistance(), "210.000000");
   av.LogErrors(std::cout);
}

TEST(AltitudeTestCase, IsInValidForInvalidRange)
{
   const usmtf::Field f{"9999"};
   usmtf::Altitude    av{};
   av.ParseAndValidate(f);
   EXPECT_FALSE(av.IsValid());
   EXPECT_EQ(av.GetVerticalDistance(), "");
   av.LogErrors(std::cout);
}


TEST(AltitudeTestCase, IsInValidForInvalidRangeThenValidAfterSet)
{
   const usmtf::Field f{"9999"};
   usmtf::Altitude    av{};
   av.ParseAndValidate(f);
   EXPECT_FALSE(av.IsValid());
   EXPECT_EQ(av.GetVerticalDistance(), "");
   av.LogErrors(std::cout);

   av.SetValidRange(0, 10000);
   av.ParseAndValidate(f);
   EXPECT_TRUE(av.IsValid());
   EXPECT_EQ(av.GetVerticalDistance(), "9999.000000");
   av.LogErrors(std::cout);
}

TEST(AltitudeTestCase, IsInValidForInvalidRangeBadType)
{
   const usmtf::Field f{"PEEP"};
   usmtf::Altitude    av{};
   av.ParseAndValidate(f);
   EXPECT_FALSE(av.IsValid());
   EXPECT_EQ(av.GetVerticalDistance(), "");
   av.LogErrors(std::cout);
}
