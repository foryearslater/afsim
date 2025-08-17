.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

ATA_LAUNCH_COMPUTER_GENERATOR
-----------------------------

.. command:: ATA_LAUNCH_COMPUTER_GENERATOR
   :block:

Overview
========

ATA_LAUNCH_COMPUTER_GENERATOR_ is a specialization of :command:`weapon_tools` to produce an air-to-air missile launch
computer.  The generator contains a :model:`WSF_ATA_LAUNCH_COMPUTER` instance internally, which is modified into a final
output product so many of the commands are common between the two.

Command Line
============

.. parsed-literal::

   tool ATA_LAUNCH_COMPUTER_GENERATOR
      ... :command:`weapon_tools` Commands ...
      altitude_and_mach_ <length-value> <float-value>
      speed_ <speed-value>
      ... :model:`WSF_ATA_LAUNCH_COMPUTER` Commands ...
   end_tool

Commands
========

.. command:: altitude_and_mach <length-value> <float-value>
   
   Provide the altitude and mach number of the air-to-air missile firing.  This setting will override any Speed value
   provided in speed_, below.
   
   Default:  10000 ft altitude, 0.6 Mach (Overrides the 0.0 altitude default of the weapon_tools base class.)

.. command:: speed <speed-value>
   
   Provide the launch platform speed for the air-to-air missile firing.  This setting will override any Mach value
   provided in altitude_and_mach_.
   
   Default:  383 knots = 0.6 Mach at 10000 ft altitude
