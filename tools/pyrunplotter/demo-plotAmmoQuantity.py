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

with rp.Analyzer(os.path.join('output', 'timeline_demo.aer')) as analyzer:
    platCompList = [
        ['north_cap_1','mrm'],
        ['north_cap_2','mrm'],
        ['sweep_1','mrm'],
        ['sweep_2','mrm'],
    ]
    analyzer.plotAmmoQuantityVsTime(platCompList)