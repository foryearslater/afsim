.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Wizard User's Guide
-------------------

The Wizard User's Guide provides step-by-step instructions on how to perform various tasks. The content in this guide answers the 'How do I...' questions.

.. contents::
   :local:
   :depth: 3

Create a new project
====================

There are two ways to create a new project:

-  Drag and drop a root scenario file (the initial file that you would pass to a WSF Application) onto the :doc:`wizard_main_window`.

OR

-  Select the File --> New menu option and create a new .afproj file.

Remember to save the project by selecting File --> Save Project. If the project was created by dropping a scenario file onto the Main Window, you will be prompted for a project name the first time the Save Project option is selected.

The next time you run Wizard, you can simply open the project by dragging the .afproj file that was previously saved onto the :doc:`wkf_plugin/wiz_project_browser` or via the File --> Open menu option.

Edit a scenario
===============

Once you've loaded a project you can begin editing your scenario. Use the :doc:`Text Editor <wizard_text_editor>` to read,
navigate, and modify scenario files. If you have any experience using tools such as Microsoft Notepad, Textpad or Notepad++, the scenario text editor should feel familiar and supports standard features such as Copy, Cut, Paste, Undo, and Redo.

Use the auto-complete feature
=============================

Auto-complete is an extremely useful feature. It is a tool that is commonly found in many software Integrated Development Environments such as Visual Studio and Eclipse. It provides command suggestions that are targeted to the context that you're working in. For example, if you're working on defining a platform, you're only interested in commands that are available to the platform. Traditionally, this was accomplished by referring to the documentation. With command auto-completion, Wizard can provide you with a list of commands that are suitable. To use this feature, place your cursor inside of the context that you're interested in (this could be global, inside a platform block, sensor block, etc.) and use the **Ctrl+Space** key sequence. This will bring up a pop-up window
containing the list of commands that are available in the current context.

For example ...

- To create a new type of :command:`platform` press **Ctrl+Space** and type 'pla'...this will filter the list down to platform, platform_availability, platform_type, etc.
- Next, use the arrow keys to highlight 'platform_type'.
- Press the 'Tab' or 'Enter' key to accept the 'platform_type' command.
- Note the :ref:`Syntax Tips <Wizard_Text_Editor.Syntax_Tips>` feature indicates the need to provide a name for the new type, type **MY_PLATFORM_TYPE**.
- Press **Ctrl+Space** again and the auto-complete tool provides a list of all the available platform_types, select WSF_PLATFORM.

By using the auto-complete tool you can easily walk through each WSF command and even have the tool suggest known values (i.e., the available platform types).

Find documentation for Wizard
==============================

Wizard comes with this User's Guide and a :doc:`Reference Guide <wizard_reference_guide>` These guides can be accessed from Wizard under the Help menu.

Find documentation for a WSF command
====================================

One really nice feature of Wizard is that WSF commands can be quickly found in the documentation. Simply click on the command (in the
:doc:`Text Editor <wizard_text_editor>` and press the 'F1' key. Or alternatively, select the command then right-click and select 'Command Documentation' from the context menu. Documentation will be displayed on the :doc:`Context Help <wizard_command_documentation>` tab at the bottom of the Main Window.

Find where a type is declared
=============================

Wizard provides a feature that allows you to quickly jump to the file
location where a WSF type is declared. To use this feature simply select
the type you're interested in finding (in the :doc:`Text Editor <wizard_text_editor>`
and right-click. This will launch the right-click menu, which has a menu item named
:ref:`Go To Definition <Wizard_Text_Editor.Go_To_Definition>`...select this item.
A tab will immediately open in the Text Editor
and the cursor will jump to the line where the type is located.

Find the locations where a type is referenced
=============================================

Complimentary to the :ref:`Go To Definition <Wizard_Text_Editor.Go_To_Definition>` feature, Wizard
provides a :ref:`Find References <Wizard_Text_Editor.Find_References>` feature, which
provides a listing of file locations where the specified WSF type is
referenced. To use this tool, simply select the type you're interested in finding
(in the :doc:`Text Editor <wizard_text_editor>` and right-click. This will launch
the right-click menu, which has a menu item named 'Find
References'...select this item. A pop-up window will appear with the
list of locations where the type was found to be referenced (if there is
only one reference, it will immediately jump to this location).

View the platforms in a scenario
================================

Wizard provides a feature called the :doc:`Platform Browser <wkf_plugin/wiz_platform_browser>`
which provides a tree-view listing of all of the platforms that exist in the currently loaded
project. The display may by expanded to show platforms' parts.

Double-clicking on a platform or platform part will open up a
:doc:`Text Editor <wizard_text_editor>` and navigate to the appropriate
definition.

View the platform types in a scenario
=====================================

The :doc:`Type Browser <wkf_plugin/wiz_type_browser>` provides a tree-view
listing of all of the types that exist in the currently loaded project.
The display may by expanded to show the derived types.

Double-clicking on a item in the browser will open up a :doc:`Text Editor <wizard_text_editor>`
tab and navigate to the appropriate definition.

.. _Wizard_Users_Guide.run_from_wizard:

Run from Wizard
===============

Wizard provides the ability to run a WSF application. To do so, you must first register your application with Wizard (this is a
one-time configuration) using the :doc:`Simulation Manager <wkf_plugin/wiz_simulation_manager>`, which is accessible via the Tools menu.

Next, you'll need to associate the application with the scenario you'd like to run using the scenario's :doc:`Settings <wizard_project_settings>` dialog. The scenario settings are accessible in the Project menu.

Now, all you need to do is press 'F5' or select 'Run' from the Project menu. The console output from WSF will be displayed in the :doc:`Output <wkf_plugin/wiz_output>` tab in the Output Panel.

View the simulation results in Mystic
=====================================

Wizard provides the ability to run the :doc:`Mystic <mystic>` application.

All you need to do is right-click on any .aer file listed in your :doc:`Project Browser <wkf_plugin/wiz_project_browser>` and choose 'Open with Mystic'.  Double clicking on the file or choosing it from the output list will work as well.
