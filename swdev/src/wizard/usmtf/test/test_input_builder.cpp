// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************


#include <fstream>
#include <stdio.h>

#include <gtest/gtest.h>

#include "InputBuilder.hpp"
#include "USMTF_Transformer.hpp"
#include "UtLocator.hpp"
#include "UtTestDataService.hpp"
#include "test_user_defined_types.hpp"
#include "test_utils.hpp"

class TranformExample : public usmtf::USMTF_Transformer
{
public:
   explicit TranformExample(const usmtf::Segment::SetLocations& aLocations)
      : USMTF_Transformer(usmtf::Segment{aLocations})
   {
   }
   usmtf::InputBlock GetInputBlock() const override
   {
      usmtf::InputBlock         human{"human", "HUMAN"};
      usmtf::InputBlockProperty money{"money", "0", " "};
      usmtf::InputBlockProperty happiness{"happiness", "=)", " "};
      human.AddProperty(money);
      human.AddProperty(happiness);
      return human;
   }
};

TEST(InputBlock, OutputsCompleteBlock)
{
   usmtf::InputBlockProperty health{"health", "12"};
   usmtf::InputBlockProperty money{"money", "0"};
   usmtf::InputBlockProperty happiness{"happiness", "=)"};


   usmtf::InputBlock human{"human", "HUMAN"};
   human.AddProperty(health);
   human.AddProperty(money);
   human.AddProperty(happiness);
   std::string expectedBlock = "human HUMAN\n   health = 12\n   money = 0\n   happiness = =)\nend_human\n";
   std::string actualBlock   = human.GetBlock();

   ASSERT_EQ(actualBlock, expectedBlock);
}

TEST(InputBlock, OutputsCompleteBlockSpaces)
{
   usmtf::InputBlockProperty money{"money", "0", " "};
   usmtf::InputBlockProperty happiness{"happiness", "=)", " "};


   usmtf::InputBlock human{"human", "HUMAN"};
   human.AddProperty(money);
   human.AddProperty(happiness);
   std::string expectedBlock = "human HUMAN\n   money 0\n   happiness =)\nend_human\n";
   std::string actualBlock   = human.GetBlock();

   ASSERT_EQ(actualBlock, expectedBlock);
}

TEST(InputBlockProperty, OutputsPropertyProperly)
{
   usmtf::InputBlockProperty health{"health", "12"};
   usmtf::InputBlockProperty money{"money", "0"};
   usmtf::InputBlockProperty happiness{"happiness", "=)"};

   EXPECT_EQ(health.GetProperty(), "health = 12");
   EXPECT_EQ(money.GetProperty(), "money = 0");
   EXPECT_EQ(happiness.GetProperty(), "happiness = =)");
}

TEST(InputBuilder, TakesVectorOfInputBlocks)
{
   const ut::TestDataService& dataService    = ut::Locator<ut::TestDataService>::get();
   const std::string          SCRIPT_BUILDER = dataService.getResource("Script_Builder_Simple.txt").GetNormalizedPath();
   const std::string   SCRIPT_BUILDER_TEMP   = dataService.getResource("Script_Builder_Temp.txt").GetNormalizedPath();
   usmtf::InputBuilder sb{SCRIPT_BUILDER_TEMP};
   usmtf::InputBlockProperty money{"money", "0", " "};
   usmtf::InputBlockProperty happiness{"happiness", "=)", " "};


   usmtf::InputBlock human{"human", "HUMAN"};
   usmtf::InputBlock human1{"human", "HUMAN"};
   human.AddProperty(money);
   human.AddProperty(happiness);
   human1.AddProperty(money);
   human1.AddProperty(happiness);
   sb.AddBlocks({human, human1});
   sb.Export();

   std::ifstream     test(SCRIPT_BUILDER);
   std::ifstream     temp(SCRIPT_BUILDER_TEMP);
   std::stringstream errStream{};
   auto              result = AreFilesIdentical(test, temp);
   errStream << "Mismatch at position: " + std::to_string(result.position) + "\n";
   errStream << "Char was: " + std::string(1, result.actual) + "\n";
   errStream << "Char expected: " + std::string(1, result.expected) << std::endl;

   std::string errMessage = errStream.str();

   EXPECT_TRUE(result.wasIdentical) << errMessage; // err only show on fail.

   temp.close();
   test.close();
   std::remove(SCRIPT_BUILDER_TEMP.c_str());
}

TEST(InputBuilder, TakesVectorOfTransforms)
{
   const ut::TestDataService& dataService    = ut::Locator<ut::TestDataService>::get();
   const std::string          SCRIPT_BUILDER = dataService.getResource("Script_Builder_Simple.txt").GetNormalizedPath();
   const std::string SCRIPT_BUILDER_TEMP     = dataService.getResource("Script_Builder_Temp.txt").GetNormalizedPath();


   usmtf::InputBuilder                   sb{SCRIPT_BUILDER_TEMP};
   UserDefinedSet                        set{"BLAST", {usmtf::Field{"Field"}}};
   UserDefinedSet                        set1{"BLAST", {usmtf::Field{"Field"}}};
   usmtf::Segment::SetLocation           sl{set, 1};
   usmtf::Segment::SetLocation           sl1{set1, 2};
   TranformExample                       t{{sl, sl1}};
   TranformExample                       t1{{sl, sl1}};
   std::vector<usmtf::Exportable const*> p{&t, &t1};
   sb.AddBlocks(p);

   sb.Export();

   std::ifstream     test(SCRIPT_BUILDER);
   std::ifstream     temp(SCRIPT_BUILDER_TEMP);
   std::stringstream errStream{};
   auto              result = AreFilesIdentical(test, temp);
   errStream << "Mismatch at position: " + std::to_string(result.position) + "\n"
             << "Char was: " + std::string(1, result.actual) + "\n"
             << "Char expected: " + std::string(1, result.expected) << std::endl;

   std::string errMessage = errStream.str();

   EXPECT_TRUE(result.wasIdentical) << errMessage; // err only show on fail.

   temp.close();
   test.close();
   std::remove(SCRIPT_BUILDER_TEMP.c_str());
}

TEST(InputBuilder, OutputsCompleteScript)
{
   const ut::TestDataService& dataService    = ut::Locator<ut::TestDataService>::get();
   const std::string          SCRIPT_BUILDER = dataService.getResource("Script_Builder_Nested.txt").GetNormalizedPath();
   const std::string   SCRIPT_BUILDER_TEMP   = dataService.getResource("Script_Builder_Temp.txt").GetNormalizedPath();
   usmtf::InputBuilder sb{SCRIPT_BUILDER_TEMP};

   usmtf::InputBlock         human{"human", "HUMAN_KIND"};
   usmtf::InputBlockProperty health{"health", "12"};
   usmtf::InputBlockProperty money{"money", "0"};
   usmtf::InputBlockProperty happiness{"happiness", "=)"};
   human.AddProperty(health);
   human.AddProperty(money);
   human.AddProperty(happiness);
   sb.AddBlock(human);

   usmtf::InputBlock         elf{"elf", "ELF_KIND"};
   usmtf::InputBlockProperty ehealth{"health", "2"};
   usmtf::InputBlockProperty emoney{"money", "22"};
   usmtf::InputBlockProperty ehappiness{"happiness", "=("};
   elf.AddProperty(ehealth);
   elf.AddProperty(emoney);
   elf.AddProperty(ehappiness);

   usmtf::InputBlock         kid{"kid", "KID"};
   usmtf::InputBlockProperty age{"age", "4"};
   usmtf::InputBlockProperty curiosity{"curiosity", "8"};
   kid.AddProperty(age);
   kid.AddProperty(curiosity);

   usmtf::InputBlock         pet{"pet", "CAT", true};
   usmtf::InputBlockProperty name{"name", "slinky"};
   pet.AddProperty(name);

   kid.AddBlock(pet);
   elf.AddBlock(kid);
   sb.AddBlock(elf);

   sb.Export();


   std::ifstream     test(SCRIPT_BUILDER);
   std::ifstream     temp(SCRIPT_BUILDER_TEMP);
   std::stringstream errStream{};
   auto              result = AreFilesIdentical(test, temp);
   errStream << "Mismatch at position: " + std::to_string(result.position) + "\n"
             << "Char was: " + std::string(1, result.actual) + "\n"
             << "Char expected: " + std::string(1, result.expected) << std::endl;

   std::string errMessage = errStream.str();

   EXPECT_TRUE(result.wasIdentical) << errMessage; // err only show on fail.

   temp.close();
   test.close();
   std::remove(SCRIPT_BUILDER_TEMP.c_str());
}
