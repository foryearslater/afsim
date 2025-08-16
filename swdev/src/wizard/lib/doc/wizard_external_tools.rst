.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

External Tools - Wizard
-----------------------

.. image:: ./images/wizard_external_tools.png
   :align: right

The **External Tools** option is accessible from the preference dialog in the
Options menu and manages all external tools known by Wizard.

To add a new tool, click **+**, and browse to the tool. The tool
will be added to the list at the top and information about the tool will
be filled in on the bottom. To remove an tool, simply click **-**.

The fields on the bottom of the dialog contain the following information:

Path
   The absolute path to the tool.
Alias:
   A name given to this tool. Used to refer to this tool in menus.
Command Line Arguments
   The command line text passed to the tool when executed. This can either be
   configured as a fixed text, or text containing variables that are expanded
   when the tool is launched. Click **Variables** to get a list of available variables.
Working Directory
   The 'current directory' as seen by the tool.
Associated File Types
   A space delimited list of file extensions that
   may be opened with this tool. These extensions are used to populate the
   context menu when you right-click on file items in the :doc:`Project Browser <wkf_plugin/wiz_project_browser>`.

Configure CME Tools
===================

VESPA, Timeview, GRIT, and Plotview can be configured quickly by adding the VESPA executable as an external tool.
