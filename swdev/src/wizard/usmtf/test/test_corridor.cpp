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

#include "Corridor.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class CorridorTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("CORRIDOR", usmtf::SetFactory::Construct<usmtf::Corridor>);
      const std::string   CORRIDOR = mDataService.getResource("Corridor.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser   = usmtf::USMTF_Parser(CORRIDOR);
      mMessage                     = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// Same deal Corridor is using all the same well tested Validators. So less testing is needed.

// CORRIDOR/5NM/LATS:203632N0594256E/LATM:2045N06000E/LATM:2215N06200E//
TEST_F(CorridorTest, CorridorIsValidForAllValidPoints)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// has invalid degree for latitude
// CORRIDOR/.0001NM/LATS:203632N0594256E/LATM:9145N06000E/LATM:2215N06200E//
TEST_F(CorridorTest, CorridorIsInvalidForSingleInvalidPoint)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
}

// has invalid width
// CORRIDOR/5NM/LATS:203632N0594256E/LATM:2045N06000E/LATM:2215N06200E//
TEST_F(CorridorTest, CorridorIsInvalidForInvalidWidth)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_FALSE(set.IsValid());
}

// CORRIDOR/5NM/LATS:203632N0594256E/LATM:2045N06000E/LATM:2215N06200E//
TEST_F(CorridorTest, ValidCorridorCanPullPoints)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());
   const usmtf::Corridor* sptr = sFact->CastIfRegistered<usmtf::Corridor>(set);
   auto                   pts  = sptr->GetPoints();
   EXPECT_EQ(pts.size(), 3);
   EXPECT_EQ(pts[0].GetRawContent(), "203632N0594256E");
   EXPECT_EQ(pts[1].GetRawContent(), "2045N06000E");
   EXPECT_EQ(pts[2].GetRawContent(), "2215N06200E");
}


// has invalid width
// CORRIDOR/5NM/LATS:203632N0594256E/LATM:2045N06000E/LATM:2215N06200E//
TEST_F(CorridorTest, InvalidCorridorWidthPullsEmptyWidth)
{
   auto& set = mMessage->GetSet(4);
   EXPECT_FALSE(set.IsValid());
   const usmtf::Corridor* sptr  = sFact->CastIfRegistered<usmtf::Corridor>(set);
   auto                   width = sptr->GetWidth();
   EXPECT_EQ(width.GetRadiusValue(), "");
}
