@echo off
rem ****************************************************************************
rem CUI
rem
rem The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
rem
rem The use, dissemination or disclosure of data in this file is subject to
rem limitation or restriction. See accompanying README and LICENSE for details.
rem ****************************************************************************
rem @echo off

echo -----------------------------------------------------------------------
echo Configuring the afnes_ide development environment.
echo -----------------------------------------------------------------------

set SWDEV_ROOT=%~d0%~p0..\..
pushd %SWDEV_ROOT%
set SWDEV_ROOT=%CD%
popd
echo SWDEV_ROOT=%SWDEV_ROOT%

set PKGROOT=%SWDEV_ROOT%\pkgroot
echo PKGROOT=%PKGROOT%
echo Calling %PKGROOT%\common\vs2008\pkgroot.bat ...
call "%PKGROOT%\common\vs2008\pkgroot.bat"

call "%vs90comntools%\vsvars32.bat"

set AFNES_IDE_DEVELOPER=1

rem set PATH=%PATH%;%PKGROOT_BIN%;%QT_BIN%;%SWDEV_ROOT%\util\vs2008\build_d_vs2008_dll;%SWDEV_ROOT%\afnes_parser\vs2008\build_d_vs2008_dll;%SWDEV_ROOT%\utilqt\vs2008\build_d_vs2008_dll

rem prevent polluting path variable - doesn't work - best guess command line limit overflow
if "!PATH:%PKGROOT_BIN%=!"=="%PATH%" set PATH=%PATH%;%PKGROOT_BIN%
if "!PATH:%QT_BIN%=!"=="%PATH%" set PATH=%PATH%;%QT_BIN%
if "!PATH:%SWDEV_ROOT%\util\vs2008\build_d_vs2008_dll=!"=="%PATH%" set PATH=%PATH%;%SWDEV_ROOT%\util\vs2008\build_d_vs2008_dll
if "!PATH:%SWDEV_ROOT%\afnes_parser\vs2008\build_d_vs2008_dll=!"=="%PATH%" set PATH=%PATH%;%SWDEV_ROOT%\afnes_parser\vs2008\build_d_vs2008_dll
if "!PATH:%SWDEV_ROOT%\utilqt\vs2008\build_d_vs2008_dll=!"=="%PATH%" set PATH=%PATH%;%SWDEV_ROOT%\utilqt\vs2008\build_d_vs2008_dll
