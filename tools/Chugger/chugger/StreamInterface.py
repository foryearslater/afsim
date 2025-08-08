# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

import datetime
import multiprocessing
from collections import deque
from platform import system


class StreamInterface(object):
    """
    StreamInterface is a object that allows for inter-process communication between instances of managers
    and Frontends. The purpose of this object is to allow for user input to control the events within
    running processes in Chugger.
    """
    def __init__(self):
        self.proc_out_total = multiprocessing.Value("i", 0)
        self.proc_out_remaining = multiprocessing.Value("i", 0)
        self.proc_out_running = multiprocessing.Value("i", 0)
        self.proc_out_queue = multiprocessing.Queue()
        self.proc_out_log = list()
        self.proc_out_history = deque()
        self.proc_out_buffer = 20
        self.term_out_queue = multiprocessing.Queue()
        self.term_out_log = list()
        self.term_out_history = deque()  # modifying this value directly through StreamInterface will not change the
        # output
        self.term_out_buffer = 10  # if you try to modify this value you will not impact anything the the actual process
        # after init
        self.ready_event = multiprocessing.Event()
        self.start_event = multiprocessing.Event()
        self.cancel_event = multiprocessing.Event()
        self.notification_event = multiprocessing.Event()  # notif. event that will set to allow any manager to launch
        self.kill_event = multiprocessing.Event()  # kill event will end all output to console
        self.input_event = multiprocessing.Event()  # input event that pauses all
        self.clear_screen = "cls"

        self.start_time = datetime.datetime.now().strftime("%H:%M:%S")

        if system() != "Windows":
            self.clear_screen = "clear"

    def set_stat_total(self, arg: int):
        self.proc_out_total.value = arg

    def set_stat_running(self, arg: int):
        self.proc_out_running.value = arg

    def set_stat_remaining(self, arg: int):
        self.proc_out_remaining.value = arg

    def write_process(self, arg):
        """
        write to the process block of the StreamInterface
        :param arg:
            Value to be placed into the process queue to be written.
        :return:
        """
        self.proc_out_queue.put(str(arg) + "\n")

    def write_terminal(self, arg):
        """
        write to the terminal block of the StreamInterface
        :param arg:
            Value to be place into the terminal queue to be written.
        :return:
        """
        self.term_out_queue.put(str(arg) + "\n")

    def get_output_string(self) -> str:
        """
        Creates a string object that will be modified with input taken from multiprocessing
        queues. These multiprocessing queues are written to by write_process and write_terminal calls
        :return:
        """
        self.update_blocks()
        output_string = ""
        current_time = datetime.datetime.now().strftime("%H:%M:%S")
        output_string += "|Time Data________________Start time: <{0}>_____Current time: <{1}>_|\n".format(
            self.start_time, current_time)
        output_string += "|Process Output________________Currently Running:<{0}>_____Processes Left:<{1}/{2}>_" \
                         "|\n".format(self.proc_out_running.value, self.proc_out_remaining.value,
                                      self.proc_out_total.value)
        for line in self.proc_out_history:
            output_string += line
        for n in range(0, self.proc_out_buffer - len(self.proc_out_history)):
            output_string += "\n"
        output_string += "|Terminal Output_____________________________________________|\n"
        for line in self.term_out_history:
            output_string += line
        for n in range(0, self.term_out_buffer - len(self.term_out_history)):
            output_string += "\n"
        output_string += "______________________________________________________________\n"
        return output_string

    def update_blocks(self):
        """
        Update the log lists with information placed within mulitprocessing queues
        :return:
        """
        while self.proc_out_queue.qsize() != 0:
            in_queue = self.proc_out_queue.get()
            if len(self.proc_out_history) == self.proc_out_buffer:
                self.proc_out_history.popleft()
            self.proc_out_history.append(in_queue)
            self.proc_out_log.append(in_queue)
        while self.term_out_queue.qsize() != 0:
            in_queue = self.term_out_queue.get()
            if len(self.term_out_history) == self.term_out_buffer:
                self.term_out_history.popleft()
            self.term_out_history.append(in_queue)
            self.term_out_log.append(in_queue)
