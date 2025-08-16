.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Warlock
-------

Overview
========

.. image:: images/warlock_displays.png

Core Capability
~~~~~~~~~~~~~~~

Warlock is an application that provides a graphical user interface (GUI) environment in which to view and control Advanced Framework for Simulation, Integration, and Modeling (AFSIM) scenarios. The primary goal of Warlock is to support Operator-in-the-Loop (OITL) war-gaming and experimentation within the AFSIM framework. The capabilities provided by Warlock span engagement, mission, operational, and strategic level simulation and analysis domains. Warlock provides an extensible architecture and corresponding interface to support custom controls and displays.

The configurable central view for Warlock defaults to a map window that shows the state of the AFSIM simulation, surrounded with customizable displays to support engagement, mission, operational, and strategic level simulations. These displays include dialogs to control platforms, sensors, weapons, comms, logistics, and more. Warlock has an unrestricted view into the entire simulation environment, for use within a white cell, and restricted views that give a team perception view, for use within a red or blue cell. An AFSIM script interface exists so that operators can easily execute AFSIM scripts. Additionally, Warlock supports controlling individual AFSIM entities using the generic cockpit capability and flying the entity using a joystick. This allows the operator to take control of an entity and execute specific actions that may otherwise be unavailable to them.

User Experience
~~~~~~~~~~~~~~~

Warlock's user experience focuses on simple and intuitive displays and controls that require a minimal amount of AFSIM knowledge to use. A user is able to use Warlock effectively without taking the AFSIM analyst training courses. Warlock presents the user with a modern user interface where data displays are configurable to the user's personal preference. Warlock supports saving the current configuration of displays, keyboard shortcuts, preferences, and more. These saved configurations can then be loaded at any time and shared between different users. This allows different configurations to exist for different roles within an experiment, such as one configuration for a white-cell operator and another configuration for a blue-cell cockpit control station.

Extensibility
~~~~~~~~~~~~~

Warlock is designed to be extended to meet the requirements of the growing AFSIM community. Warlock provides a well-defined API, enabling developers to quickly create custom extensions. This is similar to how AFSIM supports extensions.

For example, as a P6DOF enhancement, Warlock may be extended to provide additional data unique to the P6DOF model an  provide dialogs to aid the user in tuning the P6DOF model.
Warlock could also be extended to provide additional data about cyber vulnerabilities, conditions, and effects so the operator can understand the impact cyber has on the simulation. Warlock may also be extended to provide specialized cockpit displays in addition to the generic cockpit display.


Additional Information
======================

* :doc:`User's Guide <warlock_users_guide>`
* :doc:`Reference Guide <warlock_reference_guide>`
* :doc:`Change Log <warlock_change_log>`
