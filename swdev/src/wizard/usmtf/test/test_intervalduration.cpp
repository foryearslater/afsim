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
#include "IntervalDuration.hpp"

// One Timepoint is enough, well tested base class.
TEST(IntervalDurationValidatorTests, IsValidForValidTimePoint)
{
   const usmtf::Field      f{"141325ZFEB2002"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_TRUE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "14");
   EXPECT_EQ(id.GetMonth(), "FEB");
   EXPECT_EQ(id.GetYear(), "2002");
   EXPECT_EQ(id.GetTimeZone(), "Z");
   EXPECT_EQ(id.GetMinute(), "25");
   EXPECT_EQ(id.GetHour(), "13");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsValidForValidIntervalDurationtimePoint)
{
   const usmtf::Field      f{"141325ZFEB2002"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_TRUE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "14");
   EXPECT_EQ(id.GetMonth(), "FEB");
   EXPECT_EQ(id.GetYear(), "2002");
   EXPECT_EQ(id.GetTimeZone(), "Z");
   EXPECT_EQ(id.GetMinute(), "25");
   EXPECT_EQ(id.GetHour(), "13");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "");
   EXPECT_EQ(id.GetUOM(), "");
   EXPECT_EQ(id.GetContextQuantity(), "");
   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsValidForValidIntervalDurationUFN)
{
   const usmtf::Field      f{"UFN"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_TRUE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "");
   EXPECT_EQ(id.GetMonth(), "");
   EXPECT_EQ(id.GetYear(), "");
   EXPECT_EQ(id.GetTimeZone(), "");
   EXPECT_EQ(id.GetMinute(), "");
   EXPECT_EQ(id.GetHour(), "");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "UFN");
   EXPECT_EQ(id.GetUOM(), "");
   EXPECT_EQ(id.GetContextQuantity(), "");

   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsValidForValidIntervalDurationWK)
{
   const usmtf::Field      f{"4WK"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_TRUE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "");
   EXPECT_EQ(id.GetMonth(), "");
   EXPECT_EQ(id.GetYear(), "");
   EXPECT_EQ(id.GetTimeZone(), "");
   EXPECT_EQ(id.GetMinute(), "");
   EXPECT_EQ(id.GetHour(), "");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "4WK");
   EXPECT_EQ(id.GetUOM(), "WK");
   EXPECT_EQ(id.GetContextQuantity(), "4");

   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsValidForValidIntervalDurationBIWK)
{
   const usmtf::Field      f{"88BIWK"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_TRUE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "");
   EXPECT_EQ(id.GetMonth(), "");
   EXPECT_EQ(id.GetYear(), "");
   EXPECT_EQ(id.GetTimeZone(), "");
   EXPECT_EQ(id.GetMinute(), "");
   EXPECT_EQ(id.GetHour(), "");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "88BIWK");
   EXPECT_EQ(id.GetUOM(), "BIWK");
   EXPECT_EQ(id.GetContextQuantity(), "88");

   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsValidForValidIntervalDurationDAY)
{
   const usmtf::Field      f{"122DAY"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_TRUE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "");
   EXPECT_EQ(id.GetMonth(), "");
   EXPECT_EQ(id.GetYear(), "");
   EXPECT_EQ(id.GetTimeZone(), "");
   EXPECT_EQ(id.GetMinute(), "");
   EXPECT_EQ(id.GetHour(), "");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "122DAY");
   EXPECT_EQ(id.GetUOM(), "DAY");
   EXPECT_EQ(id.GetContextQuantity(), "122");

   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsValidForValidIntervalDurationMONTH)
{
   const usmtf::Field      f{"55MON"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_TRUE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "");
   EXPECT_EQ(id.GetMonth(), "");
   EXPECT_EQ(id.GetYear(), "");
   EXPECT_EQ(id.GetTimeZone(), "");
   EXPECT_EQ(id.GetMinute(), "");
   EXPECT_EQ(id.GetHour(), "");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "55MON");
   EXPECT_EQ(id.GetUOM(), "MON");
   EXPECT_EQ(id.GetContextQuantity(), "55");

   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsValidForValidIntervalDurationYR)
{
   const usmtf::Field      f{"6YR"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_TRUE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "");
   EXPECT_EQ(id.GetMonth(), "");
   EXPECT_EQ(id.GetYear(), "");
   EXPECT_EQ(id.GetTimeZone(), "");
   EXPECT_EQ(id.GetMinute(), "");
   EXPECT_EQ(id.GetHour(), "");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "6YR");
   EXPECT_EQ(id.GetUOM(), "YR");
   EXPECT_EQ(id.GetContextQuantity(), "6");

   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsInValidForInvalidLength)
{
   const usmtf::Field      f{"TOMANYCHARSINME"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_FALSE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "");
   EXPECT_EQ(id.GetMonth(), "");
   EXPECT_EQ(id.GetYear(), "");
   EXPECT_EQ(id.GetTimeZone(), "");
   EXPECT_EQ(id.GetMinute(), "");
   EXPECT_EQ(id.GetHour(), "");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "");
   EXPECT_EQ(id.GetUOM(), "");
   EXPECT_EQ(id.GetContextQuantity(), "");

   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsInValidForInvalidContextQuantity)
{
   const usmtf::Field      f{"1000WK"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_FALSE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "");
   EXPECT_EQ(id.GetMonth(), "");
   EXPECT_EQ(id.GetYear(), "");
   EXPECT_EQ(id.GetTimeZone(), "");
   EXPECT_EQ(id.GetMinute(), "");
   EXPECT_EQ(id.GetHour(), "");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "");
   EXPECT_EQ(id.GetUOM(), "");
   EXPECT_EQ(id.GetContextQuantity(), "");

   id.LogErrors(std::cout);
}

TEST(IntervalDurationValidatorTests, IsInValidForInvalidUOM)
{
   const usmtf::Field      f{"1WEEK"};
   usmtf::IntervalDuration id{};
   id.ParseAndValidate(f);
   EXPECT_FALSE(id.IsValid());
   EXPECT_EQ(id.GetDay(), "");
   EXPECT_EQ(id.GetMonth(), "");
   EXPECT_EQ(id.GetYear(), "");
   EXPECT_EQ(id.GetTimeZone(), "");
   EXPECT_EQ(id.GetMinute(), "");
   EXPECT_EQ(id.GetHour(), "");
   EXPECT_EQ(id.GetStopTimeQualifier(), "");
   EXPECT_EQ(id.GetIntervalDuration(), "");
   EXPECT_EQ(id.GetUOM(), "");
   EXPECT_EQ(id.GetContextQuantity(), "");

   id.LogErrors(std::cout);
}
