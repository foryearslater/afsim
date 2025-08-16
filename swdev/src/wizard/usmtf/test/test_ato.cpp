// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "AerialRefueling.hpp"
#include "AerialRefuelingInfo.hpp"
#include "AirAssetControl.hpp"
#include "AircraftMission.hpp"
#include "AircraftMissionData.hpp"
#include "AircraftMissionLocation.hpp"
#include "Ampn.hpp"
#include "Ato.hpp"
#include "Exer.hpp"
#include "InputBuilder.hpp"
#include "PackageCommander.hpp"
#include "PackageData.hpp"
#include "RefuelingTasking.hpp"
#include "TaskUnit.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"
#include "test_utils.hpp"


class AtoTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      RegisterDependecies();
      const std::string   ATO    = mDataService.getResource("Ato.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(ATO);
      mMessage                   = parser.ReadMessage();
   }

   void RegisterDependecies()
   {
      sFact = usmtf::SetFactory::GetFactory();
      mFact = usmtf::MessageFactory::GetFactory();
      sFact->RegisterEntity("MSNACFT", usmtf::SetFactory::Construct<usmtf::AircraftMission>);
      sFact->RegisterEntity("ARINFO", usmtf::SetFactory::Construct<usmtf::AerialRefuelingInfo>);
      sFact->RegisterEntity("AMSNLOC", usmtf::SetFactory::Construct<usmtf::AircraftMissionLocation>);
      sFact->RegisterEntity("AMSNDAT", usmtf::SetFactory::Construct<usmtf::AircraftMissionData>);
      sFact->RegisterEntity("AMPN", usmtf::SetFactory::Construct<usmtf::Ampn>);
      sFact->RegisterEntity("9PKGDAT", usmtf::SetFactory::Construct<usmtf::PackageData>);
      sFact->RegisterEntity("CONTROLA", usmtf::SetFactory::Construct<usmtf::AirAssetControl>);
      sFact->RegisterEntity("TASKUNIT", usmtf::SetFactory::Construct<usmtf::TaskUnit>);
      sFact->RegisterEntity("PKGCMD", usmtf::SetFactory::Construct<usmtf::PackageCommander>);
      sFact->RegisterEntity("REFTSK", usmtf::SetFactory::Construct<usmtf::RefuelingTasking>);
      sFact->RegisterEntity("EXER", usmtf::SetFactory::Construct<usmtf::Exer>);
      sFact->RegisterEntity("5REFUEL", usmtf::SetFactory::Construct<usmtf::AerialRefueling>);
      mFact->RegisterEntity("ATO", usmtf::MessageFactory::Construct<usmtf::Ato>);
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   usmtf::MessageFactory*          mFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

class AtoTestMissionDataLocation : public AtoTest
{
protected:
   void SetUp() override
   {
      RegisterDependecies();
      const std::string   ATO    = mDataService.getResource("AtoMissionDataLocation.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(ATO);
      mMessage                   = parser.ReadMessage();
   }
};

class AtoTestAerialRefuelLocation : public AtoTest
{
protected:
   void SetUp() override
   {
      RegisterDependecies();
      const std::string   ATO    = mDataService.getResource("AtoAerialRefuelLocation.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(ATO);
      mMessage                   = parser.ReadMessage();
   }
};

class AtoTestAirAssetLocation : public AtoTest
{
protected:
   void SetUp() override
   {
      RegisterDependecies();
      const std::string   ATO    = mDataService.getResource("AtoAirAssetLocation.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(ATO);
      mMessage                   = parser.ReadMessage();
   }
};

namespace
{
bool TestFileMatch(const std::string&              one,
                   const std::string&              two,
                   std::shared_ptr<usmtf::Message> aMessage,
                   usmtf::MessageFactory*          aFact,
                   std::string&                    aErrMessage)
{
   const ut::TestDataService& dataService     = ut::Locator<ut::TestDataService>::get();
   auto                       path            = dataService.getResource(one);
   const std::string          ATO_EXPORT      = dataService.getResource(two).GetNormalizedPath();
   const std::string          ATO_EXPORT_TEMP = path.GetNormalizedPath();
   path.Up();
   const std::string ATO_EXPORT_DIRECTORY = path.GetNormalizedPath();
   const usmtf::Ato* message              = aFact->CastIfRegistered<usmtf::Ato>(*aMessage.get());
   ExportAto(*message, ATO_EXPORT_DIRECTORY, one);

   std::ifstream     test(ATO_EXPORT);
   std::ifstream     temp(ATO_EXPORT_TEMP);
   std::stringstream errStream{};
   auto              result = AreFilesIdentical(test, temp);
   errStream << "Mismatch at position: " + std::to_string(result.position) + "\n";
   errStream << "Char was: " + std::string(1, result.actual) + "\n";
   errStream << "Char expected: " + std::string(1, result.expected) << std::endl;

   aErrMessage = errStream.str();
   temp.close();
   test.close();
   std::remove(ATO_EXPORT_TEMP.c_str());
   return result.wasIdentical;
}
} // namespace

TEST_F(AtoTest, AtoIsValidForValidSets)
{
   mMessage->LogErrors(std::cout);
   ASSERT_TRUE(mMessage->IsValid());
}

TEST_F(AtoTest, AtoExportWorks)
{
   EXPECT_TRUE(mMessage->IsValid());
   std::string errMessage{};
   bool        result = TestFileMatch("AtoExport_Temp.txt", "AtoExport.txt", mMessage, mFact, errMessage);
   EXPECT_TRUE(result) << errMessage; // err only show on fail.
}

TEST_F(AtoTestMissionDataLocation, AtoExportWorksAtoMissionDataLocation)
{
   EXPECT_TRUE(mMessage->IsValid());
   std::string errMessage{};
   bool        result =
      TestFileMatch("AtoMissionDataLocationExport_Temp.txt", "AtoMissionDataLocationExport.txt", mMessage, mFact, errMessage);
   EXPECT_TRUE(result) << errMessage; // err only show on fail.
}

TEST_F(AtoTestAerialRefuelLocation, AtoExportWorksAtoAerialRefuelLocation)
{
   EXPECT_TRUE(mMessage->IsValid());
   std::string errMessage{};
   bool        result =
      TestFileMatch("AtoAerialRefuelLocationExport_Temp.txt", "AtoAerialRefuelLocationExport.txt", mMessage, mFact, errMessage);
   EXPECT_TRUE(result) << errMessage; // err only show on fail.
}

TEST_F(AtoTestAirAssetLocation, AtoExportWorksAtoAerialRefuelLocation)
{
   EXPECT_TRUE(mMessage->IsValid());
   std::string errMessage{};
   bool        result =
      TestFileMatch("AtoAirAssetLocationExport_Temp.txt", "AtoAirAssetLocationExport.txt", mMessage, mFact, errMessage);
   EXPECT_TRUE(result) << errMessage; // err only show on fail.
}
