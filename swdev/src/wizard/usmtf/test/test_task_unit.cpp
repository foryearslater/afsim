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

#include "TaskUnit.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class TaskUnitTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("TASKUNIT", usmtf::SetFactory::Construct<usmtf::TaskUnit>);
      const std::string TASKUNIT = mDataService.getResource("TaskUnit.txt").GetNormalizedPath();

      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(TASKUNIT);
      mMessage                   = parser.ReadMessage();
   }

   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// TASKUNIT/63 RS/ICAO:VGZR/00254-00310/DET 1 FOL//
TEST_F(TaskUnitTest, TaskUnitIsValid)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// TASKUNIT/63 RS/NAME:PIZZA/00254-00310/DET 1 FOL//
TEST_F(TaskUnitTest, TaskUnitIsValidForName)
{
   auto& set = mMessage->GetSet(3);
   EXPECT_TRUE(set.IsValid());
   const usmtf::TaskUnit* sptr = sFact->CastIfRegistered<usmtf::TaskUnit>(set);
   EXPECT_EQ(sptr->GetLocation().GetLocation(), "PIZZA");
}

// TASKUNIT/63 RS/LATM:3500N07900W/00254-00310/DET 1 FOL//
TEST_F(TaskUnitTest, TaskUnitIsValidForLaTM)
{
   auto& set = mMessage->GetSet(4);
   EXPECT_TRUE(set.IsValid());
   const usmtf::TaskUnit* sptr = sFact->CastIfRegistered<usmtf::TaskUnit>(set);
   EXPECT_EQ(sptr->GetLocation().GetLocation(), "35:00:00.0n 79:00:00.0w ");
}

// TASKUNIT/63 RS/LATS:251538N0342044E/00254-00310/DET 1 FOL//
TEST_F(TaskUnitTest, TaskUnitIsValidForLaTS)
{
   auto& set = mMessage->GetSet(5);
   EXPECT_TRUE(set.IsValid());
   const usmtf::TaskUnit* sptr = sFact->CastIfRegistered<usmtf::TaskUnit>(set);
   EXPECT_EQ(sptr->GetLocation().GetLocation(), "25:15:38.0n 34:20:44.0e ");
}

// TASKUNIT/63 RS//
TEST_F(TaskUnitTest, TaskUnitIsValidForNoUsefulData)
{
   auto& set = mMessage->GetSet(6);
   EXPECT_TRUE(set.IsValid());
   const usmtf::TaskUnit* sptr = sFact->CastIfRegistered<usmtf::TaskUnit>(set);
   EXPECT_EQ(sptr->GetLocation().GetLocation(), "");
   EXPECT_EQ(sptr->GetComments(), "");
}

// TASKUNIT / 63 RS / ICAO:NOPE / 00254 - 00310 / DET 1 FOL//
TEST_F(TaskUnitTest, TaskUnitIsValidForICAONotFound)
{
   auto& set = mMessage->GetSet(7);
   EXPECT_TRUE(set.IsValid());
   const usmtf::TaskUnit* sptr = sFact->CastIfRegistered<usmtf::TaskUnit>(set);
   EXPECT_EQ(sptr->GetLocation().GetLocation(), "NOPE");
   EXPECT_EQ(sptr->GetComments(), "DET 1 FOL");
}

// TASKUNIT/63 RS/ICAO:VGZR/00254-00310/DET 1 FOL//
TEST_F(TaskUnitTest, CanPullFields)
{
   auto&                  set  = mMessage->GetSet(2);
   const usmtf::TaskUnit* sptr = sFact->CastIfRegistered<usmtf::TaskUnit>(set);
   EXPECT_TRUE(sptr->IsValid());
   auto location = sptr->GetLocation().GetLocation();
   auto comments = sptr->GetComments();
   EXPECT_EQ(location, "23:50:36.0n 90:23:52.0e ");
   EXPECT_EQ(comments, "DET 1 FOL");
}
