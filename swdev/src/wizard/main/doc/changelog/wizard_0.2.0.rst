.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Wizard v0.2.0 - 02 April 2012
-----------------------------

Compatibility
=============

* WSF v1.7.3, IWARS v10.5.8

Enhancements
============

* (CR9423) - The extended mouse buttons may now be used to navigate forward and back on web content.
* (CR9425) - Clicking the middle mouse button over a tab will close the tab and document.
* (CR9449) & (CR9482) - Added Ctrl-H and edit menu option to find and replace text.
* (CR9450) - Modified the shift-tab key combination to un-indent the cursor"s line, or selected lines.
* (CR9552) - Added an Options dialog.
* (CR9424) - RIPR Commands are recognized.
* (CR9479) - Wizard will work with the next release of sensor_plot.
* Recognition of WSF commands has been improved, more commands are processed.
* Projects may contain more than one scenario.

  * A scenario is a named set of "main" files which represent a single "run" of an WSF application.
  * Each scenario may specify an associated WSF application.
  * Each scenario is parsed as a separate unit.
  * Scenario "visibility" may be toggled, changing the content of other controls.
* The scenario file browser was split into two browsers: Project Browser and File Browser.

  * The Project browser shows the list of scenarios in the project.
  * The File browser shows all files referenced by visible scenarios in a directory-oriented view.
* Project files have been added, project files store the state of a project.
* Added the Platform and Type Browsers.
* Include statements and types may be right-clicked in the editor to obtain more information.
* The editor has been enhanced with improved type checking. References to invalid types are indicated by a red dotted line.
* Integrated a Javascript based scripting system. The scripting system has a console and the available commands are documented here

Bugs
====

* (CR9419) - Fixed a bug where Wizard would crash if the user pressed the "Show Full Page" button on the
  WSF Command Documentation panel if no documentation was present.
* (CR9421) - Fixed a bug which prevented words from being selected in the editor by double clicking on certain computers.
* (CR9481) - Fixed a bug where text could not be copied from non-editor windows.
* (CR9460) - Fixed usability problems with Create New File dialog.
* (CR9464) - WSF .log file is not recognized.
* (CR9428) - Fixed problems in renaming files.
* (CR9480) - Fixed some antenna patterns not parsing correctly.
* Fixed problem where dragging/dropping text would sometimes copy instead of move text.

Feature Requests
================

* A command chain visualizer / browser.
* Option to strip trailing spaces from lines when saving (as in Textpad)
* Option to highlight current line (as in notepad++)
* Add a document backup & recovery system.
* Find References and Definitions isn"t smart enough to understand platform parts. i.e. "task-mgr" location must be determined by which platform the context is in.
