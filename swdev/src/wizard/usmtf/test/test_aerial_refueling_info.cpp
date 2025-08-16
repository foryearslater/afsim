// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <string>
#include <tuple>

#include <gtest/gtest.h>

#include "AerialRefuelingInfo.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"

class AerialRefuelingInfoParameterizedFixtureTests : public ::testing::TestWithParam<int>
{
public:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("ARINFO", usmtf::SetFactory::Construct<usmtf::AerialRefuelingInfo>);
      const std::string   ARINFO = mDataService.getResource("AerialRefuelingInfo.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(ARINFO);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

/*
ARINFO/EXXON 11/AR123HA/B:37700/NAME:BUICK/170/ARCT:141325Z
         /NDAR:141400ZFEB/KLBS:15.0/PFREQ:243.0/SFREQ:121.5/EN11/ACTYP:KC135R
         /BOM/4/TNKRS:1,3/16-72/4-3-1//
*/
TEST_F(AerialRefuelingInfoParameterizedFixtureTests, PassesForValidAerialRefuelingInfo)
{
   auto&                             set  = mMessage->GetSet(2);
   const usmtf::AerialRefuelingInfo* sptr = sFact->CastIfRegistered<usmtf::AerialRefuelingInfo>(set);
   EXPECT_TRUE(sptr->IsValid());
   EXPECT_EQ(sptr->GetCallsign().GetText(), "EXXON 11");
   EXPECT_EQ(sptr->GetMissionNumber().GetText(), "AR123HA");
   EXPECT_EQ(sptr->GetIffSifMode().GetText(), "37700");
   usmtf::FreeText* point = sptr->GetControlPoint().IsMatch<usmtf::FreeText>();
   EXPECT_EQ(point->GetText(), "BUICK");
   EXPECT_EQ(sptr->GetAltitude().GetVerticalDistance(), "170.000000");
   EXPECT_EQ(sptr->GetRefuelTime().GetText(), "141325Z");
   EXPECT_EQ(sptr->GetRefuelEndTime().GetText(), "141400ZFEB");
   EXPECT_EQ(sptr->GetOffloadFuel().GetFuelAmount(), "15.0");
   EXPECT_EQ(sptr->GetPrimaryFrequency().IsMatch<usmtf::Frequency>()->GetFrequency(), "243.0");
   EXPECT_EQ(sptr->GetSecondaryFrequency().IsMatch<usmtf::Frequency>()->GetFrequency(), "121.5");
   EXPECT_EQ(sptr->GetLink16Callsign().GetText(), "EN11");
   EXPECT_EQ(sptr->GetAircraftType().GetText(), "KC135R");
   EXPECT_EQ(sptr->GetRefuelSystem().GetSystem(), "BOM");
   EXPECT_EQ(sptr->GetNumberOfAircraftInCell(), "4");
   EXPECT_EQ(sptr->GetCellSequence().GetText(), "1,3");
   EXPECT_EQ(sptr->GetTacanChannel().GetRawContent(), "16-72");
   EXPECT_EQ(sptr->GetBeacon().GetRawContent(), "4-3-1");
}
/*
ARINFO/EXXON 11/AR123HA/B:37700/NAME:BUICK/170/ARCT:141325Z
         /-/KLBS:15.0/PFREQ:243.0/-/-/-
   /-/-/-/-/-//
*/
TEST_F(AerialRefuelingInfoParameterizedFixtureTests, PassesForValidAerialRefuelingInfoWithNullFields)
{
   auto&                             set  = mMessage->GetSet(9);
   const usmtf::AerialRefuelingInfo* sptr = sFact->CastIfRegistered<usmtf::AerialRefuelingInfo>(set);
   EXPECT_TRUE(sptr->IsValid());
   EXPECT_EQ(sptr->GetCallsign().GetText(), "EXXON 11");
   EXPECT_EQ(sptr->GetMissionNumber().GetText(), "AR123HA");
   EXPECT_EQ(sptr->GetIffSifMode().GetText(), "37700");
   usmtf::FreeText* point = sptr->GetControlPoint().IsMatch<usmtf::FreeText>();
   EXPECT_EQ(point->GetText(), "BUICK");
   EXPECT_EQ(sptr->GetAltitude().GetVerticalDistance(), "170.000000");
   EXPECT_EQ(sptr->GetRefuelTime().GetText(), "141325Z");
   EXPECT_EQ(sptr->GetRefuelEndTime().GetText(), "-");
   EXPECT_EQ(sptr->GetOffloadFuel().GetFuelAmount(), "15.0");
   EXPECT_EQ(sptr->GetPrimaryFrequency().IsMatch<usmtf::Frequency>()->GetFrequency(), "243.0");
   EXPECT_EQ(sptr->GetSecondaryFrequency().IsMatch<usmtf::FreeText>()->GetText(), "-");
   EXPECT_EQ(sptr->GetLink16Callsign().GetText(), "-");
   EXPECT_EQ(sptr->GetAircraftType().GetText(), "-");
   EXPECT_EQ(sptr->GetRefuelSystem().GetSystem(), "");
   EXPECT_EQ(sptr->GetNumberOfAircraftInCell(), "0");
   EXPECT_EQ(sptr->GetCellSequence().GetText(), "-");
   EXPECT_EQ(sptr->GetTacanChannel().GetRawContent(), "-");
   EXPECT_EQ(sptr->GetBeacon().GetRawContent(), "-");
}


INSTANTIATE_TEST_CASE_P(AerialRefuelingInfoInValidTests,
                        AerialRefuelingInfoParameterizedFixtureTests,
                        ::testing::Values(3, /*ARINFO/EXXON 11/AR123HA/B:37700/NAME:BUICK/1000/ARCT:141325Z
                                             /NDAR:141400ZFEB/KLBS:15.0/PFREQ:243.0/SFREQ:121.5/EN11/ACTYP:KC135R
                                             /BOM/4/TNKRS:1,3/16-72/4-3-1// Bad Altitude*/
                                          4, /*
                                            ARINFO/EXXON 11/AR123HA/B:37700/NAME:BUICK/170/ARCT:141325Z
                                             /NDAR:141400ZFEB/KLBS:10000.0/PFREQ:243.0/SFREQ:121.5/EN11/ACTYP:KC135R
                                             /BOM/4/TNKRS:1,3/16-72/4-3-1// BadFuelLoad
                                            */
                                          5, /*
                                            ARINFO/EXXON 11/AR123HA/B:37700/NAME:BUICK/170/ARCT:141325Z
                                             /NDAR:141400ZFEB/KLBS:15.0/PFREQ:243.0/SFREQ:121.5/EN11/ACTYP:KC135R
                                             /NOPE/4/TNKRS:1,3/16-72/4-3-1// Bad Refuel System
                                            */
                                          6, /*
                                            ARINFO/EXXON 11/AR123HA/B:37700/NAME:BUICK/170/ARCT:141325Z
                                             /NDAR:141400ZFEB/KLBS:15.0/PFREQ:243.0/SFREQ:121.5/EN11/ACTYP:KC135R
                                             /BOM/2000/TNKRS:1,3/16-72/4-3-1// Bad Number of aircraft
                                            */
                                          7, /*
                                             ARINFO/EXXON 11/AR123HA/B:37700/NAME:BUICK/170/ARCT:141325Z
                                              /NDAR:141400ZFEB/KLBS:15.0/PFREQ:243.0/SFREQ:121.5/EN11/ACTYP:KC135R
                                              /BOM/4/TNKRS:1,3/16-1000/4-3-1// Bad Tacan Channel
                                             */
                                          8  /*
                                             ARINFO/EXXON 11/AR123HA/B:37700/NAME:BUICK/170/ARCT:141325Z
                                               /NDAR:141400ZFEB/KLBS:15.0/PFREQ:243.0/SFREQ:121.5/EN11/ACTYP:KC135R
                                               /BOM/4/TNKRS:1,3/16-72/4-7-1// Bad Beacon
                                              */
                                          ), );

TEST_P(AerialRefuelingInfoParameterizedFixtureTests, InvalidCases)
{
   auto&                             set  = mMessage->GetSet(GetParam());
   const usmtf::AerialRefuelingInfo* sptr = sFact->CastIfRegistered<usmtf::AerialRefuelingInfo>(set);
   EXPECT_FALSE(sptr->IsValid());
   sptr->LogErrors(std::cout);
}
