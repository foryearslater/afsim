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
#include "TimePoint.hpp"


TEST(TimePointValidatorTests, IsValidForValidTimePointNoYear)
{
   const usmtf::Field f{"141325ZFEB"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_TRUE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "14");
   EXPECT_EQ(tp.GetMonth(), "FEB");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "Z");
   EXPECT_EQ(tp.GetMinute(), "25");
   EXPECT_EQ(tp.GetHour(), "13");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}

TEST(TimePointValidatorTests, IsValidForValidTimePointYear)
{
   const usmtf::Field f{"141325ZFEB2001"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_TRUE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "14");
   EXPECT_EQ(tp.GetMonth(), "FEB");
   EXPECT_EQ(tp.GetYear(), "2001");
   EXPECT_EQ(tp.GetTimeZone(), "Z");
   EXPECT_EQ(tp.GetMinute(), "25");
   EXPECT_EQ(tp.GetHour(), "13");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}

TEST(TimePointValidatorTests, IsValidForValidTimePointAsQualifier)
{
   const usmtf::Field f{"UFN"};
   usmtf::TimePoint   tp{true};
   tp.ParseAndValidate(f);
   EXPECT_TRUE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "");
   EXPECT_EQ(tp.GetMonth(), "");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "");
   EXPECT_EQ(tp.GetMinute(), "");
   EXPECT_EQ(tp.GetHour(), "");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "UFN");
   tp.LogErrors(std::cout);
}

TEST(TimePointValidatorTests, IsinValidForinValidTimePointAsQualifier)
{
   const usmtf::Field f{"UFP"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_FALSE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "");
   EXPECT_EQ(tp.GetMonth(), "");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "");
   EXPECT_EQ(tp.GetMinute(), "");
   EXPECT_EQ(tp.GetHour(), "");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}


TEST(TimePointValidatorTests, IsInValidForInValidLength)
{
   const usmtf::Field f{"131325ZNOV2002222"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_FALSE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "");
   EXPECT_EQ(tp.GetMonth(), "");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "");
   EXPECT_EQ(tp.GetMinute(), "");
   EXPECT_EQ(tp.GetHour(), "");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}

TEST(TimePointValidatorTests, IsInValidForInValidDay)
{
   const usmtf::Field f{"331325ZNOV2002"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_FALSE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "");
   EXPECT_EQ(tp.GetMonth(), "");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "");
   EXPECT_EQ(tp.GetMinute(), "");
   EXPECT_EQ(tp.GetHour(), "");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}

TEST(TimePointValidatorTests, IsInValidForInValidHour)
{
   const usmtf::Field f{"132425ZNOV2002"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_FALSE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "");
   EXPECT_EQ(tp.GetMonth(), "");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "");
   EXPECT_EQ(tp.GetMinute(), "");
   EXPECT_EQ(tp.GetHour(), "");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}

TEST(TimePointValidatorTests, IsInValidForInValidMinute)
{
   const usmtf::Field f{"131361ZNOV2002"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_FALSE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "");
   EXPECT_EQ(tp.GetMonth(), "");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "");
   EXPECT_EQ(tp.GetMinute(), "");
   EXPECT_EQ(tp.GetHour(), "");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}

TEST(TimePointValidatorTests, IsInValidForInValidTimeZone)
{
   const usmtf::Field f{"131325JNOV2002"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_FALSE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "");
   EXPECT_EQ(tp.GetMonth(), "");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "");
   EXPECT_EQ(tp.GetMinute(), "");
   EXPECT_EQ(tp.GetHour(), "");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}

TEST(TimePointValidatorTests, IsInValidForInValidMonth)
{
   const usmtf::Field f{"131325ZBOV2002"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_FALSE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "");
   EXPECT_EQ(tp.GetMonth(), "");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "");
   EXPECT_EQ(tp.GetMinute(), "");
   EXPECT_EQ(tp.GetHour(), "");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}

TEST(TimePointValidatorTests, IsInValidForInValidYear)
{
   const usmtf::Field f{"11325ZNOV10000"};
   usmtf::TimePoint   tp{};
   tp.ParseAndValidate(f);
   EXPECT_FALSE(tp.IsValid());
   EXPECT_EQ(tp.GetDay(), "");
   EXPECT_EQ(tp.GetMonth(), "");
   EXPECT_EQ(tp.GetYear(), "");
   EXPECT_EQ(tp.GetTimeZone(), "");
   EXPECT_EQ(tp.GetMinute(), "");
   EXPECT_EQ(tp.GetHour(), "");
   EXPECT_EQ(tp.GetStopTimeQualifier(), "");
   tp.LogErrors(std::cout);
}
