# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
Using the COCO/R compiler generator
===================================

The COCO/R compiler generator is used to generate the 'compiler' for the WSF scripting language.

Files that define the scripting language and run COCO:
------------------------------------------------------

   script.atg                The COCO/R grammar file that defines the WSF script language.
                             This is THE input file for generating the compiler.

   update_parser.bat         Invokes update_parser.py

   update_parser.py          Invokes COCO.exe to generate the scanner and parser and then
                             edits them to produce the final product.

Additional files used by COCO/R:
--------------------------------

   Copyright.frame           Copyright information inserted by COCO.exe at the start of
                             each generated file. Modified to include our copyright header.

   Scanner.frame             COCO/R frame file for the scanner. This is a slightly modifed
                             version of the standard file.

   Parser.frame              COCO/R frame file for the parser. THis is a slightly modified
                             version of the standard file.

Reference material (not used in generating the compiler)  (See util_script/source/script_parser directory):
---------------------------------------------------------

   UserManual.pdf            The COCO/R user manual

   license.txt               The GNU license file for COCO/R

Generating the Scanner/Parser
=============================

1) Set your working directory to this directory (afsim/wizard/lib/source/Us).
2) Acquire a copy of COCO.exe and put it in this current directory.
   A copy of this can be located in '\\slfilevm1\sllab$\software\DevTools\COCO-R``.
3) Edit, if necessary, update_parser.bat to set the path to Python.
4) Invoke 'update_parser.bat' either from the command line or by double-clicking on it.
   This simply invokes 'update_parser.py'.

'update_parser.py' invokes COCO.exe, which does the following:

1) Reads 'script.atg', 'Copyright.frame', 'Scanner.frame' and 'Parser.frame'.
2) Generates 'Scanner.cpp', 'Scanner.h', 'Parser.cpp', and 'Parser.h'. Any existing versions
   of the output files are renamed with a '.old' suffix prior to generating the new output.

'Scanner.cpp/.h[.old]' and 'Parser.cpp/.h[.old]' are DEFINED TO BE IGNORED BY 'Subversion'.

These files, along with any modified .frame file or script.atg file, should be commited.

NOTE
====
Changes to these Scanner/Parser requires corresponding changes to the AFSIM tools Scanner/Parser files.
See util_script/source/script_parser/AAA_README.TXT.
