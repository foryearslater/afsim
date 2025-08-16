.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Wizard v2.0.0 - 25 March 2016
-----------------------------

New Features
============

* Added the :doc:`../wkf_plugin/wiz_demo_browser` capability.
* Added the Wizard_DIS_Interface_Setup plugin.
* Added new high-contrast, dark color theme.
* Added capability to dynamically discover, view, and select predefined color themes (cascading style sheets) via the Wizard_Options dialog.
* Added functionality to automatically re-style open editor tabs/windows upon change in theme or font/color options.
* Added functionality to dynamically discover and pre-populate the :doc:`../wkf_plugin/wiz_simulation_manager` upon first execution.
* Integrated :doc:`../wizard_command_documentation` with new Sphinx documentation system/paradigm, including duplication of look and feel.

Notes
=====

* Significant refactor/overhaul of Wizard menus and :doc:`../wizard_main_window` Toolbars, including correction of issues with context-based feature enabling/disabling.
* Migrated many core features (including Wizard_Map_Window, ***Wizard_Multi_Run***, etc.) into stand-alone plugins to ease development activities.

Bugs
====

* Fixed the ability to comment a selection with the shift+3 (#) key combination.
* Fixed the Map Window->Platform Properties->Name field always displaying " - multiple selection - " when an entity is selected. When a platform is selected, both the entity and its attachments are selected which was triggering a multi-select indication.
* Disabled the ability to open the Map Window without a project loaded. Interaction with the Map Window without a project loaded was causing Wizard to crash.
* Fixed mis-aligned line numbering on the :doc:`../wizard_find_results` dialog upon editing of matching file.
* Fixed mis-aligned breakpoint locations in the :doc:`../wkf_plugin/wiz_script_debugger` upon editing of file (Mantis Bug 29).
* Fixed incorrect population of "Externals Items" on the :doc:`../wkf_plugin/wiz_project_browser` and file path resolution when using the :command:`file_path` command (Mantis Bug 76).
* Corrected an issue with the Linux installation of the SigView executable not having correct user permissions. The CMAKE for Wizard ***SigView*** plug-in has been updated.
* Corrected an issue with the hardcoded background color (light grey) and the default text color (white) in Wizard dark made the auto pop-up intellisense  textboxes unreadable.  Changed the default text color to black so that it's readable.
* Corrected issues with new functionality to dynamically discover and pre-populate the :doc:`../wkf_plugin/wiz_simulation_manager` upon first execution. Changes include additional :doc:`Command Line Arguments <../wizard_command_line>`.

New Feature Screen-shots
========================

+----------------------------------------+
| Demo Page                              |
+========================================+
| .. image:: ../images/wiz_demo_page.png |
|    :width: 400                         |
|    :align: center                      |
+----------------------------------------+

+---------------------------------------------+--------------------------------------------+
| Light Color Theme (Existing, Default)       | Dark Color Theme (New)                     |
+=============================================+============================================+
| .. image:: ../images/wizard_light_theme.png | .. image:: ../images/wizard_dark_theme.png |
|    :width: 400                              |    :width: 400                             |
|    :align: center                           |    :align: center                          |
+---------------------------------------------+--------------------------------------------+
