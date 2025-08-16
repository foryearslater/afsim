// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
for(var test in package_test)
{
   if (package_test[test].runTest())
   {
      print("-PASS- No Errors detected");
   }
   else
   {
      print ("-FAIL- Errors detected");
   }
   IDE.Test.dumpToConsole();
   IDE.Test.dumpToCsv(package_test[test].name + ".csv");
   IDE.console().printConsoleToFile(package_test[test].name + ".out");
}
IDE.shutdown();
