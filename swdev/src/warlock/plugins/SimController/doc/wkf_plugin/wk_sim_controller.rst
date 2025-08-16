.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Simulation Controller - Warlock
-------------------------------

The Simulation Controller toolbar provides options to control the advancement of time in a simulation.

.. image:: ../images/wk_SimController.png

The following commands are supported:

* Pause / Resume - This command will pause or resume the execution of the simulation.
* Terminate - This command will terminate the simulation.
* Restart - This command will restart the current simulation.
* Clock Speed - This command will change the simulation clock's real-time multiplier.
* Advance Time - This command will fast forward the simulation in non-realtime mode until the specified time.

.. note::
   To disable the toolbar, directly modify the enableToolbar flag in the associated configuration or settings file. 

.. note:: Each of the toolbar options can also be triggered via a modifiable :doc:`keyboard shortcut<../wkf_pref_shortcuts>`.

Preferences
===========

.. image:: ../images/wk_sim_controller_preferences.png

* Status Bar - Controls what elements are displayed in the status bar.
* Wall Time Specification - Controls how wall time is displayed in the status bar.
* Networking - Controls how simulation controls interact with remote simulations.
