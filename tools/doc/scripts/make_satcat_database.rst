.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

.. _Make_Satcat_Database:

Make SATCAT Database
--------------------

**make_satcat_database.py** is a python script that converts raw SATCAT satellite definitions to both AFSIM platform definitions and a JSON database to be used in the Wizard's Satellite Inserter tool.

Usage
=====

make_satcat_database.py [-h | --help] *<data>* *<new_database>* *<new_definitions_file>*

Where:

[-h | --help] - An optional command to show help message.

*<data>* - The raw SATCAT data. The most up-to-date SATCAT data can be found at `SATCAT <https://www.celestrak.com/pub/satcat.txt>`_.

*<new_database>* - The JSON database that will contain the JSON representation of the SATCAT satellites and their file locations.

*<new_definitions_file>* - A .txt file that holds the SATCAT satellites' AFSIM platform definitions.
