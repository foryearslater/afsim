.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

User Configurations - Warlock
-----------------------------

Warlock will store the state of the application, windows and preferences in a configuration file for the user whenever the :doc:`Preferences <warlock_preferences>` are changed or the application is closed. The configuration of Warlock can be saved using the Save Configuration option in the File menu.  Configurations can also be loaded using the Load Configuration option in the File menu.

Configurations can also be imported, which allows for the user to pick and choose which parts of the configuration file to import. This is different from Load Configuration because Load Configuration will load all the configuration options in the file.

Configuration files can be loaded or imported via the command line by using the -cf or -icf option, similar to the following command : start warlock.exe -cf StrikeConfig.ini

The benefit of saving multiple configurations is to support easily transitioning between different roles for Warlock. A user may save one configuration for executing a white cell, another for a red cell, another for stick control on a single aircraft, and another for tuning p6dof models. Each of these four use cases would use different windows and potentially different preferences. Saving and loading configurations allows the user to set up the configuration once and the switch between the configurations easily.
