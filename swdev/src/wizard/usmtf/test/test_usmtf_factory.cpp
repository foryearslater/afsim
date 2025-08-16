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

#include "Message.hpp"
#include "Set.hpp"
#include "USMTF_Factory.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtMemory.hpp"
#include "UtTestDataService.hpp"
#include "test_user_defined_types.hpp"

class USMTF_FactoryTest : public ::testing::Test
{
protected:
   void SetUp() override
   {
      // Register with factory
      mFact = usmtf::MessageFactory::GetFactory();
      sFact = usmtf::SetFactory::GetFactory();
      mFact->RegisterEntity("UDM", usmtf::MessageFactory::Construct<UserDefinedMessage>);
      sFact->RegisterEntity("UDMS", usmtf::SetFactory::Construct<UserDefinedSet>);
      const std::string   EXAMPLE_UDM = mDataService.getResource("Example_UDM.txt").GetNormalizedPath();
      usmtf::USMTF_Parser parser      = usmtf::USMTF_Parser(EXAMPLE_UDM);
      mMessage                        = parser.ReadMessage();
   }

   // void TearDown() override {}
   usmtf::SetFactory*              sFact;
   usmtf::MessageFactory*          mFact;
   std::unique_ptr<usmtf::Message> mMessage;
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
};

TEST_F(USMTF_FactoryTest, FactoryUsedUserDefineMessageClass)
{
   UserDefinedMessage* ptr = dynamic_cast<UserDefinedMessage*>(mMessage.get());
   ASSERT_FALSE(ptr == nullptr); // factory cast works.
}

TEST_F(USMTF_FactoryTest, TwoCallsToGetFactoryReturnSameFactory)
{
   auto messageFactory  = usmtf::MessageFactory::GetFactory();
   auto messageFactory1 = usmtf::MessageFactory::GetFactory();
   EXPECT_EQ(messageFactory, messageFactory1);
   EXPECT_EQ(mFact, messageFactory);
   EXPECT_EQ(mFact, messageFactory1);
}

TEST_F(USMTF_FactoryTest, FactoryUsedUserDefineSetClass)
{
   UserDefinedMessage*   ptr  = dynamic_cast<UserDefinedMessage*>(mMessage.get());
   const usmtf::Set&     set  = ptr->GetSet(2);
   const UserDefinedSet* sptr = dynamic_cast<const UserDefinedSet*>(&set);
   ASSERT_FALSE(sptr == nullptr); // factory cast works.
}

TEST_F(USMTF_FactoryTest, MessageFactoryCastIfRegisteredIsFunctional)
{
   const UserDefinedMessage* ptr = mFact->CastIfRegistered<UserDefinedMessage>(*mMessage.get());
   ASSERT_FALSE(ptr == nullptr); // factory cast works.
}

TEST_F(USMTF_FactoryTest, SetFactoryCastIfRegisteredIsFunctional)
{
   const UserDefinedMessage* ptr  = mFact->CastIfRegistered<UserDefinedMessage>(*mMessage.get());
   const usmtf::Set&         set  = ptr->GetSet(2);
   const UserDefinedSet*     sptr = sFact->CastIfRegistered<UserDefinedSet>(set);
   ASSERT_FALSE(sptr == nullptr); // factory cast works.
}

TEST_F(USMTF_FactoryTest, SetFactoryCastIfRegisteredReturnsNullptrForFailedCast)
{
   class FOO
   {
   public:
      FOO(const std::string& aType, const std::vector<usmtf::Field>& aFields) {}
   };

   const UserDefinedMessage* ptr = mFact->CastIfRegistered<UserDefinedMessage>(*mMessage.get());
   const usmtf::Set&         set = ptr->GetSet(2); // Has a constructor for UDMS that creates a UserDefinedSet
   // This cast is invalid
   const FOO* sptr = sFact->CastIfRegistered<FOO>(set);
   ASSERT_TRUE(sptr == nullptr);
}
