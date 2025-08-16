// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include <gtest/gtest.h>

#include "Period.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class PeriodTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("PERIOD", usmtf::SetFactory::Construct<usmtf::Period>);
      const std::string   PERIOD = mDataService.getResource("Period.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(PERIOD);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};


// PERIOD/141325Z/142359Z//
TEST_F(PeriodTest, PeriodIsValidStartStopDayTime)
{
   // Is a valid Start/Stop DayTime w/ no conditional STP_Modifier and no USMTF_Qualifier as Stop Field
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// PERIOD/141325Z9/142359Z9//
TEST_F(PeriodTest, PeriodIsValidVerifiedStartStopDayTime)
{
   // Is a valid Start/Stop verified DateTime w/ no conditional STP_Modifier and no USMTF_Qualifier as Stop Field
   auto& set = mMessage->GetSet(3);
   ASSERT_TRUE(set.IsValid());
}

// PERIOD/141325ZA/142359ZA//
TEST_F(PeriodTest, PeriodIsInvalidVerifiedStartStopDayTime)
{
   // Is a invalid Start/Stop DayTime w/ no conditional STP_Modifier and no USMTF_Qualifier as Stop Field
   auto& set = mMessage->GetSet(4);
   ASSERT_FALSE(set.IsValid());
}

// PERIOD/141325Z9FEB/142359Z9FEB//
TEST_F(PeriodTest, PeriodIsValidVerifiedStartStopMonthDateTime)
{
   // Is a valid Start/Stop verified MonthDateTime w/ no conditional STP_Modifier and no USMTF_Qualifier as Stop Field
   auto& set = mMessage->GetSet(5);
   ASSERT_TRUE(set.IsValid());
}

// PERIOD/141325Z9FET/142359Z9FET//
TEST_F(PeriodTest, PeriodIsInvalidVerifiedStartStopMonthDateTime)
{
   // Is a invalid Start/Stop verified MonthDateTime w/ no conditional STP_Modifier and no USMTF_Qualifier as Stop Field
   auto& set = mMessage->GetSet(6);
   ASSERT_FALSE(set.IsValid());
}

// PERIOD/141325ZFEB/NLT/141600ZFEB//
TEST_F(PeriodTest, PeriodHasValidUseOfStopTimeQualifierAndModifier)
{
   // Is a valid Stop Time w/ conditions met to make StopTimeModifier Mandatory and a valid use case check
   auto& set = mMessage->GetSet(3);
   ASSERT_TRUE(set.IsValid());
}

// PERIOD/141325Z/142359Z//
TEST_F(PeriodTest, ValidPeriodHasValidDataViaValidators)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());
   const usmtf::Period* sptr = sFact->CastIfRegistered<usmtf::Period>(set);
   EXPECT_EQ(sptr->GetStartTime().GetDay(), "14");
   EXPECT_EQ(sptr->GetStartTime().GetMonth(), "");
   EXPECT_EQ(sptr->GetStartTime().GetHour(), "13");
   EXPECT_EQ(sptr->GetStartTime().GetMinute(), "25");
   EXPECT_EQ(sptr->GetStartTime().GetYear(), "");
   EXPECT_EQ(sptr->GetStartTime().GetTimeZone(), "Z");
   EXPECT_EQ(sptr->GetStartTime().GetRawContent(), "141325Z");
}
