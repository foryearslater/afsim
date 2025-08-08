# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

import multiprocessing


class DpRunList(object):
    """
    Tuple based multiprocessing Queue wrapper class
    This class is to be deprecated in future implementations.
    """
    def __init__(self):
        self.m_list = multiprocessing.Queue()

    def get_dp_run(self):
        """
        Return any value retrieved from the Queue
        returns 0, 0 tuple if there aren't any tuples available
        :return:
        """
        if self.m_list.qsize() == 0:
            return 0, 0
        else:
            value = self.m_list.get()
            return value

    def add_dp_run(self, t_tuple):
        """
        Fnunction call that adds a tuple to the
        queue
        :param t_tuple:
            Tuple pair of integers
        :return:
        """
        self.m_list.put(t_tuple)
