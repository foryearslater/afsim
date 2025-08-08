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

with rp.Analyzer(os.path.join('output', 'optical_signature_demo_%d.aer')) as analyzer:
    baseline = ('irst_sensor', 'sensor_irst', 'fighter_multi', 1)
    comparison1 = ('irst_sensor', 'sensor_irst', 'fighter_multi', 2)
    comparison2 = ('irst_sensor', 'sensor_irst', 'fighter_multi', 3)
    
    ax = plt.axes()
    analyzer.plotComparisonProbabilityOfDetection(baseline, comparison1, ax)
    analyzer.plotComparisonProbabilityOfDetection(baseline, comparison2, ax)
    