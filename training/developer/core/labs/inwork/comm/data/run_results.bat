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

if exist ..\..\..\..\..\..\..\bin (
   echo Assuming release directory structure
	set MYSTIC_DIR=..\..\..\..\..\..\..\bin
) else (
    echo Assuming development directory structure
	if exist ..\..\..\..\..\..\..\..\BUILD\wsf_install\bin (
		set MYSTIC_DIR=..\..\..\..\..\..\..\..\BUILD\wsf_install\bin
	)
)

if defined MYSTIC_DIR (
   start !MYSTIC_DIR!\mystic comm_exercise2.aer
) else (
	echo "Error: Cannot find mystic directory"
)
