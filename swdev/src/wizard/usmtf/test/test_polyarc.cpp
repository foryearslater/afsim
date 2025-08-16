// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include <gtest/gtest.h>

#include "Polyarc.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"

/********/
#include "UtLLPos.hpp"
/********/


class PolyarcTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("POLYARC", usmtf::SetFactory::Construct<usmtf::Polyarc>);
      const std::string   POLYARC = mDataService.getResource("Polyarc.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser  = usmtf::USMTF_Parser(POLYARC);
      mMessage                    = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};


// POLYARC/LATM:1510N05901E/330T/30NM/160T/LATS:203632N0594256E/LATS:155000N0594815E/LATS:155000N0591343E//
TEST_F(PolyarcTest, PolyarcIsValid)
{
   // is also a test for valid polyarc in general
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// POLYARC/LATM:1510N05901E/380T/30NM/160T/LATS:203632N0594256E/LATS:155000N0594815E/LATS:155000N0591343E//
TEST_F(PolyarcTest, PolyarcIsInvalidBearingTrue)
{
   // check for valid bearing
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
}
