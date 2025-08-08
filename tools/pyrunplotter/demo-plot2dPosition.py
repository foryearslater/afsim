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

with rp.Analyzer(os.path.join('output', 'waypoint_test_%d.aer'), endTime=2200) as analyzer:
    analyzer.plotPosition2D(['mr_mover_red','mr_mover_blu'], runNumberList=[1,2])