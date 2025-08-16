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

#include "AircraftMissionLocation.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class AircraftMissionLocationTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("AMSNLOC", usmtf::SetFactory::Construct<usmtf::AircraftMissionLocation>);
      const std::string   AMSNLOC = mDataService.getResource("AircraftMissionLocation.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser  = usmtf::USMTF_Parser(AMSNLOC);
      mMessage                    = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

// Valid Cases
// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/ELL:105M-200M-240/LATM:3510N07901W/NAME:PT ALFA//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsValid)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/ELL:105M-200M-240/LATM:3510N07901W/NAME:PT ALFA/NAME:Another Location//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsValidMultilocationAllTypes)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_TRUE(set.IsValid());
}

// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/25NM/LATM:3510N07901W/NAME:PT ALFA//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsValidRadiusAreaGeometry)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_TRUE(set.IsValid());
}

// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/SECTIO:058190T45-60NM/LATM:3510N07901W/NAME:PT ALFA//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsValidSectorGeometryAreaGeometry)
{
   auto& set = mMessage->GetSet(5);
   ASSERT_TRUE(set.IsValid());
}

// Invalid Cases
//  AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/1000/1A/ELL:105M-200M-240/LATM:3510N07901W/NAME:PT ALFA//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsInvalidForAltitude)
{
   auto& set = mMessage->GetSet(6);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}

// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/ELL:999999M-200M-240/LATM:3510N07901W/NAME:PT ALFA//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsInvalidForAreaGeometryEllArea)
{
   auto& set = mMessage->GetSet(7);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}

// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/999999NM/LATM:3510N07901W/NAME:PT ALFA//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsInvalidForAreaGeometryRadius)
{
   auto& set = mMessage->GetSet(8);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}

// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/SECTIO:300190T45-60NM/LATM:3510N07901W/NAME:PT ALFA//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsInvalidForAreaGeometrySector)
{
   auto& set = mMessage->GetSet(9);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}

// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/ELL:105M-200M-240/LATM:3510N07901S/NAME:PT
// ALFA/351025N0790125W/351025N6-0790125W4/330-PT ALFA-50/
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsInvalidLocationButValidForFreeText)
{
   auto& set = mMessage->GetSet(10);
   ASSERT_TRUE(set.IsValid());
   const usmtf::AircraftMissionLocation* sptr = sFact->CastIfRegistered<usmtf::AircraftMissionLocation>(set);
   usmtf::FreeText*                      ft   = sptr->GetLocations()[0]->IsMatch<usmtf::FreeText>();
   EXPECT_EQ(ft->GetText(), "3510N07901S");
   set.LogErrors(std::cout);
}

// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/ELL:105M-200M-240/LATM:3510N07901W/NAME:PT
// ALFA/351025N0790125W/911025N6-0790125W4/330-PT ALFA-50//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsInvalidForLocationVerifiedLatLonButValidForFreeText)
{
   auto& set = mMessage->GetSet(11);
   ASSERT_TRUE(set.IsValid());
   const usmtf::AircraftMissionLocation* sptr = sFact->CastIfRegistered<usmtf::AircraftMissionLocation>(set);
   usmtf::FreeText*                      ft   = sptr->GetLocations()[1]->IsMatch<usmtf::FreeText>();
   EXPECT_EQ(ft->GetText(), "PT ALFA");
   set.LogErrors(std::cout);
}

// AMSNLOC/141325ZFEB/142000ZFEB/MISTY ORBIT/210/1A/ELL:105M-200M-240/LATM:3510N07901W/NAME:PT
// ALFA/351025N0790125W/351025N6-0790125W4/360-PT ALFA-50//
TEST_F(AircraftMissionLocationTest, AircraftMissionLocationIsInvalidForLocationRelativeBearingRangeButValidForFreeText)
{
   auto& set = mMessage->GetSet(12);
   ASSERT_TRUE(set.IsValid());
   const usmtf::AircraftMissionLocation* sptr = sFact->CastIfRegistered<usmtf::AircraftMissionLocation>(set);
   usmtf::FreeText*                      ft   = sptr->GetLocations()[2]->IsMatch<usmtf::FreeText>();
   EXPECT_EQ(ft->GetText(), "351025E0790125W");
   set.LogErrors(std::cout);
}
