.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Sensor Controller - Warlock
---------------------------

.. image:: ../images/wk_SensorController.png

The Sensor Controller plugin gives the user basic control over sensors on the selected platform. This includes turning the sensor 
on/off and cueing to a given azimuth/elevation (AzEl) or track. A sensor can only be cued if the following conditions are met:

   - The sensor's :command:`_.articulated_part.slew_mode` and :command:`sensor_mode.cue_mode` are not set to "fixed"
   - The sensor's platform is not externally controlled.
   - The sensor is turned on and operational.

When using AzEl, the angles are relative to the horizontal plane (no pitch or roll) of the platform. Hovering over the AzEl edits
will show the acceptable slew limits for the given sensor. The azimuth limits are defined by the minimum of the 
:command:`_.articulated_part.azimuth_slew_limits` and :command:`sensor_mode.azimuth_cue_limits`. The elevation limits are defined by
the :command:`_.articulated_part.elevation_slew_limits` and the :command:`sensor_mode.elevation_cue_limits`.

.. warning:: Attempting to use the controls on a sensor that has scripting will likely have unpredictable consequences. 
