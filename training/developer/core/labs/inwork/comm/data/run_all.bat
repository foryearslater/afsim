@echo off
rem ****************************************************************************
rem CUI
rem
rem The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
rem
rem The use, dissemination or disclosure of data in this file is subject to
rem limitation or restriction. See accompanying README and LICENSE for details.
rem ****************************************************************************

setlocal EnableExtensions
setlocal EnableDelayedExpansion

if exist ..\..\..\..\..\..\..\swdev\build\wsf_install\bin (
	set MISSION_DIR=..\..\..\..\..\..\..\swdev\build\wsf_install\bin
) else (
    echo Assuming development directory structure
	if exist ..\..\..\..\..\build\wsf_install\bin (
		set MISSION_DIR=..\..\..\..\..\build\wsf_install\bin
	)
)

if defined MISSION_DIR (
   start !MISSION_DIR!\mission comm_scenario1.txt

   rem pause for 4 seconds
   choice /T 4 /D N /n

   start !MISSION_DIR!\mission comm_scenario2.txt

   rem pause for 4 seconds
   choice /T 4 /D N /n

   start .\run_results.bat
) else (
	echo "Error: Cannot find mission directory"
)
