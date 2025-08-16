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

#include "Radarc.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class RadarcTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("RADARC", usmtf::SetFactory::Construct<usmtf::Radarc>);
      const std::string   RADARC = mDataService.getResource("Radarc.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(RADARC);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// RADARC/1510N05901E/270T/030T/25NM/75NM//
TEST_F(RadarcTest, RadarcIsValid)
{
   // is also a test for valid polyarc in general
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}
