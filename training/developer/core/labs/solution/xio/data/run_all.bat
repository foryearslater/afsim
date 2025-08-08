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
	set WARLOCK_DIR=..\..\..\..\..\..\..\swdev\build\wsf_install\bin
) else (
    echo Assuming development directory structure
	if exist ..\..\..\..\..\build\wsf_install\bin (
		set WARLOCK_DIR=..\..\..\..\..\build\wsf_install\bin
	)
)

if defined WARLOCK_DIR (
   start !WARLOCK_DIR!\warlock xio_scenario.txt

   rem pause for 6 seconds
   rem choice /T 6 /D N /n

   start .\run_flight_controller.bat
   
) else (
	echo "Error: Cannot find warlock directory"
)
