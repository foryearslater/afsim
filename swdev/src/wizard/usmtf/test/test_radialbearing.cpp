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
#include "RadialBearing.hpp"


TEST(ReadialBearingValidatorTests, IsValidForValidRadialBearing)
{
   const usmtf::Field   f{"270T"};
   usmtf::RadialBearing rb{};
   rb.ParseAndValidate(f);
   EXPECT_TRUE(rb.IsValid());
   EXPECT_EQ(rb.GetBearing(), "270");
   rb.LogErrors(std::cout);
}

TEST(IntervalFrequencyValidatorTests, IsInValidForInValidRadialBearing)
{
   const usmtf::Field   f{"NOTVALID"};
   usmtf::RadialBearing rb{};
   rb.ParseAndValidate(f);
   EXPECT_FALSE(rb.IsValid());
   EXPECT_EQ(rb.GetBearing(), "");
   rb.LogErrors(std::cout);
}
