.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Zone Editor - Wizard
--------------------

.. image:: ../images/zone_editor_header.png

The **Wizard Zone Editor** allows the user to visualize the creation and editing of zones in Wizard. With this plugin, zones can be created via right-clicking on the Map Display and selecting "Create Zone at location," as well as through the text editor.

Zone Preview
============
.. image:: ../images/preview_zone.png

While creating a zone via the Create Zone dialog, a preview of that zone will appear and change as the user updates values.

Create Zone Dialog
==================
.. image:: ../images/geometric_dialog.png

Some values will enable or disable other options in the dialog. Circular zones, for example, will have a minimum and maximum radius while elliptical zones will have axes instead.

.. image:: ../images/polygonal_dialog.png

Polygonal zones can be created by adding points with the location selector button. Points can be deleted by right-clicking on the point and selecting "Delete Point." Making the polygonal zone relative will allow setting a reference platform.

Browser
=======
.. image:: ../images/dock_widget.png

The Zone Browser dock widget allows control over which zones and zone sets are visible. Updating a zone color via the :command:`color commands<zone.fill_color>` will update the corresponding color button.
Selecting a new color via the browser will add an update to the :command:`zone<zone.fill_color>` or :command:`zone_set<zone_set.fill_color>` definition.
If the zone line or fill color is not set, it will be colored as defined in the preferences.

Preferences
===========
.. image:: ../images/pref_widget.png

From Preferences, the user can select whether each zone/set should be randomly colored or if they should all have a specified color combination.
