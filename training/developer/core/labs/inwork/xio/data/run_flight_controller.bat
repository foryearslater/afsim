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
	set FLIGHT_CONTROLLER_DIR=..\..\..\..\..\..\..\swdev\build\wsf_install\bin
) else (
    echo Assuming development directory structure
	if exist ..\..\..\..\..\build\wsf_install\bin (
		set FLIGHT_CONTROLLER_DIR=..\..\..\..\..\build\wsf_install\bin
	)
)

if defined FLIGHT_CONTROLLER_DIR (
   start !FLIGHT_CONTROLLER_DIR!\flight_controller flight_controller_config.txt
   
) else (
	echo "Error: Cannot find flight controller directory"
)
