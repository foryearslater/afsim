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
import random
import socket
import threading
import time
import zipfile

from chugger import StreamInterface, PackSys
from chugger.server import DpRunList
from chugger.parser.ParseSpace import ParseSpace


class ServerManager(object):
    """
    The monolithic class that hold the current implementation of the
    server side application of Chugger. This Manager class is subject to
    massive overhauls in the future as many of its components contain
    remnants of previous implementations.
    """

    def __init__(self, stream: StreamInterface, dp_run_list: DpRunList, output: str, scen_zip: str,
                 winbin_zip: str, linbin_zip: str, args: ParseSpace):
        """
        :param stream:
            StreamInterface object that allows for interprocess communcation
        :param dp_run_list:
            DpRunList object that stores Data-point and run value tuples
        :param output:
            Special output directory which will store all Client data
        :param scen_zip:
            file path to indicate location of the scenario zip
        :param winbin_zip:
            file path to indicate location of the windows binary
        :param linbin_zip:
            file path to indicate location of the linux binary
        :param args:
            ParseSpace object to fill member variables required by the ServerManager
        """
        self.stream = stream
        self.serv_ip = args.ip
        self.serv_port = args.port
        self.serv_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.dp_run_list = dp_run_list
        self.scen_zip = scen_zip
        self.win_status = False
        self.lin_status = False
        if winbin_zip is not None and args.windows_exe is not None:
            self.winbin_zip = winbin_zip
            self.winexe = os.path.basename(args.windows_exe)
            self.win_status = True  # indicate that the server can serve Windows clients
        if linbin_zip is not None and args.linux_exe is not None:
            self.linbin_zip = linbin_zip
            self.linexe = os.path.basename(args.linux_exe)
            self.lin_status = True  # indicate that the server can serve Linux clients
        self.variables = args.variables
        self.startup = os.path.basename(args.startup)
        self.matrix = args.matrix
        self.output = output
        self.run_variable = args.run_variable
        self.client_processes = list()
        self.stream.set_stat_total(self.dp_run_list.m_list.qsize())
        self.start_distribution = False

        start_time = datetime.datetime.now().strftime("%H:%M:%S")
        if self.establish_socket():
            conn_mgr_proc = threading.Thread(target=self.accept_connections)
            conn_mgr_proc.start()
            self.stream.ready_event.set()
            while not self.stream.start_event.is_set() and not self.stream.cancel_event.is_set():
                time.sleep(1)
            if self.stream.start_event.is_set():
                self.stream.start_event.clear()
                self.start_distribution = True
                self.close_connections()
                start_time = datetime.datetime.now().strftime("%H:%M:%S")
            elif self.stream.cancel_event.is_set():
                self.stream.cancel_event.clear()
                self.start_distribution = False
                self.close_connections()
                start_time = datetime.datetime.now().strftime("%H:%M:%S")
            conn_mgr_proc.join()
            self.serv_socket.close()
            end_time = datetime.datetime.now().strftime("%H:%M:%S")
            self.stream.write_terminal("Connection manager successfully joined.")
            self.stream.write_terminal("Real finish time: {0} -> {1}".format(start_time, end_time))
        else:
            self.stream.write_terminal("Unable to launch server manager")
            self.stream.ready_event.set()
            self.stream.cancel_event.set()

    def establish_socket(self) -> bool:
        try:
            self.serv_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.serv_socket.bind((self.serv_ip, self.serv_port))
            self.serv_socket.listen(5)
            return True
        except OSError as error:
            self.stream.write_terminal(error)
            return False

    def accept_connections(self):
        client_id_roster = list()
        client_conn_roster = list()
        self.stream.write_process("Connect all clients. For CMD Frontend enter \"start\". For GUI Frontend select "
                                  "\"yes\" to begin distribution.")
        while True:
            conn_sock, addr = self.serv_socket.accept()
            client_id = random.randint(0, 100000000)
            while client_id in client_id_roster:
                client_id = random.randint(0, 100000000)
            # Ask for client information
            recv_pkg = self.request_package(conn_sock, 0, PackSys.create_baseinfo_package(0, None, None))
            # check for if package sent is a self closing package
            if recv_pkg.special_msg != PackSys.CLOSE_CONNECTION:
                time_now_str = datetime.datetime.now().strftime("%H:%M:%S")
                # Send client package declaring if they are valid for connecting.
                if recv_pkg.src_os == "win32" and self.win_status:
                    client_id_roster.append(client_id)
                    self.stream.write_process("Received Windows client: {0} -> @ {1}".format(addr, time_now_str))
                    self.stream.write_process("\t-> Client ID: {0}".format(client_id))
                    self.send_package(conn_sock, PackSys.create_clientid_package(client_id, recv_pkg.src_os,
                                                                                 PackSys.OPEN_CONNECTION))
                    client_conn_roster.append((conn_sock, client_id, recv_pkg.src_os))
                elif recv_pkg.src_os == "linux" and self.lin_status:
                    client_id_roster.append(client_id)
                    self.stream.write_process("Received Linux client: {0} -> @ {1}".format(addr, time_now_str))
                    self.stream.write_process("\t-> Client ID: {0}".format(client_id))
                    self.send_package(conn_sock, PackSys.create_clientid_package(client_id, recv_pkg.src_os,
                                                                                 PackSys.OPEN_CONNECTION))
                    client_conn_roster.append((conn_sock, client_id, recv_pkg.src_os))
                else:
                    self.stream.write_process("Received Invalid client: {0} -> @ {1}".format(addr, time_now_str))
                    self.stream.write_process("\t-> Client ID: {0}".format(client_id))
                    self.send_package(conn_sock, PackSys.create_clientid_package(0, recv_pkg.src_os,
                                                                                 PackSys.CLOSE_CONNECTION))
                    conn_sock.close()
            else:
                self.stream.write_terminal("Closing Connection port.")
                break
        if self.start_distribution:
            self.stream.write_terminal("Starting distribution.")
            for client_info in client_conn_roster:
                self.client_processes.append(multiprocessing.Process(target=self.handle_client,
                                                                     args=(client_info[0], client_info[1],
                                                                           client_info[2], False)))
        else:
            self.stream.write_terminal("Canceling distribution.")
            for client_info in client_conn_roster:
                self.client_processes.append(multiprocessing.Process(target=self.handle_client,
                                                                     args=(client_info[0], client_info[1],
                                                                           client_info[2], True)))
        for proc in self.client_processes:
            proc.start()
        for proc in self.client_processes:
            proc.join()

    def close_connections(self):
        """
        Special method that will close the connection thread for incoming clients.
        This thread connects to the Server socket and signals it to close with a
        special message CLOSE_CONNECTION the PackSys's defined macros.
        :return:
        """
        temp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        temp_sock.connect((self.serv_ip, self.serv_port))
        # special section, This will modify the package to append a special message to close
        # the connection manager.
        pkg_size = PackSys.bytehex_to_dec(temp_sock.recv(8))
        recv_pkg = PackSys.deserialize_package(PackSys.BASEINFO_PACKAGE, 0, temp_sock.recv(pkg_size))
        if recv_pkg.pkg_type != PackSys.BASEINFO_PACKAGE:
            self.stream.write_terminal("Warning received Package is of unexpected type.")
        recv_pkg.special_msg = PackSys.CLOSE_CONNECTION
        byte_pkg, pkg_size = PackSys.serialize_package(recv_pkg)
        temp_sock.sendall(pkg_size)
        temp_sock.sendall(byte_pkg)

    def handle_client(self, sock: socket.socket, client_id: int, client_os: str, cancel_distr: bool):
        """
        Client handler that will manage and distribute data to and from the client.
        :param sock:
        :param client_id:
        :param client_os:
        :param cancel_distr:
        :return:
        """
        try:
            # check if the user canceled the distribution
            if cancel_distr:  # if cancelled. Send status package to signal client to close
                self.send_package(sock, PackSys.create_status_package(client_id, PackSys.CLOSE_CONNECTION))  # 1a
            else:  # otherwise we can send the required simulation data
                self.send_package(sock, PackSys.create_status_package(client_id, PackSys.OPEN_CONNECTION))  # 1b
                if client_os == "linux":
                    self.send_package(sock, PackSys.create_sceninfo_package(client_id, client_os, self.scen_zip,
                                                                            self.startup, self.matrix, self.variables,
                                                                            self.output, self.run_variable,
                                                                            self.linbin_zip, self.linexe))  # 2b
                elif client_os == "win32":
                    self.send_package(sock, PackSys.create_sceninfo_package(client_id, client_os, self.scen_zip,
                                                                            self.startup, self.matrix, self.variables,
                                                                            self.output, self.run_variable,
                                                                            self.winbin_zip, self.winexe))  # 3b

                self.stream.write_process("Transferring scenario data to client {0}".format(client_id))
                self.accept_request(sock, client_id, PackSys.TRANSFER_PACKAGE)  # scenario folder # 4b
                self.accept_request(sock, client_id, PackSys.TRANSFER_PACKAGE)  # bin folder      # 5b
                self.accept_request(sock, client_id, PackSys.TRANSFER_PACKAGE)  # matrix file     # 6b
                self.accept_request(sock, client_id, PackSys.TRANSFER_PACKAGE)  # variables file  # 7b
                # Loop that will handle all Client requests.
                while True:
                    # Client will request for a status package first
                    status_pkg_copy = self.accept_request(sock, client_id, PackSys.STATUS_PACKAGE)  # 8b
                    # print("received status package")
                    if status_pkg_copy.special_msg == PackSys.KILL_PROCESSES:
                        break
                    # process the client information.
                    client_pkg_copy = self.accept_request(sock, client_id, PackSys.CLIENT_PACKAGE)  # 9b
                    # print("received client package")
                    if client_pkg_copy.special_msg == PackSys.TRANSFER_DATARUNS:
                        break
            sock.close()
        except ConnectionResetError as error:
            self.stream.write_process("Client {0} suffered an error {1}".format(client_id, error))
        except ConnectionAbortedError as error:
            self.stream.write_process("Client {0} suffered an error {1}".format(client_id, error))

    def request_transfer(self, sock: socket.socket, src_id: int, src_os: str, filename: str):
        """
        Request method that is specialized for requesting file transfers over the network.
        This method will send a package that will be modified by the receiver and sent back.
        This received package will contain the necessary information to open a transfer socket.
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
        if recv_pkg.pkg_type != PackSys.ERROR_PACKAGE:
            temp_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            temp_sock.connect((recv_pkg.ip_address, recv_pkg.port_number))
            current_size = 0
            with open(recv_pkg.filename, "wb") as file:
                while current_size > recv_pkg.filesize:
                    data = temp_sock.recv(2048)
                    file.write(data)
                    current_size += len(data)
            temp_sock.close()
        else:
            self.stream.write_process(recv_pkg.error_msg)

    def request_package(self, sock: socket.socket, src_id: int, pkg: PackSys.BasePackage) -> PackSys.BasePackage:
        """
        Send an empty package that will be modified by the receiver and sent back to
        the sender. This is a handshake function. It will want it's sent package back.
        :param sock:
        :param src_id:
        :param pkg:
        :return:
        """
        byte_pkg, pkg_size = PackSys.serialize_package(pkg)
        sock.sendall(pkg_size)  # 1
        sock.sendall(byte_pkg)  # 2
        recv_size = PackSys.bytehex_to_dec(sock.recv(8))  # 3
        return PackSys.deserialize_package(pkg.pkg_type, src_id, sock.recv(recv_size))  # 4

    def accept_request(self, sock: socket.socket, src_id: int, pkg_type: str) -> PackSys.BasePackage:
        """
        Accept a request for a specified package based on passed
        pkg_type value. The sender will send an empty package that
        will then be modified directly and sent back. This is a handshake
        function. It will send back the received package.
        :param sock:
        :param src_id:
        :param pkg_type:
        :return:
        """
        pkg_size = PackSys.bytehex_to_dec(sock.recv(8))  # 1
        recv_pkg = PackSys.deserialize_package(pkg_type, src_id, sock.recv(pkg_size))  # 2
        return self.process_request(sock, recv_pkg)

    def process_request(self, sock: socket.socket, pkg: PackSys.BasePackage) -> PackSys.BasePackage:
        """
        process_request will handle the package sent by the client and modify the information contained
        within the package. After modifying the contents of the package, it will send it back to the
        client. The package that is sent will also be returned as a copy. This will allow the callee
        to make decisions based on the package that is sent.
        :param sock:
        :param pkg:
        :return:
        """
        pkg_id = pkg.src_id
        if pkg.pkg_type == PackSys.TRANSFER_PACKAGE:
            try:
                filename = pkg.filename
                with open(filename, "rb") as file:
                    self.stream.write_process("Client {0} requested for file: {1}".format(pkg_id, filename))
                    temp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    temp_socket.bind((self.serv_ip, 0))
                    pkg.ip_address = self.serv_ip
                    pkg.port_number = temp_socket.getsockname()[1]
                    pkg.filesize = os.path.getsize(filename)
                    byte_pkg, pkg_size = PackSys.serialize_package(pkg)
                    sock.sendall(pkg_size)  # 7
                    sock.sendall(byte_pkg)  # 8
                    temp_socket.listen(1)
                    conn_sock, addr = temp_socket.accept()
                    data = file.read(2048)
                    while data != b'':
                        conn_sock.sendall(data)
                        data = file.read(2048)
                conn_sock.close()
                temp_socket.close()
                return pkg
            except FileNotFoundError as error:
                byte_pkg, pkg_size = PackSys.serialize_package(
                    PackSys.create_error_package(pkg_id, pkg.src_os, str(error)))
                sock.sendall(pkg_size)  # 7
                sock.sendall(byte_pkg)  # 8
                return pkg
        if pkg.pkg_type == PackSys.ERROR_PACKAGE:
            pass
        if pkg.pkg_type == PackSys.STATUS_PACKAGE:
            if self.stream.kill_event.is_set():
                pkg.special_msg = PackSys.KILL_PROCESSES
            else:
                pkg.special_msg = PackSys.ALIVE_CONNECTION
        if pkg.pkg_type == PackSys.CLIENT_PACKAGE:
            if pkg.special_msg == PackSys.TRANSFER_DATARUNS:
                self.stream.write_process("Client {0} has finished and has requested a delivery.".format(pkg.src_id))
                filesize, filename = pkg.filesize, pkg.filename
                temp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                temp_socket.bind((self.serv_ip, 0))
                transfer_pkg, pkg_size = PackSys.serialize_package(
                    PackSys.create_transfer_package(pkg_id, pkg.src_os, None, None, self.serv_ip,
                                                    temp_socket.getsockname()[1]))
                sock.sendall(pkg_size)
                sock.sendall(transfer_pkg)
                temp_socket.listen(1)
                conn_sock, addr = temp_socket.accept()
                current_size = 0
                target_file = os.path.join(self.output, os.path.basename(filename))
                with open(target_file, "wb") as file:
                    while current_size < filesize:
                        data = conn_sock.recv(2048)
                        file.write(data)
                        current_size += len(data)
                conn_sock.close()
                temp_socket.close()
                try:
                    if target_file.endswith(".zip"):
                        output_zip_file = os.path.join(self.output,
                                                       os.path.basename(target_file).strip("{0}.zip".format(pkg_id)))
                        self.stream.write_terminal("Delivery complete for file {0}. Now extracting".format(target_file))
                        transferred_zip = zipfile.ZipFile(target_file)
                        transferred_zip.extractall(output_zip_file)
                        self.stream.write_terminal(
                            "Finished extracting. Extract location -> {0}".format(output_zip_file))
                    self.stream.write_process("Delivery complete")
                except zipfile.BadZipFile as error:
                    self.stream.write_terminal(error)
                return pkg
            elif pkg.special_msg == PackSys.SKIP_PACKAGE:
                pass
            else:
                dp_run_tuple = self.dp_run_list.get_dp_run()
                self.stream.set_stat_remaining(self.dp_run_list.m_list.qsize())
                if dp_run_tuple == (0, 0):
                    self.stream.write_process(
                        "Signaling client {0} that there are no more data-points to run. -> @ {1}".format(
                            pkg_id, datetime.datetime.now().strftime("%H:%M:%S")))
                    pkg.special_msg = PackSys.NO_DATAPOINTS
                else:
                    self.stream.write_process("Sending data points {0} to client {1} -> @ {2}".format(
                        dp_run_tuple, pkg_id, datetime.datetime.now().strftime("%H:%M:%S")))
                pkg.dp_value = dp_run_tuple[0]
                pkg.run_value = dp_run_tuple[1]
        byte_pkg, pkg_size = PackSys.serialize_package(pkg)
        sock.sendall(pkg_size)  # 3
        sock.sendall(byte_pkg)  # 4
        return pkg

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
        sock.sendall(pkg_size)  # 1
        sock.sendall(byte_pkg)  # 2

    def receive_package(self, sock: socket.socket, src_id: int, pkg_type: str) -> PackSys.BasePackage:
        """
        This function directly receives a package from sender. This will
        still check for if the package that is sent is what is expected.
        If the package received is not what is expected, the package will
        be converted to an error package. This is not a handshake function.
        It will not send back the received package
        :param sock:
        :param src_id:
        :param pkg_type:
        :return:
        """
        pkg_size = PackSys.bytehex_to_dec(sock.recv(8))  # 1
        recv_pkg = PackSys.deserialize_package(pkg_type, src_id, sock.recv(pkg_size))  # 2
        return recv_pkg
