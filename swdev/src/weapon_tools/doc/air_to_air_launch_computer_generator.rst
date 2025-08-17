.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR
------------------------------------

.. model:: tool AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR

.. parsed-literal:: 

   tool AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR
    ... :command:`weapon_tools` Commands ...
    ... :ref:`AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR.Commands` Commands ...
    ... :model:`WSF_AIR_TO_AIR_LAUNCH_COMPUTER` Commands ...
   end_tool

Overview
========

AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR_ is a specialization of :command:`weapon_tools` to produce a
:model:`WSF_AIR_TO_AIR_LAUNCH_COMPUTER`.  The generator contains a :model:`WSF_AIR_TO_AIR_LAUNCH_COMPUTER` instance internally,
which is modified into a final output product so many of the commands are common between the two.  The launch computer
is created with a sizable matrix of air-to-air engagement conditions.  The Tool then traverses this matrix of
engagement conditions, placing an airborne shooter and target at selected conditions.  Engagement is repeated while
range is iterated upon to yield three values, a minimum, maximum, and a range of no-escape, and weapon times-of-flight
for each.  Iterations are a binary search to find the edge condition (range) at which the engagement transitions from
success to failure, or vice versa.  A weapon is fired at the target immediately after it is placed, and given a perfect
track to the target.  The missile flies out, and if kinematically possible, the target is intercepted and killed.  Note
that the Probability of Kill given sufficiently small miss distance, should be 1.0.  If the engagement results in a
target kill, the range and time of flight is saved into an array of results.  If the target is not killed (at any
range), a -1 is recorded.  The process is repeated for each engagement condition.  When all engagement results have
been recorded, the results are written into an WSF-input format text file, defining the conditions appropriate to
launch this weapon.

The engagement independent variables that affect lethal ranges in an air-to-air battle include shooter altitude and
Mach, target altitude and Mach, target Aspect angle, and target Lead angle.  Target aspect and lead are both measured
from velocity vector to line-of-sight to the other player in the engagement.  It is important to note that the matrix
of conditions to consider for engagement are held within the :model:`WSF_AIR_TO_AIR_LAUNCH_COMPUTER`, and that input
commands to it define the matrix of test conditions that this generator will consider.  This launch computer object is
aggregated within this generator object so it will accept those same commands as valid input.

.. warning::

   AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR_ does not support any launch platform with a deferred launch time.

.. _AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR.Commands:

COMMANDS
========

.. command:: minimum_range <distance-value>
   
   The shortest engagement range that will be tested. DEFAULT = 1000 meters.

.. command:: maximum_range <distance-value>
   
   The longest engagement range that will be tested. DEFAULT = 120,000 meters.

.. command:: range_tolerance <distance-value>
   
   This is a convergence test value; if two engagement ranges are found which bracket [success, failure], and they differ
   by less than the tolerance value, the transition edge is considered found.

.. command:: maximum_iterations <positive-integer>
   
   This is a terminate search constraint; if convergence is not attained within this many iterations per engagement,
   iteration is stopped for the engagement.


Example Input File
==================

::

   # ****************************************************************************
   #             WEAPON TOOL INPUT FILE - Air-to-Air Launch Computer Generator
   # ****************************************************************************
   # Example input data file to configure and exercise the Air-to-Air Launch Computer
   # Generator Weapon Tool.  Examples for all possible input keywords and values
   # are given below; however, default values are often sufficient, and when so,
   # the input keyword and its corresponding default value is commented out to
   # show the input is not mandatory.
   # ****************************************************************************

   include ../base_types/weapons/aam/aim-9x.txt

   # *****************************************************************************
   # This lethality is "perfect" if within lethal range.  Necessary, because
   # for the Tool to work properly we must have a kill if the missile is
   # kinematically able to pass within lethal range of its target.
   # NOTE:  The weapon's fuse should not have 'hit_proximity_range' set
   #        greater than the radius indicated below.

   weapon_effects WEAPON_TOOL_LETHALITY WSF_GRADUATED_LETHALITY
      radius_and_pk 30.0 ft 1.00 # All Platforms
   end_weapon_effects

   # The following is used as the launch platform type

   platform_type LAUNCH_PLATFORM_TYPE WSF_PLATFORM
     icon F-18E
     # (The default weapon fired by all Weapon Tools is the first one
     #  encountered on the launch platform.)
     weapon launching_weapon AIM-9X end_weapon
   end_platform_type

   # The following is used as a target

   platform_type TARGET_PLATFORM_TYPE WSF_PLATFORM
     icon F-18
     mover TARGET_MOVER end_mover
   end_platform_type

   tool AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR 

     # === Tool Input Keywords ===

     #tool_debug

     # The inputs below position the test case somewhere
     # on the globe, and indicate the firing conditions

     #altitude            10000 ft   # ATA Launcher defaults to 10000 ft alt.
     #position     00:00n 00:00w
     #heading                 0 deg
     position      34:54n 117:53w   # Near Edwards AFB

     #frame_step            0.5 sec # Time interval between Tool updates

     #target_platform_type TARGET_PLATFORM_TYPE
     #launch_platform_type LAUNCH_PLATFORM_TYPE

     # The default is to select the first weapon on the launch_platform_type
     #weapon_name <first_weapon_on_launch_platform_type> # (No matter what it is named.)

     # (The PK should usually boosted to 1.0 for this generator to work properly.)
     #weapon_effects WEAPON_TOOL_LETHALITY

     # The following two string values are used to assist with naming output
     # objects and files, if the names are not explicitly provided by user input.

     #tool_produces _ata_launch_computer

     #output_file_extension .txt

     # The user may have letter case preferences on file names, or a preferred naming
     # convention, so may choose to explicitly override the two Tool-provided names below:

     # 1. Output object name
     # (If not provided, the default name is to aggregate the two items shown below.)
     #output_object_name <weapon_platform_type><tool_produces>

     # 2. Output file name
     # (If not provided, the default name is to aggregate the two items shown below.)
     #output_file_name   <output_object_name><output_file_extension>

     output_object_name AIM-9X_ATA_LAUNCH_COMPUTER_10KFT_0.6MACH       # *** TEMPORARY NON_DEFAULT OVERRIDE
     output_file_name aim-9x_ata_launch_computer_alt10kft_0.6mach.txt  # *** TEMPORARY NON_DEFAULT OVERRIDE

     # === WsfAirToAirLaunchComputer Input Keywords ===
     
     #maximum_iterations     20
     #range_tolerance         5 m
     #minimum_range      1000.0 m
     #maximum_range     12000.0 m

     # ==== The file included and read in below will specify the independent ====
     # ==== variable engagement conditions for which results are generated    ====

     #load_table test_air_to_air_launch_computer_table.txt

     # ====        OR ALTERNATELY, READ IN IV's BELOW                 ====
     # ==== (NO DV's are given here, as the Tool will Generate them.) ====

     launch_computer_table
        no_escape_maneuver 7 g 
        independent_variables
           shooter_altitudes 10000 ft 20000 ft 30000 ft end_shooter_altitudes
           shooter_machs 0.8 1.0 1.2  end_shooter_machs
           target_altitudes 10000 ft 20000 ft 30000 ft end_target_altitudes
           target_machs 0.8 1.0 1.2   end_target_machs
           target_aspects     0 deg 30 deg 60 deg 90 deg 135 deg 180 deg  end_target_aspects
           target_lead_angles 0 deg 30 deg 60 deg 90 deg 135 deg 180 deg  end_target_lead_angles
        end_independent_variables
     end_launch_computer_table

   end_tool

   dis_interface
     record lc_generation.rep
   end_dis_interface
