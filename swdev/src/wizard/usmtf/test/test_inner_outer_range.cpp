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
#include "InnerOuterRange.hpp"

TEST(InnerOuterRangeValidatorTests, IsValidForValidInnerOuterRange)
{
   const usmtf::Field     f{"40-40NM"};
   usmtf::InnerOuterRange r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "NM");
   EXPECT_EQ(r.GetRadiusInMeters(), 74080);
   EXPECT_EQ(r.GetInnerRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetInnerRadiusInMeters(), 74080);
   EXPECT_EQ(r.GetInnerRadius(), 40.000000);

   r.LogErrors(std::cout);
}

TEST(InnerOuterRangeValidatorTests, DelegationToInternalRadiusFunctions)
{
   const usmtf::Field     f{"40-40GK"};
   usmtf::InnerOuterRange r{};
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "");
   EXPECT_EQ(r.GetUnitOfMeasure(), "");
   EXPECT_EQ(r.GetRadiusInMeters(), 0);
   EXPECT_EQ(r.GetInnerRadiusValue(), "");
   EXPECT_EQ(r.GetInnerRadiusInMeters(), 0);
   EXPECT_EQ(r.GetInnerRadius(), 0);
   r.LogErrors(std::cout);
}

TEST(InnerOuterRangeValidatorTests, IsInvalidForInvalidInnerRange)
{
   const usmtf::Field     f{"100000-40NM"};
   usmtf::InnerOuterRange r{};
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "");
   EXPECT_EQ(r.GetUnitOfMeasure(), "");
   EXPECT_EQ(r.GetRadiusInMeters(), 0);
   EXPECT_EQ(r.GetInnerRadiusValue(), "");
   EXPECT_EQ(r.GetInnerRadiusInMeters(), 0);
   EXPECT_EQ(r.GetInnerRadius(), 0);
   r.LogErrors(std::cout);
}

TEST(InnerOuterRangeValidatorTests, IsInvalidForInvalidPartsNotEnough)
{
   const usmtf::Field     f{"40NM"};
   usmtf::InnerOuterRange r{};
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "");
   EXPECT_EQ(r.GetUnitOfMeasure(), "");
   EXPECT_EQ(r.GetRadiusInMeters(), 0);
   EXPECT_EQ(r.GetInnerRadiusValue(), "");
   EXPECT_EQ(r.GetInnerRadiusInMeters(), 0);
   EXPECT_EQ(r.GetInnerRadius(), 0);
   r.LogErrors(std::cout);
}

TEST(InnerOuterRangeValidatorTests, IsInvalidForInvalidPartsToMany)
{
   const usmtf::Field     f{"40-40NM-nope"};
   usmtf::InnerOuterRange r{};
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "");
   EXPECT_EQ(r.GetUnitOfMeasure(), "");
   EXPECT_EQ(r.GetRadiusInMeters(), 0);
   EXPECT_EQ(r.GetInnerRadiusValue(), "");
   EXPECT_EQ(r.GetInnerRadiusInMeters(), 0);
   EXPECT_EQ(r.GetInnerRadius(), 0);
   r.LogErrors(std::cout);
}
