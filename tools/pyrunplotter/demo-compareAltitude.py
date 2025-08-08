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

with rp.Analyzer(os.path.join('output', 'Hohmann_leo_orbit_transfer_demo.aer'), endTime=18000) as analyzer:
    baseline = ('rendezvous-1', 1)
    comparison = ('target', 1)
    analyzer.plotComparisonAltitude(baseline, comparison)
    