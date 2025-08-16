.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

.. _Scenario_Analyzer_Check_Suites:

|  :doc:`Scenario Analyzer Tool Table of Contents <wiz_scenario_analyzer>`
|  :doc:`Previous: Using the Scenario Analyzer <scenario_analyzer_overview>`
|  :doc:`Next: Descriptions of Script Utilities <scenario_analyzer_utilities>`

Descriptions of Included Checks
===============================

Core AFSIM Checks
-----------------

.. _Comm_devices_must_have_internal_links:

Comm devices must have internal links
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every communications device (``WSF_COMM_TRANSCEIVER, WSF_COMM_XMTR, WSF_COMM_RCVR``) has at least one internal link (``internal_link``) connecting it to any other platform part. Note that passing this check does not guarantee that the data received by a comm will make its way to the platforms or platform parts expecting it.

.. _Platforms_in_command_chains_must_have_comm_devices:

Platforms in command chains must have comm devices
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform belonging to a declared command chain has at least one communications device on the platform. The check is not triggered if a platform belongs only to the default command chain, or if a platform belongs to a declared command chain in which it is its own commander and has no subordinates. The premise of the check is that organizing platforms into a command chain is useless unless those platforms are able to communicate with each other. However, note that passing the check does not guarantee that the platforms in a command chain will communicate as expected. For example, it is still necessarily to ensure that platforms meant to communicate with each other have comms on the same network, that comms are correctly connected to processors via internal links (``internal_link``), and that processors meant to disseminate information have external links to the correct platforms set up using ``report_to`` or ``report_to_group`` statements.

.. _Declared_command_chains_should_have_structure:

Declared command chains should have structure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For every platform in a declared command chain, checks that the platform is not its own commander or has at least one subordinate or peer in that command chain. A 'declared command chain' is one given a user-defined name (e.g., ``command_chain MY_COMMAND_CHAIN SELF``), in contrast to the default command chain to which platforms are added when no command chain name is specified (e.g., ``commander SELF``).

.. _Declared_commander_should_be_in_that_chain:

Declared commander should be in that chain
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform identified by another platform as its commander in a declared command chain is a member of that command chain. For example, if platform ``ew_radar`` identifies a commander using ``command_chain BLUE blue_cmdr``, then platform ``blue_cmdr`` should also identify its commander (even if that commander is ``SELF``) in the ``BLUE`` command chain so that it is recognized as belonging to ``BLUE``.

.. _Scenario_with_many_platforms_should_have_a_command_chain:

Scenarios with many platforms should have a command chain
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that any 'large' simulation (currently defined in the check as having 10 or more platforms) contains at least one command chain with some structure. A declared command chain has structure if at least one platform in it is not its own commander, has at least one subordinate, or has at least one peer. A scenario does not need to have a declared command chain to pass this check: If the default command chain has been given structure (e.g., by assigning a commander other than SELF to a platform), the check will pass. The check will be satisfied if a command chain is present anywhere in the simulation, even if one side has 10 or more platforms with no command chain on that side.

.. _All_platforms_should_have_meaningful_locations_defined:

All platforms should have meaningful locations defined
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform has a starting position defined that includes a non-zero value for at least one of latitude, longitude, and altitude. The command ``position <Latitude> <Longitude> [<Altitude>]`` defines a platform's location. 

.. _Sensors_must_be_internally_linked:

Sensors must be internally linked
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every sensor has at least one ``internal_link`` defined from that sensor to any other part on the same platform. If a sensor has no internal links defined, the information it gathers will never be used. Note that passing this check does not guarantee sensor information will make its way to the platforms or platform parts that expect to receive it.

.. _Sensor_platforms_should_have_at_least_one_sensors_turned_on:

Sensor platforms should have at least one sensor turned on
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

*If the ``wsf_iads_c2`` plug-in has not been loaded:* Checks that every platform with one or more sensors has at least one sensor turned on.

*If the ``wsf_iads_c2`` plug-in has been loaded:* If a platform is not part of an IADS C2 system, the check will give a warning if a platform has one or more sensors and no sensor is turned on. If a platform is part of an IADS C2 system, the check will also evaluate the categories of the platform's sensors (TTR, TAR, EW, RWR) and will give a warning if the platform has one or more non-TTR sensors, all which are turned off. For the purposes of this check, a platform is 'part of an IADS C2 system' if its sensors are managed by a sensors manager, i.e. the platform is subordinate on the default command chain to a platform that has a sensors manager.

.. _Sensors_should_be_linked_to_track_processor:

Sensors should be linked to track processor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

*For sensors other than ``WSF_SAR_SENSORs``:* Checks that every sensor has at least one ``WSF_TRACK_PROCESSOR`` that is 'linked and reachable' from the sensor. A track processor is linked and reachable from a sensor if: (1) the sensor has an internal link going to a track processor on the same platform, or (2) the sensor is connected by a network of internal links and external links (defined using either ``report_to`` or ``report_to_group`` commands) with compatible comms on either side of each external link. The check uses the equivalent of the 'ScenarioAnalyzerUtils.LinkedAndReachablePlatformPartsChooseProcs <#arraywsfplatformpart-linkedandreachableplatformpartschooseprocswsfplatformpart-origin-string-parttype-arraystring-processortypes-bool-followspecifiedprocs>'__ script method to find the track processors to which a sensor is linked, and excludes intermediate external links from other track processors. See that helper method's description for a more detailed explanation of how linked and reachable track processors are discovered.

*For ``WSF_SAR_SENSORs``:* Checks that every SAR sensor has at least one ``WSF_IMAGE_PROCESSOR`` or ``WSF_VIDEO_PROCESSOR`` that (1) is 'linked and reachable' from the sensor AND (2) is internally linked to a ``WSF_TRACK_PROCESSOR``. The SAR must be connected to an image or video processor in the same way that non-SAR sensors must be connected to track processors as described above.

.. _All_signatures_should_be_detectable_by_enemy_sensors:

All signatures should be detectable by enemy sensors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For every platform in the simulation, checks that at least one enemy sensor can detect each of the platform's signature types. The check considers the following signature types: ``acoustic_signature``, ``infrared_signature``, ``inherent_constrast``, ``optical_signature``, and ``radar_signature``. For the purposes of this check, a sensor 'can detect' a signature type if the sensor uses that kind of signature. For example, any ``WSF_RADAR_SENSOR`` can detect radar signatures and any ``WSF_ACOUSTIC_SENSOR`` can detect acoustic signatures, while any ``WSF_IRST_SENSOR`` can detect infrared signatures, inherent contrast, and optical signatures. A sensor is an 'enemy sensor' if it is located on a platform that belongs to a different side (set with the ``side <side>`` command) than the platform with the signature being evaluated. Note that passing this check does not guarantee that all platforms will actually be detected by an enemy sensor: A platform with a ``radar_signature`` would pass this check as long as some enemy platform has a ``WSF_RADAR_SENSOR``, but the platform will not actually be detectable if its radar signature falls outside of the sensor's frequency band.

.. _Platforms_should_have_signatures_required_by_sensors_in_scenario:

Platforms should have signatures required by sensors in scenario
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For every signature type that is required by a sensor in the simulation, checks that every platform in the simulation has that type of signature defined. For example, if a sensor on one platform has a ``WSF_RADAR_SENSOR``, the check will give a warning for every platform that lacks a ``radar_signature``.

.. _Script_processors_must_have_update:

Script processors must have update intervals defined
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that any ``WSF_SCRIPT_PROCESSOR`` or ``WSF_QUANTUM_TASK_PROCESSOR`` has a non-zero update interval defined using ``update_interval <time-reference>``. If the update interval is not defined or is set to zero, the script processor will never be updated.

.. _Track_processor_purging_intervals_must_be_long_enough_to_maintain_tracks:

Track processor purging intervals must be long enough to maintain tracks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For every track processor (``WSF_TRACK_PROCESSOR``), checks that the purging interval is long enough for it to maintain tracks from data generated by each of the sensors connected to it. The check compares the track processor's purging interval (set by the command ``purge_interval <time-value>`` or ``drop_after_inactive <time-value>``) to both the absolute minimum time required to maintain tracks and the 'detection window time' for each mode on each sensor connected to the track processor. The absolute minimum time required to maintain tracks for any sensor mode is equal to the product of the mode's frame time (set by ``frame_time <time-value>``) and the first value specified by ``hits_to_maintain_track <integer> <integer>``. A sensor mode's detection window is equal to the product of its frame time and the second value for ``hits_to_maintain_track``. For example, the following sensor has an absolute minimum required time of 30 seconds and a detection window of 50 seconds:

::

   sensor my_sensor WSF_GEOMETRIC_SENSOR
      on
      frame_time 10 s
      hits_to_maintain_track 3 5
      [. . .]
   end_sensor

If a track processor connected to this sensor has a purging interval of 20 seconds, it will impossible to maintain tracks based on data from this sensor because the sensor cannot possibly get the three hits it needs within that time. As a result, this check will generate an error-level message. If the track processor has a purging interval of 45 seconds, it is possible but unlikely for the sensor to get the three hits it needs since its detection window is greater than the purging window, and the check will generate a warning-level message. This check uses the equivalent of the 'ScenarioAnalyzerUtils.LinkedAndReachablePlatformPartsChooseProcs <#arraywsfplatformpart-linkedandreachableplatformpartschooseprocswsfplatformpart-origin-string-parttype-arraystring-processortypes-bool-followspecifiedprocs>'__ script method to find the track processors to which a sensor is linked, and excludes intermediate external links from other track processors. See that helper method's description for a more detailed explanation of how linked and reachable track processors are discovered.

.. _Track_processors_should_have_purge_interval_defined:

Track processors should have purge interval defined
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every track processor (``WSF_TRACK_PROCESSOR``) has a non-zero purge interval defined. The purge interval, defined by ``purge_interval <time-value>`` or ``drop_after_inactive <time-value>``, determines how long the track processor will wait without receiving an update on a track before dropping it. If a purge interval has not been defined or has been set to 0 seconds, tracks will never be purged.

.. _Track_processors_should_not_circularly_report_fused_tracks:

Track processors should not circularly report fused tracks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks for the situation in which two track processors (``WSF_TRACK_PROCESSOR``) report fused tracks to each other. Whether a track processor reports fused tracks is set by the command ``fused_track_reporting <boolean-value>`` and defaults to ``false``. Two track processors report to each other if they are mutually 'linked and reachable.' This check uses the equivalent of the 'ScenarioAnalyzerUtils.LinkedAndReachablePlatformPartsChooseProcs <#arraywsfplatformpart-linkedandreachableplatformpartschooseprocswsfplatformpart-origin-string-parttype-arraystring-processortypes-bool-followspecifiedprocs>'__ script method to find the track processors to which another track processor reports, and excludes intermediate external links from other track processors. See that helper method's description for a more detailed explanation of how linked and reachable track processors are discovered.

Here is an example of circular track reporting:

::

   platform wm_platform_1 WSF_PLATFORM
      commander wm_platform_2
      add processor data_mgr_1 WSF_TRACK_PROCESSOR
      purge_interval 60 s
      report_to commander
      report_fused_tracks
      end_processor
      [. . .]
   end_platform

   platform wm_platform_2 WSF_PLATFORM
      add processor data_mgr_2 WSF_TRACK_PROCESSOR
      purge_interval 60 s
      report_to subordinates
      fused_track_reporting on
      end_processor
      [. . .]
   end_platform

.. _User_configured_speed_should_be_within_mover_capabilities:
   
User configured speed should be within mover capabilities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For every platform with a mover derived from ``WsfWaypointMover``, checks that no user-configured speed for any waypoint in the platform's route exceeds the mover's ``maximum_speed``. Movers derived from ``WsfWaypointMover`` include ``WSF_AIR_MOVER``, ``WSF_GROUND_MOVER``, ``WSF_SURFACE_MOVER``, and ``WSF_SUBSURFACE_MOVER``. A user may configure the speed at a waypoint within a platform's ``route`` block with the command ``position <Latitude> <Longitude> [<Altitude>] speed <real> <speed-unit``>. Note that a mover will never exceed its maximum speed, even if the user sets the speed for a waypoint to a value greater than the maximum.


.. _Deployed_weapons_should_have_quantity_greater_than_zero:

Deployed weapons should have quantity greater than zero
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Iterates over all explicit weapons (weapons for which a ``launched_platform_type`` has been defined) on each platform in the simulation and verifies that every explicit weapon has been assigned a non-zero quantity.

IADS C2 Checks
--------------

.. _Asset_managers_should_have_update_interval_defined:

Asset managers should have update interval defined
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every asset manager (``WSF_UNCLASS_ASSET_MANAGER``) has a non-zero update interval defined using ``update_interval <time-reference>``. If the update interval is not defined or is set to zero, the asset manager will never be updated.

.. _Asset_manager_platforms_should_deploy_disseminate_C2_manager:

Asset manager platforms should deploy disseminate C2 manager
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform deploying an asset manager also has a disseminate C2 manager (``WSF_UNCLASS_DISSEMINATE_C2``).

.. _Battle_manage_platforms_must_be_C2_capable:

Battle manager platforms must be C2 capable
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform deploying a sensors manager (``WSF_SIMPLE_SENSORS_MANAGER``, ``WSF_FOV_SENSORS_MANAGER``) also deploys a track processor (``WSF_TRACK_PROCESSOR``) and a disseminate C2 manager (``WSF_UNCLASS_DISSEMINATE_C2``). Platforms with a sensors manager must also deploy an asset manager (``WSF_UNCLASS_ASSET_MANAGER``), but the check does not look for an asset manager since platform initialization will fail before the check runs if one is not present.

.. _Battle_managers_must_have_subordinate_weapons_managers_linked_by_C2_capable_platforms:

Battle managers must have subordinate weapons managers linked by C2-capable platforms
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform with a battle manager (``WSF_UNCLASS_BM``) has at least one platform with a weapons manager (``WSF_WEAPONS_MANAGER_SAM`` or ``WSF_WEAPONS_MANAGER_AI``) subordinate to it on the default command chain. Every platform between the battle manager and its subordinate weapons managers (including the battle manager and weapons manager platforms) must be 'C2-capable.' In the context of this check, a platform is 'C2-capable' if it has (1) an asset manager, (2) a disseminate C2 manager, and (3) ``max_assignments`` > 0.

.. _Battle_managers_should_be_reachable_by_subordinate_sensors:

Battle managers should be reachable by subordinate sensors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that each battle manager is reachable by every sensor that is subordinate to it on any command chain. A sensor can reach a battle manager if a track processor on the battle manager's platform is 'linked and reachable' from the sensor. This check uses the equivalent of the 'ScenarioAnalyzerUtils.LinkedAndReachablePlatformPartsChooseProcs <#arraywsfplatformpart-linkedandreachableplatformpartschooseprocswsfplatformpart-origin-string-parttype-arraystring-processortypes-bool-followspecifiedprocs>'__ script method to find the track processors to which another track processor reports, and excludes intermediate external links from other track processors. See that helper method's description for a more detailed explanation of how linked and reachable track processors are discovered.

.. _Battle_managers_should_have_subordinate_sensors_managers_linked_by_C2_capable_platforms:

Battle managers should have subordinate sensors managers linked by C2-capable platforms
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform with a battle manager (``WSF_UNCLASS_BM``) has at least one platform with a sensors manager (``WSF_SIMPLE_SENSORS_MANAGER`` or ``WSF_FOV_SENSORS_MANAGER``) subordinate to it on the default command chain. Every platform between the battle manager and its subordinate sensors managers (including the battle manager and sensors manager platforms) must be 'C2-capable.' In the context of this check, a platform is 'C2-capable' if it has (1) an asset manager and (2) a disseminate C2 manager.

.. _Battle_managers_should_not_conflict_with_each_other:

Battle managers should not conflict with each other
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that no battle manager with commit authority is subordinate to another battle manager with commit authority on the default command chain. Commit authority on a battle manager is set with the command ``commit_authority <boolean-value>``.

.. _Disseminate_C2_manager_platforms_must_have_internal_comm_links:

Disseminate C2 manager platforms must have internal comm links
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For every platform with a disseminate C2 manager (``WSF_UNCLASS_DISSEMINATE_C2``), checks that at least one communications device (``WSF_COMM_TRANSCEIVER``, ``WSF_COMM_XMTR``, ``WSF_COMM_RCVR``) on the platform has an internal link (``internal_link``) to the disseminate C2 manager.

.. _Sensors_manager_platforms_must_be_C2_capable:

Sensors manager platforms must be C2 capable
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform deploying a sensors manager (``WSF_SIMPLE_SENSORS_MANAGER``, ``WSF_FOV_SENSORS_MANAGER``) also deploys a track processor (``WSF_TRACK_PROCESSOR``), and a disseminate C2 manager (``WSF_UNCLASS_DISSEMINATE_C2``). Platforms with a sensors manager must also deploy an asset manager (``WSF_UNCLASS_ASSET_MANAGER``), but the check does not look for an asset manager since platform initialization will fail before the check runs if it is not present.

.. _Sensors_manager_platforms_must_be_connected_to_battle_manager_with_commit_authority:

Sensors manager platforms must be connected to battle manager with commit authority
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform deploying a sensors manager (``WSF_SIMPLE_SENSORS_MANAGER``, ``WSF_FOV_SENSORS_MANAGER``) is subordinate to a battle manager (``WSF_UNCLASS_BM``) with commit authority on the default command chain. Commit authority on a battle manager is set with the command ``commit_authority <boolean-value>``.

.. _Sensors_managers_must_not_conflict_with_each_other:

Sensors managers must not conflict with each other
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks for command chain configurations that result in sensors manager (``WSF_SIMPLE_SENSORS_MANAGER``, ``WSF_FOV_SENSORS_MANAGER``) conflicts, which occur from the perspective of weapons managers (``WSF_WEAPONS_MANAGER_SAM``, ``WSF_WEAPONS_MANAGER_AI``). For any weapons manager subordinate to a battle manager (``WSF_UNCLASS_BM``) with commit authority on the default command chain, a sensors manager conflict exists if there is more than one platform between the weapons manager platform and the battle manager platform (including the battle manager platform) on the default command chain that has a sensors manager. However, there is never a conflict from the perspective of a weapons manager platform if the weapons manager platform deploys its own sensors manager, regardless of the number of other sensors managers between it and the battle manager. This means that the same configuration of sensors managers in the default command chain can create sensors manager conflicts relative to some subordinate platforms while avoiding conflicts relative to others. 
.. _Sensors_manager_platforms_must_be_connected_to_TAR_or_TTR:

Sensors manager platforms must be connected to TAR or TTR
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform deploying a sensors manager (``WSF_SIMPLE_SENSORS_MANAGER``, ``WSF_FOV_SENSORS_MANAGER``) is superior on the default command chain to at least one platform that has a sensor of type TAR or TTR. The check is satisfied if the sensors manager platform has a TAR or TTR onboard. A sensor's type is determined by the command ``category <EW/TAR/TTR/RWR>``, regardless of the attributes of the sensor or the class that it instantiates. For example, a ``WSF_GEOMETRIC_SENSOR`` assigned a category of TAR will be considered a TAR, both for the purposes of this check and in the IADS C2 code.

.. _Sensors_manager_max_acquistion_times_should_be_long_enough_for_subordinate_sensors_to_form_tracks:

Sensors manager max acquisition times should be long enough for subordinate sensors to form tracks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that sensors managers have maximum TAR and TTR acquisition times that are sufficiently long for their subordinate TARs and TTRs to form tracks. The check compares the sensors manager's maximum acquisition times (set by the commands ``max_ttr_acquisition_time <time-value>`` and ``max_tar_acquisition_time <time-value>``) to both the absolute minimum time required to form tracks and the 'detection window time' for each mode on each sensor subordinate to the sensor on the default command chain. The absolute minimum time required to form tracks for any sensor mode is equal to the product of the mode's frame time (set by ``frame_time <time-value>``) and the first value specified by ``hits_to_establish_track <integer> <integer>``. A sensor mode's detection window is equal to the product of its frame time and the second value for ``hits_to_establish_track``. This check is necessary because a sensors manager will CANTCO a sensor's assignment if the sensor has not started tracking within the maximum acquisition time of being cued.

.. _Platforms_with_TAR_or_TTR_should_be_connected_to_sensors_manager:

Platforms with TAR or TTR should be connected to sensors manager
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform with a sensor of type TAR or TTR is subordinate on the default command chain to a platform deploying a sensors manager (``WSF_SIMPLE_SENSORS_MANAGER``, ``WSF_FOV_SENSORS_MANAGER``). The check is satisfied if the sensor platform deploys a sensors manager. A sensor's type is determined by the command ``category <EW/TAR/TTR/RWR>``, regardless of the attributes of the sensor or the class that it instantiates. For example, a ``WSF_GEOMETRIC_SENSOR`` assigned a category of TAR will be considered a TAR, both for the purposes of this check and in the IADS C2 code. Note that it is possible for a platform with a TAR or TTR that passes this check to receive conflicting commands from multiple sensors manager. Another IADS C2 check, 'Sensors managers must not conflict with each other <#sensors-managers-must-not-conflict-with-each-other>'__, tests for this situation.

.. _TTRs_managed_by_FOV_sensors_manager_must_not_manipulate_on_off_state:

TTRs managed by FOV sensors manager must not manipulate on/off state
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every sensor with a category of ``TTR`` that is subordinate on the default command chain to a platform with a ``WSF_SENSORS_MANAGER_FOV`` is initially turned off. While it is possible that an analyst creating a scenario may define a script that manipulates a TTR's on/off state, the Scenario Analyzer is not able to detect the content of scripts, so this check simply ensures that TTRs managed by a field-of-view sensors manager have not been turned on.

.. _TTRs_managed_by_FOV_sensors_manager_must_use_default_sensor_scheduler:

TTRs managed by FOV sensors manager must use default sensor scheduler
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every sensor with a category of ``TTR`` that is subordinate on the default command chain to a platform with a ``WSF_SENSORS_MANAGER_FOV`` uses the default sensor scheduler. A sensor's scheduler may be specified with the ``scheduler`` command, e.g., ``scheduler default`` or ``scheduler physical_scan``. A sensor will use the default sensor scheduler if no scheduler is selected.

.. _TTRs_managed_by_FOV_sensors_manager_should_provide_auxiliary_data:

TTRs managed by FOV sensors manager should provide auxiliary data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every sensor with a category of ``TTR`` that is subordinate on the default command chain to a platform with a ``WSF_SENSORS_MANAGER_FOV`` provides the following auxiliary data: ``RESTING_AZIMUTH``, ``COARSE_SLEW_RATE_AZIMUTH``, and ``FINE_SLEW_RATE_AZIMUTH``.

.. _TTRs_managed_by_FOV_sensors_manager_should_use_one_beam_per_mode:

TTRs managed by FOV sensors managers should use one beam per mode
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every sensor with a category of ``TTR`` that is subordinate on the default command chain to a platform with a ``WSF_SENSORS_MANAGER_FOV`` has exactly one beam defined for each of its modes. 
.. _Platforms_with_weapons_should_deploy_weapons_manager:

Platforms with weapons should deploy weapons manager
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform with at least one weapon also deploys a weapons manager (``WSF_WEAPONS_MANAGER_SAM`` or ``WSF_WEAPONS_MANAGER_AI``).

.. _Weapons_manager_platforms_must_be_C2_capable:

Weapons manager platforms must be C2 capable
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform deploying a weapons manager (``WSF_WEAPONS_MANAGER_SAM``, ``WSF_WEAPONS_MANAGER_AI``) also deploys a track processor (``WSF_TRACK_PROCESSOR``), and a disseminate C2 manager (``WSF_UNCLASS_DISSEMINATE_C2``). Platforms with a weapons manager must also deploy an asset manager (``WSF_ASSET_MANAGER``), but the check does not look for an asset manager since platform initialization will fail before the check runs if one is not present.

.. _Weapons_manager_platforms_must_deploy_weapons:

Weapons manager platforms must deploy weapons
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform deploying a weapons manager also has at least one weapon (e.g., ``WSF_EXPLICIT_WEAPON``, ``WSF_IMPLICIT_WEAPON``, or ``WSF_RF_JAMMER``) on the platform.

.. _Weapons_manager_platforms_must_be_connected_to_battle_manager_with_commit_authority:

Weapons manager platforms must be connected to battle manager with commit authority
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Checks that every platform deploying a weapons manager (``WSF_WEAPONS_MANAGER``, ``WSF_WEAPONS_MANAGER_SAM``, ``WSF_WEAPONS_MANAGER_AI``) is subordinate to a battle manager (``WSF_UNCLASS_BM``) with commit authority on the default command chain. Commit authority on a battle manager is set with the command ``commit_authority <boolean-value>``.

.. _Weapons_manager_platforms_should_have_access_to_required_sensor:

Weapons manager platforms should have access to required sensor
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For every platform with a weapons manager (``WEAPONS_MANAGER_SAM``, ``WEAPONS_MANAGER_AI``), checks that the platform has access to a sensor of at least the quality required by the weapons manager. The quality of sensor required by a weapons manager is determined by the ``engagement_settings`` block, which sets whether the weapons manager will engage tracks originating from sensors of each category. A weapons manager with the following engagement settings block requires, and should have access to, a TAR or a TTR:

::

   engagement_settings
      ew_targets false
      tar_targets true
      ttr_targets true
   end_engagement_settings

Keep in mind that sensor's type is determined by the command ``category <EW/TAR/TTR/RWR>``, regardless of the attributes of the sensor or the class that it instantiates. For example, a ``WSF_GEOMETRIC_SENSOR`` assigned a category ``TAR`` will be considered a TAR, both for the purposes of this check and in the IADS C2 code. A weapons manager platform 'has access' to a sensor if the sensor appears on a platform that is connected to the weapons manager platform through any of its command chains. The sensor could be on a platform that is subordinate to, superior to, or a peer of the weapons manager platform.

.. _WMAI_platforms_using_RWR_response_must_deploy_an_RWR:

WMAI platforms using RWR response must deploy an RWR
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For each platform with a ``WSF_WEAPONS_MANAGER_AI`` for which RWR response has been enabled, checks that the platform has a ``WSF_ESM_SENSOR`` with the category ``RWR`` that is internally linked to a track processor, which in turn must point to the platform's master track manager. RWR response is enabled by using an ``enable`` command within the ``rwr_response`` block. A platform will fail the check if the platform has multiple track processors, and the track processor to which the RWR is linked has been designated the ``non_master_track_processor``.

.. _WMAI_platforms_using_RWR_response_should_have_RWR_tuned_to_detect_enemy_TTRs:

WMAI platforms using RWR response should have RWR tuned to detect enemy TTRS
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For each platform with a ``WSF_WEAPONS_MANAGER_AI`` for which RWR response has been enabled, checks that the RWR's ``frequency_band`` is tuned in such a way that at least one enemy TTR is transmitting within the RWR's frequency band. The check will warn if the RWR will pick up no enemy sensors at all or if it will pick up lower-quality (EW or TAR) sensors.

.. _Weapons_on_WMAI_platforms_should_required_auxiliary_data:

Weapons on WMAI platforms should have required auxiliary data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

For each platform with a ``WSF_WEAPONS_MANAGER_AI`` for which RWR response has been enabled, checks that each weapon on the platform has the following auxiliary data: ``maxTOF``, ``avgSpd``, ``maxRng``, ``minRng``, ``shootDelay``, and ``estPK``.

Session Notes
-------------

.. _Missing_platforms:

Missing Platforms
^^^^^^^^^^^^^^^^^

Informs user of every platform that is part of the scenario but that is not active in the simulation at the time Checks are run. If the platform has a creation time that is later than the time Checks are run, informs user that platform is likely missing because it has not yet been created. If the platform has a creation time that is earlier than the time Checks are run, informs user that the platform may be missing because it has been deleted. If this Session Note concludes that any platforms are missing from the simulation, the Scenario Analyzer will offer the user the option to adjust the time at which Checks are run. For more information, see :ref:`Advanced Feature: Modifying Check Execution Time <Modifying_check_execution_time>`.

|  :doc:`Previous: Using the Scenario Analyzer <scenario_analyzer_overview>`
|  :doc:`Next: Descriptions of Script Utilities <scenario_analyzer_utilities>`
