.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Wizard v0.1.0 - 03 January 2012
-------------------------------

Enhancements
============

* Wizard can now handle non-WSF files. Signature files, antenna pattern files, etc, are added to the project automatically.
* Added Save As menu option.
* WSF parsing is more complete, including most of RIPR.
* Added action to comment or uncomment the selection by either pressing the "#" key or using the "Edit" menu.
* Added "Open Recent Project" option.
* Added a "Start" page with convenient links for users.
* Editor tabs can now be dragged for reordering or creating separate windows.
* Added documentation for Wizard, accessible through the Help menu.
* Added a feature to select the target WSF application, allowing Wizard to understand application and version specific
  commands. This feature only applies to future release of WSF tools -- WSF 1.7.3 or later.
* Other small miscellaneous usability improvements.

Bugs
====

* Fixed a crash that occurred sometimes when typing the "include" command at the end of a file.
* Fixed problem where "Find" would behave unexpectedly. Merged "Find" and "Find in Files" dialogs.
* Fixed problem where drag & drop would not work for text.
* Fixed problem where "Find References" would produce a huge menu if too many results were found.
* Fixed a problem where loading an invalid grammar file would provide poor feedback and abort.

Feature Requests
================

* Save As should replace the current tab with the newly created file.
