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

#include <fstream>

#include <gtest/gtest.h>

#include "UtPath.hpp"
#include "ViFileWatcher.hpp"

TEST(ViFileWatcher, watch)
{
   std::ofstream tmpFile("tmp.txt");
   tmpFile << "temporary file created by test_vifilewatcher" << std::endl;
   ViFileWatcher testWatcher;
   size_t        modifications = 0;
   UtPath        tmpPath("tmp.txt");
   testWatcher.WatchFile(tmpPath).Connect([&](const UtPath&) { ++modifications; });
   EXPECT_EQ(modifications, 0);
   for (size_t i = 0; i < 1000; ++i)
   {
      tmpFile << "add line: " << i << std::endl;
      testWatcher.CheckForModifications();
      EXPECT_EQ(modifications, i + 1);
   }
   tmpPath.RemoveFile();
}
