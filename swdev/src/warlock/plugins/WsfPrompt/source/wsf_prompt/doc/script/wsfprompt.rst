.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

WsfPrompt
---------

.. class:: WsfPrompt

Overview
========

:class:`WsfPrompt` provides the ability to create customizable message prompts during a simulation in script. AFSIM applications that connect to the appropriate callbacks (e.g. Warlock) can display these messages in dialogs. 

See the :doc:`Warlock WsfPrompt<../wkf_plugin/wk_wsfprompt>` plugin for more information.

Static Methods
==============

These methods may be invoked as follows::

   WsfPrompt.Display("title", "message");

.. method:: static void Display(string aTitle, string aMessage, Array<string> aScripts, Array<string> aButtons)

   Displays a prompt with a title and message given by aTitle and aMessage. The dialog will contain buttons with text given by aButtons. These buttons will execute the scripts with names given by aScripts. This function will not pause the simulation. The input arrays aScripts and aButtons must be of equal size, otherwise a script error will occur.

.. method:: static void Display(string aTitle, string aMessage)

   Displays a prompt with a title and message given by aTitle and aMessage. The dialog will contain a single "OK" button for user acknowledgment of the message. This function will not pause the simulation.

.. method:: static void DisplayAndPause(string aTitle, string aMessage, Array<string> aScripts, Array<string> aButtons)

   Displays a prompt with a title and message given by aTitle and aMessage. The dialog will contain buttons with text given by aButtons. These buttons will execute the scripts with names given by aScripts. This function will pause the simulation and resume when the dialog closes. The input arrays aScripts and aButtons must be of equal size, otherwise a script error will occur.

.. method:: static void DisplayAndPause(string aTitle, string aMessage)

   Displays a prompt with a title and message given by aTitle and aMessage. The dialog will contain a single "OK" button for user acknowledgment of the message. This function will pause the simulation and resume when the dialog closes.

.. note:: 
   This class and its methods are only applicable if the simulation is being executed by Warlock. If the simulation is being run by another application (e.g. Mission), the method calls are still valid syntax and won't produce any errors, but nothing will be displayed and the simulation will not be paused.
