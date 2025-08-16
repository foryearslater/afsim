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
#include "RegexValidator.hpp"

// Using widely used Radius checks for testing.

TEST(RegexValidatorTests, IsValidForValidRadiusWithNauticalMiles)
{
   const usmtf::Field    f{"40NM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithNauticalMilesLongerDistance)
{
   const usmtf::Field    f{"40000NM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithKiloMeters)
{
   const usmtf::Field    f{"40KM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithMeters)
{
   const usmtf::Field    f{"40M"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithCentimeters)
{
   const usmtf::Field    f{"40CM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithFeet)
{
   const usmtf::Field    f{"40FT"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithhectometers)
{
   const usmtf::Field    f{"40HM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithHundredOfFeet)
{
   const usmtf::Field    f{"40HF"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithInches)
{
   const usmtf::Field    f{"40IN"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithKilofeet)
{
   const usmtf::Field    f{"40KF"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithMillimeters)
{
   const usmtf::Field    f{"40MM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithMiles)
{
   const usmtf::Field    f{"40SM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsValidForValidRadiusWithYards)
{
   const usmtf::Field    f{"40YD"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_TRUE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsinValidForinValidRadiusValue)
{
   const usmtf::Field    f{"P899PNM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsinValidForinValidRadiusValueParts)
{
   const usmtf::Field    f{"40.aNM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsinValidForValidRaduisUnitOfMeasure)
{
   const usmtf::Field    f{"40GK"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, IsinValidForInvaidRange)
{
   const usmtf::Field    f{"9999999999NM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.SetErrorMessage(
      "Radius must be of the form <distance><UnitOfMeasure>, where distance is in range [0, 99999], for example 40NM");
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, NoErrorMessageDefaultsToSetRegexExpression)
{
   const usmtf::Field    f{"9999999999NM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("^([0-9]{1,5})(NM|KM|YD|CM|FT|HM|HF|IN|KF|MM|SM|M)$");
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, InvalidForNoRegex)
{
   const usmtf::Field    f{"9999999999NM"};
   usmtf::RegexValidator r{};
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   r.LogErrors(std::cout);
}

TEST(RegexValidatorTests, InvalidForInvalidRegex)
{
   const usmtf::Field    f{"9999999999NM"};
   usmtf::RegexValidator r{};
   r.SetRegexExpression("[");
   r.ParseAndValidate(f);
   EXPECT_FALSE(r.IsValid());
   r.LogErrors(std::cout);
}
