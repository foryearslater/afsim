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

#include "PackageCommander.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"

class PackageCommanderParameterizedFixtureTestsValid :
   public ::testing::TestWithParam<std::tuple<int, std::string, std::string, std::string, std::string, std::string>>
{
public:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("PKGCMD", usmtf::SetFactory::Construct<usmtf::PackageCommander>);
      const std::string   PKGCMD = mDataService.getResource("PackageCommander.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(PKGCMD);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

INSTANTIATE_TEST_CASE_P(
   PackageCommanderValidTests,
   PackageCommanderParameterizedFixtureTestsValid,
   ::testing::Values(
      std::make_tuple(2, "ZZ", "123 FS", "123DAH", "HAMMER 12", "HR12"), // PKGCMD/ZZ/123 FS/123DAH/HAMMER 12/HR12//
      std::make_tuple(3, "AnyText", "123 FS", "123DAH", "HAMMER 12", "HR12"), // PKGCMD/AnyText/123 FS/123DAH/HAMMER 12/HR12//
      std::make_tuple(4, "ZZ", "AnyText", "123DAH", "HAMMER 12", "HR12"),  // PKGCMD/ZZ/AnyText/123DAH/HAMMER 12/HR12//
      std::make_tuple(5, "ZZ", "123 FS", "AnyText", "HAMMER 12", "HR12"),  // PKGCMD/ZZ/123 FS/AnyText/HAMMER 12/HR12//
      std::make_tuple(6, "ZZ", "123 FS", "123DAH", "AnyText", "HR12"),     // PKGCMD/ZZ/123 FS/123DAH/AnyText/HR12//
      std::make_tuple(7, "ZZ", "123 FS", "123DAH", "HAMMER 12", "AnyText") // PKGCMD/ZZ/123 FS/123DAH/HAMMER 12/AnyText//
      ), );

TEST_P(PackageCommanderParameterizedFixtureTestsValid, PassesForValidPackageCommanderSets)
{
   int                            setNumber        = std::get<0>(GetParam());
   std::string                    packageId        = std::get<1>(GetParam());
   std::string                    taskedUnit       = std::get<2>(GetParam());
   std::string                    missionNumber    = std::get<3>(GetParam());
   std::string                    aircraftCallsign = std::get<4>(GetParam());
   std::string                    linkCalsign      = std::get<5>(GetParam());
   auto&                          set              = mMessage->GetSet(setNumber);
   const usmtf::PackageCommander* sptr             = sFact->CastIfRegistered<usmtf::PackageCommander>(set);
   EXPECT_TRUE(sptr->IsValid());
   EXPECT_EQ(sptr->GetPackageId().GetText(), packageId);
   EXPECT_EQ(sptr->GetTaskedUnit().GetText(), taskedUnit);
   EXPECT_EQ(sptr->GetMissionNumber().GetText(), missionNumber);
   EXPECT_EQ(sptr->GetAircraftCallsign().GetText(), aircraftCallsign);
   EXPECT_EQ(sptr->GetLink16Callsign().GetText(), linkCalsign);
}

// There arent really any invalid cases because the validators are all free text.
