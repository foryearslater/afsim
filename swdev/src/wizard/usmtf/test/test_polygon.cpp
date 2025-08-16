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

#include "Polygon.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class PolygonTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("POLYGON", usmtf::SetFactory::Construct<usmtf::Polygon>);
      const std::string   POLYGON = mDataService.getResource("Polygon.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser  = usmtf::USMTF_Parser(POLYGON);
      mMessage                    = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// Polygon Needs much less validation as its validation is the same functions that have already been tests thoroughly for Circle.

// POLYGON/LATM:2037N05943E/LATS:203632N0594256E/LATS:203932N0594256E/LATS:203932N0604256E/LATM:2037N05943E//
TEST_F(PolygonTest, PolygonIsValidForAllValidPoints)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// has invalid degree for latitude
// POLYGON/LATM:2037N05943E/LATS:203632N0594256E/LATS:913932N0594256E/LATS:203932N0604256E/LATM:2037N05943E//
TEST_F(PolygonTest, PolygonIsInvalidForSingleInvalidPoint)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
}

// POLYGON/LATM:2037N05943E/LATS:203632N0594256E/LATS:203932N0594256E/LATS:203932N0604256E/LATM:2037N05943E//
TEST_F(PolygonTest, ValidPolygonCanPullPoints)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());
   const usmtf::Polygon* sptr = sFact->CastIfRegistered<usmtf::Polygon>(set);
   auto                  pts  = sptr->GetPoints();
   EXPECT_EQ(pts.size(), 5);
   EXPECT_EQ(pts[0].GetRawContent(), "2037N05943E");
   EXPECT_EQ(pts[1].GetRawContent(), "203632N0594256E");
   EXPECT_EQ(pts[2].GetRawContent(), "203932N0594256E");
   EXPECT_EQ(pts[3].GetRawContent(), "203932N0604256E");
   EXPECT_EQ(pts[4].GetRawContent(), "2037N05943E");
}
