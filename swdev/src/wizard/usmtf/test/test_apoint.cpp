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

#include "APoint.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class APointTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("APOINT", usmtf::SetFactory::Construct<usmtf::APoint>);
      const std::string   APOINT = mDataService.getResource("APoint.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(APOINT);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// Same deal APOINT is using all the same well tested Validators. So less testing is needed.

// APOINT/LATM:2037N05943E//
TEST_F(APointTest, APointIsvalidForValidPoint)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// has invalid degree for latitude
// APOINT/LATM:9137N05943E//
TEST_F(APointTest, APointIsInvalidForInvalidPoint)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
}

// APOINT/LATM:2037N05943E//
TEST_F(APointTest, ValidAPointCanPullAirspacePoint)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());
   const usmtf::APoint* sptr = sFact->CastIfRegistered<usmtf::APoint>(set);
   auto                 pt   = sptr->GetAirSpacePoint();
   EXPECT_EQ(pt.GetRawContent(), "2037N05943E");
}

// APOINT/LATM:9137N05943E//
TEST_F(APointTest, InvalidAPointLatLonHasEmptyData)
{
   auto& set = mMessage->GetSet(3);
   EXPECT_FALSE(set.IsValid());
   const usmtf::APoint* sptr = sFact->CastIfRegistered<usmtf::APoint>(set);
   auto                 pt   = sptr->GetAirSpacePoint();
   EXPECT_EQ(pt.GetLatDegree(), "");
   EXPECT_EQ(pt.GetLonDegree(), "");
   EXPECT_EQ(pt.GetLatSeconds(), "");
   EXPECT_EQ(pt.GetLonSeconds(), "");
   EXPECT_EQ(pt.GetLatMinutes(), "");
   EXPECT_EQ(pt.GetLonMinutes(), "");
   EXPECT_EQ(pt.GetLatHemisphere(), "");
   EXPECT_EQ(pt.GetLonHemisphere(), "");
   EXPECT_EQ(pt.GetFormattedLatLon(), "00:00:00.0n 00:00:00.0e ");
}
