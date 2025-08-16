// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <gtest/gtest.h>

#include "Field.hpp"
#include "FreeText.hpp"

TEST(FreeTextTests, IsValidForFreeText)
{
   const usmtf::Field f{"Random text that is free."};
   usmtf::FreeText    val{};
   val.ParseAndValidate(f);
   EXPECT_TRUE(val.IsValid());
   EXPECT_EQ(val.GetText(), "Random text that is free.");
   val.LogErrors(std::cout);
}

TEST(FreeTextTests, IsValidForEmpty)
{
   const usmtf::Field f{""};
   usmtf::FreeText    val{};
   val.ParseAndValidate(f);
   EXPECT_TRUE(val.IsValid());
   EXPECT_EQ(val.GetText(), "");
   val.LogErrors(std::cout);
}
