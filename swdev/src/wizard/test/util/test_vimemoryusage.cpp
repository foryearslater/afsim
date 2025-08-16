// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <array>

#include <gtest/gtest.h>

#include "UtMemory.hpp"
#include "ViMemoryUsage.hpp"

TEST(ViMemoryUsage, DISABLED_IncreasingMemory)
{
   ViMemoryUsage                             testUsage;
   auto                                      baseLine = testUsage.GetMemoryUsageInBytes();
   std::array<std::unique_ptr<char[]>, 1024> data;
   // This loop should allocate 1MB
   for (size_t i = 0; i < data.size(); ++i)
   {
      data[i] = ut::make_unique<char[]>(size_t{1024});
      ASSERT_LE(baseLine, testUsage.GetMemoryUsageInBytes());
   }
}

TEST(ViMemoryUsage, DISABLED_DecreasingMemory)
{
   ViMemoryUsage                             testUsage;
   std::array<std::unique_ptr<char[]>, 1024> data;
   for (size_t i = 0; i < data.size(); ++i)
   {
      data[i] = ut::make_unique<char[]>(size_t{1024});
   }
   auto baseLine = testUsage.GetMemoryUsageInBytes();
   for (size_t i = 0; i < 1024; ++i)
   {
      data[i].reset(nullptr);
   }
   ASSERT_GT(baseLine, testUsage.GetMemoryUsageInBytes());
}

TEST(ViMemoryUsage, GetMemoryUsageString)
{
   const double kb = 1024.0;
   const double mb = 1024.0 * kb;
   const double gb = 1024.0 * mb;
   EXPECT_EQ("1 B", ViMemoryUsage::GetMemoryUsageString(1));
   EXPECT_EQ("1023 B", ViMemoryUsage::GetMemoryUsageString(1023));
   EXPECT_EQ("1 KB", ViMemoryUsage::GetMemoryUsageString(kb));
   EXPECT_EQ("1.5 KB", ViMemoryUsage::GetMemoryUsageString(1.5 * kb));
   EXPECT_EQ("1023 KB", ViMemoryUsage::GetMemoryUsageString(1023 * kb));
   EXPECT_EQ("1 MB", ViMemoryUsage::GetMemoryUsageString(mb));
   EXPECT_EQ("1.5 MB", ViMemoryUsage::GetMemoryUsageString(1.5 * mb));
   EXPECT_EQ("1 GB", ViMemoryUsage::GetMemoryUsageString(gb));
   EXPECT_EQ("1.5 GB", ViMemoryUsage::GetMemoryUsageString(1.5 * gb));
   EXPECT_EQ("1025 GB", ViMemoryUsage::GetMemoryUsageString(1025 * gb));
}
