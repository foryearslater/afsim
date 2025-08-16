.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Warlock
=======

**Warlock** is AFSIM's Operator-in-the-Loop visual application. It provides a graphic environment to view and control nearly every element of an AFSIM simulation during runtime.
Warlock's modular architecture enables the operator to customize controls and displays to meet scenario specific requirements and optimize interaction with the simulation. 

Quick Start
-----------

Warlock is directly accessible from the command line. It may also be started from the Warlock desktop shortcut, 
included with default AFSIM installation, and selected as the execution target through Wizard's Sim Execution Preferences.

Launch Warlock
""""""""""""""

Warlock (warlock.exe) may be launched via desktop shortcut, or from the command line. If launched from the command line, a project or scenario file name may be provided as an argument.

   .. note ::

      If Warlock does not launch as expected, ensure that the AFSIM installation path is set in the system environment variables.  The Warlock executable (warlock.exe) is located in the '/bin' directory of the AFSIM release file structure.
	  
The Warlock command line syntax is provided below:

.. code-block::

   >> warlock.exe <file_name>.txt
   or
   >> warlock.exe

Complete the steps below to open the *parachute_example* demo in Warlock:

1. Using the command line, navigate to the *parachute_example.txt* file location and launch Warlock with the *parachute_example* scenario as an argument.

   * Alternatively, launch Warlock from the desktop shortcut, and navigate to the scenario from the **Demo Browser**

   Example:

   .. code-block::

      >> cd demos/parachute
      >> warlock.exe parachute_example.txt

2. Warlock opens and the simulation begins.

.. image:: ../images/warlock_parachute_display.png
   :scale: 40%
   :align: center

Launch Warlock From Wizard
""""""""""""""""""""""""""

1. Launch the *parachute* demo from Wizard.

  .. note ::

     If "Warlock" is not displayed next to the Run Button, click the displayed name to open Wizard's **Sim Execution** preferences and select Warlock from the Executables list. Then click "Apply" to save the change.
   
.. image:: ../images/warlock_sim_exec.png
   :scale: 25%
   :align: center
   
2. Verify **Warlock** is displayed next to the Wizard Sim Execution Toolbar  |WIZARD_EXE_TOOLBAR|.

.. |WIZARD_EXE_TOOLBAR| image:: ../images/wiz_run_warlock_toolbar.png
   :scale: 60%
   
3. Click the green "Run button".
4. Warlock opens and the simulation begins.

Warlock Displays and Controls
-----------------------------

The display details and data controls in Warlock are primarily found in the View Menu.  Platform *Context* menus are displayed when a platform is right-clicked within Warlock's **Map Display**.

Warlock's **Map Display** provides the user the ability to alter the geographic view to enhance visualization during simulation. The **Map Display** may be be adjusted using the following mouse controls:

* 'Left-Click + Drag' - Translates the map with the mouse cursor.
* 'Double-Left-Click' - Zooms the map, centered on the mouse cursor.
* 'Mouse-Wheel' - Zooms the map in and out, centered on the mouse cursor.
* 'Middle-Click + Drag' - Rotates and tilts the view.

.. image:: ../images/warlock_map_display_controls.png
   :scale: 35%
   :align: center

The following list includes several of Warlock's core platform display options:

.. hlist::
   :columns: 4

   * Platform Browser
   * Platform Options
   * Platform Details
   * Platform Context

The **Platform Browser** window lists each platform in the scenario.  

.. image:: ../images/warlock_platform_browser.png
   :align: center
   :scale: 60%

The **Platform Options** window lists the map display view options for a platform or a group of platforms, allowing the user to visualize important platform interactions and hide everything else.

.. image:: ../images/warlock_platform_options_1.png
   :align: center
   :scale: 50%

The **Platform Details** window displays state information/data for the selected platform.

.. image:: ../images/warlock_platform_details_1.png
   :align: center
   :scale: 50%

A **Platform Context** Menu is available for each platform, allowing the user to interact with the selected platform in various ways.
Right-click on a platform in the Map Display to open its Platform Context Menu.

.. image:: ../images/warlock_platform_context.png
   :align: center
   :scale: 25%
   
Related Topics
--------------

   :doc:`p1-0_primer_mission`
   
   :doc:`p1-1_primer_wizard`
   
   :doc:`p1-3_primer_mystic`

