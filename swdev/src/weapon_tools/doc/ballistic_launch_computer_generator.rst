.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

BALLISTIC_LAUNCH_COMPUTER_GENERATOR
-----------------------------------

.. model:: tool BALLISTIC_LAUNCH_COMPUTER_GENERATOR

.. parsed-literal::

   tool BALLISTIC_LAUNCH_COMPUTER_GENERATOR
      ... :command:`weapon_tools` Commands ...
      launch_altitudes_ <minimum-altitude> <delta-altitude> <number-of-altitudes>
      launch_speeds_ <minimum-speed> <delta-speed> <number-of-speeds>
      target_altitudes_ <minimum-altitude> <delta-altitude> <number-of-altitudes>
   end_tool

Overview
========

:model:`BALLISTIC_LAUNCH_COMPUTER_GENERATOR` is a specialization of :command:`weapon_tools` to produce a launch computer to release
unguided ballistic weapons, such as a gravity bomb.  The tool will release weapons at a variety of engagement altitudes
and speeds, and capture the resulting downrange travel and time of flight to impact.  A launch computer definition file
is created that will examine the current flight speed and altitude above the target, and anticipate a weapon release
far enough uprange to drop the weapon on the target track location.  (Since no lateral guidance is available in the
weapon, the launching aircraft MUST pass vertically over the target.)

.. block:: BALLISTIC_LAUNCH_COMPUTER_GENERATOR

Commands
========

.. command:: launch_altitudes <minimum-altitude> <delta-altitude> <number-of-altitudes>

   Desired launcher altitudes for weapon release. Altitudes are assumed
   to be the height of the launcher above an ellipsoidal earth.

   **Default:** 1000.0 meters (minimum), 1000.0 meters (delta), 10 altitudes

.. command:: launch_speeds <minimum-speed> <delta-speed> <number-of-speeds>

   Speeds of launcher for weapon release.

   **Default:** 120.0 m/s (minimum), 30.0 m/s (delta), 4 speeds

.. command:: target_altitudes <minimum-altitude> <delta-altitude> <number-of-altitudes>

   Altitudes to place targets for prosecution. Altitudes are assumed
   to be the height of the target above an ellipsoidal earth.

   **Default:** 0.0 meters (minimum), 50.0 meters (delta), 10 altitudes
