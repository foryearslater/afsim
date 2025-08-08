# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
import afpy.route_network_tools as rnt
import sys

if __name__ == '__main__':
   if len(sys.argv) == 3:
      inFile = sys.argv[1]
      outFile = sys.argv[2]
      routes = rnt.read_network(sys.argv[1])
      goodRoutes = rnt.get_largest_connected_component(routes)
      rnt.print_route_network(open(outFile, 'w'), goodRoutes)
   else:
      print '  Removes unconnected route segments from the route network'
      print '  so that the resulting network is fully connected.'
      print '  Usage:'
      print '   python remove_disconnected_routes.py <input-network-file> <output-network-file>'

