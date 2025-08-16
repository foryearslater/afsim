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

#include "PackageData.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class PackageData : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("9PKGDAT", usmtf::SetFactory::Construct<usmtf::PackageData>);
      const std::string   PACKAGEDATA = mDataService.getResource("PackageData.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser      = usmtf::USMTF_Parser(PACKAGEDATA);
      mMessage                        = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

/*
9PKGDAT
/PKGID/UNIT                /MSNNO   /PMSN  /NO/ACTYPE   /ACSIGN
/ZZ   /123 FS              /123DAH  /CAS   / 4/AC:FA18  /HAMMER 12
/AA   /37 AS               /ABC123  /-     / 2/AC:C130E /HERKY 45
//
*/
TEST_F(PackageData, PackageDataIsvalidForProperFields)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

// Given all fields field are free text  except for range of airplanes, there isnt much to test on validity

/*
9PKGDAT
/PKGID/UNIT                /MSNNO   /PMSN  /NO/ACTYPE   /ACSIGN
/ZZ   /123 FS              /123DAH  /CAS   / 4/AC:FA18  /HAMMER 12
/AA   /37 AS               /ABC123  /-     / 100/AC:C130E /HERKY 45
//
*/
TEST_F(PackageData, PackageDataIsInvalidForBadPlaneRange)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}

/*
9PKGDAT
/PKGID/UNIT                /MSNNO   /PMSN  /NO/ACTYPE   /ACSIGN
/ZZ   /123 FS              /123DAH  /CAS   / 4/AC:FA18
/AA   /37 AS               /ABC123  /-     / 12/AC:C130E /HERKY 45
//
*/
TEST_F(PackageData, PackageDataIsInvalidForBadFieldCount)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_FALSE(set.IsValid());
   set.LogErrors(std::cout);
}


/*
9PKGDAT
/PKGID/UNIT                /MSNNO   /PMSN  /NO/ACTYPE   /ACSIGN
/ZZ   /123 FS              /123DAH  /CAS   / 4/AC:FA18  /HAMMER 12
/AA   /37 AS               /ABC123  /-     / 2/AC:C130E /HERKY 45
//
*/
TEST_F(PackageData, PackageDataCanPullUnits)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
   const usmtf::PackageData* sptr  = sFact->CastIfRegistered<usmtf::PackageData>(set);
   usmtf::PackageData::Units units = sptr->GetUnits();
   usmtf::PackageData::Unit  unit  = units[0];
   EXPECT_EQ(std::get<0>(unit).GetText(), "ZZ");
   EXPECT_EQ(std::get<1>(unit).GetText(), "123 FS");
   EXPECT_EQ(std::get<2>(unit).GetText(), "123DAH");
   EXPECT_EQ(std::get<3>(unit).GetText(), "CAS");
   EXPECT_EQ(std::get<4>(unit), 4);
   EXPECT_EQ(std::get<5>(unit).GetText(), "FA18");
   EXPECT_EQ(std::get<6>(unit).GetText(), "HAMMER 12");
}
