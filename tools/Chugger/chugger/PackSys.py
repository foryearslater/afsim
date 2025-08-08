# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

import json

# BasePackage types. Used for deserialization
SCENINFO_PACKAGE = "SCENINFO"  # transfer needed scenario information
TRANSFER_PACKAGE = "TRANSFER"  # indicate to host to establish file transfer socket
BASEINFO_PACKAGE = "BASEINFO"  # get basic client info
CLIENTID_ASSIGN = "ASSIGNID"  # special package meant to assign the client with a unique id
ERROR_PACKAGE = "ERRORPKG"  # indicate that the package had some unexpected information
STATUS_PACKAGE = "STATSPKG"  # indicate that the package contains server status information
CLIENT_PACKAGE = "CLINTPKG"  # indicate that the package contains client status information
# Status Keywords
CLOSE_CONNECTION = "CLOSECON"  # close the connection that is established
OPEN_CONNECTION = "OPENCONN"  # assert that the connection can stay open
KILL_PROCESSES = "KILPROCS"  # kill all running processes and don't transfer data
END_PROCESSES = "ENDPROCS"  # kill all running processes and transfer data
# Client Request keywords
ALIVE_CONNECTION = "ALIVECON"  # indicate that the client connection is alive
DATA_RUN_PACKAGE = "DATARPKG"  # indicate that the client requested for datapoints to run
NO_DATAPOINTS = "NODATAPT"  # indicate that there are no more data-points
TRANSFER_DATARUNS = "DATATRAN"  # indicate that the client requests for it's datapoint runs to be transferred
SKIP_PACKAGE = "SKIPPKGE"  # a null package that can be ignored. does not store any information.


class BasePackage(object):
    """
    BasePackage json serializable class object that can transfer and communicate information between
    clients. This represents the parameter specifications for what information can be transferred.
    """
    def __init__(self, src_id: int, src_os: str, pkg_type: str, filename: str, filesize: int, ip_address: str,
                 port_number: int, scenario_folder: str, startup_file: str, bin_folder: str, exe_file: str,
                 matrix_file: str, variable_file: str, server_output: str, run_variable: str, dp: int, run: int,
                 error_msg: str, special_msg: str):
        self.src_id = src_id
        self.src_os = src_os
        self.pkg_type = pkg_type
        self.filename = filename
        self.filesize = filesize
        self.ip_address = ip_address
        self.port_number = port_number
        self.scenario_folder = scenario_folder
        self.startup_file = startup_file
        self.bin_folder = bin_folder
        self.exe_file = exe_file
        self.variable_file = variable_file
        self.matrix_file = matrix_file
        self.server_output = server_output
        self.run_variable = run_variable
        self.dp_value = dp
        self.run_value = run
        self.error_msg = error_msg
        self.special_msg = special_msg


def serialize_package(pkg: BasePackage) -> (bytes, bytes):
    """
    Takes the parameter BasePackage and serializes it into a byte package.
    The function returns a tuple of the serialized package and it's size.
    :param pkg:
    :return:
    """
    serialized_pkg = json.dumps(pkg.__dict__).encode("utf-8")
    real_hex_len = hex(len(serialized_pkg))
    padd_hex_len = (real_hex_len + (8 - len(real_hex_len)) * 'X').encode("utf-8")
    return serialized_pkg, padd_hex_len


def deserialize_package(pkg_type: str, src_id: int, byte_package: bytes) -> BasePackage:
    """
    Takes a incoming package that has been sent over the network. This function translates
    that package into a BasePackage object. It will check the package's source id and it's
    package type. If the package does not match the required source id and it's expected type, the function
    will convert the package into a error package.
    :param pkg_type:
    :param src_id:
    :param byte_package:
    :return:
    """
    recv_pkg = json.loads(byte_package.decode("utf-8"))
    pkg_src_id = recv_pkg["src_id"]
    pkg_src_os = recv_pkg["src_os"]
    recv_pkg_type = recv_pkg["pkg_type"]
    if recv_pkg_type == pkg_type and pkg_type == CLIENTID_ASSIGN:
        return create_clientid_package(pkg_src_id, pkg_src_os, recv_pkg["special_msg"])
    if recv_pkg_type == pkg_type and src_id == pkg_src_id:
        if recv_pkg_type == SCENINFO_PACKAGE:
            return create_sceninfo_package(pkg_src_id, pkg_src_os, recv_pkg["scenario_folder"],
                                           recv_pkg["startup_file"], recv_pkg["matrix_file"], recv_pkg["variable_file"],
                                           recv_pkg["server_output"], recv_pkg["run_variable"], recv_pkg["bin_folder"],
                                           recv_pkg["exe_file"])
        elif recv_pkg_type == TRANSFER_PACKAGE:
            return create_transfer_package(pkg_src_id, pkg_src_os, recv_pkg["filename"], recv_pkg["filesize"],
                                           recv_pkg["ip_address"], recv_pkg["port_number"])
        elif recv_pkg_type == BASEINFO_PACKAGE:
            return create_baseinfo_package(pkg_src_id, pkg_src_os, recv_pkg["special_msg"])
        elif recv_pkg_type == ERROR_PACKAGE:
            return create_error_package(pkg_src_id, pkg_src_os, recv_pkg["error_msg"])
        elif recv_pkg_type == STATUS_PACKAGE:
            return create_status_package(pkg_src_id, recv_pkg["special_msg"])
        elif recv_pkg_type == CLIENT_PACKAGE:
            return create_client_package(pkg_src_id, pkg_src_os, recv_pkg["dp_value"], recv_pkg["run_value"],
                                         recv_pkg["filename"], recv_pkg["filesize"], recv_pkg["special_msg"])
        else:
            create_error_package(pkg_src_id, pkg_src_os, "ERROR: invalid package type")
    # print(pkg_type)
    # print(recv_pkg)
    return create_error_package(pkg_src_id, pkg_src_os, "ERROR: invalid id or type")


def create_sceninfo_package(src_id: int, src_os: str, scenario_folder: str, startup_file: str, matrix_file: str,
                            variable_file: str, server_output: str, run_variable: str, bin_folder: str,
                            exe_file: str) -> BasePackage:
    """
    Create a BasePackage containing all required scenario information needed to run the sim remotely.
    This created package will be returned to the caller.
    :param src_id:
    :param src_os:
    :param scenario_folder:
    :param startup_file:
    :param matrix_file:
    :param variable_file:
    :param server_output:
    :param run_variable:
    :param bin_folder:
    :param exe_file:
    :return:
    """
    return BasePackage(src_id, src_os, SCENINFO_PACKAGE, None, None, None, None, scenario_folder, startup_file,
                       bin_folder, exe_file, matrix_file, variable_file, server_output, run_variable, None, None, None,
                       None)


def create_transfer_package(src_id: int, src_os: str, filename: str, filesize: int, ip_address: str,
                            port_number: int) -> BasePackage:
    """
    Create a package requesting for a certain file. This BasePackage does not guarantee that the recipient has the
    requested file.
    This created package will be returned to the caller.
    :param src_id:
    :param src_os:
    :param filename:
    :param filesize:
    :param ip_address:
    :param port_number:
    :return:
    """
    return BasePackage(src_id, src_os, TRANSFER_PACKAGE, filename, filesize, ip_address, port_number, None,
                       None, None, None, None, None, None, None, None, None, None, None)


def create_baseinfo_package(src_id: int, src_os: str, spec_msg: str) -> BasePackage:
    """
    Create a basic BasePackage containing arbitrary information.
    This created package will be returned to the caller.
    :param src_id:
    :param src_os:
    :param spec_msg:
    :return:
    """
    return BasePackage(src_id, src_os, BASEINFO_PACKAGE, None, None, None, None, None,
                       None, None, None, None, None, None, None, None, None, None, spec_msg)


def create_clientid_package(src_id: int, src_os: str, spec_msg: str) -> BasePackage:
    """
    A special exception package that assigns a client a special id value. However, in
    the future, the user should create a whitelist of clients that the ServerManager will use to verify connecting
    clients.This created package will be returned to the caller.
    :param src_id:
    :param src_os:
    :param spec_msg:
    :return:
    """
    return BasePackage(src_id, src_os, CLIENTID_ASSIGN, None, None, None, None,
                       None, None, None, None, None, None, None, None, None, None, None, spec_msg)


def create_error_package(src_id: int, src_os: str, err_msg: str) -> BasePackage:
    """
    A error package indicating some invalid behavior or package has been found. This package should be given a err_msg
    to indicate the error that occurred.
    This created package will be returned to the caller.
    :param src_id:
    :param src_os:
    :param err_msg:
    :return:
    """
    return BasePackage(src_id, src_os, ERROR_PACKAGE, None, None,
                       None, None, None, None, None, None, None, None, None, None, None, None, err_msg, None)


def create_status_package(src_id: int, special_msg: str) -> BasePackage:
    """
    Status package that indicates the current state of the client or the server. This is mainly used to relay whether
    or not the server has closed.
    :param src_id:
    :param special_msg:
    :return:
    """
    return BasePackage(src_id, None, STATUS_PACKAGE, None, None, None, None,
                       None, None, None, None, None, None, None, None, None, None, None, special_msg)


def create_client_package(src_id: int, src_os: str, dp: int, run: int, filename: str, filesize: int,
                          special_msg: str) -> BasePackage:
    """
    Client package that holds information that the client will use to execute datapoints that are in queue.
    This created package will be returned to the caller.
    :param src_id:
    :param src_os:
    :param dp:
    :param run:
    :param filename:
    :param filesize:
    :param special_msg:
    :return:
    """
    return BasePackage(src_id, src_os, CLIENT_PACKAGE, filename, filesize, None, None,
                       None, None, None, None, None, None, None, None, dp, run, None,
                       special_msg)


def bytehex_to_dec(hex_bytes: bytes) -> int:
    """
    Auxiliary function meant to convert encoded package sizes to decimal values.
    :param hex_bytes:
    :return:
    """
    return int(hex_bytes.decode("utf-8").rstrip("X"), 16)
