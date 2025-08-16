// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include <string>

#include <gtest/gtest.h>

#include "AircraftMission.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class AircraftMissionTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("MSNACFT", usmtf::SetFactory::Construct<usmtf::AircraftMission>);
      const std::string MSNACFT = mDataService.getResource("AircraftMission.txt").GetNormalizedPath();

      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(MSNACFT);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// MSNACFT/2/ACTYP:FA18/HAMMER 53/2HARM/2S2WG/HR53/30/00201/B:20153/00202/B:30153//
TEST_F(AircraftMissionTest, AircraftMissionIsValid)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// MSNACFT/NOPE/ACTYP:FA18/HAMMER 53/2HARM/2S2WG/HR53/30/00201/B:20153/00202/B:30153//
TEST_F(AircraftMissionTest, AircraftMissionIsInvalidForNANNumberOfAircraft)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
}

// MSNACFT/NOPE/ACTYP:FA18/HAMMER 53/2HARM/2S2WG/HR53/30/00201/B:20153/00202/B:30153//
TEST_F(AircraftMissionTest, AircraftMissionIsInvalidForMissingType)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_FALSE(set.IsValid());
}

// MSNACFT/2/ACTYP:FA18//
TEST_F(AircraftMissionTest, AircraftMissionIsInvalidForMissingCallsign)
{
   auto& set = mMessage->GetSet(5);
   ASSERT_FALSE(set.IsValid());
}

// MSNACFT/2/ACTYP:FA18/HAMMER 53/2HARM/2S2WG/HR53/30/00201/B:20153/00202/B:30153///
TEST_F(AircraftMissionTest, CanPullFields)
{
   auto&                         set  = mMessage->GetSet(2);
   const usmtf::AircraftMission* sptr = sFact->CastIfRegistered<usmtf::AircraftMission>(set);
   EXPECT_TRUE(sptr->IsValid());
   EXPECT_EQ(sptr->GetNumberOfAircraft(), 2);
   EXPECT_EQ(sptr->GetAircraftType(), "FA18");
   EXPECT_EQ(sptr->GetAircraftCallsign(), "HAMMER 53");
}
