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

BINARY_DIR=
LD_LIB_PATH=

cd ../../../../..
TRAINING_PATH=$PWD

cd ../.. 
ROOT=$PWD

if test -d $ROOT/swdev; then
   if test -d $ROOT/swdev/build/wsf_install/bin; then
      cd $ROOT/swdev/build/wsf_install/bin 
      BINARY_DIR=$PWD      
      LD_LIB_PATH=$PWD/lib 
      echo "Assuming release directory structure"
      echo "Binary directory: $BINARY_DIR"
      echo "Library Path directory: $LD_LIB_PATH"
   elif test -d $ROOT/swdev/BUILD/wsf_install/bin; then
      cd $ROOT/swdev/BUILD/wsf_install/bin
      BINARY_DIR=$PWD      
      LD_LIB_PATH=$PWD/lib 
      echo "Assuming release directory structure"
      echo "Binary directory: $BINARY_DIR"
      echo "Library Path directory: $LD_LIB_PATH"
   else
      echo "Assuming release directory structure"
      echo "Failed to find binary executable directory for Warlock"
      cd $CWD
      exit
   fi
elif test -d $ROOT/afsim; then
   cd $TRAINING_PATH
   if test -d $TRAINING_PATH/build/wsf_install/bin; then
      cd $TRAINING_PATH/build/wsf_install/bin 
      BINARY_DIR=$PWD      
      LD_LIB_PATH=$PWD/lib 
      echo "Assuming development directory structure"
      echo "Binary directory: $BINARY_DIR"
      echo "Library Path directory: $LD_LIB_PATH"
   elif test -d $TRAINING_PATH/BUILD/wsf_install/bin; then
      cd ../../../../../BUILD/wsf_install/bin 
      BINARY_DIR=$PWD      #"../../../../../BUILD/wsf_install/bin" 
      LD_LIB_PATH=$PWD/lib #"../../../../../BUILD/wsf_install/bin/lib" 
      echo "Assuming development directory structure"
      echo "Binary directory: $BINARY_DIR"
      echo "Library Path directory: $LD_LIB_PATH"
   else
      echo "Assuming development directory structure"
      echo "Failed to find binary executable directory for Warlock"
      cd $CWD
      exit
   fi
else
   echo "Failed to find either release or developer directory structure"
   echo "Please fix this and rerun this script"
   cd $CWD
   exit
fi

#change back to original directory and run warlock and flight_controller
cd $CWD

WARLOCK_PID=
FLIGHTCONTROLLER_PID=
if test -d $BINARY_DIR; then
      #export LD_LIBRARY_PATH=$LD_LIB_PATH
      echo "*********** printint LD_LIBRARY_PATH ***********"
      export LD_LIBRARY_PATH="$LD_LIB_PATH"
      printenv | grep LD_LIBRARY_PATH

      echo "*********** executing warlock on comm_scenario1.txt ***********"
      $BINARY_DIR/warlock $CWD/xio_scenario.txt&
      WARLOCK_PID=$!

      echo "************ Warlock job PIDs: $WARLOCK_PID ************"

      echo "*********** executing flight_controller on flight_controler_config.txt ***********"
      $BINARY_DIR/flight_controller $CWD/flight_controller_config.txt&
      FLIGHTCONTROLLER_PID=$!

      echo "************ flight_controller job PIDs: $FLIGHTCONTROLLER_PID ************"

      wait $WARLOCK_PID

      echo "************ killing flight_controller process ************"
      echo "Process IDs: $FLIGHTCONTROLLER_PID"
      /usr/bin/kill -s SIGKILL $FLIGHTCONTROLLER_PID
else
   echo "Failed to find binary executable directory for Warlock and Flight Controller"
fi
   
