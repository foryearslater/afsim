// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

IDE.Console.clear();
IDE.Test.startTest("test1");
IDE.Console.help();
IDE.Console.printMemoryUsage();
for (var i = 0; i < 1000; i++)
{
   print(i);
}
IDE.Test.stopTest(true, "");

IDE.Console.clear();
IDE.Test.startTest("test2");
for (var i = 0; i < 1000; i++)
{
   print(i);
}
var pass = false;
var i = 1 + 2;
if (i == 3)
{
   print("success");
   pass = true;
}
IDE.Test.stopTest(pass, "");

IDE.Test.dumpToConsole();
IDE.Test.dumpToJUnit();
