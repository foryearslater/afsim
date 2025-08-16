.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Platform Details - Wizard
-------------------------

The Platform Details dialog lists the selected platform's mutable attributes.

.. image:: ../images/wizard_platform_details.png

Mutable attributes include: 

.. list-table::
   :widths: 20 80

   * - Side
     - The platform's :command:`platform.side` / team.
   * - Type
     - The platform's type, as defined by :command:`platform.platform_type`.
   * - Icon
     - The platform's visual representation in the :doc:`Map Display<../wkf_plugin/wiz_map_display>`.
   * - Latitude
     - The platform's initial latitudinal :command:`platform.position`.
   * - Longitude
     - The platform's initial longitudinal :command:`platform.position`.
   * - Altitude MSL
     - The platform's initial :command:`platform.altitude`.
   * - Heading
     - The platform's initial :command:`platform.heading`.
   * - Pitch
     - The platform's initial pitch.
   * - Roll
     - The platform's initial roll.	 
	 
To edit an attribute: double-click the attribute's value, type the desired value, then press enter or select a different attribute.

   .. note::
      Some platform attributes are also mutable via dropdown menu. For said attributes, a dropdown menu is expandable via a down-arrow button, located to the right of the attribute's value.

When an attribute is modified in the Platform Details dialog, the value is updated in the AFSIM script definition and in text.

Changes are visible in the :doc:`Map Display<../wkf_plugin/wiz_map_display>`.

Toggle the Platform Details dialog via the :doc:`View Menu<../wizard_view_menu>`.
