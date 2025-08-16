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

#include "Efflevel.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class EfflevelTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("EFFLEVEL", usmtf::SetFactory::Construct<usmtf::Efflevel>);
      const std::string   EFFLEVEL = mDataService.getResource("Efflevel.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser   = usmtf::USMTF_Parser(EFFLEVEL);
      mMessage                     = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// Valid Cases
// EFFLEVEL/RARA:000AGL-020AGL//
TEST_F(EfflevelTest, EfflevelIsValidForRARA)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// EFFLEVEL/FLFL:FL250-FL290//
TEST_F(EfflevelTest, EfflevelIsValidForFLFL)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_TRUE(set.IsValid());
}

// EFFLEVEL/RAFL:100AGL-FL230
TEST_F(EfflevelTest, EfflevelIsValidForRAFL)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_TRUE(set.IsValid());
}

// EFFLEVEL/BRFL:MSL-FL230
TEST_F(EfflevelTest, EfflevelIsValidForBRFL)
{
   auto& set = mMessage->GetSet(5);
   ASSERT_TRUE(set.IsValid());
}

// EFFLEVEL/BRRA:GL-100AGL
TEST_F(EfflevelTest, EfflevelIsValidForBRRA)
{
   auto& set = mMessage->GetSet(6);
   ASSERT_TRUE(set.IsValid());
}

// Invalid Cases
// Given Theses are composed of each others Validators, only a subset is needed for testing,
// more would be redundant.

// EFFLEVEL/RARA:1000AGL-020AGL//
TEST_F(EfflevelTest, EfflevelIsInvalidForRARAInvalidAltitude)
{
   auto& set = mMessage->GetSet(7);
   ASSERT_FALSE(set.IsValid());
}

// EFFLEVEL/RARA:072SGL-020AGL//
TEST_F(EfflevelTest, EfflevelIsInvalidForRARAInvalidAltitudeUOM)
{
   auto& set = mMessage->GetSet(8);
   ASSERT_FALSE(set.IsValid());
}

// EFFLEVEL/FLFL:BL250-FL290//
TEST_F(EfflevelTest, EfflevelIsInvalidForFLFLInvalidFlightIndicator)
{
   auto& set = mMessage->GetSet(9);
   ASSERT_FALSE(set.IsValid());
}

// EFFLEVEL/FLFL:FL50-FL290//
TEST_F(EfflevelTest, EfflevelIsInvalidForFLFLInvalidAltitude)
{
   auto& set = mMessage->GetSet(10);
   ASSERT_FALSE(set.IsValid());
}

// EFFLEVEL/BRRA:DL-100AGL//
TEST_F(EfflevelTest, EfflevelIsInvalidForFBRRAInvalidBaseReference)
{
   auto& set = mMessage->GetSet(11);
   ASSERT_FALSE(set.IsValid());
}

// EFFLEVEL/RARA:000AGL-020AGL//
TEST_F(EfflevelTest, ValidEfflevelCanPullFields)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());
   const usmtf::Efflevel* sptr = sFact->CastIfRegistered<usmtf::Efflevel>(set);
   auto                   vd   = sptr->GetVerticalDimension();
   EXPECT_EQ(vd.GetRawContent(), "000AGL-020AGL");
}

// EFFLEVEL/BRRA:DL-100AGL//
TEST_F(EfflevelTest, InvalidEfflevelCantPullFields)
{
   auto& set = mMessage->GetSet(11);
   EXPECT_FALSE(set.IsValid());
   const usmtf::Efflevel* sptr = sFact->CastIfRegistered<usmtf::Efflevel>(set);
   auto                   vd   = sptr->GetVerticalDimension();
   EXPECT_EQ(vd.GetMaxAltitude(), "");
}
