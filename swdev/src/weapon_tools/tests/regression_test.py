# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
# ============================================================================
#            Invoke the common automatic regression test driver.
#
#            Use 'python regression_test.py -?' to get help.

# Calls misc/regression_test.py, adding a -x <exe-path> if one is found
import glob
import os
import subprocess
import sys

sys.path.append('../../tools/misc')
import system_type
exes = []
regression_tests = []
# Try to find the regression_test.py script
regression_tests += glob.glob('../../tools/misc/regression_test.py')
regression_tests += glob.glob('../../misc/regression_test.py')
regression_tests += glob.glob('../../*/misc/regression_test.py')
regression_tests += glob.glob('../../../misc/regression_test.py')
regression_tests += glob.glob('../../../*/misc/regression_test.py')
regression_tests += glob.glob('../../../../*/misc/regression_test.py')
if len(regression_tests) > 1:
    print('Found misc/regression_test.py files:')
    for s in regression_tests:
        print('   ' + s)
    print('using the first one')
elif len(regression_tests) == 1:
    print('Found misc/regression_test.py:' + regression_tests[0])
else:
    print('No misc/regression_test.py found')

# Try to find the .exe in the common source tree relative locations
if '-x' not in sys.argv:
    if os.name == 'nt':
        exes += glob.glob('../../*BUILD*/Release/weapon_tools*.exe')
        exes += glob.glob('../../*BUILD*/Debug/weapon_tools*.exe')
        exes += glob.glob('../../../*BUILD*/Release/weapon_tools*.exe')
        exes += glob.glob('../../../*BUILD*/Debug/weapon_tools*.exe')
        exes += glob.glob('../../../../*BUILD*/Release/weapon_tools*.exe')
        exes += glob.glob('../../../../*BUILD*/Debug/weapon_tools*.exe')
    else:
        sysType = system_type.get_type()
        model = os.getenv('MODEL','')
        exes += glob.glob('../../*BUILD*/weapon_tools_' + sysType + model + '.exe')
        exes += glob.glob('../../../*BUILD*/weapon_tools_' + sysType + model + '.exe')
        exes += glob.glob('../../../../*BUILD*/weapon_tools_' + sysType + model + '.exe')

    if len(exes) > 1:
        print('Found exes:')
        for e in exes:
            print('   ' + e)
        print('using the first one')
    elif len(exes) == 1:
        print('Found exe:' + exes[0])
    else:
        print('No .exe found')

if len(exes) > 0:
    args = [sys.executable, regression_tests[0], '-x', exes[0]] + sys.argv[1:]
else:
    args = [sys.executable,regression_tests[0]] + sys.argv[1:]
print(' '.join(args))
proc = subprocess.Popen(args)
proc.wait()
sys.exit(proc.returncode)

