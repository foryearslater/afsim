# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

import argparse
import os
import sys
import threading
import time

from chugger import StreamInterface
from chugger.parser.BaseParser import BaseParser
from chugger.parser.ParseSpace import ParseSpace


class CmdParser(BaseParser):
    """
    Fall back class and original implementation of the Chugger script
    Heavily utilizes the argparse module to parse commandline arguments.
    This class does not utilize any GUI assets and merely outputs and interfaces
    directly with the terminal that hosts the script.
    """
    def __init__(self, stream: StreamInterface):
        """
        :param stream:
            StreamInterface object that allows for interprocess communication
        """
        self.stream = stream
        self.args = ParseSpace()
        # argparse is the module used to accept and specify what command lines we will accept on execution.
        # If this fails, the program will exit entirely.
        self.parser = argparse.ArgumentParser("This is a Chugger CMD Frontend.")
        self.parser.add_argument(self.AFSIM_INPUT, type=str, help="Specify the AFSIM startup file. Required.")
        self.parser.add_argument(self.MATRIX_INPUT, type=str, help="Specify the Matrix file. Required.")
        self.parser.add_argument(self.OUTPUT_FOLDER, type=str, help="Specify output path. Required.")
        #
        # ## this section is needed if the user wishes to run Chugger as a server
        self.parser.add_argument(self.SERVER_MANAGER, action="store_true",
                                 help="Specify Chugger to setup a server execution environment.")
        self.parser.add_argument(self.IP_ADDRESS, type=str,
                                 help="Specify the ip address of the host server. Required.")
        self.parser.add_argument(self.PORT_NUMBER, type=int,
                                 help="Specify the port number to bind to the ip address. Required.")
        self.parser.add_argument(self.SCENARIO_FOLDER, type=str,
                                 help="Specify the folder containing all scenario related data. Required.")
        # These flags are associated with Linux client support
        self.parser.add_argument(self.LINUX_SUPPORT, action="store_true",
                                 help="Specify support for Linux Clients.")
        self.parser.add_argument(self.LINUX_EXEC_FILE, type=str,
                                 help="Specify the Linux executable file to be used by Linux Clients.")
        self.parser.add_argument(self.LINUX_BINARY_FOLDER, type=str,
                                 help="Specify binary folder containing Linux executables")
        # These flags are associated with Windows client support
        self.parser.add_argument(self.WINDOWS_SUPPORT, action="store_true",
                                 help="Specify support for Windows Clients.")
        self.parser.add_argument(self.WINDOWS_EXEC_FILE, type=str,
                                 help="Specify the Windows executable file to be used by Windows Clients.")
        self.parser.add_argument(self.WINDOWS_BINARY_FOLDER, type=str,
                                 help="Specify binary folder containing Windows executables")

        # this section is needed if the user wishes to run Chugger as a local executor
        self.parser.add_argument(self.LOCAL_MANAGER, action="store_true",
                                 help="Specify Chugger to setup a local execution environment.")
        self.parser.add_argument(self.LOCAL_THREADS, default=1, type=int,
                                 help="Specify max threads to run locally.")
        self.parser.add_argument(self.LOCAL_EXEC, type=str, help="Specify the path of Mission.exe to use.")

        # optional modifiers
        self.parser.add_argument(self.RUNS, default=1, type=int,
                                 help="Specify the amount of times you wish to run through the Matrix file.")
        self.parser.add_argument(self.DP_RANGE, default=[0, 0], type=int, nargs=2,
                                 help="Specify the range of DP values you wish to run. "
                                      "Takes two arguments 'N out-of M'")
        self.parser.add_argument(self.VARIABLES,
                                 help="Specify a pre-made variables files that Chugger will use to run mission with.")
        self.parser.add_argument(self.RUN_VARIABLE_NAME, default="run_seed", type=str,
                                 help="Override the default \"run_seed\" variable name with a user-defined one.")

        terminal_thread = threading.Thread(target=self.update_terminal)  # create thread that will update the terminal
        status: bool = self.get_args()
        if status:
            manager_thread = threading.Thread(target=self.launch_mgr)
            terminal_thread.start()
            manager_thread.start()
            self.user_input_terminal()
            terminal_thread.join()
            manager_thread.join()
        else:
            self.stream.kill_event.set()
            if terminal_thread.is_alive():
                terminal_thread.join()

    def get_args(self) -> bool:
        """
        Parse CMD-line arguments and validate arguments with
        process_args call.
        :return:
        """
        try:
            self.parser.parse_args(namespace=self.args)
            log = self.process_args(self.args)
            if log.error_flag:
                print(log.error_string)
                print("Error detected.")
                return False
            elif log.notice_flag:
                self.stream.write_terminal(log.notice_string)
                return True
            else:
                return True
        except SystemExit:
            input("Enter any key to exit application")
            return False

    def launch_mgr(self):
        """
        Handles the creation of Server and Client Managers.
        :return:
        """
        if self.args.server:
            server_thread = threading.Thread(target=self.gen_server_mgr, args=(self.args, self.stream,))
            server_thread.start()
            # self.gen_server_mgr(self.args, self.stream)
            self.stream.ready_event.wait()  # wait for server to be ready
            while not self.stream.start_event.is_set() and not self.stream.cancel_event.is_set():
                time.sleep(1)
            server_thread.join()
            self.stream.write_terminal("Server Manager thread joined")
        else:
            self.gen_local_mgr(self.args, self.stream)
            self.stream.write_terminal("Local Manager thread joined")

    def update_terminal(self):
        """
        Function to be run on a separate thread that will utilize the
        StreamInterface's output_string to update the terminal.
        :return:
        """
        while True:  # keep looping until the kill switch is enabled
            if self.stream.kill_event.is_set():
                print("Closing terminal output window.")
                break
            elif not self.stream.input_event.is_set():
                os.system(self.stream.clear_screen)  # go clear the terminal for update
                sys.stdout.write(self.stream.get_output_string())
            time.sleep(2)  # sleep for 2 seconds before updating the screen

    def user_input_terminal(self):
        """
        Function to be operating on the main thread. Handles all user input
        and acts as the intermediary between components of Chugger and the user.
        :return:
        """
        while True:
            input()
            self.stream.input_event.set()
            os.system(self.stream.clear_screen)
            sys.stdout.write(self.stream.get_output_string())
            sys.stdout.write("The following commands are available:\n")
            sys.stdout.write("back - Resume program. ; exit - Close/terminate program. ; gen_script - Generate script "
                             "based on params.\n"
                             "start - Start any waiting managers. ; cancel - Cancel any waiting managers.\n")
            sys.stdout.write(">")
            usr_input = input().lower()
            if usr_input == "back":
                print("Resuming terminal processes")
            elif usr_input == "exit":
                print("Closing terminal input window.")
                self.stream.kill_event.set()
                self.stream.input_event.clear()
                break
            elif usr_input == "gen_script":
                print("Generating script")
                self.gen_script(self.args)
                self.stream.write_terminal("Completed Script Generation")
            elif usr_input == "start":
                if not self.stream.ready_event.is_set():
                    self.stream.write_terminal("A server manager has not been prepared/launched.")
                else:
                    self.stream.start_event.set()
            elif usr_input == "cancel":
                if not self.stream.ready_event.is_set():
                    self.stream.write_terminal("A Server Manager has not been prepared/launched.")
                else:
                    self.stream.cancel_event.set()
            elif usr_input == "thanks":
                print("You're pretty good!")
            else:
                print("ERROR: Invalid command. Resuming program.")
            time.sleep(1)
            self.stream.input_event.clear()
