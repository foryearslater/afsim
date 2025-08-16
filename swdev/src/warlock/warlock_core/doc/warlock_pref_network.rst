.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Network Preferences - Warlock
=============================

.. image:: images/warlock_network_prefs.png

The Network Preferences page has the settings for a multicast network that Warlock plugins can use to share information not handled by :command:`DIS<dis_interface>` or :command:`XIO<xio_interface>`.

* Address - This is the multicast address. All connected sims should use the same address.
* Interface - This is the address of the local port. This list is set by scanning network interfaces. The '+' button adds a new interface that was not detected automatically.
* Port - This is the port used for sending and receiving data. All connected sims should use the same port.

Affected Features
-----------------

..include:: wk_network_extension.txt
