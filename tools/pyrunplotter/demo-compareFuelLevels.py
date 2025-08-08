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

import os
import pyrunplotter as rp
import matplotlib.pyplot as plt

with rp.Analyzer(os.path.join('output', 'waypoint_test_%d.aer'), endTime=2200) as analyzer:
    redPointMass = ('mr_mover_red', 1)
    redRigidBody = ('mr_mover_red', 2)
    bluPointMass = ('mr_mover_blu', 1)
    bluRigidBody = ('mr_mover_blu', 2)
    
    ax = plt.axes()
    analyzer.plotComparisonFuelLevels(redPointMass, redRigidBody, ax)
    analyzer.plotComparisonFuelLevels(bluPointMass, bluRigidBody, ax)
    