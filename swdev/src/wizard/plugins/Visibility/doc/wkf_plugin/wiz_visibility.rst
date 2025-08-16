.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Visibility - Wizard
===================

The Visibility dialog allows the user to quickly choose which teams are visible in the current scenario. The Visibility dialog and the :doc:`Team Visibility preferences<../wkf_pref_team_visibility>` reflect the same visibility information and update one another when changes are made.  Changes made in the visibility dialog will be recalled in future sessions of the application.

Team Visibility
---------------

Team Visibility provides a checkable list of all platform teams in the scenario. The visibility of a specified platform team is controlled by toggling its associated checkbox.

To display the dialog, enable it in the view menu under "Visibility".

.. image:: ../images/wiz_team_visibility.png

On startup, Team visibility will be loaded based on the Team Visibility preferences. Unless there are previously defined settings, all platform teams are visible by default. 

The Team Visibility dialog is effectively an extension of the preferences; therefore whenever one is modified, the other is changed as well.

View Context Menu
-----------------

Visibility also adds 3 actions to the view right-click context menu: "Filter Out Selected Platforms", "Filter Out Unselected Platforms", and "Remove all Filters". These options are within the sub-menu "Visibility".

The first two actions will hide any number of platforms based on which platforms are selected. These actions are only available if at least one platform is selected.

The third action is able to make any platforms hidden via the other two actions visible again.

.. image:: ../images/wkf_visibility_context.png