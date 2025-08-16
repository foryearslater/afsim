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

#include "Geoline.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class GeolineTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("GEOLINE", usmtf::SetFactory::Construct<usmtf::Geoline>);
      const std::string   CORRIDOR = mDataService.getResource("Geoline.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser   = usmtf::USMTF_Parser(CORRIDOR);
      mMessage                     = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// GEOLINE/LATS:203632N0594256E/LATM:2045N06000E/LATM:2215N06200E//
TEST_F(GeolineTest, CorridorIsValidForAllValidPoints)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// has invalid degree for latitude
// GEOLINE/LATS:203632N0594256E/LATM:9145N06000E/LATM:2215N06200E//
TEST_F(GeolineTest, CorridorIsInvalidForSingleInvalidPoint)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
}

// GEOLINE/LATS:203632N0594256E/LATM:2045N06000E/LATM:2215N06200E//
TEST_F(GeolineTest, ValidCorridorCanPullPoints)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());
   const usmtf::Geoline* sptr = sFact->CastIfRegistered<usmtf::Geoline>(set);
   auto                  pts  = sptr->GetPoints();
   EXPECT_EQ(pts.size(), 3);
   EXPECT_EQ(pts[0].GetRawContent(), "203632N0594256E");
   EXPECT_EQ(pts[1].GetRawContent(), "2045N06000E");
   EXPECT_EQ(pts[2].GetRawContent(), "2215N06200E");
}
