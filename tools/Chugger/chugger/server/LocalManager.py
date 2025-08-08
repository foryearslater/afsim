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
import os
import re
import subprocess
import time
import timeit
from collections import deque

from chugger import StreamInterface
from chugger.server import DpRunList
from chugger.parser.ParseSpace import ParseSpace


class LocalManager(object):
    """
    Local Manager is the Local Execution Manager object that will
    control and handle all running process on a user's machine.
    """
    def __init__(self, stream: StreamInterface, dp_run_queue: DpRunList,
                 output: str, dict_data: dict, arg: ParseSpace):
        """
        :param stream:
            StreamInterface object that allows for interprocess communication
        :param dp_run_queue:
            DpRunList object that contains tuples of Data-points and runs to be processed
        :param output:
            Special output directory where all simulation data will be stored
        :param dict_data:
            Dictionary containing matrix data
        :param arg:
            ParseSpace object used to fill class variable definitions
        """
        self.stream = stream
        self.dp_run_list = dp_run_queue
        self.process_list = deque()
        self.variables = arg.variables
        self.exe = arg.local_exe
        self.startup = arg.startup
        self.matrix = arg.matrix
        self.output = output
        self.threads = arg.local_threads
        self.close_manager = False
        self.key_pair_data = dict_data
        self.variable_data = list()
        self.run_variable = arg.run_variable

        self.setup_local_mgr()
        self.process_manager()

    def setup_local_mgr(self):
        """
        Create all the folder and directories necessary to hold
        all generated information. This function also creates a
        "process_list" which contains tuples of a Data-Point, Run, and
        output directory. This "process_list" is to be used in the process_manager
        :return:
        """
        try:
            with open(self.variables, "r") as input_variable_file:
                self.variable_data = input_variable_file.readlines()
        except FileNotFoundError as error:
            self.stream.write_terminal("Unable to open file: {0}".format(error))

        while self.dp_run_list.m_list.qsize() != 0:
            dp_run_tuple = self.dp_run_list.m_list.get()
            dp_folder_path = os.path.join(self.output, "DP_" + str(dp_run_tuple[0]))
            run_folder_path = os.path.join(dp_folder_path, "RUN_" + str(dp_run_tuple[1]))
            if not os.path.exists(os.path.join(self.output, "DP_" + str(dp_run_tuple[0]))):
                os.mkdir(dp_folder_path)
            if os.path.exists(run_folder_path):
                temp_dir_num = 1
                while os.path.exists(run_folder_path + "_" + str(temp_dir_num)):
                    temp_dir_num += 1
                run_folder_path = run_folder_path + "_" + str(temp_dir_num)
            os.mkdir(run_folder_path)
            self.process_list.append((dp_run_tuple[0], dp_run_tuple[1], run_folder_path))

    def process_manager(self):
        """
        The essential manager function that will control the lifetime and the processess
        that need to run.
        Note: Future implementation may use multiprocessing pools, however previous attempts
        resulted in runtime errors.
        :return:
        """
        self.stream.write_terminal("Starting process manager")
        current_queue = deque()
        running_queue = deque()
        trash_queue = deque()
        lock = multiprocessing.Event()
        self.stream.set_stat_total(len(self.process_list))
        while (not self.close_manager or not len(current_queue) == 0) and not self.stream.kill_event.is_set():
            if not self.stream.input_event.is_set():
                if len(current_queue) < self.threads and not self.close_manager:
                    if len(self.process_list) == 0:
                        self.close_manager = True
                    else:
                        process_data = self.process_list.popleft()
                        current_queue.append(multiprocessing.Process(target=self.write_call, args=(
                            process_data[0] - 1, process_data[1], process_data[2], lock,)))
                        self.stream.set_stat_remaining(len(current_queue) + len(self.process_list))
                if len(running_queue) < self.threads and len(current_queue) != 0:
                    running_queue.append(current_queue.popleft())
                    running_queue[len(running_queue) - 1].start()
                    self.stream.set_stat_remaining(len(current_queue) + len(self.process_list))
                for started_process in running_queue:
                    if not started_process.is_alive():
                        started_process.join()
                        trash_queue.append(started_process)
                if len(trash_queue) == 0 and len(running_queue) == self.threads:
                    lock.wait()
                    time.sleep(.25)
                    lock.clear()
                else:
                    for dead_process in trash_queue:
                        running_queue.remove(dead_process)
                    trash_queue.clear()
            else:
                time.sleep(4)
        # if self.stream.kill_event.is_set():
        #     print("Kill switch activated")
        #     print("Current_Queue: " + str(current_queue))
        #     print("Running Queue: " + str(running_queue))
        #     print("Trash Queue: " + str(trash_queue))
        for started_process in running_queue:
            started_process.join()

    def write_call(self, dp_value, run_value, run_directory, lock):
        """
        Actual function all required in order to launch mission on its own seperate process.
        The function call parses a given variable file and instantiates its own
        version of the variable file with values from a given data-point index.
        :param dp_value:
            dp_value is used as an index in the matrix dictionary
        :param run_value:
            run_value is used to specify the run_seed value for the given process instance
        :param run_directory:
            Output directory for this associated process
        :param lock:
            Event that signals to the process manager to launch another process if possible
        :return:
        """
        self.stream.proc_out_running.value += 1
        start_time = timeit.default_timer()  # start the timer for process execution
        start_time_full = datetime.datetime.now().strftime("%H:%M:%S")
        os.chdir(run_directory)
        self.stream.write_process("Beginning process for Data-Point<{0}> Run<{1}> @ {2}"
                                  .format(dp_value + 1, run_value, start_time_full))
        instance_var_file_path = os.path.join(run_directory, "DP_" + str(dp_value + 1) + "_variable_file.txt")
        with open(instance_var_file_path, "w") as instance_var_file:  # open variable file to begin substituting values
            for lines in self.variable_data:
                partition = re.findall(r"(^\$define)[ \t]+([\w\d\.-]+)[ \t]+(\"?[\w\d\.\/-]*\"?[^#\n\/\/])", lines)
                for keys in self.key_pair_data:
                    if partition:
                        if partition[0][1] == self.run_variable:
                            lines = partition[0][0] + " " + partition[0][1] + " " + str(run_value) + "\n"
                        elif partition[0][1] == keys:
                            lines = partition[0][0] + " " + partition[0][1] + " " + self.key_pair_data[keys][
                                dp_value] + "\n"
                instance_var_file.writelines(lines)
        console_out = os.path.join(run_directory, "console_output_data.txt")
        with open(console_out, "w") as output_file:
            process = subprocess.Popen([self.exe, instance_var_file_path, self.startup],
                                       stdout=output_file)
            while process.poll() is None:  # while it hasn't terminated
                if self.stream.kill_event.is_set():
                    print("Terminating process for DP " + str(dp_value + 1) + " Run " + str(run_value))
                    process.terminate()
                    break
                time.sleep(1)
        self.stream.write_process("Ending process for Data-Point<{0}> Run<{1}>".format(dp_value + 1, run_value))
        self.stream.write_process("\t-->Elapsed time: {0} -> {1} == {2:.2f} seconds"
                                  .format(start_time_full, datetime.datetime.now().strftime("%H:%M:%S"),
                                          timeit.default_timer() - start_time))
        lock.set()
        self.stream.proc_out_running.value -= 1
