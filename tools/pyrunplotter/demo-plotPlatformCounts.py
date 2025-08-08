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

with rp.Analyzer(os.path.join('output', 'outer_air_battle-mrc-erc_%d.aer')) as analyzer:
    analyzer.plotPlatformCounts(runNumberList=[1,2])
    