// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include <chrono>
#include <iostream>

#include <gtest/gtest.h>

#include "APeriod.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class APeriodTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("APERIOD", usmtf::SetFactory::Construct<usmtf::APeriod>);
      const std::string APERIOD = mDataService.getResource("APeriod.txt").GetNormalizedPath();

      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(APERIOD);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// APERIOD/DISCRETE/141325ZFEB/142359ZFEB//
TEST_F(APeriodTest, APeriodIsValidForAllValidFieldsDiscrete)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// APERIOD/INTERVAL/131325ZNOV2002/132359ZJAN2003/WEEKLY/4WK/
TEST_F(APeriodTest, APeriodIsValidForAllValidFieldsInterval)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_TRUE(set.IsValid());
}

// APERIOD/WRONG/131325ZNOV2002/132359ZJAN2003/WEEKLY/4WK/
TEST_F(APeriodTest, APeriodIsInvalidForWrongAirspaceTimeMode)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_FALSE(set.IsValid());
}

// APERIOD/INTERVAL/131325ZNOV2002222/132359ZJAN2003/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsInvalidForWrongBeginningTimeLength)
{
   auto& set = mMessage->GetSet(5);
   ASSERT_FALSE(set.IsValid());
}

// APERIOD/INTERVAL/331325ZNOV2002/132359ZJAN2003/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsInvalidForWrongBeginningTimeDay)
{
   auto& set = mMessage->GetSet(6);
   ASSERT_FALSE(set.IsValid());
}
// APERIOD/INTERVAL/132425ZNOV2002/132359ZJAN2003/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsInvalidForWrongBeginningTimeHour)
{
   auto& set = mMessage->GetSet(7);
   ASSERT_FALSE(set.IsValid());
}
// APERIOD/INTERVAL/131361ZNOV2002/132359ZJAN2003/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsInvalidForWrongBeginningTimeMinute)
{
   auto& set = mMessage->GetSet(8);
   ASSERT_FALSE(set.IsValid());
}
// APERIOD/INTERVAL/131325JNOV2002/132359ZJAN2003/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsInvalidForWrongBeginningTimeTimeZone)
{
   auto& set = mMessage->GetSet(9);
   ASSERT_FALSE(set.IsValid());
}

// APERIOD/INTERVAL/131325ZBOV2002/132359ZJAN2003/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsInvalidForWrongBeginningTimeMonth)
{
   auto& set = mMessage->GetSet(10);
   ASSERT_FALSE(set.IsValid());
}

// APERIOD/INTERVAL/11325ZNOV10000/132359ZJAN2003/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsInvalidForWrongBeginningTimeYear)
{
   auto& set = mMessage->GetSet(11);
   ASSERT_FALSE(set.IsValid());
}

// APERIOD/INTERVAL/110325ZNOV1000/141325ZBEV/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsInvalidForWrongBeginningTimeMonthNonGroupFormat)
{
   auto& set = mMessage->GetSet(12);
   ASSERT_FALSE(set.IsValid());
}

// Given endTime is the same validation as beginning, except for time qualifier, well focus there.

// valid for TimeQualifier
// APERIOD/INTERVAL/110325ZNOV1000/UFN/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsValidWithTimeQualifier)
{
   auto& set = mMessage->GetSet(13);
   ASSERT_TRUE(set.IsValid());
}

// APERIOD/INTERVAL/110325ZNOV1000/UFP/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodIsInvalidForWrongTimeQualifier)
{
   auto& set = mMessage->GetSet(14);
   ASSERT_FALSE(set.IsValid());
}

// proves no validation for interval frequency or duration for discrete
// APERIOD/DISCRETE/110325ZNOV1000/UFN/MONEY/PLOP//
TEST_F(APeriodTest, APeriodIsValidForAnyIntervalDurationOrFrequencyWhenDiscrete)
{
   auto& set = mMessage->GetSet(15);
   ASSERT_TRUE(set.IsValid());
}

// APERIOD/INTERVAL/110325ZNOV1000/UFN/MONEY/WEEKLY//
TEST_F(APeriodTest, APeriodIsInvalidForWrongFrequency)
{
   auto& set = mMessage->GetSet(16);
   ASSERT_FALSE(set.IsValid());
}

// APERIOD/INTERVAL/110325ZNOV1000/UFN/WEEKLY/PLOP//
TEST_F(APeriodTest, APeriodIsInvalidForWrongDuration)
{
   auto& set = mMessage->GetSet(17);
   ASSERT_FALSE(set.IsValid());
}


// APERIOD/INTERVAL/131325ZNOV2002/132359ZJAN2003/WEEKLY/4WK//
TEST_F(APeriodTest, APeriodValidIntervalCanPullFields)
{
   auto& set = mMessage->GetSet(3);
   EXPECT_TRUE(set.IsValid());
   const usmtf::APeriod* sptr = sFact->CastIfRegistered<usmtf::APeriod>(set);
   auto                  atm  = sptr->GetAirspaceTimeMode();
   auto                  bt   = sptr->GetBeginningTime();
   auto                  et   = sptr->GetEndingTime();
   auto                  ifr  = sptr->GetIntervalFrequency();
   auto                  id   = sptr->GetIntervalDuration();
   EXPECT_EQ(atm.GetTimeMode(), "INTERVAL");
   EXPECT_EQ(bt.GetRawContent(), "131325ZNOV2002");
   EXPECT_EQ(et.GetRawContent(), "132359ZJAN2003");
   EXPECT_EQ(ifr.GetIntervalFrequency(), "WEEKLY");
   EXPECT_EQ(id.GetIntervalDuration(), "4WK");
}

// APERIOD/DISCRETE/141325ZFEB/142359ZFEB//
TEST_F(APeriodTest, APeriodValidDiscreteCanPullFields)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());
   const usmtf::APeriod* sptr = sFact->CastIfRegistered<usmtf::APeriod>(set);
   auto                  atm  = sptr->GetAirspaceTimeMode();
   auto                  bt   = sptr->GetBeginningTime();
   auto                  et   = sptr->GetEndingTime();
   auto                  ifr  = sptr->GetIntervalFrequency();
   auto                  id   = sptr->GetIntervalDuration();
   EXPECT_EQ(atm.GetTimeMode(), "DISCRETE");
   EXPECT_EQ(bt.GetRawContent(), "141325ZFEB");
   EXPECT_EQ(et.GetRawContent(), "142359ZFEB");
   EXPECT_EQ(ifr.GetIntervalFrequency(), "");
   EXPECT_EQ(id.GetContextQuantity(), "");
}

// APERIOD/INTERVAL/110325ZNOV1000/UFN/WEEKLY/PLOP//
TEST_F(APeriodTest, APeriodInvalidCanStillPullRelevantValidFields)
{
   auto& set = mMessage->GetSet(17);
   EXPECT_FALSE(set.IsValid());
   const usmtf::APeriod* sptr = sFact->CastIfRegistered<usmtf::APeriod>(set);
   auto                  atm  = sptr->GetAirspaceTimeMode();
   auto                  bt   = sptr->GetBeginningTime();
   auto                  et   = sptr->GetEndingTime();
   auto                  ifr  = sptr->GetIntervalFrequency();
   auto                  id   = sptr->GetIntervalDuration();
   EXPECT_EQ(atm.GetTimeMode(), "INTERVAL");
   EXPECT_EQ(bt.GetRawContent(), "110325ZNOV1000");
   EXPECT_EQ(et.GetRawContent(), "UFN");
   EXPECT_EQ(ifr.GetIntervalFrequency(), "WEEKLY");
   EXPECT_EQ(id.GetIntervalDuration(), "");
}
