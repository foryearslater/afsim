@echo off
rem ****************************************************************************
rem CUI
rem
rem The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
rem
rem The use, dissemination or disclosure of data in this file is subject to
rem limitation or restriction. See accompanying README and LICENSE for details.
rem ****************************************************************************
echo on

call auto_setup.bat

perl auto_test.pl -v %*
perl transform_csv.pl
