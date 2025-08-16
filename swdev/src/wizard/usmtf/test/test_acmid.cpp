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


#include <iostream>

#include <gtest/gtest.h>

#include "Acmid.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class AcmidTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("ACMID", usmtf::SetFactory::Construct<usmtf::Acmid>);
      const std::string ACMID = mDataService.getResource("Acmid.txt").GetNormalizedPath();

      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(ACMID);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// ACMID/OTR:RFHHLL/DESIG:C34/CORRIDOR/OTR:SPAA//
TEST_F(AcmidTest, AcmidSetIsValidTypeOfAirSpaceShape)
{
   auto& set = mMessage->GetSet(2); // Airspace Type is Valid
   ASSERT_TRUE(set.IsValid());
}

// ACMID/OTR:RFHHLL/DESIG:C34/TESSERACT/OTR:SPAA//
TEST_F(AcmidTest, AcmidSetIsInValidTypeOfAirSpaceShape)
{
   auto& set = mMessage->GetSet(3); // Airspace Type is InValid
   ASSERT_FALSE(set.IsValid());
}


// ACMID//
TEST_F(AcmidTest, NoACMIDFieldNotValid)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_FALSE(set.IsValid());
}

// ACMID/OTR:RFHHLL/DESIG:C34/CORRIDOR/OTR:SPAA//
TEST_F(AcmidTest, ACMIDFieldPresentIsValid)
{
   auto& set = mMessage->GetSet(5);
   ASSERT_TRUE(set.IsValid());
}
