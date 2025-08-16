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
#include "Radius.hpp"

TEST(RadiusValidatorTests, IsValidForValidRadiusWithNauticalMiles)
{
   const usmtf::Field f{"40NM"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "NM");
   EXPECT_EQ(r.GetRadiusInMeters(), 74080);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithKiloMeters)
{
   const usmtf::Field f{"40KM"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "KM");
   EXPECT_EQ(r.GetRadiusInMeters(), 40000);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithMeters)
{
   const usmtf::Field f{"40M"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "M");
   EXPECT_EQ(r.GetRadiusInMeters(), 40);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithCentimeters)
{
   const usmtf::Field f{"40CM"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "CM");
   EXPECT_EQ(r.GetRadiusInMeters(), 0.4);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithFeet)
{
   const usmtf::Field f{"40FT"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "FT");
   EXPECT_EQ(r.GetRadiusInMeters(), 12.192);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithhectometers)
{
   const usmtf::Field f{"40HM"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "HM");
   EXPECT_EQ(r.GetRadiusInMeters(), 4000);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithHundredOfFeet)
{
   const usmtf::Field f{"40HF"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "HF");
   EXPECT_EQ(r.GetRadiusInMeters(), 1219.2);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithInches)
{
   const usmtf::Field f{"40IN"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "IN");
   EXPECT_EQ(r.GetRadiusInMeters(), 1.016);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithKilofeet)
{
   const usmtf::Field f{"40KF"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "KF");
   EXPECT_EQ(r.GetRadiusInMeters(), 12192);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithMillimeters)
{
   const usmtf::Field f{"40MM"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "MM");
   EXPECT_EQ(r.GetRadiusInMeters(), 0.040000000000000001);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithMiles)
{
   const usmtf::Field f{"40SM"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "SM");
   EXPECT_EQ(r.GetRadiusInMeters(), 64373.760000000002);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsValidForValidRadiusWithYards)
{
   const usmtf::Field f{"40YD"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "YD");
   EXPECT_EQ(r.GetRadiusInMeters(), 36.576);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsinValidForinValidRadiusValue)
{
   const usmtf::Field f{"P899PNM"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "");
   EXPECT_EQ(r.GetUnitOfMeasure(), "");
   EXPECT_EQ(r.GetRadiusInMeters(), 0);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, ResiliantCasting)
{
   const usmtf::Field f{"40.aNM"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "40.000000");
   EXPECT_EQ(r.GetUnitOfMeasure(), "NM");
   EXPECT_EQ(r.GetRadiusInMeters(), 74080);
   r.LogErrors(std::cout);
}

TEST(RadiusValidatorTests, IsinValidForValidRaduisUnitOfMeasure)
{
   const usmtf::Field f{"40GK"};
   usmtf::Radius      r{};
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   EXPECT_EQ(r.GetRadiusValue(), "");
   EXPECT_EQ(r.GetUnitOfMeasure(), "");
   EXPECT_EQ(r.GetRadiusInMeters(), 0);
   r.LogErrors(std::cout);
}
