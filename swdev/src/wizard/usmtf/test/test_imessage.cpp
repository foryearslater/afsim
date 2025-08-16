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
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtMemory.hpp"
#include "UtTestDataService.hpp"
#include "test_user_defined_types.hpp"

class MessageTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      usmtf::MessageFactory::GetFactory()->UnregisterEntity("ACO");
      const std::string EXAMPLE_ACO = mDataService.getResource("Example_ACO.txt").GetNormalizedPath();
      const std::string EXAMPLE_ATO = mDataService.getResource("Example_ATO.txt").GetNormalizedPath();
      const std::string EXAMPLE_UDM = mDataService.getResource("Example_UDM.txt").GetNormalizedPath();


      usmtf::USMTF_Parser parserAto = usmtf::USMTF_Parser(EXAMPLE_ATO);
      mMessageATO                   = parserAto.ReadMessage();

      usmtf::USMTF_Parser parserAco = usmtf::USMTF_Parser(EXAMPLE_ACO);
      mMessageACO                   = parserAco.ReadMessage();

      mFact = usmtf::MessageFactory::GetFactory();
      mFact->RegisterEntity("UDM", usmtf::MessageFactory::Construct<UserDefinedMessage>);
      sFact = usmtf::SetFactory::GetFactory();
      sFact->RegisterEntity("UDMS", usmtf::SetFactory::Construct<UserDefinedSet>);

      usmtf::USMTF_Parser parserUDM = usmtf::USMTF_Parser(EXAMPLE_UDM);
      mMessageUD                    = parserUDM.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::MessageFactory*          mFact;
   usmtf::SetFactory*              sFact;
   std::shared_ptr<usmtf::Message> mMessageATO;
   std::shared_ptr<usmtf::Message> mMessageACO;
   std::shared_ptr<usmtf::Message> mMessageUD;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

TEST_F(MessageTest, GetTypeReturnsType)
{
   std::string type  = mMessageATO->GetType();
   std::string type1 = mMessageACO->GetType();

   EXPECT_EQ(type, "ATO");
   EXPECT_EQ(type1, "ACO");
}

TEST_F(MessageTest, SetCountIsAccurateForSeveral)
{
   size_t count  = mMessageATO->GetNumberOfSets();
   size_t count1 = mMessageACO->GetNumberOfSets();

   EXPECT_EQ(count, 161);
   EXPECT_EQ(count1, 45);
}

TEST_F(MessageTest, IsValidReturnTrueForBase)
{
   bool validity = mMessageATO->IsValid();
   mMessageATO->LogErrors(std::cout);
   bool validity1 = mMessageACO->IsValid();

   EXPECT_TRUE(validity);
   EXPECT_TRUE(validity1);
}

TEST_F(MessageTest, GetFieldGetsField)
{
   std::string field_value  = mMessageATO->GetSet(3).GetType();
   std::string field_value2 = mMessageACO->GetSet(3).GetType();

   EXPECT_EQ(field_value, "TIMEFRAM");
   EXPECT_EQ(field_value2, "REF");
}

TEST_F(MessageTest, EmptyFunctions)
{
   bool empty  = mMessageATO->IsEmpty();
   bool empty2 = mMessageACO->IsEmpty();

   EXPECT_FALSE(empty);
   EXPECT_FALSE(empty2);
}

TEST_F(MessageTest, EmptyReturnsTrueForNoSets)
{
   std::vector<std::unique_ptr<usmtf::Set>> empty{};
   usmtf::Message                           m("ATO", std::move(empty));
   ASSERT_TRUE(m.IsEmpty());
}

TEST_F(MessageTest, EmptySetsThrowsOnInvalidIndexEmpty)
{
   std::vector<std::unique_ptr<usmtf::Set>> empty{};
   usmtf::Message                           m("ATO", std::move(empty));
   ASSERT_THROW(m.GetSet(12), usmtf::MessageError);
}

TEST_F(MessageTest, EmptySetsThrowsOnInvalidIndexNotEmpty)
{
   auto                                     s = ut::make_unique<usmtf::Set>(usmtf::Set("MSGID", {}));
   std::vector<std::unique_ptr<usmtf::Set>> v;
   v.push_back(std::move(s));
   usmtf::Message m("ATO", std::move(v));
   ASSERT_THROW(m.GetSet(12), usmtf::MessageError);
}

TEST_F(MessageTest, DerivedMessagePassesWithCustomValidation)
{
   // should be polymorphic because our fixture registers UDM type.
   bool isValid = mMessageUD->IsValid();
   ASSERT_TRUE(isValid);
}

TEST_F(MessageTest, DerivedMessageFailsWithCustomValidationForUnregisteredSet)
{
   // Not using fixture, as its constructing valid state. I wish to override that with invalid examples
   const std::string EXAMPLE_UDM = mDataService.getResource("Example_UDM.txt").GetNormalizedPath();
   // This will make it so that I can simulate not having a set type the
   // message needs.
   sFact->UnregisterEntity("UDMS");

   // rerunning so set will be base instance now, not UDMS
   usmtf::USMTF_Parser parserUDM = usmtf::USMTF_Parser(EXAMPLE_UDM);
   mMessageUD                    = parserUDM.ReadMessage();

   bool isValid = mMessageUD->IsValid();
   ASSERT_FALSE(isValid);
}

TEST_F(MessageTest, DerivedMessageFailsWithCustomValidationForMalformedSet)
{
   // Not using fixture, as its constructing valid state. I wish to override that with invalid examples.
   const std::string EXAMPLE_UDM_MALFORMED_SET =
      mDataService.getResource("Example_UDM_Malformed_Set.txt").GetNormalizedPath();
   // rerunning so set will be base instance now, not UDMS
   usmtf::USMTF_Parser parserUDM = usmtf::USMTF_Parser(EXAMPLE_UDM_MALFORMED_SET);
   mMessageUD                    = parserUDM.ReadMessage();

   bool isValid = mMessageUD->IsValid();
   ASSERT_FALSE(isValid);
}

TEST_F(MessageTest, DerivedMessageFailsWithCustomValidationForMissingSet)
{
   // Not using fixture, as its constructing valid state. I wish to override that with invalid examples.
   const std::string EXAMPLE_UDM_MISSING_SET = mDataService.getResource("Example_UDM_Missing_Set.txt").GetNormalizedPath();
   usmtf::USMTF_Parser parserUDM             = usmtf::USMTF_Parser(EXAMPLE_UDM_MISSING_SET);
   mMessageUD                                = parserUDM.ReadMessage();

   bool isValid = mMessageUD->IsValid();
   ASSERT_FALSE(isValid);
}

TEST_F(MessageTest, DerivedMessagePassesValidationAndInterpretsData)
{
   bool isValid = mMessageUD->IsValid();
   EXPECT_TRUE(isValid);

   const UserDefinedMessage* ptr = mFact->CastIfRegistered<UserDefinedMessage>(*mMessageUD.get());
   EXPECT_FALSE(ptr->HasAnsweredYes());
}
