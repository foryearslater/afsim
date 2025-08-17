.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

ATG_LAR_AND_LC_GENERATOR
------------------------

.. model:: tool ATG_LAR_AND_LC_GENERATOR

.. parsed-literal::

   tool ATG_LAR_AND_LC_GENERATOR
      ... :command:`weapon_tools` Commands ...
      altitude_and_mach_ <length-value> <float-value>
      altitude_and_speed_ <length-value> <speed-value>
      target_lateral_offsets_ <length-value> <quantity>
      target_offsets_ <length-value> <quantity>
      target_forward_offsets_ <minimum-value> <delta-length-value> <quantity>
      shrink_factor_ <positive-floating-point-quantity>
      switch_sides
   end_tool   
   
Overview
========

ATG_LAR_AND_LC_GENERATOR_ is a specialization of :command:`weapon_tools` to produce launch acceptability regions (LARs) and
the launch computers (which will employ the LARs) needed to fire air-to-ground guided weapons against ground targets. 
To produce a LAR, stationary ground targets will be placed at a matrix of lateral and longitudinal offsets from the
launch aircraft, and fired upon.  When the launched weapon is able to maneuver to intercept a given ground target, it
will be identified as prosecutable.  A polygon-shaped LAR will be defined from the extreme edge of all prosecutable
target locations.  To allow for conservatism in the employment of a weapon against unknown unknowns, a
shrink_factor_ can be applied that will linearly reduce the LAR about its own centroid, if desired.

.. block:: ATG_LAR_AND_LC_GENERATOR

Commands
========

.. command:: altitude_and_mach <length-value> <float-value>
   
   Provide the altitude and mach number of the air-to-ground weapon firing.  Functionally equivalent to
   altitude_and_speed_.  Multiple launch conditions will be accepted.

.. command:: altitude_and_speed <length-value> <speed-value>
   
   Provide the launch platform speed for the air-to-air missile firing.  Multiple launch conditions will be accepted.

.. command:: target_lateral_offsets <length-value> <quantity>
   
   Provide the length and quantity of lateral offsets (from the centerline of the launch platform) at which ground targets
   will be placed and fired upon.  Zero lateral offset from the launch aircraft (target on the centerline of the launch
   aircraft) is always tested, and the LAR is assumed to be symmetric about the lateral plane of the launch aircraft.
   
   Default:  500.0 meters, 5 offsets

.. command:: target_offsets <length-value> <quantity>
   
   Synonym for "target_lateral_offsets" as above (for backward compatibility).

.. command:: target_forward_offsets <minimum-value> <delta-length-value> <quantity>
   
   Provide the minimum forward offset (most negative; a negative number is allowed), delta length, and quantity of target
   offsets (from the launch platform) at which ground targets will be placed and fired upon.  If the LAR is expected to
   contain points behind the current aircraft location, then the minimum value must be less than zero. lateral offset from
   the launch aircraft (target on the centerline of the launch aircraft) is always tested, and the LAR is assumed to be
   symmetric about the lateral plane of the launch aircraft.
   
   Default:  1000.0 meters (minimum), 500.0 meters (delta), 10 offsets

.. command:: shrink_factor <positive-floating-point-quantity>
   
   Supply a factor (0.0 < factor < 1.0) to reduce the LAR size to apply conservatism in the employment of the weapon.
   
   Default:  1.0

.. command:: switch_sides
   
   The default output of the resultant replay file shows the output on the right side of the launching platform. This
   command switches the output to the left side.

Example Input File
==================

::


   # include the weapon file of choice
   include myweapon.txt # 500lb version

   weapon_effects WEAPON_TOOL_LETHALITY WSF_GRADUATED_LETHALITY
      radius_and_pk 15.0 ft 1.00 # All Platforms
   end_weapon_effects
   platform_type LAUNCH_PLATFORM_TYPE WSF_PLATFORM
     icon F-18E
     weapon launching_weapon MYWEAPON
     end_weapon
   end_platform_type

   platform_type TARGET_PLATFORM_TYPE WSF_PLATFORM
     icon Ground_Radar
   end_platform_type

   tool ATG_LAR_AND_LC_GENERATOR
     position     00:00n 00:00w
     output_object_name myweapon_ATG_LAUNCH_COMPUTER_20KFT_MACH_0.8    # *** TEMPORARY NON_DEFAULT OVERRIDE
     output_file_name myweapon_atg_launch_computer_20kft_mach_0.8.txt  # *** TEMPORARY NON_DEFAULT OVERRIDE
   #  altitude_and_mach 40000 ft  0.8    target_ranges 40 miles 5.0 miles  26
   #  altitude_and_mach 40000 ft  1.2    target_ranges 50 miles 5.0 miles  27
   #  altitude_and_mach 40000 ft  1.4    target_ranges 60 miles 5.0 miles  28
   #  altitude_and_mach 35000 ft  0.8    target_ranges 40 miles 5.0 miles  23
   #  altitude_and_mach 35000 ft  1.2    target_ranges 40 miles 5.0 miles  27
   #  altitude_and_mach 35000 ft  1.4    target_ranges 55 miles 5.0 miles  28
   #  altitude_and_mach 25000 ft  0.8    target_ranges 40 miles 5.0 miles  20
   #  altitude_and_mach 25000 ft  1.2    target_ranges 50 miles 5.0 miles  23
   #  altitude_and_mach 25000 ft  1.4    target_ranges 50 miles 5.0 miles  24
     altitude_and_mach 20000 ft  0.8    target_ranges 40 miles 5.0 miles  18
   #  altitude_and_mach 20000 ft 1.2     target_ranges 40 miles 5.0 miles  22
   #  altitude_and_mach 20000 ft 1.4     target_ranges 45 miles 5.0 miles  23
   #  altitude_and_mach 15000 ft 0.8     target_ranges 30 miles 5.0 miles 17
   #  altitude_and_mach 15000 ft 1.2     target_ranges 35 miles 5.0 miles 19
   #  altitude_and_mach 15000 ft 1.4     target_ranges 40 miles 5.0 miles 20
   #  altitude_and_mach 10000 ft  0.8    target_ranges 25 miles 5.0 miles 15
   #  altitude_and_mach 10000 ft  1.2    target_ranges 35 miles 5.0 miles 16
   #  altitude_and_mach 10000 ft  1.4    target_ranges 40 miles 5.0 miles 17
   #  altitude_and_mach 5000 ft  0.8     target_ranges 25 miles 5.0 miles 13
   #  altitude_and_mach  1000 ft  0.8    target_ranges 25 miles 5.0 miles 10

   #  target_ranges     40 miles 5.0 miles 21 # min, delta, number of target offsets forward of the launcher
   #  target_ranges     40 miles 2.5 miles 50 # min, delta, number of target offsets forward of the launcher
   #  target_offsets    10.0 miles 4 # delta, number of target offsets lateral to the launcher
    target_offsets    15.0 miles 3 # delta, number of target offsets lateral to the launcher
   #  target_offsets    .1 meter 2 # delta, number of target offsets lateral to the launcher
    #shrink_factor         0.8
   end_tool

   dis_interface
     #record myweapon.rep
   end_dis_interface
   #event_output
   #   file myweapon.evt
   #   enable WEAPON_HIT
   #   enable WEAPON_MISSED
   #   enable WEAPON_TERMINATED
   #end_event_output
   script void WeaponHit(WsfWeaponEngagement aWeaponEngagement, WsfPlatform aTargetPlatform)
       WsfPlatform weapon = aWeaponEngagement.WeaponPlatform();
       WsfGeoPoint launchpoint = aWeaponEngagement.WeaponLocationAtLaunch();
       WsfGeoPoint weaponpoint = aWeaponEngagement.WeaponLocation();
       double groundrange = launchpoint.GroundRangeTo(weaponpoint);
       double timeofflight = aWeaponEngagement.TimeSinceStarted();
       writeln(weapon.MachNumber(), " ", weapon.Speed(), " ", weapon.Pitch(), " ", groundrange, " ",timeofflight);
   end_script
   observer
      enable WEAPON_HIT
   end_observer
