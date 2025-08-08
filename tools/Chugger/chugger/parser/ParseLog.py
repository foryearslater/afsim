# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************


class ParseLog(object):
    """
    Helper class object that stores information
    regarding errors or notices when parsing user input
    """
    notice_string = ""
    error_string = ""
    error_flag = False
    notice_flag = False

    def append_error(self, arg: str):
        self.error_string += arg + "\n"
        self.error_flag = True

    def append_notice(self, arg: str):
        self.notice_string += arg + "\n"
        self.notice_flag = True
