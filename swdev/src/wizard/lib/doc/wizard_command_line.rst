.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Command Line Arguments - Wizard
-------------------------------

Usage::

   wizard.exe  [ <file_name.txt> ]
               [ <project_file.afproj> ]
               { -console ]

<file_name1.txt> <file_name2.txt> <...>
   Specifies an input file(s) to load as a scenario. More
   than one file may be specified, each one is added to the scenario.

<project_file.afproj>
   Specifies the path to an existing project file which will be opened.

`-console`
   Commands Wizard to show the console window when starting up. This window displays additional status and debugging information.

Example: Launches Wizard and loads both **strike.txt** and
**dis_interface.txt**::

   On Windows:
      >> wizard.exe strike.txt dis_inteface.txt
   or
      >> wizard.exe c:\project1\scenario1.afproj
   On Linux:
      >> wizard strike.txt dis_interface.txt
   or
      >> wizard /home/user/project1/scenario1.afproj

.. note::
   The '>>' represents a command line.
