// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include <gtest/gtest.h>

#include "Ampn.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class AmpnTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("AMPN", usmtf::SetFactory::Construct<usmtf::Ampn>);
      const std::string AMPN = mDataService.getResource("Ampn.txt").GetNormalizedPath();

      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(AMPN);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// AmpnTest is free text.

// AMPN/I AM AMPLIFYING//
TEST_F(AmpnTest, ValidAMPNPullsAmpText)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());
   const usmtf::Ampn* sptr = sFact->CastIfRegistered<usmtf::Ampn>(set);
   auto               at   = sptr->GetAmplificationText();

   EXPECT_EQ(at, "I AM AMPLIFYING");
}
