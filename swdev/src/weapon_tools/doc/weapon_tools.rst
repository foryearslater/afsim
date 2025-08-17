.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

weapon_tools
------------

.. command:: weapon_tools
   :block:

Overview
========

:command:`weapon_tools` is an optional WSF module that may be incorporated into WSF applications, and is also distributed as a stand-alone executable. weapon_tools repeatedly fires a predefined :model:`WSF_EXPLICIT_WEAPON` over widely varied engagement conditions. Hit/miss results are captured to quantify the engagement capabilities of that defined weapon type, permitting successful weapon employment against target tracks. The tool produces one or more files that define run-time software objects for a future simulation or scenario that will employ the weapon.  :command:`weapon_tools` is extensible, and currently allows for creation/generation of the following weapon employment objects:

* Air-To-Air Launch Computers, via :model:`AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR`.
* Air-To-Ground (ATG) Launch Acceptable Regions (LARs) and Launch Computers, via :model:`ATG_LAR_AND_LC_GENERATOR`.
* Ballistic Launch Computers, via :model:`BALLISTIC_LAUNCH_COMPUTER_GENERATOR`.
* Ballistic Missile Launch Computers, via :model:`BALLISTIC_MISSILE_LAUNCH_COMPUTER_GENERATOR`.
* Orbital Launch Computers, via :model:`ORBITAL_LAUNCH_COMPUTER_GENERATOR`.
* Surface-to-Air-Missile (SAM) Launch Computers, via :model:`SAM_LAUNCH_COMPUTER_GENERATOR`.

Command Line
============

Using the stand-alone weapon_tools.exe::

 weapon_tools.exe <input-files>

For other WSF applications supporting weapon_tools::

 wsf_application.exe --weapon-tools <input-files>

Commands
========

.. command:: tool <tool-type-name> ... end_tool
   
   .. parsed-literal::

      tool *<tool-type-name>*
         ... :ref:`weapon_tools.Tool_Commands` ...
      end_tool

   Where *<tool-type-name>* is one of the options given in the bulleted list in the overview above.

.. _weapon_tools.Tool_Commands:

Tool Commands
=============

.. command:: tool_debug

   Enable run-time debug output.

   **Default:** off
   
.. command:: terminate_on_launch_failure <boolean-value>

   If true, the tool will terminate when a weapon fails to fire. This command is only applicable for the :model:`AIR_TO_AIR_LAUNCH_COMPUTER_GENERATOR`, the :model:`ATG_LAR_AND_LC_GENERATOR`, and the :model:`BALLISTIC_LAUNCH_COMPUTER_GENERATOR`.

   **Default:** true

.. command:: position <latitude> <longitude>

   Specify the geographic location of the launch platform.

   **Default:**  0.0 n, 0.0 e

.. command:: altitude <length-value>

   Specify the altitude from which the weapon launches take place.

   **Default:** 0.0 m

.. command:: heading <angle-value>

   Specify the compass-orientation of the launch platform.

   **Default:**  0.0 deg

.. command:: frame_step <time-value>

   Time interval between tool updates.

   **Default:**  0.5 sec

.. command:: launch_platform_type <platform-type>

   Specify the platform type that will launch weapons.

   **Default:** LAUNCH_PLATFORM_TYPE

.. command:: target_platform_type <platform-type>

   Specify the platform type that will be fired upon.

   **Default:** TARGET_PLATFORM_TYPE

.. command:: weapon_name <weapon-name>

   Specify the name of the weapon to use on the launch platform. type that will be fired upon.

   **Default:**  The first weapon encountered on the launch platform type.

.. command:: weapon_effects <effect-name>

   Specify the weapon_effects (lethality) model the launched weapon platform will use to determine target effect.

   **Default:** WEAPON_TOOL_LETHALITY

.. command:: tool_produces <product-name>

   Specify the output product of the weapon tool.  This value is used in automatically naming output files.  Typical
   values would be _LAUNCH_COMPUTER, or _LAUNCH_ACCEPTABLE_REGION, etc.

   **Default:** _UNKNOWN_PRODUCT

.. command:: output_object_name <output-name>

   Specify the output product of the weapon tool.  This value is used in automatically naming output files.  Typical
   values would be _LAUNCH_COMPUTER, or _LAUNCH_ACCEPTABLE_REGION, etc.

   **Default:** _UNKNOWN_PRODUCT

.. command:: output_file_extension <extension-name>

   Specify the file extension of the output product of the weapon tool.

   **Default:** .txt

.. command:: output_file_name <file-name>

   Specify the name of the file produced by the tool.

   **Default:**  Concatenation of output_object_name + output_file_extension
