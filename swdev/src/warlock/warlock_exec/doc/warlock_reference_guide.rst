.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Warlock Reference Guide
-----------------------

The Warlock Reference Guide lists the features included within Warlock organized into various categories.

Start-up
========

* :doc:`Command Line Options<warlock_command_line>`
* :doc:`Start-up Dialog<warlock_start_dialog>`

Application Layout
==================

.. image:: images/warlock_application_screen.png

This image above shows the layout of Warlock with the major components labeled.

Many of these components are dockable and may be moved to other locations in the window, or into their own 'floating' windows.

* Menus

 * :doc:`File<warlock_file_menu>`
 * :doc:`View<warlock_view_menu>`
 * :doc:`Options<warlock_options_menu>`
 * :doc:`Tools<warlock_tools_menu>`
 * :doc:`Developer<warlock_developer_menu>`
 * :doc:`Run<warlock_run_menu>`
 * :doc:`Help<warlock_help>`
 
* :doc:`Application Launcher<wkf_plugin/wk_application_launcher>`
* :doc:`Central View<warlock_central_display>`
* :doc:`Map Layer Manager<wkf_map_layer_manager>`
* :doc:`Platform Browser<wkf_plugin/wk_platform_browser>`
* :doc:`Platform Details<wkf_plugin/wk_platform_data>`
* :doc:`Platform Options<warlock_platform_options>`
* :doc:`Simulation Controller<wkf_plugin/wk_sim_controller>`
* :doc:`Map Toolbar<wkf_plugin/wk_map_toolbar>`
* Status Bar
 
Dialogs, Views and Toolbars
===========================

Most of the Dialogs and Toolbars can be accessed through the :doc:`View <warlock_view_menu>` menu.  A few are accessed through the :doc:`Context<warlock_platform_context_menus>` menu.

.. include:: wk_windows_extension.txt

Application Configuration
=========================

The following items control the appearance of Warlock.

* :doc:`Configuration Files<warlock_user_configurations>`
* :doc:`Map Definitions<warlock_map_definitions>`
* :doc:`Model Configuration<wkf_model_definitions>`
* :doc:`Permission Manager<warlock_permission_manager>`
* :doc:`Platform Options<warlock_platform_options>`
* :doc:`Plug-in Manager<warlock_plugin_manager>`
* :doc:`Preferences<warlock_preferences>`

Platforms
=========

The following items describe interactions with Platforms not mentioned in the above sections.

* :doc:`Context Menus<warlock_platform_context_menus>`
* :doc:`Platform Selection<warlock_selection>`

Plug-in Features
================

The following is a comprehensive list of functionality provided by Warlock extensions.

.. toctree::
   :glob:
   :titlesonly:

   wkf_plugin/wk_*

Demos
=====

The following demos are provided to demonstrate a variety of Warlock capabilities.

.. include:: warlock_demos.txt
