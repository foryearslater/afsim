.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Command Line - Warlock
----------------------

Warlock supports the most of the same command line options as the :doc:`mission <mission>` executable.  Warlock will always run in realtime and as such the frame-stepped option (-fs) should not be used.

Warlock supports starting it with no command line arguments and provides a :doc:`Start Dialog <warlock_start_dialog>` where the user can select what scenario file they want to run.

.. list-table::
   :header-rows: 1

   * - Command Line Options
     -
   * - -?, -h, -help
     - Display command line :doc:`options<warlock_help>` and quit
   * - -cf <filename>
     - Use the specified configuration file, modifications will be saved to specified file
   * - -icf <filename>
     - Imports the specified configuration file, modifications will be not be saved to specified file
   * - -permission_file <filename>
     - Use the specified permissions file. Prevents the user from editing permissions.
   * - -console
     - Enables the console window
   * - -ups
     - Uses the previous scenario, if no scenario was specified
   * - -minimized 
     - The application will start minimized
