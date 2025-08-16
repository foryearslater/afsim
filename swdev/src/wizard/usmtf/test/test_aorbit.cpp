// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include <chrono>
#include <string>

#include <gtest/gtest.h>

#include "AOrbit.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class AOrbitTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("AORBIT", usmtf::SetFactory::Construct<usmtf::AOrbit>);
      const std::string AORBIT = mDataService.getResource("AOrbit.txt").GetNormalizedPath();

      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(AORBIT);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// Same deal AOrbit is using all the same well tested Validators. So less testing is needed.

// AORBIT/LATM:2037N05943E/LATS:203632N0594256E/15NM/C//
TEST_F(AOrbitTest, AOrbitIsValidForValidFields)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// AORBIT/LATM:9137N05943E/LATS:203632N0594256E/15NM/C//
TEST_F(AOrbitTest, AOrbitIsInvalidForFP)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
}

// AORBIT/LATM:2037N05943E/LATS:203632N1814256E/15NM/C//
TEST_F(AOrbitTest, AOrbitIsInvalidForSP)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_FALSE(set.IsValid());
}

// AORBIT/LATM:2037N05943E/LATS:203632N0594256E/100000M/C//
TEST_F(AOrbitTest, AOrbitIsInvalidForWidth)
{
   auto& set = mMessage->GetSet(5);
   ASSERT_FALSE(set.IsValid());
}

// AORBIT/LATM:2037N05943E/LATS:203632N0594256E/15NM/F//
TEST_F(AOrbitTest, AOrbitIsInvalidForOrbitAlignment)
{
   auto& set = mMessage->GetSet(6);
   ASSERT_FALSE(set.IsValid());
}


// AORBIT/LATM:2037N05943E/LATS:203632N0594256E/15NM/C//
TEST_F(AOrbitTest, ValidAOrbitCanPullFields)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());
   const usmtf::AOrbit* sptr = sFact->CastIfRegistered<usmtf::AOrbit>(set);
   const usmtf::LatLon& fp   = sptr->GetFirstOrbitPoint();
   const usmtf::LatLon& sp   = sptr->GetSecondOrbitPoint();
   const usmtf::Radius& w    = sptr->GetWidth();
   EXPECT_EQ(fp.GetRawContent(), "2037N05943E");
   EXPECT_EQ(sp.GetRawContent(), "203632N0594256E");
   EXPECT_EQ(w.GetRawContent(), "15NM");
}


// AORBIT/LATM:2037N05943E/LATS:203632N0594256E/100000M/C//
TEST_F(AOrbitTest, InvalidAOrbitCanStillPullValidFields)
{
   auto&                set  = mMessage->GetSet(5);
   const usmtf::AOrbit* sptr = sFact->CastIfRegistered<usmtf::AOrbit>(set);
   EXPECT_FALSE(sptr->IsValid());
   auto fp = sptr->GetFirstOrbitPoint();
   auto sp = sptr->GetSecondOrbitPoint();
   auto w  = sptr->GetWidth();
   EXPECT_EQ(fp.GetRawContent(), "2037N05943E");
   EXPECT_EQ(sp.GetRawContent(), "203632N0594256E");
   EXPECT_EQ(w.GetRawContent(), "100000M");
   EXPECT_EQ(w.GetRadiusValue(), "");
}

// AORBIT/LATM:2037N05943E//
TEST_F(AOrbitTest, InvalidAOrbitForMissingFieldHasDefaultValidatorFieldAndNullSubField)
{
   auto&                set  = mMessage->GetSet(7);
   const usmtf::AOrbit* sptr = sFact->CastIfRegistered<usmtf::AOrbit>(set);
   EXPECT_FALSE(sptr->IsValid());
   auto fp = sptr->GetFirstOrbitPoint();
   auto sp = sptr->GetSecondOrbitPoint();
   auto w  = sptr->GetWidth();
   EXPECT_EQ(fp.GetLatDegree(), "20");
   EXPECT_EQ(sp.GetLatDegree(), "");
   EXPECT_EQ(w.GetRadiusValue(), "");
}
