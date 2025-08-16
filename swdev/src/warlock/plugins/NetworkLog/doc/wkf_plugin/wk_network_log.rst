.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Network Log - Warlock
=====================

Summary
-------

The Network Log is primarily a debugging tool for the :doc:`Warlock Network<../warlock_pref_network>` interface. It displays a filterable list of all the sent and received packets, their contents, and every time a network error occurs or the network reconnects.

The Network Log is accessible from Developer->Network Log. If the Developer menu is not visible, enable it in :doc:`Preferences<../warlock_preferences>`.

.. image:: ../images/wk_network_log.png

As an example, this filter will accept all ChatMessage packets sent by the white_cell before a certain time. There are some additional filters for blue_cell and red_cell, but they are disabled and will not be used.

In the top half of this dialog is the Filter List, and in the bottom half is the Results List. Only the packets that pass the filters will be shown.

The first several columns contain the packet metadata, including ``Status``, ``Time``, ``Type``, ``Address``, ``Port``, and ``PID``. These columns have a grey background. The rest of the columns contain the packet's contents.

Along the top are the current Interface and Port from Preferences, and the current Process ID (PID) assigned by the operating system. The PID is used as a disambiguator when multiple instances of Warlock are running on the same computer.

In the top right corner are three buttons.

* **Ping**: Sends a ``NetworkLogPing`` packet to all users on the multicast network. When the packet is received, all the other users will automatically respond with a ``NetworkLogAck`` packet.
* **Clear**: Clears the list.
* **Preferences...**: Opens the network preferences page.

Filter Types
------------

There are 3 types of filters that can be chosen from the dropdown menu.

**Condition**: Allows the user to test how the value in a given column compares to some value. For example, a filter testing for column *Address* compares *==* to *10.7.1.1* will allow all packets sent from that address to pass.

**And**: This filter groups other filters. A packet will pass this filter if it passes all of its sub-filters. If there are no enabled sub-filters, the packet passes.

**Or**: This filter groups other filters. A packet will pass this filter if it passes at least one of its sub-filters. If there are no enabled sub-filters, the packet passes.

Comparison Types
----------------

The following comparisons are always available:

- **==**: Passes if the value in the packet is equal to the given value.
- **!=**: Passes if the value in the packet is NOT equal to the given value.
- **contains**: Passes if the value in the packet, treated as text, contains the given text. This check is not case sensitive.

The following comparisons are available if the given value is numerical:

- **<**: Passes if the value in the packet is less than the given value.
- **<=**: Passes if the value in the packet is less than or equal to the given value.
- **>**: Passes if the value in the packet is greater than the given value.
- **>=**: Passes if the value in the packet is greater than or equal to the given value.

The following comparisons are available if the given value is a Platform ID:

- **is on side**: Passes if the platform is on the given side.
- **is of type**: Passes if the platform is of the given type.
- **is of category**: Passes if the platform is of the given category.

Adding Filters
--------------

There are 2 main ways to add a filter.

1) For And/Or filters, select "Add Child" from its context menu or click the Add (+) button to give it a sub-filter that can be filled in by the user.

2) Select "Create Filter" from a context menu in the Results List. This will create a new filter at the top level.

Rearranging Filters
-------------------

Often times a user may need to rearrange filters, either because their logic has changed, or because a filter was created in the wrong location.
Selecting "Cut" from the filter context menu will remove the selected filter and its children so that they can be pasted elsewhere. A cut or copied filter can be pasted as many times as necessary.

To remove a filter without copying it for later, click the Remove (-) button in its row.
To quickly remove all filters, right-click on the root (top) filter and select "Clear Children".

Enable/Disable Filters
----------------------

The Enable check box indicates whether a filter is enabled. A filter that is not enabled will not be considered when updating the Results List.
If a grouping filter is disabled but it has sub-filters that are enabled, none of the sub-filters will be considered.
I.e. in order for a filter to be checked, all of its ancestors must also be enabled.

To disable all filters, one simply needs to disable the root item.

Invert Filters
--------------

The Invert check box allows the user to invert the result of a filter. E.g. the "contains" test becomes "does not contain". An inverted AND, aka NAND, has the logic "Passes if at least one sub-filter does not pass". An inverted OR, aka NOR, has the logic "Passes if all sub-filters do not pass".

Import & Export
---------------

In the filter context menu there are two options called "Import" and "Export".

Export will save the selected filter, including all its children, so it can be imported later. Exported filters are given names to make them easy to find again in the future.

Import will paste an exported filter at the selected filter. Once pasted, modifying the imported filter will not affect the exported filter.

Exported filters are saved and can still be imported after closing and re-opening the application.

Inspect Packet
--------------

.. image:: ../images/wk_network_log_details.png

Double-clicking an item in the Results List will bring up a small dialog with a copy of the row's contents. List and object data in this dialog is expandable, and is the only way to view list contents.

Hide Columns
------------

In the column header context menu there are two options called "Hide <column-name>" and "Show All Hidden Columns".

Hidden columns will still participate in filtering, but will not be shown.

Export to CSV
-------------

Selecting "Export to CSV" from the Results List context menu will save the filtered rows to a CSV file.
