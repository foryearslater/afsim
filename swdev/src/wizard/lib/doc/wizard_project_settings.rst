.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Project Settings - Wizard
-------------------------

A project consists of a few main items: a project directory, a working directory,
an application selection, and command line options. Projects are saved to
.afproj files for loading later.

If a project was opened by dragging and dropping a root scenario file onto the Wizard Main Window,
then you must select Save Project on the File menu or
all project settings will be lost on opening another project or exiting the
application.

.. image:: ./images/wizard_project_settings.png
   :align: right

The **Project Settings** dialog is accessible from the
Project The dialog provides the interface to change the following settings:

Root Directory:
   The directory displayed at the top level in the :doc:`Project Browser <wkf_plugin/wiz_project_browser>`.
Working Directory:
   The directory from which scenario executes and to which :command:`include`
   paths are referenced if different from the root directory.
Command Line:
   The command line given when executing the scenario.
   ``$(SCENARIO_FILES)`` is replaced by the set of main files in the
   scenario. Selecting the "Configure Startup Files" button displays the
   current startup file(s) and provides the capability to re-order them.
