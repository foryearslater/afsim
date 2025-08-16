.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Wizard v1.9.0 - August 2014
---------------------------

Overview
========

Wizard 1.9 provides support for the new WSF 1.9.  Important features added this release include Sigview integration for easy viewing of antenna patterns and signatures, and PD Map file visualization as an alternative to plotview.

New Features
============

* Added brace matching in the text editor. When the cursor is on one of these characters: <tt>{}[]()</tt>, the corresponding match is highlighted.
* The working directory and root project directory can now be configured separately in the Project Settings dialog.
* Added the Command-chain Org. Chart Export plug-in. This plug-in exports command-chains into SVG files.
* Changes made while in Visual Mode are now displayed in the Visual Mode banner in the Wizard Main Window.
* SigView has been integrated with Wizard and can be more easily and directly used to visualize signatures and antenna patterns.
* Change History

  * Project startup files and working directory are now saved in revisions.
  * Database name and location are now based on the project file path
* WSF Output Window

  * Scenario output is now limited to 10,000 lines. The full log can be output to a file.
  * Files that are output by running simulations will now be accessible from a "Results" button.
* PD map files can now be visualized in a **Plot Viewer**. The PD map file names must end with a <tt>.pd</tt> extension.
* The new WSF Preprocessor declarations are parsed and highlighted in the text editor.
* Auto-Complete popup now shows fuzzy matches like 'limits' -> 'azimuth_slew_limits'  or  'az_lim' -> 'azimuth_slew_limits'

Notes
=====

* Linux build provided is now Red Hat Enterprise Linux 6, 64-bit.  (previously RHEL 5 was supported)

Bugs
====

* Fixed bug where command chain browser would not unassign from command chain.
* Fixed bug where 'format selection' would add a newline on the first line of some scripts.
* The Change History feature will now scan all included files for changes upon opening a project.
* Fixed bug relating to the use of project directory and working directory.  Now these can be set independently and operate as expected.
* Fixed bug where script parsing / autocomplete would fail in certain configurations of inherited types.
* Fixed script parsing to work for types declared before their base type.

New Feature Screen-shots
========================

Fuzzy Auto-Completion
^^^^^^^^^^^^^^^^^^^^^

.. image:: ../images/Wizard_Fuzzy_Completion.png
