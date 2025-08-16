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

#include "AircraftMissionData.hpp"
#include "NamedLocation.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"

class AircraftMissionDataParameterizedFixtureTests :
   public ::testing::TestWithParam<
      std::tuple<int, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string>>
{
public:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("AMSNDAT", usmtf::SetFactory::Construct<usmtf::AircraftMissionData>);
      const std::string   AMSNDAT = mDataService.getResource("AircraftMissionData.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser  = usmtf::USMTF_Parser(AMSNDAT);
      mMessage                    = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

INSTANTIATE_TEST_CASE_P(
   AircraftMissionDataValidTests,
   AircraftMissionDataParameterizedFixtureTests,
   ::testing::Values(
      std::make_tuple(2, "N", "D123HA", "-", "ZZ", "MC", "CAS", "SEAD", "", "", "", "", ""), // AMSNDAT/N/D123HA/-/ZZ/MC/CAS/SEAD//
      std::make_tuple(3, "N", "D123HB", "-", "ZZ", "-", "CAS", "SEAD", "-", "KGZ6", "241200ZAPR", "KDZ7", "241300ZAPR") // AMSNDAT/N/D123HB/-/ZZ/-/CAS/SEAD/-/DEPLOC:KGZ6/241200ZAPR/ARRLOC:KDZ7/241300ZAPR/
      ), );

TEST_P(AircraftMissionDataParameterizedFixtureTests, ValidCases)
{
   int                               setNumber            = std::get<0>(GetParam());
   std::string                       rmi                  = std::get<1>(GetParam());
   std::string                       missionNumber        = std::get<2>(GetParam());
   std::string                       amc                  = std::get<3>(GetParam());
   std::string                       packageId            = std::get<4>(GetParam());
   std::string                       missionCommander     = std::get<5>(GetParam());
   std::string                       primaryMissionType   = std::get<6>(GetParam());
   std::string                       secondaryMissionType = std::get<7>(GetParam());
   std::string                       alertStatus          = std::get<8>(GetParam());
   std::string                       departurelocation    = std::get<9>(GetParam());
   std::string                       timeOfDeparture      = std::get<10>(GetParam());
   std::string                       recoveryLocation     = std::get<11>(GetParam());
   std::string                       timeOfRecovery       = std::get<12>(GetParam());
   auto&                             set                  = mMessage->GetSet(setNumber);
   const usmtf::AircraftMissionData* sptr                 = sFact->CastIfRegistered<usmtf::AircraftMissionData>(set);
   EXPECT_TRUE(sptr->IsValid());
   EXPECT_EQ(sptr->GetResidualMissionIndicator().GetText(), rmi);
   EXPECT_EQ(sptr->GetMissionNumber().GetText(), missionNumber);
   EXPECT_EQ(sptr->GetEventNumber().IsMatch<usmtf::FreeText>()->GetText(), amc);
   EXPECT_EQ(sptr->GetPackageIdentification().GetText(), packageId);
   EXPECT_EQ(sptr->GetMissionCommander().GetText(), missionCommander);
   EXPECT_EQ(sptr->GetPrimaryMissionType().GetText(), primaryMissionType);
   EXPECT_EQ(sptr->GetSecondaryMissionType().GetText(), secondaryMissionType);
   EXPECT_EQ(sptr->GetAlertStatus().GetText(), alertStatus);
   if (setNumber == 2) // not one present as its optional and not provided
   {
      EXPECT_EQ(sptr->GetDepartureLocation().IsMatch<usmtf::FreeText>(), nullptr);
      EXPECT_EQ(sptr->GetTimeOfDeparture().GetText(), timeOfDeparture);
      EXPECT_EQ(sptr->GetRecoveryLocations().size(), 0);
      EXPECT_EQ(sptr->GetRecoverTimes().size(), 0);
   }
   else
   {
      EXPECT_EQ(sptr->GetDepartureLocation().IsMatch<usmtf::NamedLocation>()->GetLocation(), departurelocation);
      EXPECT_EQ(sptr->GetTimeOfDeparture().GetText(), timeOfDeparture);
      EXPECT_EQ(sptr->GetRecoveryLocations()[0]->IsMatch<usmtf::NamedLocation>()->GetLocation(), recoveryLocation);
      EXPECT_EQ(sptr->GetRecoverTimes()[0].GetText(), timeOfRecovery);
   }
   sptr->LogErrors(std::cout);
}

// There literally can be no invalid cases except for missing mandatory fields which are forbidden by the spec and have
// not been tested by any set yet. In fact thats why coverage is only in the high 80's, because we cover the emergency
// case but dont test it. Should be tested honestly.
