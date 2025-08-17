.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

SAM_LAUNCH_COMPUTER_GENERATOR
-----------------------------

.. model:: tool SAM_LAUNCH_COMPUTER_GENERATOR

.. parsed-literal::

   tool SAM_LAUNCH_COMPUTER_GENERATOR
      ... :command:`weapon_tools` Commands ...
      ... :model:`WSF_SAM_LAUNCH_COMPUTER` Commands ...
      
      ... test_matrix_ ... end_test_matrix ...
      ... post_generation_interpolation_tests_ ... end_post_generation_interpolation_tests ...
           
      re_parse_produced_input_file_
      
      // Tracker Information
      tracker_height_ ...
      tracker_name_ ...
      tracker_elevation_limits_ ...
      effective_earth_radius_ ...
      
   end_tool

Overview
========

SAM_LAUNCH_COMPUTER_GENERATOR_ is a specialization of :command:`weapon_tools` to produce a launch computer used to fire
surface-to-air missiles.

Commands
========

.. command:: test_matrix ... end_test_matrix
   :block:

   Within this block are delineated the list of target track test conditions to be evaluated for successful missile
   intercept.  Four different arrays are to be provided:  lateral_offsets_, altitudes_, speeds_,
   and ranges_.  Each of the four arrays may be set by using the following syntax:  a) from <start-value> to
   <end-value> by <delta-value>, or b) a listing, such as <value-1> <value-2> <value-3> ... <end-value>.  All values must
   be in increasing order, but the intervals need not be even, and the from-to-by syntax and enumerated list may be used
   in combination.

   .. command:: altitudes <distance-values> end_altitudes

      The altitudes at which tracks are evaluated for intercept. Format as given in test_matrix_ above.

   .. command:: lateral_offsets <altitude-values> end_lateral_offsets

      The lateral_offsets at which tracks are evaluated for intercept. Format as given in test_matrix_ above.

   .. command:: ranges <range-values> end_ranges

      The target ground ranges at which tracks are evaluated for intercept. Format as given in test_matrix_ above.

   .. command:: speeds <speed-values> end_speeds

      The speeds at which tracks are evaluated for intercept. Format as given in test_matrix_ above.

.. command:: post_generation_interpolation_tests  ... end_post_generation_interpolation_tests
   :block:

   For debug purposes only, and is optional.  Inside this block one may place one or more lateral_offset_,
   altitude_, ground_range_, and target_speed_ specifications.  Each time the keyword 'test' appears,
   the most recent value of (offset, altitude, speed, range) is saved off as a test point, and stored into a test point
   array.  After the launch computer is generated, each of these test points will be evaluated, and the corresponding
   time-of-flight value will be printed to the screen.  The example given below saves off three points for testing.

   .. command:: lateral_offset <length-value>

      Specify the track lateral_offset of the interpolation test point.

   .. command:: altitude <length-value>

      Specify the track altitude of the interpolation test point.

   .. command:: ground_range <length-value>

      Specify the track ground_range of the interpolation test point.

   .. command:: target_speed <speed-value>

      Specify the track target speed of the interpolation test point.

   .. command:: test

      Save off the most recent values for (offset, altitude, range, and speed), and store them as a point to be interpolation
      tested.  Results will be printed to the screen.

.. command:: re_parse_produced_input_file

   After generating and writing the launch computer, re-open the file and attempt to read it back in through normal stream
   input processing.  The file is not checked by default.

.. command:: tracker_height <length-value>

   Set the height above ground for a tracking radar.  This is used in horizon masking calculations, which will disable
   weapon launch for a target engagement that is masked by curvature of earth.  If no value is input for this parameter,
   horizon masking is not considered, and missile kinematics alone are used to calculate a launch envelope.
   
.. command:: tracker_name <sensor-name>

   Alternate way to set the tracker height by specifying the sensor's name. If a tracker height wasn't specified and tracker name was, 
   the tracker height is resolved from its definition. The specified sensor must be present on the launching platform.

.. command:: tracker_elevation_limits <angle-value> <angle-value>

   Set the minimum and maximum elevation angles at which the tracking radar will function.  Target elevation angles
   outside these limits will inhibit weapon launch.

   **Default:**  -180 deg, + 180 deg (unlimited)
   
.. command:: effective_earth_radius <ratio-value>

   Set the effective earth radius ratio.  Must be greater than zero.

   **Default:**  4/3   

Examples
========

test_matrix example
*******************

::

   test_matrix
      lateral_offsets ... end_lateral_offsets
      altitudes ... end_altitudes
      speeds ... end_speeds
      ranges ... end_ranges
   end_test_matrix

interpolation_test example
**************************

::

   post_generation_interpolation_tests
      lateral_offset 4000 m
      altitude       2000 m
      ground_range  10000 m
      target_speed    120 m/s
              test
      ground_range  20000 m
              test
      lateral_offset 2000 m
              test
   end_post_generation_interpolation_tests

complete input file example
***************************

.. parsed-literal::

   # include the explicit weapon
   include ../../data/weapons/sam_missile.txt

   weapon_effects WEAPON_TOOL_LETHALITY WSF_GRADUATED_LETHALITY
      radius_and_pk 65.0 m 1.00 # All Platforms
   end_weapon_effects

   platform_type LAUNCH_PLATFORM_TYPE WSF_PLATFORM
     icon SA-10_Launcher
     weapon launching_weapon SAM_MISSILE end_weapon
   end_platform_type

   platform_type TARGET_PLATFORM_TYPE WSF_PLATFORM
     icon F-18
     mover WSF_AIR_MOVER end_mover
   end_platform_type

   tool SAM_LAUNCH_COMPUTER_GENERATOR
     tool_debug
     position      34:54n 117:53w
     output_object_name SAM_MISSILE_LAUNCH_COMPUTER
     output_file_name   sam_missile_launch_computer.txt
     tracker_height 2.5 m
     test_matrix
        altitudes
           from 1000 m to 5000 m by 1000 m
        end_altitudes
        lateral_offsets
           from 2 km to 8 km by 2 km  15 km 20 km
        end_lateral_offsets
        ranges
           from 0 km to 100 km by 2 km
           150 km 200 km 280 km
        end_ranges
        speeds
          from 0 m/s to 750 m/s by 50 m/s
        end_speeds
     end_test_matrix
   end_tool

   dis_interface
     record lc_generation.rep
   end_dis_interface
