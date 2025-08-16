.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Warlock User's Guide
--------------------

Warlock is an Operator-in-the-loop (OITL) application that is integrated with AFSIM.

.. contents::
   :local:

How to run Warlock
==================

The Warlock application is similar to the :doc:`mission <mission>` application, and it supports the same AFSIM scenarios, command line arguments, and plugins.

The following is a list of differences between Warlock and the mission application.

- Warlock is intended to be run in real-time.  It can be run constructively however some interfaces may not perform as intended such as the ability to control entities. See :command:`realtime`.
- Warlock supports starting it with no command line arguments.  If no command line arguments are used then Warlock will open a :doc:`Start Dialog <warlock_start_dialog>` where the user can select what scenario file they want to run.
- Warlock supports not just AFSIM plugins, but Warlock plugins. Warlock plugins can have graphical displays and interfaces.
- Warlock is a multi-threaded application, in which AFSIM runs on its own thread and the graphical portion runs on a different thread.

Launching Warlock from Wizard
=============================

Warlock can be registered with Wizard so that Warlock can be executed when the Run button is pressed within Wizard.  For more information read :ref:`Run from Wizard <Wizard_Users_Guide.run_from_wizard>` documentation.

Storing User Data
=================

Many parts of Warlock will store the applications state and store it, so that the next time Warlock is started the state is the same.

This information is stored in a user settings file referred to as the :doc:`configuration file<warlock_user_configurations>`.

The default configuration file on Windows is stored in the C:\\Users\\<user_name>\\AppData directory.
