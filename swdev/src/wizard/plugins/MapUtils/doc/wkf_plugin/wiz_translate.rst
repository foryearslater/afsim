.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Translate Dialog - Wizard
-------------------------

The "Translate Scenario" dialog can be used to translate an entire scenario or a selection of objects.  This tool can be found in the Map Utilities menu under Tools.

.. warning:: The Translate tools are incapable of changing the position and orientation of platforms with space movers, as most of the time, the movers are not defined with the 6 degrees of freedom, but by other parameters.

The Menu
========

.. image:: ../images/wiz_translate_menu.png

The menu has the following items:

* "Translate Scenario to...":  this item sets the dialog in a mode that allows the user to translate an entire scenario to another latitude/longitude (absolute translation).
* "Translate Selection to...":  this item sets the dialog in a mode that allows the user to translate a selection of objects to another latitude/longitude (absolute translation).

.. note:: If there is nothing on the map, the menu options are disabled.

The Input Methods
=================

There are two line edits:  one for latitude and one for longitude.

.. note:: The format is d:m:s.f(N/S) for latitude, d:m:s.f(E/W) for longitude, and "v u," where v is the decimal value and u is the units for distances.  A space is between the value and unit.

It initializes the line edits as followed:

* If in the absolute translation modes, the line edits initialize with the latitude and longitude of the entire scenario/selection of objects.

There are two sliders:  one for latitude and one for longitude.  This allows the user to translate in an "animated" style.

.. note:: The sliders only translate by the degree of latitude/longitude.
