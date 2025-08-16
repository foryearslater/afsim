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
#include "IntervalFrequency.hpp"


TEST(IntervalFrequencyValidatorTests, IsValidForValidIFDaily)
{
   const usmtf::Field       f{"DAILY"};
   usmtf::IntervalFrequency ivf;
   ivf.ParseAndValidate(f);
   EXPECT_TRUE(ivf.IsValid());
   EXPECT_EQ(ivf.GetIntervalFrequency(), "DAILY");
   ivf.LogErrors(std::cout);
}

TEST(IntervalFrequencyValidatorTests, IsInValidForInValidIF)
{
   const usmtf::Field       f{"NOTVALID"};
   usmtf::IntervalFrequency ivf;
   ivf.ParseAndValidate(f);
   EXPECT_FALSE(ivf.IsValid());
   EXPECT_EQ(ivf.GetIntervalFrequency(), "");
   ivf.LogErrors(std::cout);
}
