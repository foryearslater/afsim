.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

WsfPrompt - Warlock
-------------------

An AFSIM script can create customizable dialogs within Warlock to display to the user by using the :class:`WsfPrompt` Display methods.  The user can configure button names and the scripts each button executes, as well as the title and message displayed in the dialog. If desired, the prompt can pause the simulation until the operator makes a selection.

The methods in the :class:`WsfPrompt` script class may be invoked as follows::

   WsfPrompt.Display("title", "message", {"script_1", "script_2", ... , "script_n"}, {"button_1", "button_2", ... , "button_n"});

If the user does not want a button to execute a script, an empty string should be used for the script name. Only scripts with no arguments are supported.

The image below shows a simple use case.

.. image:: ../images/wk_WsfPromptDialog.png
