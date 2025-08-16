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

#include "AirAssetControl.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"

class AirAssetControlParameterizedFixtureTestsValid : public ::testing::TestWithParam<int>
{
public:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("CONTROLA", usmtf::SetFactory::Construct<usmtf::AirAssetControl>);
      const std::string   CONTROLA = mDataService.getResource("AirAssetControl.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser   = usmtf::USMTF_Parser(CONTROLA);
      mMessage                     = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

INSTANTIATE_TEST_CASE_P(AirAssetControlValidTests,
                        AirAssetControlParameterizedFixtureTestsValid,
                        ::testing::Values(2, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/NAME:ALFA//
                                          3, // CONTROLA/CRC/Any Text/PFREQ:342.5/SDESIG:TAD02/IT01/NAME:ALFA//
                                          4, // CONTROLA/CRC/INVERT/Any Text/SDESIG:TAD02/IT01/NAME:ALFA//
                                          5, // CONTROLA/CRC/INVERT/PFREQ:342.5/Any Text/IT01/NAME:ALFA//
                                          6, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/Any Text/NAME:ALFA//
                                          7, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/LATM:3510N07901W//
                                          8, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/LATS:351025N0790125W//
                                          9, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/32WDL12341234//
                                          10, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/32WDL123123//
                                          11, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/UTM:32WDL1234512345//
                                          12, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/VGEOK:3510.234N8-07901.123W3//
                                          13, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/VGEOT:351025.3N9-0790125.7W1//
                                          14, // CONTROLA/CRC/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/Any Text//
                                          15 // CONTROLA/Nope/INVERT/PFREQ:342.5/SDESIG:TAD02/IT01/NAME:ALFA// ONLY INVALID
                                          ), );

TEST_P(AirAssetControlParameterizedFixtureTestsValid, PassesForValidAirAssetControl)
{
   int                           setNumber = GetParam();
   auto&                         set       = mMessage->GetSet(setNumber);
   const usmtf::AirAssetControl* sptr      = sFact->CastIfRegistered<usmtf::AirAssetControl>(set);
   if (setNumber == 15)
   {
      // im getting lazy and creating another fixture because i cant reuse the parameterized one is very annoying.
      EXPECT_FALSE(sptr->IsValid());
   }
   else
   {
      EXPECT_TRUE(sptr->IsValid());
   }
   const usmtf::VariantValidator<usmtf::LatLon, usmtf::UTM_Location, usmtf::VerifiedLatLonTM, usmtf::VerifiedLatLonDS, usmtf::FreeText>&
      reportPoint = sptr->GetReportInPoint();
   if (setNumber == 2 || setNumber == 14)
   {
      usmtf::FreeText* rp = reportPoint.IsMatch<usmtf::FreeText>();
      EXPECT_TRUE(rp != nullptr);
      usmtf::LatLon* wrong = reportPoint.IsMatch<usmtf::LatLon>();
      EXPECT_TRUE(wrong == nullptr);
   }
   if (setNumber == 7 || setNumber == 8)
   {
      usmtf::LatLon* rp = reportPoint.IsMatch<usmtf::LatLon>();
      EXPECT_TRUE(rp != nullptr);
      usmtf::FreeText* wrong = reportPoint.IsMatch<usmtf::FreeText>();
      EXPECT_TRUE(wrong == nullptr);
   }
   if (setNumber == 9 || setNumber == 10 || setNumber == 11)
   {
      usmtf::UTM_Location* rp = reportPoint.IsMatch<usmtf::UTM_Location>();
      EXPECT_TRUE(rp != nullptr);
      usmtf::VerifiedLatLonTM* wrong = reportPoint.IsMatch<usmtf::VerifiedLatLonTM>();
      EXPECT_TRUE(wrong == nullptr);
   }
   if (setNumber == 12)
   {
      usmtf::VerifiedLatLonTM* rp = reportPoint.IsMatch<usmtf::VerifiedLatLonTM>();
      EXPECT_TRUE(rp != nullptr);
      usmtf::VerifiedLatLonDS* wrong = reportPoint.IsMatch<usmtf::VerifiedLatLonDS>();
      EXPECT_TRUE(wrong == nullptr);
   }
   if (setNumber == 13)
   {
      usmtf::VerifiedLatLonDS* rp = reportPoint.IsMatch<usmtf::VerifiedLatLonDS>();
      EXPECT_TRUE(rp != nullptr);
      usmtf::FreeText* wrong = reportPoint.IsMatch<usmtf::FreeText>();
      EXPECT_TRUE(wrong == nullptr);
   }
}
