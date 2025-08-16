.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Wizard v0.3.0 - 12 June 2013
----------------------------

Compatibility
=============

* WSF v1.7.5, IWARS v10.5.12

Enhancements
============

* Improved **WSF Script editing**

  * Improved script syntax highlighting.  Uses different text styles to indicate types, variables, arguments, keywords and more.
  * Autocomplete function and variable names with Ctrl-Space.

    * Shows definition location of variables and method documentation.
  * "Call Tips" show argument types while writing code that calls script functions.
  * Documentation for script types on F1
* New Script Debugger

  * Breakpoints, variable watches and more for WSF scripts.  Requires WSF version > 1.7.5.
* Added a **Regression Testing** Plugin
* Added a command chain browser.
* The Project and File Browsers have been combined.  Multiple scenarios at once are no longer supported, project management is now more simple.
* Added a short cut editor Option to change keyboard shortcuts.
* Added Format Selection to format a block of WSF input text.
* Right-Clicking a unit value in the text editor provides a "Convert To..." option performing unit conversions between units WSF supports.
* Right-Clicking a file in the File Browser or an Editor Tab will provide a "Find Included Locations..." option which lists
  locations the file is included.
* The Platform Browser and Type Browser have been updated; users may notice minor changes in functionality.
* Editor syntax highlighting has been improved to distinguish between begin/end blocks, commands, and keywords.
* Editor text colors can now be configured in Options menu.
* Added a basic application toolbar.
* Text editor now supports code-folding for input blocks.

Bugs
====

* Corrected miscellaneous problems with user interface for project management

Feature Requests
================

Text editor completing script method, showing inline documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: ../images/wizard_030_feature1.png

Text editor completing script, showing symbol definition
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: ../images/wizard_030_feature2.png

Debug breakpoints in text editor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: ../images/wiz_Script_Breakpoint_In_Editor.png

The Debugger Watch window
^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: ../images/wiz_Script_Watch_Control.png
