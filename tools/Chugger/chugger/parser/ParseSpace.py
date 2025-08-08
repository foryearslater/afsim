# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************


class ParseSpace(object):
    """
    Namespace object that is used to define the expected
    variable space returned from a parse action.
    Mainly defined in light of the usage of argparse module.
    """
    # WARNING if you change any of the variable names here
    # you must change the destination variable in the cmd parser.
    # this is an unfortunate double edge of the argparse module
    startup = ""
    matrix = ""
    output = ""
    server = False
    ip = ""
    port = 1
    scenario = ""
    linux = False
    linux_binary = ""
    linux_exe = ""
    windows = False
    windows_binary = ""
    windows_exe = ""
    local = False
    local_threads = 1
    local_exe = ""
    runs = 1
    dp_range = [0, 0]
    variables = ""
    run_variable = "run_seed"

    # moving matrix object class variables to ParseSpace
    matrix_dict = dict()
    matrix_value_len = 0
    matrix_keys = list()