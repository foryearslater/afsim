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

#include "RefuelingTasking.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"

class RefuelingTaskingParameterizedFixtureTestsValid :
   public ::testing::TestWithParam<
      std::tuple<int, std::string, std::string, std::string, std::string, std::string, std::string, std::string>>
{
public:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("REFTSK", usmtf::SetFactory::Construct<usmtf::RefuelingTasking>);
      const std::string   REFTSK = mDataService.getResource("RefuelingTasking.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(REFTSK);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

INSTANTIATE_TEST_CASE_P(
   RefuelingTaskingValidTests,
   RefuelingTaskingParameterizedFixtureTestsValid,
   ::testing::Values(std::make_tuple(2, "BOM", "25.5", "5.1", "TAD01", "TAD02", "16-72", "4-3-1"), // REFTSK/BOM/KLBS:25.5/KLBS:5.1/PDESIG:TAD01/SDESIG:TAD02/16-72/4-3-1//
                     std::make_tuple(3, "BOM", "25.5", "5.1", "342.5", "91.5", "16-72", "4-3-1") // REFTSK/BOM/KLBS:25.5/KLBS:5.1/PDESIG:342.5/S91.5/16-72/4-3-1//
                     ), );

TEST_P(RefuelingTaskingParameterizedFixtureTestsValid, PassesForValidRefuelingTasking)
{
   int                            setNumber       = std::get<0>(GetParam());
   std::string                    refuelingSystem = std::get<1>(GetParam());
   std::string                    offloadFuel     = std::get<2>(GetParam());
   std::string                    contOffloadFuel = std::get<3>(GetParam());
   std::string                    primFreq        = std::get<4>(GetParam());
   std::string                    secFreq         = std::get<5>(GetParam());
   std::string                    tacan           = std::get<6>(GetParam());
   std::string                    beacon          = std::get<7>(GetParam());
   auto&                          set             = mMessage->GetSet(setNumber);
   const usmtf::RefuelingTasking* sptr            = sFact->CastIfRegistered<usmtf::RefuelingTasking>(set);
   EXPECT_TRUE(sptr->IsValid());
   EXPECT_EQ(sptr->GetRefuelingSystem().GetSystem(), refuelingSystem);
   EXPECT_EQ(sptr->GetOffloadFuel().GetFuelAmount(), offloadFuel);
   EXPECT_EQ(sptr->GetContingencyOffloadFuel().GetFuelAmount(), contOffloadFuel);
   if (setNumber == 3)
   {
      usmtf::Frequency* pFreq = sptr->GetPrimaryFrequency().IsMatch<usmtf::Frequency>();
      usmtf::Frequency* sFreq = sptr->GetSecondaryFrequency().IsMatch<usmtf::Frequency>();
      EXPECT_EQ(pFreq->GetFrequency(), primFreq);
      EXPECT_EQ(sFreq->GetFrequency(), secFreq);
   }
   else
   {
      usmtf::FreeText* pFreq = sptr->GetPrimaryFrequency().IsMatch<usmtf::FreeText>();
      usmtf::FreeText* sFreq = sptr->GetSecondaryFrequency().IsMatch<usmtf::FreeText>();
      EXPECT_EQ(pFreq->GetText(), primFreq);
      EXPECT_EQ(sFreq->GetText(), secFreq);
   }
   EXPECT_EQ(sptr->GetTacanChannel().GetFieldContent(), tacan);
   EXPECT_EQ(sptr->GetBeacon().GetFieldContent(), beacon);
}

// Invalid cases
class RefuelingTaskingParameterizedFixtureTestsInValid : public ::testing::TestWithParam<int>
{
public:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("REFTSK", usmtf::SetFactory::Construct<usmtf::RefuelingTasking>);
      const std::string   REFTSK = mDataService.getResource("RefuelingTasking.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(REFTSK);
      mMessage                   = parser.ReadMessage();
   }
   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

INSTANTIATE_TEST_CASE_P(
   RefuelingTaskingInValidTests,
   RefuelingTaskingParameterizedFixtureTestsInValid,
   ::testing::Values(4, // REFTSK/NOPE/KLBS:25.5/KLBS:5.1/PDESIG:TAD01/SDESIG:TAD02/16-72/4-3-1// Bad Refuel System
                     5, // REFTSK/BOM/KLBS:10000000/KLBS:5.1/PDESIG:TAD01/SDESIG:TAD02/16-72/4-3-1// Bad Fuel Load
                     6, // REFTSK/BOM/KLBS:25.5/KLBS:10000000/PDESIG:TAD01/SDESIG:TAD02/16-72/4-3-1// Bad Backup FuelLoad
                     7, // REFTSK/BOM/KLBS:25.5/KLBS:5.1/PDESIG:TAD01/SDESIG:TAD02/Nope/4-3-1// Bad Tacan Channel
                     8  // REFTSK/BOM/KLBS:25.5/KLBS:5.1/PDESIG:TAD01/SDESIG:TAD02/16-72/NOPE// Bad Beacon
                     ), );


TEST_P(RefuelingTaskingParameterizedFixtureTestsInValid, PassesForInValidRefuelingTasking)
{
   int                            setNumber = GetParam();
   auto&                          set       = mMessage->GetSet(setNumber);
   const usmtf::RefuelingTasking* sptr      = sFact->CastIfRegistered<usmtf::RefuelingTasking>(set);
   EXPECT_FALSE(sptr->IsValid());
   sptr->LogErrors(std::cout);
}
