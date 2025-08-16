// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <fstream>

#include <gtest/gtest.h>

#include "AOrbit.hpp"
#include "APeriod.hpp"
#include "APoint.hpp"
#include "Acmid.hpp"
#include "Aco.hpp"
#include "Ampn.hpp"
#include "Circle.hpp"
#include "Corridor.hpp"
#include "Efflevel.hpp"
#include "Exer.hpp"
#include "InputBuilder.hpp"
#include "Onetrack.hpp"
#include "Polyarc.hpp"
#include "Polygon.hpp"
#include "Radarc.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"
#include "test_utils.hpp"

class CircularZoneTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      mFact = usmtf::MessageFactory::GetFactory();
      sFact->RegisterEntity("ACMID", usmtf::SetFactory::Construct<usmtf::Acmid>);
      sFact->RegisterEntity("APOINT", usmtf::SetFactory::Construct<usmtf::APoint>);
      sFact->RegisterEntity("AMPN", usmtf::SetFactory::Construct<usmtf::Ampn>);
      sFact->RegisterEntity("AORBIT", usmtf::SetFactory::Construct<usmtf::AOrbit>);
      sFact->RegisterEntity("APERIOD", usmtf::SetFactory::Construct<usmtf::APeriod>);
      sFact->RegisterEntity("CIRCLE", usmtf::SetFactory::Construct<usmtf::Circle>);
      sFact->RegisterEntity("CORRIDOR", usmtf::SetFactory::Construct<usmtf::Corridor>);
      sFact->RegisterEntity("EFFLEVEL", usmtf::SetFactory::Construct<usmtf::Efflevel>);
      sFact->RegisterEntity("EXER", usmtf::SetFactory::Construct<usmtf::Exer>);
      sFact->RegisterEntity("POLYGON", usmtf::SetFactory::Construct<usmtf::Polygon>);
      sFact->RegisterEntity("POLYARC", usmtf::SetFactory::Construct<usmtf::Polyarc>);
      sFact->RegisterEntity("1TRACK", usmtf::SetFactory::Construct<usmtf::OneTrack>);
      sFact->RegisterEntity("RADARC", usmtf::SetFactory::Construct<usmtf::Radarc>);

      mFact->RegisterEntity("ACO", usmtf::MessageFactory::Construct<usmtf::Aco>);
      const std::string CZACO = mDataService.getResource("CircularZoneAco.txt").GetNormalizedPath();

      usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(CZACO);
      mMessage                   = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   usmtf::MessageFactory*          mFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};


TEST_F(CircularZoneTest, CirculrZoneAcoExportIsValidForValidSets)
{
   ASSERT_TRUE(mMessage->IsValid());
}

TEST_F(CircularZoneTest, CirculrZoneAcoExportExportWorks)
{
   EXPECT_TRUE(mMessage->IsValid());

   const ut::TestDataService& dataService = ut::Locator<ut::TestDataService>::get();
   const std::string          ACO_EXPORT  = dataService.getResource("CircularZoneAcoExport.txt").GetNormalizedPath();
   const std::string   ACO_EXPORT_TEMP = dataService.getResource("CircularZoneAcoExport_Temp.txt").GetNormalizedPath();
   const usmtf::Aco*   message         = mFact->CastIfRegistered<usmtf::Aco>(*mMessage.get());
   usmtf::InputBuilder sb{ACO_EXPORT_TEMP};
   ExportAco(*message, sb);

   std::ifstream     test(ACO_EXPORT);
   std::ifstream     temp(ACO_EXPORT_TEMP);
   std::stringstream errStream{};
   auto              result = AreFilesIdentical(test, temp);
   errStream << "Mismatch at position: " + std::to_string(result.position) + "\n";
   errStream << "Char was: " + std::string(1, result.actual) + "\n";
   errStream << "Char expected: " + std::string(1, result.expected) << std::endl;

   std::string errMessage = errStream.str();

   EXPECT_TRUE(result.wasIdentical) << errMessage; // err only show on fail.


   temp.close();
   test.close();
   std::remove(ACO_EXPORT_TEMP.c_str());
}
