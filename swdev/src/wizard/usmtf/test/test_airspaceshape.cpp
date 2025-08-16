// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <gtest/gtest.h>

#include "AirspaceShape.hpp"
#include "Field.hpp"


TEST(AirspaceShapeValidatorTests, IsValidForValidShape)
{
   const usmtf::Field   f{"CORRIDOR"};
   usmtf::AirspaceShape as{};
   as.ParseAndValidate(f);
   EXPECT_TRUE(as.IsValid());
   EXPECT_EQ(as.GetShape(), "CORRIDOR");
   as.LogErrors(std::cout);
}

TEST(AirspaceShapeValidatorTests, IsInValidForInValidShape)
{
   const usmtf::Field   f{"TESSERACT"};
   usmtf::AirspaceShape as{};
   as.ParseAndValidate(f);
   EXPECT_FALSE(as.IsValid());
   EXPECT_EQ(as.GetShape(), "");
   as.LogErrors(std::cout);
}
