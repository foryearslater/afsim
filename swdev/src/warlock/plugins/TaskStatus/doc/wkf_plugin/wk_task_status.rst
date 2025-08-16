.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

Task Status - Warlock
---------------------

The Warlock Task Status provides the user an interface for reviewing and canceling task assignments for the selected platform.

.. image:: ../images/wk_TaskStatus.png

Status
======

The following table presents that information that may be displayed. For a given assigner, the number of assigned and received tasks will be shown for each :model:`WSF_TASK_PROCESSOR` on the assigner along with that processor's name.

The tasks will be group according to their associated track. This information is equivalent to invoking the script methods :method:`WsfTaskManager.AssignedTaskList` and :method:`WsfTaskManager.ReceivedTaskList` for a given WsfTrackId.

+-----------------------+---------------------+----------------+-----------+-----------+-----------+-------------+
| Column 1              | Column 2            | Column 3       | Column 4  | Column 5  | Column 6  | Column 7    |
+=======================+=====================+================+===========+===========+===========+=============+
| Assigned/Received (#) | Task processor name |                |           |           |           |             |
+-----------------------+---------------------+----------------+-----------+-----------+-----------+-------------+
| - Track #             | Local track number  |                |           |           |           |             |
+-----------------------+---------------------+----------------+-----------+-----------+-----------+-------------+
|   - Fire              | Assignee            | Weapon name    | Task type |           |           |             |
+-----------------------+---------------------+----------------+-----------+-----------+-----------+-------------+
|   - Jam               | Assignee            | Jammer name    | Task type | Frequency | Bandwidth | Beam number |
+-----------------------+---------------------+----------------+-----------+-----------+-----------+-------------+
|   - Track             | Assignee            | Sensor name    | Task type |           |           |             |
+-----------------------+---------------------+----------------+-----------+-----------+-----------+-------------+
|   - Custom            | Assignee            | Processor name | Task type |           |           |             |
+-----------------------+---------------------+----------------+-----------+-----------+-----------+-------------+

Cancel
======

* To cancel a task, right-click on the task entry and select "Cancel Task".
* To cancel all tasks for a track, right-click on the track entry and select "Cancel All Tasks".
* To cancel all weapon tasks for a track, right-click on the track entry and select "Abort Firing At".
* To cancel all assignments, right-click on the Assigned entry and select  "Cancel All Tasks".
