# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
"""Extracts segments from a shapefile, and builds an AFSIM route network.
usage:
       python shape_to_road_network.py shapefile.shp"""

import afpy.shapefile as shapefile
import sys, math

def lat_to_string(l):
   if l < 0:
      return str(-l) + 's'
   return str(l) + 'n'
def lon_to_string(l):
   if l < 0:
      return str(-l) + 'w'
   return str(l) + 'e'

class Vertex:
   def __init__(self, lat, lon, index, routeId, node):
      self.lat = lat
      self.lon = lon
      self.index = index
      self.routeId = routeId
      self.node = node

if __name__ == '__main__':
   if len(sys.argv)<2:
      print sys.modules[__name__].__doc__
   else:   
      all_verts = []
      lines = shapefile.ReadShapefile(sys.argv[1])
      index=0
      routeId=0
      for l in lines:
         for v in l:
            vert = Vertex(v[1], v[0], index, routeId, 0) #[v[0], v[1], index, routeId, 0]
            all_verts.append(vert)
            index+=1
         routeId+=1
      all_verts.sort(key=lambda v: v.lat)
      cCLOSE_ENOUGH = 1.0E-15
      nextNodeId = 1
      for i in range(len(all_verts)):
         v = all_verts[i]
         nodeId = v.node
         ni = i+1
         matches=[]
         while ni < len(all_verts):
            n = all_verts[ni]
            if n.lat - v.lat < cCLOSE_ENOUGH:
               if math.fabs(n.lon - v.lon) < cCLOSE_ENOUGH:
                  matches.append(ni)
            else:
               break
            ni+=1
         if nodeId == 0:
            for j in matches:
               id = all_verts[j].node
               nodeId = id
         if nodeId == 0 and len(matches)>0: 
            nodeId = nextNodeId
            nextNodeId+=1
         if nodeId != 0:
            v.node = nodeId
            for j in matches:
               all_verts[j].node = nodeId
      all_verts.sort(key=lambda v: v.index)
      print 'route_network road_network'
      routeIndex=-1
      for i in range(len(all_verts)):
         v = all_verts[i]
         if v.routeId != routeIndex:
            if routeIndex != -1:
               print 'end_route'
            routeIndex = v.routeId
            print 'route'
         print ' position', lat_to_string(v.lat), lon_to_string(v.lon),
         if v.node != 0:
            print 'node_id n_' + str(v.node)
         else:
            print ''
      print 'end_route'
      print 'end_route_network'
