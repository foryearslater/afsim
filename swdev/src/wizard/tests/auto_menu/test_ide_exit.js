// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

(function(context){
   context.exit = function() {
      return {
         name:"test_ide_exit",
         runTest:function() {
            // output normal
            IDE.Test.startTest("test1.1");
            IDE.Test.dumpToConsole();
            IDE.console().printConsoleToFile("test_ide_exit.out");
            IDE.Test.dumpToCsv();

            IDE.shutdown();

            // output failure should not get here.
            IDE.Test.stopTest(false, "test1.1");
            IDE.Test.dumpToConsole();
            IDE.console().printConsoleToFile("test_ide_exit.out");
            IDE.Test.dumpToCsv();
         },
         type:"Test"
      };
   }();
})(package_test);

//Debug
//print (package_test.exit.runTest());
//print (package_test.exit.type);