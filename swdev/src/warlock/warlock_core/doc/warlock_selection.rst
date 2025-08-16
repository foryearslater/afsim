.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Platform Selection - Warlock
----------------------------

Platforms can be selected multiple ways such as left-clicking on the platform name in the Platform Browser or left-clicking on the platform icon on the :doc:`Map Display <wkf_plugin/wk_map_display>`. Multiple platforms can be selected at the same time by holding ctrl and left-clicking multiple platforms.

Many dialogs in Warlock change based on the selected platforms. These dialogs are split into two categories; single selection and multiple selection.

* Single Selection Dialogs operate on a single platform regardless of how many platforms are selected.  If multiple platforms are selected, the last platform to be selected will be the platform shown in the dialog.
* Multiple Selection Dialogs operate on all selected platform, and will update whenever any of the selected platforms change.

Box Selection
=============

The :doc:`Map Display <wkf_plugin/wk_map_display>` supports selecting all platform within a box by holding down Shift + Ctrl + Left Mouse and dragging the cursor to select all platforms within the box.

Selection Shortcuts
===================

A single platform or group of platforms can be assign to a platform group using Ctrl+[0-9] to assign the group to the 0 through 9 key. After assigning platform to a group, you can select all platforms in the group by pressing the 0 through 9 key to select the associated group. You can change the keyboard shortcut for these actions on :doc:`Keyboard Shortcut <wkf_pref_shortcuts>` page in the :doc:`Preferences <warlock_preferences>`.
