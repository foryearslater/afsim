.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Wizard v1.10.0 - March 2015
---------------------------

Overview
========

Wizard 1.10 provides support for the new WSF 1.10.

New Features
============

* Drag & Drop of files onto Wizard works for most files supported by Wizard, including external applications.
* Added the multi-run manager for design of experiments.

Notes
=====

* Known issue exists where un-docked editor/browser tabs fail to re-dock on Linux

Bugs
====

* Fixed failure to launch external applications with paths/files containing spaces in Windows
* Fixed failure to locate/load "colorname.xml" file from "data/vespa"
* Fixed multiple Qt warnings displayed on Wizard startup and shutdown
* Fixed crash when dragging a platform to "Unassigned Platforms" in the "Command Chain Browser"
* Fixed crash when loading any map after having selected "No Map" in the "Map Window"
* Fixed failure to display "Command Chain Organization Chart" by enforcing a ".svg" file extension on export
* Fixed repeating "Configure Startup Files" dialog in "Project Settings"
