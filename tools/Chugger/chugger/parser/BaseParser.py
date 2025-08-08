# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

import csv
import datetime
import os
import subprocess
import sys
import time
import zipfile
from platform import system

from chugger.server.ServerManager import ServerManager
from chugger.server.LocalManager import LocalManager
from chugger import StreamInterface
from chugger.server.DpRunList import DpRunList
from chugger.parser.ParseSpace import ParseSpace
from chugger.parser.ParseLog import ParseLog


class BaseParser(object):
    """
    BaseParser is a the Parent class of both the CmdParser and GuiParser objects.
    These functions are shared between both child parsers, and much of their behaviors
    are defined here.
    """
    # flags. Used to identify command arguments. Change these to modify the flag names
    # these flag names will name impact the variable assignment for the ParseSpace object
    AFSIM_INPUT = "-startup"
    MATRIX_INPUT = "-matrix"
    OUTPUT_FOLDER = "-output"
    SERVER_MANAGER = "-server"
    IP_ADDRESS = "-ip"
    PORT_NUMBER = "-port"
    SCENARIO_FOLDER = "-scenario"
    LINUX_SUPPORT = "-linux"
    LINUX_BINARY_FOLDER = "-linux_binary"
    LINUX_EXEC_FILE = "-linux_exe"
    WINDOWS_SUPPORT = "-windows"
    WINDOWS_BINARY_FOLDER = "-windows_binary"
    WINDOWS_EXEC_FILE = "-windows_exe"
    LOCAL_MANAGER = "-local"
    LOCAL_THREADS = "-local_threads"
    LOCAL_EXEC = "-local_exe"
    RUNS = "-runs"
    DP_RANGE = "-dp_range"
    VARIABLES = "-variables"
    RUN_VARIABLE_NAME = "-run_variable"

    def parse_matrix(self, args: ParseSpace) -> bool:
        """
        Reads the given matrix file_path from the ParseSpace object, and
        assigns ParseSpace's Matrix associated class variables with information
        gathered
        :param args:
            ParseSpace object that stores required Manager information.
        :return:
        """
        try:
            with open(args.matrix, "r") as csv_file:
                # Explicitly clear out the dictionary. We do not know what changes
                # have been made to the matrix file since previous parse.
                if len(args.matrix_dict) != 0:
                    args.matrix_dict.clear()
                raw_csv_input = csv.DictReader(csv_file)
                args.matrix_keys = raw_csv_input.fieldnames
                for key in args.matrix_keys:
                    args.matrix_dict[key] = list()
                for row in raw_csv_input:
                        for key in args.matrix_keys:
                            args.matrix_dict[key].append((row[key]))
                args.matrix_value_len = len(args.matrix_dict[args.matrix_keys[0]])
                return True
        except FileNotFoundError:
            return False

    def gen_dp_run_queue(self, dp_start: int, dp_end: int, dp_run: int) -> DpRunList:
        """
        Initializes a DpRunList object that contains a multiprocessing queue object.
        The values stored in the DpRunList are tuples that contain a dp value and a run value
        DpRunList object is to be deprecated in the future
        :param dp_start:
            starting data-point value in range of the given matrix
        :param dp_end:
            ending data-point value in range of the given matrix
        :param dp_run:
            the number of runs to be associated with each data-point
        :return:
        """
        dp_run_queue = DpRunList()
        for dp in range(dp_start, dp_end):
            for run in range(1, dp_run):
                dp_run_queue.add_dp_run((dp, run))
        return dp_run_queue

    def gen_variables_file(self, dictionary, filepath, run_var_name) -> str:
        """
        Generates a variable file based on a given dictionary. This dictionary
        is retrieved from MatrixObject in the current implementation.
        :param dictionary:
            dictionary object containing all matrix file information
        :param filepath:
            output location for the generated variable file
        :param run_var_name:
            run variable name that will be the used to insert seed values
        :return:
        """
        with open(filepath, "w") as variable_file:
            for key in dictionary:
                variable_file.write("$define " + key + " " + dictionary[key][0] + "\n")
            variable_file.write("$define {0} 1\n".format(run_var_name))
        return filepath  # return file_path of generated file

    def gen_zip_file(self, target_file, output_path, extension="_CHUGGERZIP") -> str:
        """
        Generates a zip file given target file, utilizes CompressPackage to
        accomplish the task. NOTE: The _CHUGGER extension is used to identify files
        that were packaged by Chugger. If the Compression function detects a filename
        containing this extension, it will skip the compression of that file.
        :param target_file:
            file that is to be compressed
        :param output_path:
            location of the compressed target file copy
        :param extension:
            optional extension to the filename to help differentiate files of similar names.
        :return:
        """
        temp_zip_path = os.path.join(output_path, os.path.basename(target_file) + extension) + ".zip"
        with zipfile.ZipFile(temp_zip_path, "w") as temp_zip:
            self.compress_package(target_file, "", temp_zip, temp_zip_path)
        return temp_zip_path

    def gen_script(self, args: ParseSpace) -> bool:
        """
        Generates script/config file based on the ParseSpace object
        values. If its successful it will return true.
        :param args:
            ParseSpace object that contains all relevant manager information passed
            by the user
        :return:
        """
        if not os.path.exists(args.output):
            os.mkdir(args.output)
        if system() == "Windows":
            script_file = open(os.path.join(args.output, "ChuggerConfig.bat"), "w")
            print("Building batch script.")
            script_file.write(os.path.abspath(sys.argv[0]) + " ")
        elif system() == "Linux":
            script_file = open(os.path.join(args.output, "ChuggerConfig.sh"), "w")
            print("Building shell script.")
            which_call = subprocess.Popen(["which", "bash"], stdout=subprocess.PIPE)
            which_call.wait()
            script_file.write("#!{0}\n".format(which_call.stdout.read().decode()))
            script_file.write(os.path.abspath(sys.argv[0]) + " ")
        else:
            print("Invalid Operating system")
            return False

        def insert_quote(x) -> str:
            return "\"{0}\"".format(x)

        if script_file is not None:
            script_file.write(self.AFSIM_INPUT + " {0} ".format(insert_quote(args.startup)))
            script_file.write(self.MATRIX_INPUT + " {0} ".format(insert_quote(args.matrix)))
            script_file.write(self.OUTPUT_FOLDER + " {0} ".format(insert_quote(args.output)))
            if args.server:
                script_file.write(self.SERVER_MANAGER + " ")
                script_file.write(self.IP_ADDRESS + " " + args.ip + " ")
                script_file.write(self.PORT_NUMBER + " " + str(args.port) + " ")
                script_file.write(self.SCENARIO_FOLDER + " {0} ".format(insert_quote(args.scenario)))
                if args.linux:
                    script_file.write(self.LINUX_SUPPORT + " ")
                    script_file.write(self.LINUX_EXEC_FILE + " {0} ".format(insert_quote(args.linux_exe)))
                    script_file.write(self.LINUX_BINARY_FOLDER + " {0} ".format(insert_quote(args.linux_binary)))
                if args.windows:
                    script_file.write(self.WINDOWS_SUPPORT + " ")
                    script_file.write(self.WINDOWS_EXEC_FILE + " {0} ".format(insert_quote(args.windows_exe)))
                    script_file.write(self.WINDOWS_BINARY_FOLDER + " {0} ".format(insert_quote(args.windows_binary)))
            else:
                script_file.write(self.LOCAL_MANAGER + " ")
                script_file.write(self.LOCAL_THREADS + " " + str(args.local_threads) + " ")
                script_file.write(self.LOCAL_EXEC + " {0} ".format(insert_quote(args.local_exe)))
            if args.runs != "":
                script_file.write(self.RUNS + " " + str(args.runs) + " ")
            if args.variables != "":
                script_file.write(self.VARIABLES + " {0} ".format(insert_quote(args.variables)))
            script_file.write(self.DP_RANGE + " " + str(args.dp_range[0]) + " " + str(args.dp_range[1]) + " ")
            if args.run_variable != "":
                script_file.write(self.RUN_VARIABLE_NAME + " " + args.run_variable + " ")
        script_file.close()
        print("Script file completed. Output: ", args.output)
        return True

    def process_args(self, args: ParseSpace) -> ParseLog:
        """
        Parse a given ParseSpace object. Utilizes ParseLog to hold
        information regarding notice and error data.
        :param args:
            ParseSpace object that contains all relevant manager information passed
            by the user
        :return:
        """
        log = ParseLog()
        if not args.startup:
            log.append_error("ERROR: AFSIM startup file is not specified.\n")
        else:
            args.startup = os.path.realpath(args.startup)
        if not args.matrix:
            log.append_error("ERROR: Matrix file is not specified.\n")
        else:
            args.matrix = os.path.realpath(args.matrix)
            result = self.parse_matrix(args)
            if not result:
                log.append_error("ERROR: Unable to parse Matrix file.\n")
            else:
                if args.dp_range[0] != 0 or args.dp_range[1] != 0:
                    if args.dp_range[0] > args.dp_range[1]:
                        log.append_error("ERROR: starting_dp can't be greater than ending_dp.\n")
                    elif args.dp_range[0] <= 0 or args.dp_range[1] <= 0:
                        log.append_error("ERROR: starting_dp/ending_dp must be greater than zero.\n")
                    elif args.dp_range[1] > args.matrix_value_len:
                        log.append_error("Max data-point value specified by user exceeds what available from Matrix "
                                         "file.\n")
                    elif args.dp_range[0] == args.dp_range[1]:
                        log.append_notice("NOTICE: running a single DP. DP = " + str(args.dp_range[0]) + ".\n")
                elif args.dp_range[0] == 0 and args.dp_range[1] == 0:
                    args.dp_range[0] = 1
                    args.dp_range[1] = args.matrix_value_len
        if not args.output:
            log.append_error("ERROR: Output file not specified.\n")
        else:
            args.output = os.path.realpath(args.output)
            if not os.path.exists(args.output):
                os.makedirs(args.output, exist_ok=True)
        if args.server:
            if not args.ip:  # check if the user specified the ip address
                log.append_error("ERROR: Port number specified, but not IP address.\n")
            if not args.port:  # check if the user specified the port number
                log.append_error("ERROR: IP address specified, but not Port Number.\n")
            if not args.scenario:  # check if the user specified the scenario folder
                log.append_error("ERROR: Scenario folder not specified. Needed for Server transfer.\n")
            if args.linux:
                if not args.linux_exe:
                    log.append_error("ERROR: Linux executable must be specified.\n")
                if not args.linux_binary:
                    log.append_error("ERROR: Linux binary folder must be specified.\n")
            if args.windows:
                if not args.windows_exe:
                    log.append_error("ERROR: Windows executable must be specified.\n")
                if not args.windows_binary:
                    log.append_error("ERROR: Windows binary folder must be specified.\n")
            if not args.linux and not args.windows:
                log.append_error("ERROR: You must specify support for either Windows, Linux, or both.\n")
        elif args.local:
            if args.local_threads == 1:
                log.append_notice("NOTICE: Default parameters used: Cores/Thread = 1.\n")
            elif args.local_threads < 1:
                log.append_error("ERROR: Cannot specify less that 1 thread of execution.\n")
            if not args.local_exe:
                log.append_error("ERROR: You must specify the local executable to run.\n")
            else:
                args.local_exe = os.path.realpath(args.local_exe)
        else:
            log.append_error("ERROR: Specify either local execution or server execution.\n")
        if args.runs == 1:
            log.append_notice("NOTICE: Default parameters used: Runs = 1.\n")
        elif args.runs <= 0:
            log.append_error("ERROR: Run parameter must be greater than 0.\n")
        if not args.variables:
            log.append_notice("NOTICE: No variables file specified, generating variable file based on Matrix file.\n")
        else:
            if not os.path.exists(args.variables):
                log.append_error("ERROR: This variable file does not exist.\n")

        return log

    def gen_server_mgr(self, args: ParseSpace, stream: StreamInterface):
        """
        Base generator function that handles the initialization and lifetime of Server Managers.
        :param args:
        :param stream:
        :return:
        """
        start_time = datetime.datetime.now().strftime("%H:%M:%S")
        stream.write_terminal("Initializing @ time: {0}".format(start_time))
        timestamp_dir = os.path.join(args.output, datetime.datetime.now().strftime("%m-%d-%Y@%H'%M'%S"))
        try:
            os.mkdir(timestamp_dir)
        except FileExistsError:
            time.sleep(.25)
            os.mkdir(timestamp_dir)

        if not args.variables:
            args.variables = self.gen_variables_file(args.matrix_dict,
                                                     os.path.join(timestamp_dir, "CHUGGER_AUTO_GEN.txt"),
                                                     args.run_variable)
            stream.write_terminal("Auto generated variable file: " + args.variables)
        dp_run_queue = self.gen_dp_run_queue(args.dp_range[0], args.dp_range[1] + 1, args.runs + 1)
        stream.write_terminal("Creating server manager")
        args.scenario = os.path.realpath(args.scenario)
        linx_zip = None
        wind_zip = None
        if args.windows:
            args.windows_binary = os.path.realpath(args.windows_binary)
            args.windows_exe = os.path.realpath(args.windows_exe)
            wind_zip = self.gen_zip_file(args.windows_binary, args.output, "_CHUGGERZIP_win")
        if args.linux:
            args.linux_binary = os.path.realpath(args.linux_binary)
            args.linux_exe = os.path.realpath(args.linux_exe)
            linx_zip = self.gen_zip_file(args.linux_binary, args.output, "_CHUGGERZIP_lin")
        scen_zip = self.gen_zip_file(args.scenario, args.output)
        ServerManager(stream, dp_run_queue, timestamp_dir, scen_zip, wind_zip, linx_zip, args)
        self.gen_logs(stream, timestamp_dir)

    def gen_local_mgr(self, args: ParseSpace, stream: StreamInterface):
        """
        Base generator function that handles the initialization and lifetime of Local Managers
        :param args:
        :param stream:
        :return:
        """
        start_time = datetime.datetime.now().strftime("%H:%M:%S")
        stream.write_terminal("Initializing @ time: {0}".format(start_time))
        timestamp_dir = os.path.join(args.output, datetime.datetime.now().strftime("%m-%d-%Y@%H'%M'%S"))
        try:
            os.mkdir(timestamp_dir)
        except FileExistsError:
            time.sleep(.25)
            os.mkdir(timestamp_dir)

        if not args.variables:
            args.variables = self.gen_variables_file(args.matrix_dict,
                                                     os.path.join(timestamp_dir, "CHUGGER_AUTO_GEN.txt"),
                                                     args.run_variable)
            stream.write_terminal("Auto generated variable file: " + args.variables)
        dp_run_queue = self.gen_dp_run_queue(args.dp_range[0], args.dp_range[1] + 1, args.runs + 1)
        LocalManager(stream, dp_run_queue, timestamp_dir, args.matrix_dict, args)
        stream.write_terminal("Finish time: {0} -> {1}".format(start_time,
                                                               datetime.datetime.now().strftime("%H:%M:%S")))
        self.gen_logs(stream, timestamp_dir)

    def gen_logs(self, stream: StreamInterface, output_dir: str, clear_log: bool = True, optional_prepend: str = None):
        """
        Create log files that will be used to store all information outputted to the stream interface
        during the lifetime of a Manager.
        :param stream:
            StreamInterface object that allows for interprocess communication
        :param output_dir:
            string parameter to specify the location of the tobe generated log file
        :param clear_log:
            boolean to indicate whether to clear the logs
        :param optional_prepend:
            special string to be added to the first line of the log file
        :return:
        """
        stream.update_blocks()
        term_file_path = os.path.join(output_dir, "terminal_out.log")
        proc_file_path = os.path.join(output_dir, "process_out.log")
        stream.write_terminal("Generating Log file: {0}".format(term_file_path))
        with open(os.path.join(output_dir, "terminal_out.log"), "w") as terminal_log:
            if optional_prepend is not None:
                terminal_log.write(optional_prepend)
            for line in stream.term_out_log:
                terminal_log.write(line)
        stream.write_terminal("Generating Log file: {0}".format(proc_file_path))
        with open(os.path.join(output_dir, "process_out.log"), "w") as process_log:
            if optional_prepend is not None:
                process_log.write(optional_prepend)
            for line in stream.proc_out_log:
                process_log.write(line)
        stream.update_blocks()
        if clear_log:
            stream.term_out_log.clear()
            stream.proc_out_log.clear()
        stream.write_terminal("Finished generating log files")

    def compress_package(self, target_dir: str, output_folder: str, active_zip_file: zipfile.ZipFile,
                         active_zip_file_path: str):
        """
        Helper function that compresses a given target directory and makes a zipped copy
        of its target. It utilizes the zipfile module to handle all files that need to be
        compressed into a zip. Both the active_zip_file_path and extensions of the zipfile name
        are used to avoid repackaging already zipped files.
        :param target_dir:
            File path to indicate which file is to be compressed
        :param output_folder:
            Location of the copied target zip file
        :param active_zip_file:
            zipfile object to keep track of all compressed file
        :param active_zip_file_path:
            Zipfile path that will identify the location of the compressed file.
        :return:
        """
        # print("Output Folder: ", output_folder)
        if os.path.exists(target_dir):
            for filenames in os.listdir(target_dir):
                child_file = os.path.join(target_dir, filenames)
                if os.path.isdir(child_file):
                    active_zip_file.write(child_file, os.path.join(output_folder, filenames))
                    # print("Adding Directory: ", child_file)
                    self.compress_package(child_file, os.path.join(output_folder, filenames),
                                          active_zip_file, active_zip_file_path)
                else:
                    if not child_file == active_zip_file_path:
                        # print("Adding file: ", child_file)
                        active_zip_file.write(os.path.join(target_dir, filenames),
                                              os.path.join(output_folder, filenames))
                    # else:
                    #     print("active zip file found in output folder!")
        else:
            print("This path does not exist!")

