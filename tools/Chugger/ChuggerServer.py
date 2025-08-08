#!/usr/bin/python3
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Chugger v0.91 Matrix Execution script
# Chugger is a execution script that will run a series of simulations based on data-points found within a matrix (DOE)
# file in a *.csv format. Chugger v0.91 features a Local Execution Manager and a Server Execution Manager.
# This script serves as an alternative to PIANO, and is compatible with Windows and Linux.
# Disclaimer
# Chugger is still in its development stage with more features and improvements to be added. **Use Chugger at its your
# own risk**. This script has been tested in multiple instances in both controlled and real-application runs, but it
# does not mean its completely bullet-proof.
# Please report any bugs, glitches, and possible suggestions to what features should be added.

import sys
import importlib.util

from chugger import StreamInterface
from chugger.parser.CmdParser import CmdParser

tk_module = importlib.util.find_spec("tkinter")
if tk_module is not None:
    from chugger.parser.GuiParser import GuiParser

    if __name__ == "__main__":
        stream = StreamInterface()
        if len(sys.argv) != 1:
            CmdParser(stream)
        else:
            GuiParser(stream)
else:
    if __name__ == "__main__":
        stream = StreamInterface()
        stream.write_terminal("Tkinter module unavailable. GUI-Frontend disabled.")
        if len(sys.argv) != 1:
            CmdParser(stream)
        else:
            print("Invalid number of args passed.")




