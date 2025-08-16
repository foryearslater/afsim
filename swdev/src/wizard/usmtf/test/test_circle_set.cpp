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

#include "Circle.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class CircleTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("CIRCLE", usmtf::SetFactory::Construct<usmtf::Circle>);
      const std::string   CIRCLE = mDataService.getResource("Circle.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(CIRCLE);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// Test LatM
// CIRCLE/LATM:2037N05934E/15NM//
TEST_F(CircleTest, CircleSetIsValidForLatMConent)
{
   auto& set = mMessage->GetSet(2); // circle set latM valid.
   ASSERT_TRUE(set.IsValid());
}

// CIRCLE /LATM:2037N05934EASDFA / 15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatMConentLength)
{
   auto& set = mMessage->GetSet(3); // circle set latM invalid.
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:9137N05934E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatMConentLatDeg)
{
   auto& set = mMessage->GetSet(4); // circle set latM invalid.
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:2071N05934E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatMConentLatMin)
{
   auto& set = mMessage->GetSet(5); // circle set latM invalid.
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:2037E05934E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatMConentLatHem)
{
   auto& set = mMessage->GetSet(6); // circle set latM invalid.
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:2037N18134E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatMConentLonDeg)
{
   auto& set = mMessage->GetSet(7); // circle set latM invalid.
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:2037N05962E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatMConentLonMin)
{
   auto& set = mMessage->GetSet(8); // circle set latM invalid.
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:2037N05934S/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatMConentLonHem)
{
   auto& set = mMessage->GetSet(9); // circle set latM invalid.
   ASSERT_FALSE(set.IsValid());
}


// LatS
// CIRCLE/LATS:203632N0594256E/15NM//
TEST_F(CircleTest, CircleSetIsValidForLatSConent)
{
   auto& set = mMessage->GetSet(10);
   ASSERT_TRUE(set.IsValid());
}

// CIRCLE/LATS:203632N0594256EASDF/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatSConentLength)
{
   auto& set = mMessage->GetSet(11);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATS:913632N0594256E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatSConentLatDeg)
{
   auto& set = mMessage->GetSet(12);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATS:206532N0594256E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatSConentLatMin)
{
   auto& set = mMessage->GetSet(13);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATS:203666N0594256E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatSConentLatSec)
{
   auto& set = mMessage->GetSet(14);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATS:203632W0594256E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatSConentLatHem)
{
   auto& set = mMessage->GetSet(15);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATS:203632N1814256E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatSConentLonDeg)
{
   auto& set = mMessage->GetSet(16);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATS:203632N0596556E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatSConentLonMin)
{
   auto& set = mMessage->GetSet(17);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATS:203632N0594265E/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatSConentLonSec)
{
   auto& set = mMessage->GetSet(18);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATS:203632N0594256S/15NM//
TEST_F(CircleTest, CircleSetIsInValidForLatSConentLonHem)
{
   auto& set = mMessage->GetSet(19);
   ASSERT_FALSE(set.IsValid());
}


// CIRCLE/LATS:203632N0594256E/15NM//
TEST_F(CircleTest, ValidCircleSetPullsCircleCenterLatS)
{
   auto& set = mMessage->GetSet(10);
   ASSERT_TRUE(set.IsValid());
   const usmtf::Circle* sptr = sFact->CastIfRegistered<usmtf::Circle>(set);
   ASSERT_EQ(sptr->GetCenterOfCircle().GetRawContent(), "203632N0594256E");
}

// CIRCLE/LATM:2037N05934E/15NM//
TEST_F(CircleTest, ValidCircleSetPullsCircleCenterLatM)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
   const usmtf::Circle* sptr = sFact->CastIfRegistered<usmtf::Circle>(set);
   ASSERT_EQ(sptr->GetCenterOfCircle().GetRawContent(), "2037N05934E");
}

// CIRCLE/LATM:2037N05934E/100000NM//
TEST_F(CircleTest, CircleInValidForRadiusInvalidLength)
{
   auto& set = mMessage->GetSet(20);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:2037N05934E/100000M//
TEST_F(CircleTest, CircleInValidForRadiusInvalidNumber)
{
   auto& set = mMessage->GetSet(21);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:2037N05934E/100000K//
TEST_F(CircleTest, CircleInValidForRadiusNoDelimeterForUnit)
{
   auto& set = mMessage->GetSet(22);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:2037N05934E/9999NMS//
TEST_F(CircleTest, CircleInValidForRadiusInvalidUnit)
{
   auto& set = mMessage->GetSet(23);
   ASSERT_FALSE(set.IsValid());
}

// CIRCLE/LATM:2037N05934E/999.9M//
TEST_F(CircleTest, CircleValidForRadiusDecimal)
{
   auto& set = mMessage->GetSet(24);
   ASSERT_TRUE(set.IsValid());
}

// CIRCLE/LATM:2037N05934E/999.9M//
TEST_F(CircleTest, CircleValidForRadiusAndPullsRadius)
{
   auto& set = mMessage->GetSet(24);
   EXPECT_TRUE(set.IsValid());
   const usmtf::Circle* sptr = sFact->CastIfRegistered<usmtf::Circle>(set);
   EXPECT_EQ(sptr->GetRadius().GetRawContent(), "999.9M");
   EXPECT_EQ(sptr->GetRadius().GetRadiusValue(), "999.900000");
   EXPECT_EQ(sptr->GetRadius().GetUnitOfMeasure(), "M");
}

// CIRCLE/LATM:2037N05934E/9999NMS//
TEST_F(CircleTest, InvalidCircleCanPullValidFields)
{
   auto& set = mMessage->GetSet(23);
   EXPECT_FALSE(set.IsValid());
   const usmtf::Circle* sptr = sFact->CastIfRegistered<usmtf::Circle>(set);
   EXPECT_EQ(sptr->GetRadius().GetRadiusValue(), "");
   EXPECT_EQ(sptr->GetCenterOfCircle().GetLatDegree(), "20");
}
