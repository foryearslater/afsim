#!/usr/bin/env python3

# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright (C) 2022 Stellar Science; U.S. Government has Unlimited Rights.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

import argparse
import os
import subprocess
import shutil

parser = argparse.ArgumentParser( description='Run demos' )
parser.add_argument( '--executable', nargs=1, required=True, help='Path to the mission executable' )
parser.add_argument( '--demos_path', nargs=1, required=True, help='Path to the AFSIM demos directory' )
args = parser.parse_args()

demos_path = args.demos_path[0]
with open("demo_filelist.txt") as f:
    for line in f:
        path = line.strip()
        origpath = '/'.join((demos_path, path))
        destpath = '/'.join(('demos', path))
        os.makedirs(os.path.split(destpath)[0], exist_ok=True)
        shutil.copy(origpath, destpath)

oab_demo_path = os.path.join("demos", "outer_air_battle", "OAB_mrc_erc.txt")
optical_signature_demo_path = os.path.join("demos", "multiresolution_demos", "optical_signature_demo.txt")
sat_demo_path = os.path.join("demos", "satellite_demos", "6th_Hohmann_leo_orbit_transfer_demo.txt")
waypoint_demo_path = os.path.join("demos", "six_dof", "six_dof_waypoint.txt")
timeline_demo_path = os.path.join("demos", "timeline", "timeline_demo.txt")

executable = args.executable[ 0 ]

print(oab_demo_path)
process_oab_demo = subprocess.run( [ executable, oab_demo_path], shell=True, capture_output=True )
print(sat_demo_path)
process_sat_demo = subprocess.run( [ executable, sat_demo_path], shell=True, capture_output=True )
print(waypoint_demo_path)
process_waypoint_demo = subprocess.run( [ executable, waypoint_demo_path], shell=True, capture_output=True )
print(timeline_demo_path)
process_timeline_demo = subprocess.run( [ executable, timeline_demo_path], shell=True, capture_output=True )
print(optical_signature_demo_path)
process_optical_signature_demo = subprocess.run( [ executable, optical_signature_demo_path], shell=True, capture_output=True )
