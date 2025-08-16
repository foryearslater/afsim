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

#include "Onetrack.hpp"
#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"


class OnetrackTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("1TRACK", usmtf::SetFactory::Construct<usmtf::OneTrack>);
      const std::string   ONETRACK = mDataService.getResource("Onetrack.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser   = usmtf::USMTF_Parser(ONETRACK);
      mMessage                     = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

/*
1TRACK
/LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
/ 01/152345N0505657E/192646N0531226E/30.5NML-60.9NMR/050AMSL-100AMSL
/ 02/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
/ 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL
/ 04/280628N0562901E/330938N0574603E/25NML-25NMR    /120AMSL-180AMSL
//
*/
TEST_F(OnetrackTest, OneTrackIsvalidForProperFieldsCount)
{
   auto& set = mMessage->GetSet(2);
   ASSERT_TRUE(set.IsValid());
}

/*
1TRACK
/LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
/ 01/152345N0505657E/192646N0531226E/30.5NML-60.9NMR/050AMSL-100AMSL
/ 02/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
/ 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL
/ 04/280628N0562901E/330938N0574603E/25NML-25NMR
//
*/
TEST_F(OnetrackTest, OneTrackIsInvalidForFieldCount)
{
   auto& set = mMessage->GetSet(3);
   ASSERT_FALSE(set.IsValid());
}

/*
1TRACK
/LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
/ 01/152345N0505657E/192646N0531226E/30.5NML-60.9NMR/050AMSL-100AMSL
/ 03/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
/ 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL
/ 04/280628N0562901E/330938N0574603E/25NML-25NMR    /120AMSL-180AMSL
//
*/
TEST_F(OnetrackTest, OneTrackIsInvalidForInvalidLegSequence)
{
   auto& set = mMessage->GetSet(4);
   ASSERT_FALSE(set.IsValid());
}

/*
1TRACK
/LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
/ 01/152345W0505657E/192646N0531226E/30.5NML-60.9NMR/050AMSL-100AMSL
/ 03/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
/ 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL
/ 04/280628N0562901E/330938N0574603E/25NML-25NMR    /120AMSL-180AMSL
//
*/
TEST_F(OnetrackTest, OneTrackIsInvalidForInvalidLegBegin)
{
   auto& set = mMessage->GetSet(5);
   ASSERT_FALSE(set.IsValid());
}

/*
1TRACK
/LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
/ 01/152345N0505657E/192646N0531226N/30.5NML-60.9NMR/050AMSL-100AMSL
/ 03/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
/ 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL
/ 04/280628N0562901E/330938N0574603E/25NML-25NMR    /120AMSL-180AMSL
//
*/
TEST_F(OnetrackTest, OneTrackIsInvalidForInvalidLegEnd)
{
   auto& set = mMessage->GetSet(6);
   ASSERT_FALSE(set.IsValid());
}

/*
1TRACK
/LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
/ 01/152345N0505657E/192646N0531226E/30.5NML60.9NMR/050AMSL-100AMSL
/ 03/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
/ 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL
/ 04/280628N0562901E/330938N0574603E/25NML-25NMR    /120AMSL-180AMSL
//
*/
TEST_F(OnetrackTest, OneTrackIsInvalidForInvalidLegWidthNoHpyhen)
{
   auto& set = mMessage->GetSet(7);
   ASSERT_FALSE(set.IsValid());
}

/*
1TRACK
/LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
/ 01/152345N0505657E/192646N0531226E/30.5NML-60.9NML/050AMSL-100AMSL
/ 03/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
/ 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL
/ 04/280628N0562901E/330938N0574603E/25NML-25NMR    /120AMSL-180AMSL
//
*/
TEST_F(OnetrackTest, OneTrackIsInvalidForInvalidLegWidthWrongSide)
{
   auto& set = mMessage->GetSet(8);
   ASSERT_FALSE(set.IsValid());
}

/*
1TRACK
/LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
/ 01/152345N0505657E/192646N0531226E/30.5NML-60.9NMR/050AMSL100AMSL
/ 03/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
/ 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL
/ 04/280628N0562901E/330938N0574603E/25NML-25NMR    /120AMSL-180AMSL
//
*/
TEST_F(OnetrackTest, OneTrackIsInvalidForInvalidLegMinMaxAlt) // well tested in others as well
{
   auto& set = mMessage->GetSet(9);
   ASSERT_FALSE(set.IsValid());
}

/*
1TRACK
/LEG/LEG-BEGIN      /LEG-END        /LEG-WIDTH      /MINALT-MAXALT
/ 01/152345N0505657E/192646N0531226E/30.5NML-60.9NMR/050AMSL-100AMSL
/ 02/192646N0531226E/231031N0545323E/60.5NML-60.5NMR/080AMSL-120AMSL
/ 03/231031N0545323E/280628N0562901E/60NML-80NMR    /100AMSL-150AMSL
/ 04/280628N0562901E/330938N0574603E/25NML-25NMR    /120AMSL-180AMSL
//
*/
TEST_F(OnetrackTest, OneTrackLegsIncludeRequiredValidators)
{
   auto& set = mMessage->GetSet(2);
   EXPECT_TRUE(set.IsValid());

   const usmtf::OneTrack*       sptr          = sFact->CastIfRegistered<usmtf::OneTrack>(set);
   const usmtf::OneTrack::Legs& legs          = sptr->GetLegs();
   const auto&                  leg           = legs[0];
   const auto                   begin         = std::get<1>(leg);
   const auto                   end           = std::get<2>(leg);
   const auto                   leftOffset    = std::get<3>(leg).GetLeftRadius();
   const auto                   rightOffset   = std::get<3>(leg).GetRightRadius();
   const auto                   leftDistance  = leftOffset->GetRadiusValue();
   const auto                   rightDistance = rightOffset->GetRadiusValue();
   const auto                   minAlt        = std::get<4>(leg).GetNumericalMinAltitude();
   const auto                   maxAlt        = std::get<4>(leg).GetNumericalMaxAltitude();
   EXPECT_EQ(begin.GetRawContent(), "152345N0505657E");
   EXPECT_EQ(end.GetRawContent(), "192646N0531226E");
   EXPECT_EQ(leftDistance, "30.500000");
   EXPECT_EQ(rightDistance, "60.900000");
   EXPECT_EQ(minAlt, 5000);
   EXPECT_EQ(maxAlt, 10000);
}
