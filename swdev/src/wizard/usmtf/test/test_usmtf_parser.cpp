// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <gtest/gtest.h>

#include "USMTF_Exceptions.hpp"
#include "USMTF_Parser.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"

// No fixture because parsing is the point of the module and these tests.

TEST(USMTF_ParserTests, ParsesNonMalformedUSMTFMessage)
{
   usmtf::MessageFactory::GetFactory()->UnregisterEntity("ACO");
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
   const std::string               EXAMPLE_ACO  = mDataService.getResource("Example_ACO.txt").GetNormalizedPath();
   usmtf::USMTF_Parser             parser       = usmtf::USMTF_Parser(EXAMPLE_ACO);
   std::unique_ptr<usmtf::Message> baseMessage  = parser.ReadMessage();

   EXPECT_EQ(baseMessage->GetType(), "ACO");
   EXPECT_EQ(baseMessage->GetNumberOfSets(), 45);

   const usmtf::Set& final_set      = baseMessage->GetSet(baseMessage->GetNumberOfSets() - 1);
   std::string       final_set_type = final_set.GetType();

   EXPECT_EQ(final_set_type, "APERIOD"); // check final set.
   EXPECT_EQ(final_set.GetFieldCount(), 4);
   EXPECT_EQ(final_set.GetField(final_set.GetFieldCount() - 1).GetContent(), "UFN"); // check final set, final field
   EXPECT_TRUE(baseMessage->IsValid()); // All bases are valid. Validity come with concrete derived implementations.
   EXPECT_FALSE(baseMessage->IsEmpty());
   EXPECT_TRUE(final_set.IsValid());
   EXPECT_FALSE(final_set.IsEmpty());
}

TEST(USMTF_ParserTests, ParsesNonMalformedUSMTFMessageWithNullFields)
{
   usmtf::MessageFactory::GetFactory()->UnregisterEntity("ACO");
   const ut::TestDataService& mDataService  = ut::Locator<ut::TestDataService>::get();
   const std::string EXAMPLE_ACO_NULL_FIELD = mDataService.getResource("Example_ACO_Null_field.txt").GetNormalizedPath();
   usmtf::USMTF_Parser             parser   = usmtf::USMTF_Parser(EXAMPLE_ACO_NULL_FIELD);
   std::unique_ptr<usmtf::Message> baseMessage = parser.ReadMessage();

   EXPECT_EQ(baseMessage->GetType(), "ACO");
   EXPECT_EQ(baseMessage->GetNumberOfSets(), 45);

   const usmtf::Set& nullFieldSet     = baseMessage->GetSet(8);
   std::string       nullFieldSetType = nullFieldSet.GetType();

   EXPECT_TRUE(nullFieldSet.IsValid());
   EXPECT_EQ(nullFieldSetType, "ACMSTAT");
   EXPECT_EQ(nullFieldSet.GetFieldCount(), 9);
   EXPECT_EQ(nullFieldSet.GetField(3).GetContent(), "-");
   EXPECT_FALSE(baseMessage->IsEmpty());
   EXPECT_TRUE(baseMessage->IsValid()); // All bases are valid. Validity come with concrete derived implementations.
   EXPECT_FALSE(nullFieldSet.IsEmpty());
}

TEST(USMTF_ParserTests, ParsesNonMalformedUSMTFMessageATO)
{
   const ut::TestDataService&      mDataService = ut::Locator<ut::TestDataService>::get();
   const std::string               EXAMPLE_ATO  = mDataService.getResource("Example_ATO.txt").GetNormalizedPath();
   usmtf::USMTF_Parser             parser       = usmtf::USMTF_Parser(EXAMPLE_ATO);
   std::unique_ptr<usmtf::Message> baseMessage  = parser.ReadMessage();

   EXPECT_EQ(baseMessage->GetType(), "ATO");
   EXPECT_EQ(baseMessage->GetNumberOfSets(), 161);

   const usmtf::Set& final_set      = baseMessage->GetSet(baseMessage->GetNumberOfSets() - 1);
   std::string       final_set_type = final_set.GetType();

   EXPECT_EQ(final_set_type, "AIRMOVE"); // check final set.
   EXPECT_EQ(final_set.GetFieldCount(), 8);
   EXPECT_EQ(final_set.GetField(final_set.GetFieldCount() - 1).GetContent(), "TNN"); // check final set, final field
   EXPECT_TRUE(baseMessage->IsValid()); // All bases are valid. Validity come with concrete derived implementations.
   EXPECT_FALSE(baseMessage->IsEmpty());
   EXPECT_TRUE(final_set.IsValid());
   EXPECT_FALSE(final_set.IsEmpty());
}

TEST(USMTF_ParserTests, ThrowsExceptionForEmptyFile)
{
   std::string         file   = "";
   usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(file);
   ASSERT_THROW(parser.ReadMessage(), usmtf::ImportError);
}

TEST(USMTF_ParserTests, ThrowsExceptionForMissingExerOrOper)
{
   const ut::TestDataService& mDataService = ut::Locator<ut::TestDataService>::get();
   const std::string          EXAMPLE_MALFORMED_MISSING_EXER_OPER =
      mDataService.getResource("Example_Malformed_Missing_Exer_Oper.txt").GetNormalizedPath();
   usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(EXAMPLE_MALFORMED_MISSING_EXER_OPER);
   ASSERT_THROW(parser.ReadMessage(), usmtf::ImportError);
}

TEST(USMTF_ParserTests, ThrowsExceptionForMissingMsgID)
{
   const ut::TestDataService& mDataService = ut::Locator<ut::TestDataService>::get();
   const std::string          EXAMPLE_MALFORMED_MISSING_MSG_SET =
      mDataService.getResource("Example_Malformed_Missing_Msg_Set.txt").GetNormalizedPath();
   usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(EXAMPLE_MALFORMED_MISSING_MSG_SET);
   ASSERT_THROW(parser.ReadMessage(), usmtf::ImportError);
}


TEST(USMTF_ParserTests, ThrowsExceptionForMissingMandatoryNumber)
{
   const ut::TestDataService& mDataService = ut::Locator<ut::TestDataService>::get();
   const std::string          EXAMPLE_UDM_MISSING_MANDATORY_SETS =
      mDataService.getResource("Example_Malformed_Missing_Mandatory_Sets.txt").GetNormalizedPath();
   usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(EXAMPLE_UDM_MISSING_MANDATORY_SETS);
   ASSERT_THROW(parser.ReadMessage(), usmtf::ImportError);
}

TEST(USMTF_ParserTests, ThrowsExceptionForWrongPositionMsgID)
{
   const ut::TestDataService& mDataService = ut::Locator<ut::TestDataService>::get();
   const std::string          EXAMPLE_MALFORMED_WRONG_POSITION_MSG_SET =
      mDataService.getResource("Example_Malformed_Wrong_Position_Msg_Set.txt").GetNormalizedPath();
   usmtf::USMTF_Parser parser = usmtf::USMTF_Parser(EXAMPLE_MALFORMED_WRONG_POSITION_MSG_SET);
   ASSERT_THROW(parser.ReadMessage(), usmtf::ImportError);
}
