// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include <string>

#include <gtest/gtest.h>

#include "TaskCountry.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class TaskCountryTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("TSKCNTRY", usmtf::SetFactory::Construct<usmtf::TaskCountry>);
      const std::string TSKCNTRY = mDataService.getResource("TaskCountry.txt").GetNormalizedPath();

      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(TSKCNTRY);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// TSKCNTRY/PS//
TEST_F(TaskCountryTest, TaskCountryIsValid)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// TSKCNTRY///
TEST_F(TaskCountryTest, TaskCountryIsInValidForMissingCountry)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
}

// TSKCNTRY/PS//
TEST_F(TaskCountryTest, CanPullFields)
{
   auto&                     set  = mMessage->GetSet(2);
   const usmtf::TaskCountry* sptr = sFact->CastIfRegistered<usmtf::TaskCountry>(set);
   EXPECT_TRUE(sptr->IsValid());
   auto country = sptr->GetCountry();
   EXPECT_EQ(country, "PS");
}
