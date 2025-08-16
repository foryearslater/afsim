// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include <chrono>

#include <gtest/gtest.h>

#include "AerialRefueling.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class AerialRefuelingTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("5REFUEL", usmtf::SetFactory::Construct<usmtf::AerialRefueling>);
      const std::string   REFUEL = mDataService.getResource("AerialRefueling.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(REFUEL);
      mMessage                   = parser.ReadMessage();
   }

   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

/*
 5REFUEL
   /MSNNO   /RECCS       /NO/ACTYPE   /OFLD     /ARCT   /SEQ  /TYP  /ARS
   /0151H   /HAMMER 51   / 2/AC:FA18  / KLB:15.0/141325Z/  A:3/A:JP8/CLD
   /AMC123  /LIFTER 33   / 1/AC:C5A   / KLB:95.0/141500Z/B:1,2/A:JP4/BOM
   //
*/
TEST_F(AerialRefuelingTest, AerialRefuelingIsvalidForProperFields)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

/*
5REFUEL
/MSNNO   /RECCS       /NO/ACTYPE   /OFLD     /ARCT   /SEQ  /TYP  /ARS
/0151H   /HAMMER 51   / 2/AC:FA18  / KLB:15.0/141325Z/  -/-/-
/AMC123  /LIFTER 33   / 1/AC:C5A   / KLB:95.0/141500Z/B:1,2/A:JP4/BOM
//
*/
TEST_F(AerialRefuelingTest, AerialRefuelingIsvalidForProperFieldsWithNull)
{
   auto& set = mMessage->GetSet(7);
   ASSERT_TRUE(set.IsValid());
}

/*
 5REFUEL
   /MSNNO   /RECCS       /NO/ACTYPE   /OFLD     /ARCT   /SEQ  /TYP  /ARS
   /0151H   /HAMMER 51   / 2/AC:FA18  / KLB:15.0/141325Z/  A:3/A:JP8/CLD
   /AMC123  /LIFTER 33   / 1/AC:C5A   / KLB:95.0/141500Z/B:1,2/A:JP4/BOM
   //
*/
TEST_F(AerialRefuelingTest, AerialRefuelingCanPullValues)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
   const usmtf::AerialRefueling*              sptr         = sFact->CastIfRegistered<usmtf::AerialRefueling>(set);
   usmtf::AerialRefueling::RefuelInstructions instructions = sptr->GetRefuelInstructions();
   usmtf::AerialRefueling::RefuelInstruction  instruction  = instructions[0];
   EXPECT_EQ(instruction.mMissionNumber.GetText(), "0151H");
   EXPECT_EQ(instruction.mRecieverCallsign.GetText(), "HAMMER 51");
   EXPECT_EQ(instruction.mAllocatedAircraft, 2);
   EXPECT_EQ(instruction.mTypeOfAircraft.GetText(), "FA18");
   EXPECT_EQ(instruction.mFuelLoad.GetFuelAmount(), "15.0");
   EXPECT_EQ(instruction.mControlTime.GetText(), "141325Z");
   EXPECT_EQ(instruction.mCellSequence.GetText(), "3");
   EXPECT_EQ(instruction.mRefuelType.GetType(), "JP8");
   EXPECT_EQ(instruction.mRefuelSystem.GetSystem(), "CLD");
}
//
////Given most field are free text, there isnt much to test on validity
//

/* Bad number of aircraft
5REFUEL
/MSNNO   /RECCS       /NO/ACTYPE   /OFLD     /ARCT   /SEQ  /TYP  /ARS
/0151H   /HAMMER 51   / 100/AC:FA18  / KLB:15.0/141325Z/  A:3/A:JP8/CLD
/AMC123  /LIFTER 33   / 1/AC:C5A   / KLB:95.0/141500Z/B:1,2/A:JP4/BOM
//
*/
TEST_F(AerialRefuelingTest, BadNumberOfAircraft)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}

/* Bad fuel load
5REFUEL
/MSNNO   /RECCS       /NO/ACTYPE   /OFLD     /ARCT   /SEQ  /TYP  /ARS
/0151H   /HAMMER 51   / 100/AC:FA18  / KLB:15000.0/141325Z/  A:3/A:JP8/CLD
/AMC123  /LIFTER 33   / 1/AC:C5A   / KLB:95.0/141500Z/B:1,2/A:JP4/BOM
//
*/
TEST_F(AerialRefuelingTest, BadFuelLoad)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}

/* Bad fuel type
5REFUEL
/MSNNO   /RECCS       /NO/ACTYPE   /OFLD     /ARCT   /SEQ  /TYP  /ARS
/0151H   /HAMMER 51   / 2/AC:FA18  / KLB:15.0/141325Z/  A:3/A:NotIT/CLD
/AMC123  /LIFTER 33   / 1/AC:C5A   / KLB:95.0/141500Z/B:1,2/A:JP4/BOM
//
*/
TEST_F(AerialRefuelingTest, BadFuelType)
{
   auto& set = mMessage->GetSet(5);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}

/* Bad fuel system
5REFUEL
/MSNNO   /RECCS       /NO/ACTYPE   /OFLD     /ARCT   /SEQ  /TYP  /ARS
/0151H   /HAMMER 51   / 2/AC:FA18  / KLB:15.0/141325Z/  A:3/A:JP8/NotIT
/AMC123  /LIFTER 33   / 1/AC:C5A   / KLB:95.0/141500Z/B:1,2/A:JP4/BOM
//
*/
TEST_F(AerialRefuelingTest, BadFuelSystem)
{
   auto& set = mMessage->GetSet(6);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}
