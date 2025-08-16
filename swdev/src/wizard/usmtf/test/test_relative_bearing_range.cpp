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
#include "RelativeBearingRange.hpp"


// LatLon has been tested several times over in the sets its used in, however, a dedicated suite provides more insight
// into potential errors.

TEST(RelativeBearingRangeTests, IsValidForValidRelativeBearingRange)
{
   const usmtf::Field          f{"330-PT ALFA-50"};
   usmtf::RelativeBearingRange ll{};
   ll.ParseAndValidate(f);
   EXPECT_TRUE(ll.IsValid());
   EXPECT_EQ(ll.GetDegrees(), "330");
   EXPECT_EQ(ll.GetFreeTextOne(), "PT");
   EXPECT_EQ(ll.GetFreeTextTwo(), "ALFA");
   EXPECT_EQ(ll.GetDistance(), "50");
   ll.LogErrors(std::cout);
}

TEST(RelativeBearingRangeTests, IsValidForArbitraryFreeTextOne)
{
   const usmtf::Field          f{"330-OHMANICANGO ALFA-50"};
   usmtf::RelativeBearingRange ll{};
   ll.ParseAndValidate(f);
   EXPECT_TRUE(ll.IsValid());
   EXPECT_EQ(ll.GetDegrees(), "330");
   EXPECT_EQ(ll.GetFreeTextOne(), "OHMANICANGO");
   EXPECT_EQ(ll.GetFreeTextTwo(), "ALFA");
   EXPECT_EQ(ll.GetDistance(), "50");
   ll.LogErrors(std::cout);
}

TEST(RelativeBearingRangeTests, IsValidForArbitraryFreeTextTwo)
{
   const usmtf::Field          f{"330-OHMANICANGO OHMANICANGO-50"};
   usmtf::RelativeBearingRange ll{};
   ll.ParseAndValidate(f);
   EXPECT_TRUE(ll.IsValid());
   EXPECT_EQ(ll.GetDegrees(), "330");
   EXPECT_EQ(ll.GetFreeTextOne(), "OHMANICANGO");
   EXPECT_EQ(ll.GetFreeTextTwo(), "OHMANICANGO");
   EXPECT_EQ(ll.GetDistance(), "50");
   ll.LogErrors(std::cout);
}

TEST(RelativeBearingRangeTests, IsInvalidForInvalidDegree)
{
   const usmtf::Field          f{"360-PT ALFA-50"};
   usmtf::RelativeBearingRange ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetDegrees(), "");
   EXPECT_EQ(ll.GetFreeTextOne(), "");
   EXPECT_EQ(ll.GetFreeTextTwo(), "");
   EXPECT_EQ(ll.GetDistance(), "");
   ll.LogErrors(std::cout);
}

TEST(RelativeBearingRangeTests, IsInvalidForInvalidDistance)
{
   const usmtf::Field          f{"330-PT ALFA-9999"};
   usmtf::RelativeBearingRange ll{};
   ll.ParseAndValidate(f);
   EXPECT_FALSE(ll.IsValid());
   EXPECT_EQ(ll.GetDegrees(), "");
   EXPECT_EQ(ll.GetFreeTextOne(), "");
   EXPECT_EQ(ll.GetFreeTextTwo(), "");
   EXPECT_EQ(ll.GetDistance(), "");
   ll.LogErrors(std::cout);
}
