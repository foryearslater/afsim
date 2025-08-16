.. ****************************************************************************
.. CUI//REL TO USA ONLY
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

ENGAGE - Wizard
---------------

.. contents::
   :local:

Overview
========
   
The **ENGAGE Wizard** is a tool for creating generic :doc:`ENGAGE<../engage>` scenarios. Values input into the various fields in the wizard will be used to generate a file that can be used directly by the ENGAGE executable built with AFSIM, or via a ENGAGE executable added to the simulation manager for use directly through Wizard. In future versions, the generated scenario will be loaded automatically into the Wizard environment for further use.

Scenario Settings
=================

File Path Settings
******************

**Root Directory**

    If specified, the wizard will use this location to look for pre-defined types at a root directory location. The current types that are supported in their own directory structure are:

    * launchers       (ROOT\launchers)
    * trackers        (ROOT\trackers)
    * target platforms(ROOT\targets)
    * weapons         (ROOT\weapons)

Simulation Settings
*******************

**Thread Count**

    The number of threads requested to be used while executing multiple runs. Setting this value at one executes runs serially. Setting the value to 0 will utilize a number of threads based on the number of processor cores to provide generally good performance. This value can not exceed the number of actual tasks required in the scenario input, and doing so will default the number of threads to the number of tasks.

**Random Seed**

    The integer value used to initialize the pseudo-random number generator.

**Frame Time**

    The increment of time between steps in the simulation.

**End Time**

    The amount of time to allow each task to run before terminating. Note that if this value is not sufficiently long, tasks may terminate while a weapon engagement is in progress, possibly resulting in an incorrect outcome. Excessively long run times may result in poor performance in runs where weapons are not engaged, as tasks that do not produce an active weapon engagement will continue to run to the time specified.

Event Output
************

**Time Format**

    The format for output of time. Currently not utilized.

**Lat/Lon Format**

    The format for output of latitude/longitude values. Currently not utilized.

**Print track in message**

    Select this value to include target track data in the event output

**Event Table**

    This table holds the events that will be tracked during task runs. If enabled, these values will be written to the event output. The following events are currently supported.

    .. toctree::
       :maxdepth: 1
       :glob:

       ../engage_event/*

**Output Rate**

    This table defines the rates to be used to generate event output. Each entry in the table refers to separate output files. Clicking add will open the Output Rate Dialog, in which the name of the particular set of rates is specified, and values entered by clicking on the Add button.

    Entries in the Output Rate Dialog are specified by a starting time, and a period (frequency) of rate output. This rate will be maintained for output generation unless additional time(s) are specified in the table, at which the newly listed rate will dictate output generation. A table entry of time without a specified period, or with a value of less than or equal to zero will terminate any additional output beyond the listed time.

Run Information
===============

Center Location
***************

The default starting position for site or target grid runs is 0n latitude, 0e longitude. If selected, an alternate position may be selected for the center of the grid. For target grids, this value will specify the initial position of the target. For site grids (currently not supported), this value will specify the initial position of the site.

Site Laydown
************

The site characteristics can be specified in the site laydown table. Predefined types that have been parsed by setting the root directory will be automatically populated here. Those that have been selected will be used when the scenario file is generated.

**Custom Laydown**

    Individual customized site locations may be specified here. Values populated into the various fields will be used for a single run in the scenario. Multiple locations may be specified, with each resulting in a new one-on-one run in the ENGAGE scenario.

**Grid**

   Determines the overall number of runs based on the site grid characteristics set by the user. The site is placed in a central location, and each target instantiation results in a individual engagement between the single site and target for that run. Additional runs are generated until all combination of variables are satisfied. Downrange, crossrange,and target altitude values may be used, with the final field "by:" determining the granularity of equal distance steps between the two values in the "from:" to "to" fields. Speed is constant for all runs. Negative values are supported for crossrange and downrange values to represent positions with the site behind the target or to the left of the target, respectively. Note that using a site grid laydown, the target altitude and target speed values are obviously settings for the target, not for the site. Thus, the other forms of possible target laydown input are disabled and invalid for use (currently) when a site grid is defined.

Target Laydown
**************

The target characteristics can be specified in the target laydown table. Predefined types that have been parsed by setting the root directory will be automatically populated here. Those that have been selected will be used when the scenario file is generated.

**Target Behavior**

    Allows for the selection of how targets are instantiated in ENGAGE runs.

    **Grid**

        Determines the overall number of runs based on the target grid characteristics set by the user. The target is placed in a central location, and each site instantiation results in a individual engagement between the single site and target for that run. Additional runs are generated until all combination of variables are satisfied. Downrange, crossrange, and altitude values may be used, with the final field "by:" determining the granularity of equal distance steps between the two values in the "from:" to "to" fields. Speed is constant for all runs. Negative values are supported for crossrange and downrange values to represent positions with the site behind the target or to the left of the target, respectively.

    **Simple Path**

        Allows the user to define a simple path the target will take. The x, y, and z, values are the starting position of the target in reference to the center point. The target will move from this position at the speed and heading specified.

    **Flight Path**

        A more specific flight path may be defined here, by adding waypoints for the target, providing values for the specified fields in the table.

    **Flight Route**

        Similar to a flight path, but with less options for less complexity.

    **Use Target Path**

        Use the path defined by the target type, outside of the methods listed above.

Repetition Count
****************

Given the user provided crossrange, downrange, and altitude values, a specific number of unique runs are required to cover all combinations of these values. Each of these unique combinations may be run multiple times if desired, by changing the repetition count.

Perform Flyouts
***************

Specifies if weapon flyouts will occur on runs.

Generate Record File
********************

Specifies if a record file is generated.

Generate Event File
*******************

Specifies if event files will be created.

Output
******

This table defines variable output files that can be created from runtime conditions. Clicking to add an output prompts for providing the file name to save the data, as well as the variable name, the phase in which to output the variable, and the unit and format of the variable output. For each variable file, event and summary output may also be appended to each file, if required, by selecting the appropriate checkboxes. The current variables supported are:

* time
* target_x
* target_y
* target_z
* target_vx
* target_vy
* target_zy
* target_ax
* target_ay
* target_az
* weapon_flight_time
* weapon_x
* weapon_y
* weapon_z
* weapon_vx
* weapon_vy
* weapon_vz
* weapon_ax
* weapon_ay
* weapon_az
* weapon_speed
* weapon_mach
* weapon_gee_force
* weapon_to_target_range

Pk Table Generation
===================

This table determines the settings to be used to generate Pk tables from site grid runs, if required. Note that a site grid must be defined to generate Pk table output, otherwise, input is disabled.

   **Enable Pk Table Generation**

      Selects whether to generate Pk tables from site grid data.

   **Table Site Name**

      Sets the name that will be used to identify the site in the Pk table output.

   **Table Target Name**

      Sets the name that will be used to identify the target in the Pk table output.

   **Output Path**

      Clicking the file dialog button will allow the indication and selection of a directory where Pk table data will be written upon completion of all simulation runs.

   **Enable Periodic Flush**

      Enables the writing of Pk table data during simulation runs instead of the default (at completion).

   **Output Length Units**

      Selects the length units that will be used in generated Pk table values.

   **Output Speed Units**

      Selects the speed units that will be used in generated Pk table values.
