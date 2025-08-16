.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

.. _Scenario_Analyzer_Code_Samples:

|  :doc:`Scenario Analyzer Tool Table of Contents <wiz_scenario_analyzer>`
|  :doc:`Previous: Adding New Scenario Analyzer Checks <scenario_analyzer_add_checks_tutorial>`

Appendix: Sample Code from New Checks Tutorial
==============================================

.. _Demo_1:

Demo 1: Sensors should be linked to track processors (simplified)
--------------------------------------------------------------------

::

 script void Sensors_should_be_linked_to_track_processors()
   string suite = "demo";
   string check = "Sensors should be linked to track processors";
   string severity = "WARNING";
   string message = "This is just a test";
   
   bool checkPassed = true;
   
   for (int i = 0; i != WsfSimulation.PlatformCount(); i += 1)
   {
      // For every sensor on each platform...
      WsfPlatform platform = WsfSimulation.PlatformEntry(i);
      for (int j = 0; j != platform.SensorCount(); j += 1)
      {
         WsfSensor sensor = platform.SensorEntry(j);
         
         // ... find all track processors that it can reach.
         // Do NOT follow external links from WsfTrackProcessors
         // (which may have a "type" of either WSF_TRACK_PROCESSOR or WSF_TRACK_MANAGER)
         Array<string> ignoreExternalLinks = Array<string>();
         ignoreExternalLinks.PushBack("WSF_TRACK_PROCESSOR");
         ignoreExternalLinks.PushBack("WSF_TRACK_MANAGER");
         
         // Track processors may have a "type" of either WSF_TRACK_PROCESSOR or
         // WSF_TRACK_MANAGER: Look for both, since a link to either is sufficient.
         Array<WsfPlatformPart> linkedTrackProcs = 
            ScenarioAnalyzerUtils.LinkedAndReachablePlatformPartsChooseProcs(sensor,      
            "WSF_TRACK_PROCESSOR", ignoreExternalLinks, false);
         
         Array<WsfPlatformPart> linkedTrackMgrs = 
            ScenarioAnalyzerUtils.LinkedAndReachablePlatformPartsChooseProcs(sensor, 
            "WSF_TRACK_MANAGER", ignoreExternalLinks, false);
 
         // sensor fails the Check if it is not linked to any WsfTrackProcessor
         if (linkedTrackProcs.Size() <= 0 && linkedTrackMgrs.Size() <= 0)
         {
            checkPassed = false;
            
            Array<string> fileLocationTypes = Array<string>();
            Array<string> fileLocationNames = Array<string>();
            
            fileLocationTypes.PushBack("platform");
            fileLocationNames.PushBack(platform.Name());
            
            fileLocationTypes.PushBack("sensors");
            fileLocationNames.PushBack(sensor.Name());
            
            string message = "Sensor " + sensor.Name() + " on platform " + platform.Name() + 
               " is not linked directly or indirectly to a track processor";
            ScenarioAnalyzerUtils.SendResultMessageWithLinks(suite, check, severity, 
               message, fileLocationTypes, fileLocationNames);
         }
      }
   }
   
   if (checkPassed)
   {
      ScenarioAnalyzerUtils.SendPassMessage(suite, check);
   }
 end_script

.. _Demo_2:

Demo 2: Sensors manager platforms must be linked to battle managers with commit authority
--------------------------------------------------------------------------------------------

**check_suites/includes/helpers.txt** (Must be included in demos.txt)

::

 // Helper script that will be passed to ScenarioAnalyzerUtils.CheckUpOneCommandChain()
 
 script bool PlatformHasBMWithCommitAuthority(WsfPlatform p)
   for (int i = 0, proc_count = p.ProcessorCount(); i < proc_count; i += 1)
   {
      WsfProcessor proc = p.ProcessorEntry(i);
      if (proc.IsA_TypeOf("WSF_UNCLASS_BM"))
      {
         WsfUnclassBM bm = (WsfUnclassBM)proc;
         return bm.HasCommitAuthority();
      }
   }
   return false;
 end_script

**check_suites/demos.txt**

::

 include_once includes/helpers.txt
 
 script void SM_platforms_must_be_connected_to_BM_with_commit_authority()
   string suite = "demo";
   string check = "SM platforms must be connected to BM with commit authority";
   string severity = "ERROR";
 
   bool check_passed = true;
 
   int platform_count = WsfSimulation.PlatformCount();
   for (int i = 0; i < platform_count; i += 1)
   {
      WsfPlatform platform = WsfSimulation.PlatformEntry(i);
 
      for (int j = 0, proc_count = platform.ProcessorCount(); j < proc_count; j += 1)
      {
         // If processor is a sensors manager...
         WsfProcessor proc = platform.ProcessorEntry(j);
         if (proc.IsA_TypeOf("WSF_SENSORS_MANAGER"))
         {
 
            // ... ensure that at least one platform superior to this platform on 
            // default command chain has a battle manager with commit authority
            string script_name = "PlatformHasBMWithCommitAuthority";
            if (!ScenarioAnalyzerUtils.CheckUpOneCommandChain(platform, script_name, "default"))
            {
               // If no BM with commit authority is found, send a result message
               // describing the check failure.
               check_passed = false;
 
               Array<string> fileLocationTypes = Array<string>();
               Array<string> fileLocationNames = Array<string>();
 
               fileLocationTypes.PushBack("platform");
               fileLocationNames.PushBack(platform.Name());
 
               fileLocationTypes.PushBack("processor");
               fileLocationNames.PushBack(proc.Name());
 
               string message = "Platform " + platform.Name() +  " deploys a sensors manager, " + 
                  "but neither this platform nor any platform above it in the default " + 
                  "command chain deploys a battle manager with commit authority";
 
               ScenarioAnalyzerUtils.SendResultMessageWithLinks(suite, check, severity, message, 
                  fileLocationTypes, fileLocationNames);
            }
         }
      }
   }
 
   if (check_passed)
   {
      ScenarioAnalyzerUtils.SendPassMessage(suite, check);
   }
 
 end_script

:doc:`Previous: Adding New Scenario Analyzer Checks <scenario_analyzer_add_checks_tutorial>`
