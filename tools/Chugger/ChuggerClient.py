#!/usr/bin/python3
# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Chugger v0.91 Matrix Execution script
# Chugger is a execution script that will run a series of simulations based on data-points found within a matrix (DOE)
# file in a *.csv format. Chugger v0.91 features a Local Execution Manager and a Server Execution Manager.
# This script serves as an alternative to PIANO, and is compatible with Windows and Linux.
# Disclaimer
# Chugger is still in its development stage with more features and improvements to be added. **Use Chugger at its your
# own risk**. This script has been tested in multiple instances in both controlled and real-application runs, but it
# does not mean its completely bullet-proof.
# Please report any bugs, glitches, and possible suggestions to what features should be added.

import argparse
import os

from chugger import StreamInterface
from chugger.client import ClientManager

if __name__ == "__main__":
    parser = argparse.ArgumentParser("This is a Chugger CMD Frontend client")
    parser.add_argument("--ip_address", "-ip", type=str, help="Specify ip address of server.", required=True)
    parser.add_argument("--port_number","-port", type=int, help="Specify port number of server.", required=True)
    parser.add_argument("--output_folder", "-out", type=str, help="Specify output folder to place file.", required=True)
    parser.add_argument("--threads", "-thrd", type=int, default=1,
                        help="Specify the amount of threads you want to execute at once.")
    try:
        args = parser.parse_args()
        stream = StreamInterface()

        args.output_folder = os.path.realpath(args.output_folder)

        if not os.path.exists(args.output_folder):  # check if parent folder exists
            os.mkdir(args.output_folder)

        temp_client_dir = os.path.join(args.output_folder, "CLIENT_CHUGGER_FOLDER")

        client_mgr = ClientManager(args.ip_address, args.port_number, temp_client_dir, args.threads, stream)
    except SystemExit:
        input("Enter any key to exit application")


