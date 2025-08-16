// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

(function(context){
   context.help = function() {
      return {
         name:"test_ide_help",
         runTest:function() {
            IDE.Test.startTest("IDE.Help");
            var expectedString = "String help() - Returns the help string.\n" +
                                 "void print(object) - Prints the specified object.\n" +
                                 "ViScriptManager script() - Returns the script manager object.\n" +
                                 "ViTestSystem test() - Returns the test system object.\n" +
                                 "ViScriptConsoleControl console() - Returns the console object.\n" +
                                 "ViProjectWorkspace workspace() - Returns the workspace object.";
            var pass = (expectedString == IDE.help());
            IDE.Test.stopTest(pass, "IDE.Help");
            return pass;
         },
         type:"Test"
      };
   }();
})(package_test);

//Debug
//print(package_test.help.runTest());
//print(package_test.help.type);
