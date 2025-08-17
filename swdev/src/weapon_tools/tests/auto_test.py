# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2003-2015 The Boeing Company. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# ============================================================================
#                   Invoke the common automatic test driver.
#
#                    Use 'python auto_test.py -?' to get help.

# Calls misc/auto_test.py, adding a -x <exe-path> if one is found
import glob
import os
import subprocess
import sys

sys.path.append('../../core/wsf/tools')
import system_type
exes = []
dirs = []
auto_tests = []
# Try to find the auto_test.py script
auto_tests += glob.glob('../../misc/auto_test.py')
auto_tests += glob.glob('../../*/misc/auto_test.py')
auto_tests += glob.glob('../../../misc/auto_test.py')
auto_tests += glob.glob('../../../*/misc/auto_test.py')
auto_tests += glob.glob('../../../../*/misc/auto_test.py')
if len(auto_tests) > 1:
    print('Found misc/auto_test.py files:')
    for s in auto_tests:
        print('   ' + s)
    print('using the first one')
elif len(auto_tests) == 1:
    print('Found misc/auto_test.py:' + auto_tests[0])
else:
    print('No misc/auto_test.py found')

# Try to find the .exe in the common source tree relative locations
if '-x' not in sys.argv:
    if os.name == 'nt':
        exes += glob.glob('../../*BUILD*/Release/weapon_tools*.exe')
        exes += glob.glob('../../*BUILD*/Debug/weapon_tools*.exe')
        exes += glob.glob('../../../*BUILD*/Release/weapon_tools*.exe')
        exes += glob.glob('../../../*BUILD*/Debug/weapon_tools*.exe')
        exes += glob.glob('../../*BUILD*/Release/weapon_tools*.exe')
        exes += glob.glob('../../*BUILD*/Debug/weapon_tools*.exe')
    else:
        sysType = system_type.get_type()
        model = os.getenv('MODEL','')
        exes += glob.glob('../../*BUILD*/weapon_tools_' + sysType + model + '.exe')
        exes += glob.glob('../../../*BUILD*/weapon_tools_' + sysType + model + '.exe')
        exes += glob.glob('../../*BUILD*/weapon_tools_' + sysType + model + '.exe')

    if len(exes) > 1:
        print('Found exes:')
        for e in exes:
            print('   ' + e)
        print('using the first one')
    elif len(exes) == 1:
        print('Found exe:' + exes[0])
    else:
        print('No .exe found')

# If root directories were not specified, make default selections that include:
#   1) The 'auto_test' directories in this directory.
#   2) The core included project specific 'tests' directories (e.g.: ../../wsf_project/test_weapon_tools)
#   3) The extension project specific 'tests' directories (e.g.: ../../../wsf_project/test_weapon_tools)
if '-d' not in sys.argv:
    dirs += ['-d', 'auto_*']
    dirs += ['-d', '../../*/test_weapon_tools']
    dirs += ['-d', '../../*/*/test_weapon_tools']

if len(exes) > 0:
    args = [sys.executable, auto_tests[0], '-x', exes[0]] + dirs + sys.argv[1:]
else:
    args = [sys.executable,auto_tests[0]] + dirs + sys.argv[1:]
print(' '.join(args))
proc = subprocess.Popen(args)
proc.wait()
sys.exit(proc.returncode)

