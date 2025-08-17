.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

BALLISTIC_MISSILE_LAUNCH_COMPUTER_GENERATOR
-------------------------------------------

.. model:: tool BALLISTIC_MISSILE_LAUNCH_COMPUTER_GENERATOR

.. parsed-literal::

   tool BALLISTIC_MISSILE_LAUNCH_COMPUTER_GENERATOR
      ... :command:`weapon_tools` Commands ...

      air_target_file_
      surface_target_file_
      gnuplot_file_
      minimum_output_time_
      maximum_output_time_
      minimum_output_range_
      maximum_output_range_
      maximum_output_altitude_
      limited_to_preapogee_
      loft_angle_
      loft_angles_
      burn_time_
      burn_times_
      pass_1_loft_angle_start_
      pass_1_loft_angle_stop_
      pass_1_loft_angle_step_
      pass_2_loft_angle_step_
      loft_angle_range_
      depress_angle_range_

   end_tool

Overview
========

:model:`BALLISTIC_MISSILE_LAUNCH_COMPUTER_GENERATOR` is a specialization of :command:`weapon_tools` to produce a launch computer to
assist targeting and firing guided ballistic missiles.  The tool will fire the defined missile type with a variety of
loft angles and/or propellant burn times, and capture the resulting trajectory profiles, for use later in targeting an
impact or intercept time and location for air intercepts of ballistic missiles.  Any time a missile definition is
modified in a way that will affect its kinematics, this generator should be re-applied to create a new launch computer
data table.  In order to allow the same launch data table to be used anywhere on the globe, a spherical non-rotating
earth model is assumed.

Loft Angle
==========
Large, long range ballistic missiles fire vertically, and then quickly pitch over to some nominal "loft angle",
from which a gravity-induced turn holds them on a ballistic trajectory toward the desired (maximum range) target point.
The maximum achievable downrange trajectory is very sensitive to even small variations in the initial loft angle
assumed (a two-degree increment in loft angle might easily make the difference between a short-range earth-impact, or
going orbital with the missile).  This weapon tool generator allows the user to choose a range of loft angles to use,
and the angular resolution needed to iteratively converge on an "optimal" firing solution.  Usually lesser range
intercepts or impacts still use the same "optimal" loft angle, but prematurely terminate propulsion to cause a lower,
shorter-range trajectory to impact or intercept.   Alternate means of achieving shorter range impacts are lofted or
depressed trajectories, where a loft angle higher or lower (respectively) than optimal is selected to achieve a desired
target range impact.   Lofted trajectories sacrifice a longer time of flight, and depressed trajectories may suffer
aerodynamic heating issues from high speed flight in the upper atmosphere.

Usage with Guided Movers
========================
Certain :model:`WSF_GUIDED_MOVER` commands should not be used when generating data for a :model:`WSF_BALLISTIC_MISSILE_LAUNCH_COMPUTER`, as they may alter the flight paths of the missile in unexpected ways, potentially leading to erroneous data being generated.  These commands include, but may not be limited to, the divert commands (:command:`WSF_GUIDED_MOVER.divert_thrust`, :command:`WSF_GUIDED_MOVER.divert_fuel_mass`, :command:`WSF_GUIDED_MOVER.divert_fuel_flow_rate`, :command:`WSF_GUIDED_MOVER.divert_altitude_limits`).  However, once the launch computer data has been generated, these commands can be used in the :model:`WSF_GUIDED_MOVER` defined for the weapon when the scenario is being executed by mission or Warlock.

Generation Process
==================
The generator first iteratively fires the missile at low (flat) loft angles, increasing the loft
angle at each firing.  Terminal impact of the missile will initially increase each time.  As the optimal loft angle is
approached, terminal range will reach a maximum, and then begin decreasing again.  As noted in the paragraph above,
this effect is highly non-linear.  Because of this, a two-step iteration approach is used, beginning with a "gross"
larger delta loft angle (0.2 degrees a good starting point), and when the approximate optimal loft angle is found, a
"fine" iteration loop is re-run to capture a more accurate value for optimal loft angle (approximately a 0.001 degree
step).  This process requires either a) intelligent choices made regarding loft angle limits and increments, with
human-in-the-loop adjustments, or b) a vast numbers of iterations and much computational time to generate the tables. 
Once the optimal loft angle (and its resulting range) is determined, then sub-optimal target ranges are achieved by
lofted or depressed trajectories as noted above.

.. block:: BALLISTIC_MISSILE_LAUNCH_COMPUTER_GENERATOR

Commands
========
    
.. command:: air_target_file <air-target-file-name>
   
   Name for the output file containing the generated surface-to-air trajectory data.

.. command:: surface_target_file <surface-target-file-name>
   
   Name for the output file containing the generated surface-to-surface trajectory data.  

.. command:: gnuplot_file <gnu-plot-file-name>
   
   Name of the plot file written out for post-process visualization.

.. command:: minimum_output_time <time-value>
   
   Minimum missile time of flight value below which no intercept or impact is considered valid.

.. command:: maximum_output_time <time-value>
   
   Maximum missile time of flight value beyond which no intercept or impact is considered valid.

.. command:: minimum_output_range <distance-value>
   
   Minimum missile flight ground range below which no intercept or impact is considered valid.

.. command:: maximum_output_range <distance-value>
   
   Maximum missile flight ground range beyond which no intercept or impact is considered valid.

.. command:: maximum_output_altitude <altitude-value>
   
   Maximum altitude at which an intercept solution is tested for success.

.. command:: limited_to_preapogee
   
   Flag that precludes consideration of an intercept as viable if the intercept occurs post-apogee.

.. command:: loft_angle <angle-value>
   
   Single value of loft angle to be used for trajectory propagation.  Generally used only for small short range ballistic
   missiles.  The independent variable to achieve various downrange trajectories is then propellant burn time.

.. command:: loft_angles from <min-angle-value> to <max-angle-value> by <delta-angle-value> 
   
   Range of loft angles to be used for trajectory propagation tests.

.. command:: burn_time <time-duration-value>
   
   Single value of burn time to be used for trajectory propagation.

.. command:: burn_times from <min-time-duration-value> to <max-time-duration-value> by <delta-duration-value> 
   
   Used for generating nominal trajectory profile launch tables (burn time) as a function of desired target range.  Make
   sure to add in the inter-stage delay times when determining min and max duration values.  Cannot be used with either
   loft_angle_range_ or depress_angle_range_ keywords.

.. command:: pass_1_loft_angle_start <angle-value>
   
   Beginning loft angle for pass one (coarse traversal) to find optimal loft angle.  This value is highly dependent upon
   missile kinematics... larger (more vertical) for lumbering missiles, smaller (more horizontal) for energetic missiles. 
   CAUTION:  Choice of too high or too low a value may cause the generator to erroneously predict the optimal loft angle.

.. command:: pass_1_loft_angle_stop <angle-value>
   
   Ending loft angle for pass one (coarse traversal) to find optimal loft angle.  Since iteration is with increasing
   loft-angle, this value must be larger than pass_1_loft_angle_start.   This value is highly dependent upon missile
   kinematics... larger (more vertical) for lumbering missiles, smaller (more horizontal) for energetic missiles.  
   CAUTION:  Choice of too high or too low a value may cause the generator to erroneously predict the optimal loft angle.

.. command:: pass_1_loft_angle_step <angle-value>
   
   Increment value for pass one loft angle (coarse traversal).   Recommended starting point is 0.2 degrees.

.. command:: pass_2_loft_angle_step <angle-value>
   
   Increment value for pass one loft angle (fine traversal).  Recommended starting point is 0.001 degrees.

.. command:: loft_angle_range max_angle <angle-value> max_loft <angle-value> by <angle-value>
   
   Used only for generating lofted trajectory profile launch tables.  Parameters max_angle and max_loft are both
   independently able to terminate the iteration loop.  The max_angle parameter is an absolute value of loft angle, while
   the max_loft parameter is relative to the pre-calculated optimal loft angle.  Cannot be used with burn_times_ or
   depress_angle_range_ keyword.

.. command:: depress_angle_range min_angle <angle-value> max_depress <angle-value> by <angle-value>
   
   Used only for generating depressed trajectory profile launch tables.  Parameters min_angle and max_depress are both
   independently able to terminate the iteration loop.  The min_angle parameter is an absolute value of loft angle, while
   the max_loft parameter is relative to the pre-calculated optimal loft angle.  Cannot be used with burn_times_ or
   loft_angle_range_ keyword.
