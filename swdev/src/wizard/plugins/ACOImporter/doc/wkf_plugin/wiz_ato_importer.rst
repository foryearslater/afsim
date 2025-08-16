.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

ATO Importer - Wizard
------------------------

Overview
********
The Air Tasking Order (ATO) Importer provides the capability to extract platforms from a valid, well-formed ATO file and produce an AFSIM scenario laydown file with the extracted platforms.

Quick Start
***********
Within Wizard, select the ATO Importer option from the Tools -> Importers menu (Figure 1).  The main dialog box used for converting ATO/ACO files will appear (Figure 2).

.. image:: ../images/wiz_tool_menu.png

*Figure 1.  Opening the ACO/ATO Importer Dialog from the Wizard Tools Importers menu*

The Importer has a few user entry options.

* Source Files (Required entry) - The user adds files to this text list using the Add File/Folder buttons for input files that are to be converted to AFSIM Scenario files.
* Alternatively to the use of Add File/Folder you can drag and drop files in the Source Files display box.
* Delete Button will remove selected files from the menu.
* Directory - Specifies the folder in which the generated AFSIM Scenario files will be placed in.

.. image:: ../images/aco_ato_Importer_dialog.png

*Figure 2.  The ACO/ATO Importer Dialog.*

* Import Button
   The importer will then process each file specified and will place potentially two files in the selected output directory. One file indicating any errors that occurred in the process.
   And another which is the scenario file with the extracted platforms. Each file will be prepended with the name of the input file that was processed.

   Additionally, ATO's that are being imported may have needed type definitions. These type definitions will be automatically generated and placed in a subfolder named type_definitions.

   For example:
   If you select an ATO file named myAto.txt then the result will be potentially be two files and one directory with multiple type definition files:

   * myAto_Export.txt - The Scenario file for inclusion in your simulation.
   * myAto_ImportErrors.txt - If errors occurred this file will be produced outlining what they were.
   * type_definition/
          - F16.txt (This file will be generated if the ATO references a platform of type F16)

   A few dialog boxes may appear indicating success or failure of import for each file being processed.

   .. note:: The ATO Importer will extract correct platforms despite encountering errors in others. The error file will include the details of the specific issues found.
   .. note:: When importing an ATO, if there are definitions already defined in type_definitions directory, the importer will NOT overwrite those files. This is expected behavior, so that scenario designers can apply their definitions to any new incoming platform of that type.


* Close Button - There is a close button on the status dialog and the main importer dialog, which will close the respective dialog box.

.. note:: The ATO is a high level document defining potentially many mission assignments. There are many complications from this level of fidelity, and the Limitations section below describes many of them.

Limitations
***********
* The importer can only process segments defined by Unit Tasking assignments that include an AMSNDAT and MSNACFT set. This means, that some missions types that do not include those sets are not currently processed.
* Given the abundant optional fields defined in the USMTF Specification MIL-STD-6040, there are not many fields guaranteed to be present that are representable as AFSIM entities. Because of this fact,
  we have created a system of prioritized assumptions regarding the extraction of a platform and its attributes. They method employed is detailed below:

  1. The import first extracts out all segments that are defined by a TASKUNIT set. This is the mandatory set described by rule 14A, distinguished as Set 14 in the MIL-STD-6040.
  2. Once those segments are found and grouped, they are processed for platform extraction. Processing first identifies the sets that can identify platforms, their type, and how many are needed. There are several sets that may describe platforms.

   a. TASKUNIT
   b. MSNACFT
   c. 5REFUEL
   d. 9PKGDAT  //Note, there are more sets in the ATO that might describe platforms. We hope to work with the users in the future to add them into the supported list.


  3. Once platforms, types, and number are extracted, for platforms to have immediate usefulness in the scenario laydown, they must have a global position. The processor now works to fill in that positional information as best it can with whats provided.
  4. As noted above, a platforms positional data is often described by the spec in an optional field. The strategy for selecting which positional data is appropriate, if present, is a priority one. If no positional data is found a Macro will be defined for the user to fill out manually. The priority search is detailed below:


   a. If Task Unit has its task unit location field defined, this takes priority. (This is field 2 of the TASKUNIT set defined by the MIL-STD-6040).

       - The task unit location may come in many forms.
         I. ICAO Base Code - If this is provided, resolution of its known latitude / longitude coordinates are attempted. If found, they are the position of all found platforms in the segment.
         II. Place name, a literal value of free text. If this is found, a macro substitution will be required, and its written for you via the name of the literal.
         III. Geographic Lat/Lon in Minutes. If found, position is directly used as is.


   b. If Task unit location is not present, Aircraft Mission Datas Departure Location is now considered. (This is Field 9 of the AMSNDAT set defined by the MIL-STD-6040.)
       - This to can come in many forms. This includes the same format as task unit above and additionally:
         I. UTM-100 Meter, which will get converted to lat/long.
   c. If none above are present, then first found Aerial Refueling Info's Refueling Control Points is used. (This is Field 4 of the ARINFO set defined by the MIL-STD-6040.)
      - As with the others this takes many forms, but they essentially align with the above options.
   d. If none of the above are found, Air Asset Control Report in Point is considered next. (This is Field 6 of CONTROLA set defined by the MIL-STD-6040.)
      - As with the others, this takes many forms. If convertible to lat/long, it will be, if not, macro substitution as described above occurs.
   e. And lastly, if no positional data can be found in the above criteria, a macro substitution is defined with the taskunit unit data.


  5. The above process described in step 4 repeats for all found MSNACFT set within the TaskUnit segment.


