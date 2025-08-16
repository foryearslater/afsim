.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Export Project - Wizard
-----------------------

Wizard can **export a project** to a specified directory. This
functionality can be accessed in the File --> Export Project...
or by selecting :Project --> Change History...
(see :doc:`wizard_change_history`).

Only scenario input files that are included with the :command:`include`
statements or :command:`include_once` statements commands are exported.

When exporting a project that includes external items outside the
project directory, Wizard will create additional layers of directories
during the export in an attempt to preserve as much of the directory layout
as possible. Ideally, a project could be exported to another directory or
machine, opened, and executed as-is without the need to adjust include
paths. On Windows, this is not possible if files for a scenario are kept
on different drives (e.g. C:\\ and D:\\). In the case where the
directory structure cannot be preserved for certain external items, the
affected items are placed in a folder called ``_external_items``
in the export directory.
