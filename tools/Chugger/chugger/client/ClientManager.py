# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

import datetime
from collections import deque
import csv
import multiprocessing
import os
import re
import shutil
import socket
import subprocess
import sys
import threading
import time
import timeit
import zipfile
from chugger import PackSys, StreamInterface


class ClientManager(object):
    """
    ClientManager is the object that allows for communication
    between the Server Manager on the ChuggerServer.py script
    This manager controls the lifetime and data transfer of all
    processes.
    """

    def __init__(self, host_name: str, port: int, output_dir: str, threads: int, stream: StreamInterface):
        """
        :param host_name:
            IPV4 string to attempt connection to
        :param port:
            port number to attempt connection to
        :param output_dir:
            main output directory to store simulation output
        :param threads:
            number of threads to run on this machine
        :param stream:
            StreamInterface object to allow for interprocess communication
        """
        self.main_sock = None
        self.serv_ip = host_name
        self.serv_port = port
        self.os = sys.platform
        self.client_id = 0
        self.threads = threads
        self.output_dir = output_dir
        self.dp_run_dir = None
        self.scenario_folder = None
        self.bin_folder = None
        self.exe_file = None
        self.variable_file = None
        self.variable_data = list()
        self.startup_file = None
        self.matrix_file = None
        self.matrix_dict = dict()
        self.server_output = None
        self.run_variable = None
        self.stream = stream

        terminal_thread = threading.Thread(target=self.update_terminal)
        terminal_thread.start()
        if self.establish_socket():
            if self.setup_client():
                self.stream.write_terminal("Ready to begin")
                self.process_manager()
                if os.path.exists(self.output_dir):
                    shutil.rmtree(self.output_dir) # clean up only when the client has successfully connected
                    print("Cleaning up directory: {0}".format(self.output_dir))
        self.stream.kill_event.set()
        self.main_sock.close()


    def establish_socket(self) -> bool:
        """
        Try to intialize a connection with the remote server manager.
        Return True or False depending on if it was successful.
        :return:
        """
        try:
            self.main_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.main_sock.settimeout(5)  # five second timeout before cancelling
            self.stream.write_terminal("connecting to IP address: " + self.serv_ip)
            self.stream.write_terminal("connecting to Port number: " + str(self.serv_port))
            self.main_sock.connect((self.serv_ip, self.serv_port))
            self.main_sock.settimeout(None)  # if we connect, go ahead and set block
            self.accept_request(self.main_sock, self.client_id, PackSys.BASEINFO_PACKAGE)
            recv_pkg = self.receive_package(self.main_sock, self.client_id, PackSys.CLIENTID_ASSIGN)
            # self.stream.write_terminal(recv_pkg.__dict__)
            if recv_pkg.special_msg == PackSys.CLOSE_CONNECTION or recv_pkg.pkg_type == PackSys.ERROR_PACKAGE:
                print("The Server signaled to close: Unable to serve client.")
                self.main_sock.close()
                return False
            self.client_id = recv_pkg.src_id
            self.stream.write_terminal("Successful Server connection.")
            return True
        except OSError as error:
            print("Server Connection Error: {0}".format(error))
            return False

    def setup_client(self) -> bool:
        """
        Setup the environment that allows the client to remotely
        execute simulation. First establish output directory, then
        request for scenario information.
        :return:
        """
        self.stream.write_terminal("Now Waiting for Server response.")
        try:
            distr_pkg = self.receive_package(self.main_sock, self.client_id, PackSys.STATUS_PACKAGE)
            # print(distr_pkg.__dict__)
            if distr_pkg.special_msg == PackSys.OPEN_CONNECTION:
                if os.path.exists(self.output_dir):  # check if temporary directory exists
                    self.stream.write_terminal("Output Directory already exists")
                    temp_count = 0
                    while os.path.exists(self.output_dir + '_' + str(temp_count)):
                        temp_count += 1
                    self.output_dir = self.output_dir + '_' + str(temp_count)
                self.stream.write_terminal("Creating new temporary directory: {0}".format(self.output_dir))
                os.mkdir(self.output_dir)
                self.dp_run_dir = os.path.join(self.output_dir, "DP_RUNS")
                os.mkdir(self.dp_run_dir)

                recv_pkg = self.receive_package(self.main_sock, self.client_id, PackSys.SCENINFO_PACKAGE)
                # print("PACKAGE: ", recv_pkg.__dict__)
                self.server_output = recv_pkg.server_output
                self.run_variable = recv_pkg.run_variable
                self.scenario_folder = self.request_transfer(self.main_sock, recv_pkg.src_id, self.os,
                                                             recv_pkg.scenario_folder)
                self.bin_folder = self.request_transfer(self.main_sock, recv_pkg.src_id, self.os, recv_pkg.bin_folder)
                self.matrix_file = self.request_transfer(self.main_sock, recv_pkg.src_id, self.os, recv_pkg.matrix_file)
                self.variable_file = self.request_transfer(self.main_sock, recv_pkg.src_id, self.os,
                                                           recv_pkg.variable_file)
                self.startup_file = os.path.realpath(os.path.join(self.scenario_folder, recv_pkg.startup_file))
                self.exe_file = os.path.realpath(os.path.join(self.bin_folder, recv_pkg.exe_file))
                if self.os == "linux":
                    os.chmod(self.exe_file, 744)
                with open(self.matrix_file, mode="r") as csv_file:
                    raw_csv_input = csv.DictReader(csv_file)
                    field_names = raw_csv_input.fieldnames
                    for key in field_names:
                        self.matrix_dict[key] = list()
                    for raw in raw_csv_input:
                        for key in field_names:
                            self.matrix_dict[key].append(raw[key])
                with open(self.variable_file, "r") as input_variable_file:
                    self.variable_data = input_variable_file.readlines()
                return True
            else:
                print("The Server cancelled the distribution.")
        except OSError as error:
            print("OS Error: {0}".format(error))
            return False

    def accept_request(self, sock: socket.socket, src_id: int, pkg_type: str) -> PackSys.BasePackage:
        """
        Accept a request for a specified package based on passed
        pkg_type value. The sender will send an empty package that
        will then be modified directly and sent back. This is a handshake
        function. It will send back the received package.
        :param sock:
        :param pkg_type:
        :return:
        """
        pkg_size = PackSys.bytehex_to_dec(sock.recv(8))  # 1
        recv_pkg = PackSys.deserialize_package(pkg_type, src_id, sock.recv(pkg_size))  # 2
        return self.process_request(sock, recv_pkg)

    def process_request(self, sock: socket.socket, pkg: PackSys.BasePackage) -> PackSys.BasePackage:
        """
        Process the empty pkg that was sent over with information regarding this end
        of the connection. After modifying the values in the pkg, send it back.
        :param sock:
        :param pkg:
        :return:
        """
        if pkg.pkg_type == PackSys.ERROR_PACKAGE:
            pass
        elif pkg.pkg_type == PackSys.BASEINFO_PACKAGE:
            pkg.src_os = self.os
        byte_pkg, pkg_size = PackSys.serialize_package(pkg)
        sock.sendall(pkg_size)  # 3
        sock.sendall(byte_pkg)  # 4
        return pkg

    def request_package(self, sock: socket.socket, src_id: int, pkg: PackSys.BasePackage) -> PackSys.BasePackage:
        """
        Send an empty package that will be modified by the receiver and sent back to
        the sender. This is a handshake function. It will want it's sent package back.
        :param src_id:
        :param sock:
        :param pkg:
        :return:
        """
        byte_pkg, pkg_size = PackSys.serialize_package(pkg)
        sock.sendall(pkg_size)  # 1
        sock.sendall(byte_pkg)  # 2
        recv_size = PackSys.bytehex_to_dec(sock.recv(8))  # 3
        return PackSys.deserialize_package(pkg.pkg_type, src_id, sock.recv(recv_size))  # 4

    def send_package(self, sock: socket.socket, pkg: PackSys.BasePackage):
        """
        This function sends a package defined by the user to the receiver.
        The receiver end must expect this package and it's type or else
        it will result in an error package conversion. This is not a handshake
        function. It will not care for it's package after it's sent.
        :param sock:
        :param pkg:
        :return:
        """
        byte_pkg, pkg_size = PackSys.serialize_package(pkg)
        sock.sendall(pkg_size)  # 9
        sock.sendall(byte_pkg)  # 10

    def receive_package(self, sock: socket.socket, src_id: int, pkg_type: str) -> PackSys.BasePackage:
        """
        This function directly receives a package from sender. This will
        still check for if the package that is sent is what is expected.
        If the package received is not what is expected, the package will
        be converted to an error package. This is not a handshake function.
        It will not send back the received package
        :param src_id:
        :param sock:
        :param pkg_type:
        :return:
        """
        pkg_size = PackSys.bytehex_to_dec(sock.recv(8))  # 9
        recv_pkg = PackSys.deserialize_package(pkg_type, src_id, sock.recv(pkg_size))  # 10
        return recv_pkg

    def request_transfer(self, sock: socket.socket, src_id: int, src_os: str, filename: str):
        """
        Specialized handshake function that handles the process of transferring data on a different socket.
        This function must be used in order to successfully request for file transfers.
        :param sock:
        :param src_id:
        :param src_os:
        :param filename:
        :return:
        """
        byte_pkg, pkg_size = PackSys.serialize_package(PackSys.create_transfer_package(src_id, src_os, filename, None,
                                                                                       None, None))
        sock.sendall(pkg_size)  # 5
        sock.sendall(byte_pkg)  # 6
        recv_size = PackSys.bytehex_to_dec(sock.recv(8))  # 7
        recv_pkg = PackSys.deserialize_package(PackSys.TRANSFER_PACKAGE, src_id, sock.recv(recv_size))  # 8
        # self.stream.write_terminal(recv_pkg.__dict__)
        if recv_pkg.pkg_type != PackSys.ERROR_PACKAGE:
            temp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            temp_sock.connect((recv_pkg.ip_address, recv_pkg.port_number))
            current_size = 0
            base_name = os.path.basename(filename.replace("\\", "/"))
            if base_name is not None:
                with open(os.path.join(self.output_dir, base_name), "wb") as file:
                    while current_size < recv_pkg.filesize:
                        data = temp_sock.recv(2048)
                        file.write(data)
                        current_size += len(data)
                temp_sock.close()
                try:
                    if base_name.endswith(".zip"):
                        # print("FULLNAME: ", filename)
                        # print("BASENAME: ", base_name)
                        self.stream.write_terminal("Transfer complete for file " + base_name + " Now extracting")
                        transferred_zip = zipfile.ZipFile(os.path.join(self.output_dir, base_name))
                        transferred_zip.extractall(os.path.join(self.output_dir, base_name.strip(".zip")))
                        transferred_zip.close()
                        # os.remove(os.path.join(self.output_dir, base_name))
                        self.stream.write_terminal("Finished extracting")
                        return os.path.join(self.output_dir, base_name.strip(".zip"))
                    else:
                        return os.path.join(self.output_dir, base_name)
                except zipfile.BadZipFile as error:
                    self.stream.write_process(error)
                    raise OSError("Corrupted zipfile.")
            else:
                self.stream.write_terminal("Unable to resolve basename.")
                return ""
        else:
            self.stream.write_process(recv_pkg.error_msg)

    def request_delivery(self, sock: socket.socket, src_id: int, src_os: str, filename: str, extension= "_CHUGGERZIP"):
        """
        Specialized handshake function meant to communicate and transfer datapoint run data to the
        Server. This function must be used to establish a separate socket connection for data transfer.
        :param sock:
        :param src_id:
        :param src_os:
        :param filename:
        :return:
        """
        # target_zip_file = os.path.join(self.output_dir, os.path.basename(filename) + str(src_id)) + ".zip"
        target_zip_file = os.path.join(self.output_dir, os.path.basename(filename) + str(src_id) + extension) + ".zip"
        with zipfile.ZipFile(target_zip_file, "w") as temp_zip:
            self.compress_package(filename, "", temp_zip, target_zip_file)
        self.stream.write_process("Requesting Delivery")
        byte_pkg, pkg_size = PackSys.serialize_package(PackSys.create_client_package(src_id, src_os, None, None,
                                                                                     target_zip_file,
                                                                                     os.path.getsize(target_zip_file),
                                                                                     PackSys.TRANSFER_DATARUNS))
        sock.sendall(pkg_size)
        sock.sendall(byte_pkg)
        recv_size = PackSys.bytehex_to_dec(sock.recv(8))
        transfer_package = PackSys.deserialize_package(PackSys.TRANSFER_PACKAGE, src_id, sock.recv(recv_size))
        temp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        temp_sock.connect((transfer_package.ip_address, transfer_package.port_number))
        with open(target_zip_file, "rb") as target_file:
            data = target_file.read(2048)
            while data != b'':
                temp_sock.sendall(data)
                data = target_file.read(2048)
        temp_sock.close()

    def process_manager(self):
        """
        Process_manager handles the lifetime of the Client by communicating with the server for status updates
        and data points to run. Near the end of the Client's lifetime, the process_manager function will transfer
        all finished simulation data.
        Note: As of this version, when the Server requests to end processes, the Client must be explicitly closed.
        :return:
        """
        self.stream.write_terminal("Managing Processes")
        empty_process = "EMPTY_PROC"
        dprun_process = "DPRUN_PROC"
        current_query = multiprocessing.Process(name=empty_process)
        running_queue = deque()  # holds all currently running processes
        trash_queue = deque()  # holds all dead/finished processes
        # multiprocessing lock that will notify scheduler to schedule another process to run
        lock = multiprocessing.Event()
        # indicate whether we are in a state to close client. If true don't request for more processes
        close_client = False
        try:
            while not close_client:
                # request for server status
                status_pkg = self.request_package(self.main_sock, self.client_id,
                                                  PackSys.create_status_package(self.client_id, None))
                # print("status pkg: ", status_pkg.__dict__)
                if status_pkg.special_msg != PackSys.KILL_PROCESSES:
                    # When the client has the avenue to run more processes and the client wasn't signalled to close
                    if len(running_queue) < self.threads and not close_client:
                        recv_pkg = self.request_package(self.main_sock, self.client_id,
                                                        PackSys.create_client_package(self.client_id, self.os, None,
                                                                                      None, None, None,
                                                                                      PackSys.DATA_RUN_PACKAGE))
                        if recv_pkg.special_msg != PackSys.CLOSE_CONNECTION and \
                                recv_pkg.special_msg != PackSys.NO_DATAPOINTS:
                            dp_tuple = (recv_pkg.dp_value, recv_pkg.run_value)
                            # retrieve the dp and run values
                            self.stream.write_terminal("Successful request for: " + str(dp_tuple))
                            dp_folder = os.path.join(self.dp_run_dir, "DP_" + str(dp_tuple[0]))
                            run_folder = os.path.join(dp_folder, "RUN_" + str(dp_tuple[1]))
                            if not os.path.exists(dp_folder):  # attempt to make the dp directory
                                os.mkdir(dp_folder)
                            os.mkdir(run_folder)  # create the run directory
                            # create a un-started processes that will eventually be transferred to the running queue
                            current_query = multiprocessing.Process(target=self.write_call, name=dprun_process,
                                                                    args=(dp_tuple[0] - 1, dp_tuple[1], run_folder,
                                                                          lock,))
                        else:
                            self.stream.write_process("Wrapping up processes")
                            close_client = True
                    # The server expects a second request. Be sure to not use that request pkg for anything.
                    else:
                        empty_pkg = self.request_package(self.main_sock, self.client_id,
                                                         PackSys.create_client_package(self.client_id, self.os, None,
                                                                                       None, None, None,
                                                                                       PackSys.SKIP_PACKAGE))
                        # print("empty pkg: ", empty_pkg.__dict__)
                    if len(running_queue) < self.threads and current_query.name == dprun_process:
                        running_queue.append(current_query)
                        current_query = multiprocessing.Process(name=empty_process)
                        running_queue[len(running_queue) - 1].start()
                    for started_process in running_queue:
                        if not started_process.is_alive():
                            started_process.join()
                            trash_queue.append(started_process)
                    if len(trash_queue) == 0 and len(running_queue) == self.threads:
                        while not lock.is_set():
                            status_pkg = self.request_package(self.main_sock, self.client_id,
                                                              PackSys.create_status_package(self.client_id, None))
                            # print("status pkg2: ", status_pkg.__dict__)
                            if status_pkg.special_msg == PackSys.KILL_PROCESSES:
                                self.stream.kill_event.set()
                                close_client = True
                                break
                            empty_pkg = self.request_package(self.main_sock, self.client_id,
                                                             PackSys.create_client_package(self.client_id, self.os,
                                                                                           None,
                                                                                           None, None, None,
                                                                                           PackSys.SKIP_PACKAGE))
                            # print("empty pkg2: ", empty_pkg.__dict__)
                            time.sleep(.50)
                        time.sleep(.25)
                        lock.clear()  # set the lock back to it's initial state until notified again.
                    else:
                        for dead_process in trash_queue:  # remove all dead processes
                            running_queue.remove(dead_process)
                        trash_queue.clear()
                else:
                    self.stream.kill_event.set()
                    close_client = True
            for started_process in running_queue:  # join all still running processes
                started_process.join()
            if not self.stream.kill_event.is_set():
                status_pkg = self.request_package(self.main_sock, self.client_id,
                                                  PackSys.create_status_package(self.client_id, None))
                if not status_pkg.special_msg == PackSys.KILL_PROCESSES:
                    self.request_delivery(self.main_sock, self.client_id, self.os, self.dp_run_dir)
                else:
                    self.stream.write_terminal("Unable to make delivery")
        except ConnectionResetError as error:
            self.stream.write_terminal("Connection with the server was lost {0}".format(error))

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
        # this file is used for mission execution
        instance_var_file_path = os.path.join(run_directory, "DP_" + str(dp_value + 1) + "_variable_file.txt")
        # instance_var_file_path = "DP_" + str(dp_value + 1) + "_variable_file.txt"
        # open a temp variable file to begin substituting values
        with open(instance_var_file_path, "w") as temp_var_file:
            for lines in self.variable_data:  # look through each raw data line from the original data file we read from
                # regex matches for : "$define <variable_name> [value]" angle brackets and square brackets are not part
                # of the syntax.
                # use regex to match patterns
                partition = re.findall(r"(^\$define)[ \t]+([\w\d\.-]+)[ \t]+(\"?[\w\d\.\/-]*\"?[^#\n\/\/])", lines)
                for keys in self.matrix_dict:  # cycle through all dictionary keys
                    if partition:  # if the line matches
                        if partition[0][1] == self.run_variable:
                            # special keyword to indicate the value should be the current run value
                            lines = partition[0][0] + " " + partition[0][1] + " " + str(run_value) + "\n"
                        elif partition[0][1] == keys:
                            # substitute any matching words with the appropriate value
                            lines = partition[0][0] + " " + partition[0][1] + " " + self.matrix_dict[keys][
                                dp_value] + "\n"
                temp_var_file.writelines(lines)  # write the modified line to the temporary file
        console_out = os.path.join(run_directory, "console_output_data.txt")  # file to output all console data to
        # print("Console output file: ", console_out)
        # print("Exe path: ", self.exe_file)
        # print("Instance var path", instance_var_file_path)
        # print("startup file: ", self.startup_file)
        # console_out = "console_output_data.txt"  # file to output all console data to
        # open output file and call the executable file w/ temp variable file and startup file
        with open(console_out, "w") as output_file:
            process = subprocess.Popen([self.exe_file, instance_var_file_path, self.startup_file],
                                       stdout=output_file)
            while process.poll() is None:
                if self.stream.kill_event.is_set():
                    self.stream.write_process("Terminating process for DP " + str(dp_value + 1) + " Run "
                                              + str(run_value))
                    process.terminate()
                time.sleep(1)
        self.stream.write_process("Ending process for Data-Point<{0}> Run<{1}>".format(dp_value + 1, run_value))
        self.stream.write_process("\t-->Elapsed time: {0} -> {1} == {2:.2f} seconds"
                                  .format(start_time_full, datetime.datetime.now().strftime("%H:%M:%S"),
                                          timeit.default_timer() - start_time))
        lock.set()  # notify the scheduler to launch another process
        self.stream.proc_out_running.value -= 1

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

    def update_terminal(self):
        """
        Special function that utilizes the stream interface to relay information to the user.
        Currently, this is in use as there is not a GUI interface implemented.
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
