# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

import importlib.util
tk_module = importlib.util.find_spec("tkinter")
if tk_module is not None:
    from . import BaseParser, CmdParser, GuiParser
else:
    from . import BaseParser, CmdParser
