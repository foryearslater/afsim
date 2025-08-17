.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

ORBITAL_LAUNCH_COMPUTER_GENER
---------------------------------

.. model:: tool ORBITAL_LAUNCH_COMPUTER_GENERATOR

    .. parsed-literal::

     tool ORBITAL_LAUNCH_COMPUTER_GENERATOR
        ... :command:`weapon_tools` Commands ...
        
        speed_ ...
        pitch_ ...
        leo_data_file_ ...
        launch_heading_ ...
        launch_headings_ ...
        loft_angle_ ...
        loft_angles_ ...
        pitch_rate_ ...
        pitch_rates_ ...

     end_tool

    ORBITAL_LAUNCH_COMPUTER_GENERATOR is a specialization of :command:`weapon_tools` to produce a launch computer to
    assist in launching orbital launch vehicles.  The tool will fire the defined launch vehicle type with a variety of
    launch headings and loft angles and capture the resulting data for later use in launch-to-orbit calculations.
    
    Any time the launch vehicle definition is modified in a way that will affect its kinematics, this generator should be
    re-executed to create a new launch computer data table.

General Commands
================

The general weapon_tool commands :command:`weapon_tools.position` and :command:`weapon_tools.altitude` should be used to specify the location of
the launch site. The commands speed_ and pitch_ can be used for airborne launch platforms.

At least one launch heading (launch_heading_, launch_headings_ or :command:`weapon_tools.heading`) and one loft angle (loft_angle_ or loft_angles_) must be provided.

.. command:: speed <speed-value>

   Specifies the speed of the launching platform. This should used only when the launching platform is an aircraft.
   
   **Default:** 0 m/s
   
   :note: The launching platform must not have a mover defined.

.. command:: pitch <angle-value>

   Specifies the pitch angle of the launching platform. This should be used only when the launch platform is an aircraft
   and the speed_ is greater than zero.
   
   **Default:** 0 degrees
   
   :note: The launching platform must not have a mover defined.

.. command:: leo_data_file <leo-data-file-name>
   
   Name of the file generated for low Earth orbit (LEO) calculations.

   **Default:** None - must be provided.
 
.. command:: launch_heading <angle-value>
   
   Single value of launch headings to be used for trajectory propagation.

   **Default:** If neither launch_heading_ or launch_headings_ is specified then value specified in the weapon_tools
   command :command:`weapon_tools.heading` will be used.
   
.. command:: launch_headings from <angle-value> to <angle-value> by <angle-value> 
   
   Range of launch headings to be used for trajectory propagation tests.

   **Default:** If neither launch_heading_ or launch_headings_ is specified then value specified in the weapon_tools
   command :command:`weapon_tools.heading` will be used.
   
.. command:: loft_angle <angle-value>
   
   Single value of loft angle to be used for trajectory propagation.
   
   **Default** None. At least one loft_angle or loft_angles_ command must be specified.
   
.. command:: loft_angles from <angle-value> to <angle-value> by <angle-value> 
   
   Range of loft angles to be used for trajectory propagation tests.
   
   **Default** None. At least one loft_angle or loft_angles_ command must be specified.

Pitch Rate Tuning Commands
==========================

Orbital launch vehicles typically include a phase that causes the vehicle to transition from vertical flight to its ascent
trajectory. Because of the general nature of the methods of the guidance, the ascent trajectory and resulting orbit altitude
are extremely sensitive to the conditions as they exist and the end of the 'pitch-over' phase. For these commands
to be effective, the vehicle must use the :ref:`guidance_program_types.FLIGHT_PATH_ANGLE_PROGRAM` in the pitch over phase.
One of the commands in this program is the 'pitch_rate' command. The conditions at the end of the program are governed by this
rate, and therefore it has a huge effect on the resulting orbit.

Determining a good value for this rate requires some iteration. These commands let you define a collection of pitch rates to be
used for executing the launch_heading/loft_angle matrix. For each pitch angle the entire set of launch_headings and loft_angles are
executed and summarized. You typically run a single launch_heading (90 deg) and a reasonable set of loft angles. The first run
is typically done with a large pitch angle range and increment (0.20 deg/sec to 0.50 deg/sec by 0.10 deg/sec). The second and
third runs typically hone in on a candidate by narrowing the range and shrinking the increment. Once a value is found,
the 'best' value is then used as the value of 'pitch_rate' in the FLIGHT_PATH_ANGLE_PROGRAM. You can then proceed to generate
the actual launch computer using the full range of launch heading and loft angles.

.. command:: pitch_rate <angle-rate-value>
   
   Single value of pitch rate that overrides the value of 'pitch_rate' by any FLIGHT_PATH_ANGLE_PROGRAM on the vehicle.
   
   **Default:** None, not required.
   
.. command:: pitch_rates from <angle-rate-value> to <angle-rate-value> by <angle-rate-value> 
   
   Range of pitch rate values that overrides the value of 'pitch_rate' by any FLIGHT_PATH_ANGLE_PROGRAM on the vehicle.
   
   **Default:** None, not required.
