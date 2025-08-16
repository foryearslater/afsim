// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <gtest/gtest.h>

#include "EllipticalArea.hpp"
#include "Field.hpp"


TEST(EllipticalAreaValidatorTests, IsValidForValidEllipses)
{
   const usmtf::Field    f{"105M-200M-240.0"};
   usmtf::EllipticalArea ea{};
   ea.ParseAndValidate(f);
   EXPECT_TRUE(ea.IsValid());
   EXPECT_EQ(ea.GetHorizontalAxisDistance(), "105");
   EXPECT_EQ(ea.GetHorizontalAxisUOM(), "M");
   EXPECT_EQ(ea.GetVerticalAxisDistance(), "200");
   EXPECT_EQ(ea.GetVerticalAxisUOM(), "M");
   EXPECT_EQ(ea.GetAngleFromTrueNorth(), 240.0f);
   ea.LogErrors(std::cout);
}

TEST(EllipticalAreaValidatorTests, IsInvalidForBadFormat)
{
   const usmtf::Field    f{"105M200M240.0"};
   usmtf::EllipticalArea ea{};
   ea.ParseAndValidate(f);
   EXPECT_FALSE(ea.IsValid());
   EXPECT_EQ(ea.GetHorizontalAxisDistance(), "");
   EXPECT_EQ(ea.GetHorizontalAxisUOM(), "");
   EXPECT_EQ(ea.GetVerticalAxisDistance(), "");
   EXPECT_EQ(ea.GetVerticalAxisUOM(), "");
   EXPECT_EQ(ea.GetAngleFromTrueNorth(), 0);
   ea.LogErrors(std::cout);
}

TEST(EllipticalAreaValidatorTests, IsInvalidForBadNumberOfParts)
{
   const usmtf::Field    f{"105M-200M240.0"};
   usmtf::EllipticalArea ea{};
   ea.ParseAndValidate(f);
   EXPECT_FALSE(ea.IsValid());
   EXPECT_EQ(ea.GetHorizontalAxisDistance(), "");
   EXPECT_EQ(ea.GetHorizontalAxisUOM(), "");
   EXPECT_EQ(ea.GetVerticalAxisDistance(), "");
   EXPECT_EQ(ea.GetVerticalAxisUOM(), "");
   EXPECT_EQ(ea.GetAngleFromTrueNorth(), 0);
   ea.LogErrors(std::cout);
}

TEST(EllipticalAreaValidatorTests, IsInvalidForBadDegree)
{
   const usmtf::Field    f{"105M-200M-900.0"};
   usmtf::EllipticalArea ea{};
   ea.ParseAndValidate(f);
   EXPECT_FALSE(ea.IsValid());
   EXPECT_EQ(ea.GetHorizontalAxisDistance(), "");
   EXPECT_EQ(ea.GetHorizontalAxisUOM(), "");
   EXPECT_EQ(ea.GetVerticalAxisDistance(), "");
   EXPECT_EQ(ea.GetVerticalAxisUOM(), "");
   EXPECT_EQ(ea.GetAngleFromTrueNorth(), 0);
   ea.LogErrors(std::cout);
}

TEST(EllipticalAreaValidatorTests, IsInvalidForBadHAxisDistance)
{
   const usmtf::Field    f{"100000M-200M-240.0"};
   usmtf::EllipticalArea ea{};
   ea.ParseAndValidate(f);
   EXPECT_FALSE(ea.IsValid());
   EXPECT_EQ(ea.GetHorizontalAxisDistance(), "");
   EXPECT_EQ(ea.GetHorizontalAxisUOM(), "");
   EXPECT_EQ(ea.GetVerticalAxisDistance(), "");
   EXPECT_EQ(ea.GetVerticalAxisUOM(), "");
   EXPECT_EQ(ea.GetAngleFromTrueNorth(), 0);
   ea.LogErrors(std::cout);
}

TEST(EllipticalAreaValidatorTests, IsInvalidForBadVAxisDistance)
{
   const usmtf::Field    f{"105M-2000000M-240.0"};
   usmtf::EllipticalArea ea{};
   ea.ParseAndValidate(f);
   EXPECT_FALSE(ea.IsValid());
   EXPECT_EQ(ea.GetHorizontalAxisDistance(), "");
   EXPECT_EQ(ea.GetHorizontalAxisUOM(), "");
   EXPECT_EQ(ea.GetVerticalAxisDistance(), "");
   EXPECT_EQ(ea.GetVerticalAxisUOM(), "");
   EXPECT_EQ(ea.GetAngleFromTrueNorth(), 0);
   ea.LogErrors(std::cout);
}


TEST(EllipticalAreaValidatorTests, IsInvalidForBadHAxisUOM)
{
   const usmtf::Field    f{"105P-200M-240.0"};
   usmtf::EllipticalArea ea{};
   ea.ParseAndValidate(f);
   EXPECT_FALSE(ea.IsValid());
   EXPECT_EQ(ea.GetHorizontalAxisDistance(), "");
   EXPECT_EQ(ea.GetHorizontalAxisUOM(), "");
   EXPECT_EQ(ea.GetVerticalAxisDistance(), "");
   EXPECT_EQ(ea.GetVerticalAxisUOM(), "");
   EXPECT_EQ(ea.GetAngleFromTrueNorth(), 0);
   ea.LogErrors(std::cout);
}

TEST(EllipticalAreaValidatorTests, IsInvalidForBadVAxisUOM)
{
   const usmtf::Field    f{"105M-200z-240.0"};
   usmtf::EllipticalArea ea{};
   ea.ParseAndValidate(f);
   EXPECT_FALSE(ea.IsValid());
   EXPECT_EQ(ea.GetHorizontalAxisDistance(), "");
   EXPECT_EQ(ea.GetHorizontalAxisUOM(), "");
   EXPECT_EQ(ea.GetVerticalAxisDistance(), "");
   EXPECT_EQ(ea.GetVerticalAxisUOM(), "");
   EXPECT_EQ(ea.GetAngleFromTrueNorth(), 0);
   ea.LogErrors(std::cout);
}
