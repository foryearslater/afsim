# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

#!/bin/bash

CWD=$PWD
echo
echo "CWD: $CWD"

cd ../../../../..

# save the training path (asssumed to be .../training/developer)
TRAINING_PATH=$PWD

cd ../..

# save the root directory (contains either swdev or afsim folder)
ROOT=$PWD

# ------------------------------------------------------------------------------
# Determine if we have release or developer directory structure

MYSTIC_DIR=
MISSION_DIR=
LD_LIB_PATH=

if test -d $ROOT/swdev; then
   echo "Assuming release directory structure";
   # find the directory containing the Mystic executable
   if test -d $ROOT/bin; then
      MYSTIC_DIR="$ROOT/bin";
      LD_LIB_PATH="$ROOT/bin/lib";
   else
      echo "Failed to find Mystic directory - exiting";
      cd $CWD;
      exit;
   fi

   # find the directory containing the training mission executable
   if test -d $ROOT/swdev/build/wsf_install/bin; then
      MISSION_DIR="$ROOT/swdev/build/wsf_install/bin";
   elif test -d $ROOT/swdev/BUILD; then
      MISSION_DIR="$ROOT/swdev/BUILD/wsf_install/bin";
   else
      echo "Failed to find Mission directory - exiting";
      cd $CWD;
      exit;
   fi
elif test -d $ROOT/afsim; then
   echo "Assuming development directory structure";
   # find the directory containing the Mystic executable
   if test -d $ROOT/build/wsf_install/bin; then
      MYSTIC_DIR="$ROOT/build/wsf_install/bin";
      LD_LIB_PATH="$ROOT/build/wsf_install/bin/lib";
   elif test -d $ROOT_DIR/BUILD; then
      MYSTIC_DIR="$ROOT/BUILD/wsf_install/bin";
      LD_LIB_PATH="$ROOT/BUILD/wsf_install/bin/lib";
   else
      echo "Failed to find Mystic directory - exiting";
      cd $CWD;
      exit;
   fi

   # find the directory containing the training mission executable
   if test -d $TRAINING_PATH/build/wsf_install/bin; then
      MISSION_DIR="$TRAINING_PATH/build/wsf_install/bin";
   elif test -d $TRAINING_PATH/BUILD/wsf_install/bin; then
      MISSION_DIR="$TRAINING_PATH/BUILD/wsf_install/bin";
   else
      echo "Failed to find Mission directory - exiting";
      cd $CWD;
      exit;
   fi
else 
   echo Error:  Failed to Find Root Directory - Exiting
   cd $CWD;
   exit;
fi

echo " ";
echo "********************************************************";
echo "Root directory: $ROOT";
echo "Mystic directory: $MYSTIC_DIR";
echo "Mission directory: $MISSION_DIR";
echo "********************************************************";
echo " ";

# ------------------------------------------------------------------------------
# Determine the system platform (Windows or Linux)
echo "********************************************************";
echo "Determining whether platform is Windows or Linux";
SYSTEM="$(uname)";
NEED_LIB_PATH="false";
case $SYSTEM in
   linux*|Linux*|LINUX*)
      echo "Platform is a Linux system";
      # need to define th library path since platform is Linux
      export LD_LIBRARY_PATH="$LD_LIB_PATH";
      echo -n "Library Path: " 
      printenv | grep LD_LIBRARY_PATH;
      ;;
   msys*|Msys*|MSys*|MSYS*|cygwin*|Cygwin*|CYGWIN*|mingw*|Mingw*|MINGW*|nt*|NT*|win*|Win*|WIN*)
      echo "Platform is a Windows system"
      ;;
  *)
      exit
      ;;
esac
echo "********************************************************";
echo " "
echo " "

# ------------------------------------------------------------------------------
# change back to starting directory
cd $CWD;
echo "********************************************************";
echo "Preparing to execute mission and mystic for scenario"
echo " "
if test -d $MISSION_DIR; then
   if test -d $MYSTIC_DIR; then
      echo " "
      echo "---> Executing Mission on comm_scenario1.txt"
      echo " "
      $MISSION_DIR/mission comm_scenario1.txt&
      MISSION_PIDS=$!
      # pause for 4 seconds
      sleep 4

      echo " "
      echo "---> Executing Mission on comm_scenario2.txt"
      echo " "
      $MISSION_DIR/mission comm_scenario2.txt&
      MISSION_PIDS="$MISSION_PIDS $!"

      echo " "
      echo "********************************************************";
      echo "Mission job PIDs: $MISSION_PIDS"
      echo "********************************************************";
      echo " "

      # pause for 4 seconds
      sleep 4

      echo " "
      echo "---> Executing Mystic on comm_scenario2.aer"
      echo " "
      $MYSTIC_DIR/mystic comm_exercise2.aer

      echo " "
      echo "********************************************************";
      echo "Mystic has terminated - Killing Mission processes"
      echo "Process IDs: $MISSION_PIDS"
      echo " "
      /usr/bin/kill -s SIGKILL $MISSION_PIDS
   else
      echo Error: Cannot find mystic directory
   fi
else
	echo Error: Cannot find mission directory
fi
