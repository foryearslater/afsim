.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Rotate Dialog - Wizard
----------------------

.. image:: ../images/wiz_rotate.png

The "Rotate Scenario" dialog can be used to rotate an entire scenario or a selection of objects on the Map Display.  This tool can be found in the Map Utilities menu under Tools.

.. warning:: The Rotate tools are incapable of changing the position and orientation of platforms with space movers, as most of the time, the movers are not defined with the 6 degrees of freedom, but by other parameters.

The Menu
========

.. image:: ../images/wiz_rotate_menu.png

The menu has the following items:

* "Rotate Scenario":  this item sets the dialog in a mode that allows the user to rotate an entire scenario.
* "Rotate Selection":  this item sets the dialog in a mode that allows the user to rotate a selection of objects.

.. note:: If there is nothing on the map, no menu option is enabled.  If there is nothing selected on the map, only the "Rotate Scenario" option is enabled.

The Input Methods
=================

The line edit allows the user to type in an angle of rotation.

.. note:: The format for the angle is:  "v u," where v is the decimal value and u is the units.  A space is between the value and unit.

It initializes the line edit with the angle 0.  The unit is determined by preferences, but can be changed to any of the supported units in AFSIM.

The dial allows the user to rotate in an "animated" style.

.. note:: The dial only rotates by an integral degree.
