// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
//==============================================================================

#include <gtest/gtest.h>

#include "USMTF_Exceptions.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtMemory.hpp"
#include "UtTestDataService.hpp"
#include "test_user_defined_types.hpp"

class SetTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      mFact = usmtf::MessageFactory::GetFactory();
      sFact = usmtf::SetFactory::GetFactory();
      mFact->RegisterEntity("UDM", usmtf::MessageFactory::Construct<UserDefinedMessage>);
      sFact->RegisterEntity("UDMS", usmtf::SetFactory::Construct<UserDefinedSet>);
      const std::string EXAMPLE_ATO = mDataService.getResource("Example_ATO.txt").GetNormalizedPath();
      const std::string EXAMPLE_UDM = mDataService.getResource("Example_UDM.txt").GetNormalizedPath();

      usmtf::USMTF_Parser parser    = usmtf::USMTF_Parser(EXAMPLE_ATO);
      mMessage                      = parser.ReadMessage();
      usmtf::USMTF_Parser UDMparser = usmtf::USMTF_Parser(EXAMPLE_UDM);
      mUDMessage                    = UDMparser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   usmtf::MessageFactory*          mFact;
   std::shared_ptr<usmtf::Message> mMessage;
   std::shared_ptr<usmtf::Message> mUDMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};


TEST_F(SetTest, GetTypeReturnsType)
{
   std::string type  = mMessage->GetSet(3).GetType();
   std::string type1 = mMessage->GetSet(2).GetType();
   std::string type2 = mMessage->GetSet(7).GetType();
   std::string type3 = mMessage->GetSet(9).GetType();

   EXPECT_EQ(type, "TIMEFRAM");
   EXPECT_EQ(type1, "AKNLDG");
   EXPECT_EQ(type2, "AMSNDAT");
   EXPECT_EQ(type3, "ARINFO");
}

TEST_F(SetTest, FieldCountIsAccurateForSeveral)
{
   const int count  = mMessage->GetSet(3).GetFieldCount();
   const int count1 = mMessage->GetSet(2).GetFieldCount();
   const int count2 = mMessage->GetSet(7).GetFieldCount();
   const int count3 = mMessage->GetSet(9).GetFieldCount();

   EXPECT_EQ(count, 4);
   EXPECT_EQ(count1, 2);
   EXPECT_EQ(count2, 7);
   EXPECT_EQ(count3, 18);
}

TEST_F(SetTest, IsValidReturnTrueForBase)
{
   bool validity = mMessage->GetSet(3).IsValid();
   EXPECT_TRUE(validity);
}

TEST_F(SetTest, GetFieldGetsField)
{
   std::string field_value = mMessage->GetSet(3).GetField(2).GetContent();

   EXPECT_EQ(field_value, "020559ZOCT1998");
}

TEST_F(SetTest, EmptyFunctions)
{
   bool empty = mMessage->GetSet(3).IsEmpty();

   EXPECT_FALSE(empty);
}

TEST_F(SetTest, EmptyReturnsTrueForNoFields)
{
   std::vector<usmtf::Field> a = {};
   usmtf::Set                s = usmtf::Set("MSGID", a);
   ASSERT_TRUE(s.IsEmpty());
}

TEST_F(SetTest, EmptyFieldsThrowsOnInvalidIndexEmpty)
{
   std::vector<usmtf::Field> a = {};
   usmtf::Set                s = usmtf::Set("MSGID", a);
   ASSERT_THROW(s.GetField(12), usmtf::SetError);
}

TEST_F(SetTest, EmptyFieldsThrowsOnInvalidIndexNotEmpty)
{
   std::vector<usmtf::Field> a = {usmtf::Field(std::string("context"))};
   usmtf::Set                s = usmtf::Set("MSGID", a);
   ASSERT_THROW(s.GetField(12), usmtf::SetError);
}

TEST_F(SetTest, DerivedSetPassesWithCustomValidation)
{
   const usmtf::Set& set     = mUDMessage->GetSet(2); // our UserDefinedSet without casting.
   bool              isValid = set.IsValid();         // should polymorphically call UserDefinedSet.IsValid()
   ASSERT_TRUE(isValid);
}

TEST_F(SetTest, DerivedSetFailsWithCustomValidation)
{
   const usmtf::Set& set     = mUDMessage->GetSet(3); // Also a UDMS Set, but fails validation
   bool              isValid = set.IsValid();
   ASSERT_FALSE(isValid);
}

TEST_F(SetTest, DerivedSetFailsWithCustomValidationPositioning)
{
   const usmtf::Set& set     = mUDMessage->GetSet(4); // Also a UDMS Set, but fails validation
   bool              isValid = set.IsValid();
   ASSERT_FALSE(isValid);
}

TEST_F(SetTest, DerivedSetCustomDataInterpretation)
{
   const usmtf::Set& set     = mUDMessage->GetSet(2);
   bool              isValid = set.IsValid();
   EXPECT_TRUE(isValid);

   const UserDefinedSet* sptr = sFact->CastIfRegistered<UserDefinedSet>(set);
   std::string           no;
   if (sptr != nullptr)
   {
      no = sptr->WhatIsYourAnswer();
   }

   ASSERT_EQ(no, "NO");
}

TEST_F(SetTest, GetReconstructedSetReturnsCorrectly)
{
   const usmtf::Set& set     = mUDMessage->GetSet(2);
   bool              isValid = set.IsValid();
   ASSERT_TRUE(isValid);

   const std::string rs = set.GetReconstructedSet();
   ASSERT_EQ(rs, "UDMS/YES/NO//");
}
