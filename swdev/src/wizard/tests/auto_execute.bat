@echo off
rem ****************************************************************************
rem CUI
rem
rem The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
rem
rem The use, dissemination or disclosure of data in this file is subject to
rem limitation or restriction. See accompanying README and LICENSE for details.
rem ****************************************************************************
rem This is invoked by auto_test.pl to execute a test case under Windows
rem   echo "Usage auto_execute.sh <exe-file> <input-base> <output-file>"

rem Let the application know that it is running an automated test.
set AUTO_TEST=true

if exist %2.bat goto CallBatch
if exist %2.pl goto CallPerl

   %1 -script %5 -script %2 -script %4>>%3 2>&1
   goto Exit

:CallBatch
   call %2.bat %1 -script %2 >>%3 2>&1
   goto Exit

:CallPerl
   perl %2.pl %1 -script %2 >>%3 2>&1
   goto Exit

:Exit
   exit %ERRORLEVEL%


