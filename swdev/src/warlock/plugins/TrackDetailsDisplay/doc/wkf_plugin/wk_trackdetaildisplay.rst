.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Track Details Display - Warlock
-------------------------------

The Track Details Display shows a Platform's Track Data in a more visible and consumable manner.

.. image:: ../images/wk_TrackDetailsDisplay.png

The table will populate with tracks based on the platform currently selected in the drop down menu in the upper left hand corner labeled Platform.

Selecting a track requires clicking on the track of interests respective row. Once a track has been selected the bottom display will immediatly populate.

The table can be sorted by column by clicking on a column's header.

The bottom display is broken up into four smaller displays named Basic Info, Track Manager, State Info, Relative Geometry, and Sensor Info.

The Target Platform button allows you to select a platform on the map to view associated tracks.

* In Basic Info you will find general information about your currently selected track.
* The Track Manager display will give some background on how the track came into existance.
* State Info gives detailed information on the selected tracks state as it pertains to the sim and its physical state withing the scenario.
    - The Relative Heading frame of reference is from the currently selected platform to the target.
    - The Frequency refers to the detected frequency of the track.
    - State Info also contains the Covarience Ellipsoid display. This display gives information on the statistical ellipsoid that can be drawn to represent a tracks physical location.
* Relative Geometry display gives information on the track relative to the platform currently selected in the upper left hand corner.
* Sensor Info will give a detailed look into the sensor that is attached to the currently selected platform if it has a attached sensor.
    - The LLA fields here refer to the platforms position not the platform part.
    - The Originator provides the name of platform the track originated from. If the currently selected Platform is the Originator the fields value will be true.
