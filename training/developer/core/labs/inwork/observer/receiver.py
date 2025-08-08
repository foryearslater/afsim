# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# This python script demonstrates how to receive the data sent by the 
# observer exercise.  If you have python installed, execute this script using:
# >  python receiver.py
import socket

# This port number should match the AFSIM input file
addr = ("localhost", 19240)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(addr)

print ('Waiting for data...')
while 1:
   data,addr = s.recvfrom(10000)
   print (data.decode("utf-8"))

