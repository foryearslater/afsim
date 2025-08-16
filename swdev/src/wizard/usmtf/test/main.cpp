// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include <gtest/gtest.h>

#include "UtLocator.hpp"
#include "UtMemory.hpp"
#include "UtPath.hpp"
#include "UtTestDataService.hpp"

int main(int argc, char* argv[])
{
   ::testing::InitGoogleTest(&argc, argv);
   if (argc < 2)
   {
      std::cerr << "No message_examples path supplied" << std::endl;
      return 1;
   }

   const std::string examples_path   = argv[1];
   auto              testDataService = std::make_shared<ut::TestDataService>(UtPath(argv[1]));
   ut::Locator<ut::TestDataService>::provide(testDataService);
   return RUN_ALL_TESTS();
}
