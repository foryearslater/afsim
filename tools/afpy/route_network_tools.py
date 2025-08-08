# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
import sys,re
import afsim_preprocess
from disjoint_set import DisjointSet

def lat_to_string(l):
   if l < 0:
      return str(-l) + 's'
   return str(l) + 'n'
def lon_to_string(l):
   if l < 0:
      return str(-l) + 'w'
   return str(l) + 'e'

def LL_ComponentToReal(comp):
   factor=1.0
   result=0
   for c in comp.split(":"):
      result+=factor*float(c)
      factor/=60
   return result

def LL_FromString(comp):
   sign = 1.0
   if comp.endswith('s') or comp.endswith('w') or comp.endswith('S') or comp.endswith('W'):
      sign = -1.0
   return sign * LL_ComponentToReal(comp[:-1])

def find_networks(text):
   networkStrings = re.findall(r'route_network\s.+?end_route_network\s', text, re.DOTALL)
   return networkStrings

def find_routes(text):
   routes=[]
   #routes = re.findall('route.+?end_route', text)
   routeStrings = re.findall(r'route\s.+?end_route\s', text, re.DOTALL)
   print 'found', len(routeStrings), 'routes'
   for r in routeStrings:
      route = []
      if type(r) == type(''):
         for pos in re.findall(r'(\s*position\s+[0-9.:]+[nsNS]\s+[0-9.:]+[ewEW])(\s+node_id\s+\S+)?', r):
            pos = ' '.join(pos)
            words=pos.strip().split()[1:]
            if len(words)==2:
               lat, lon = words
               node = ''
               route.append( (LL_FromString(lat), LL_FromString(lon), node) )
            elif len(words)==4:
               lat, lon = words[0], words[1]
               node = words[3]
               route.append( (LL_FromString(lat), LL_FromString(lon), node) )
         if len(route)>0:
            routes.append(route)
         else:
            print 'empty route'
      else:
         print 'group...'
   return routes 

def read_network(afsimFile):
   lines = afsim_preprocess.preprocess(afsimFile)
   text = '\n'.join(lines)
   nets = find_networks(text)
   if len(nets) != 1:
      print 'Found', len(nets), ' route_network(s), expected 1'
   text = nets[0]
   routes = find_routes(text)
   return routes

def get_largest_connected_component(routes):
   components = DisjointSet()
   components.insert_set('root-id')
   for i in range(len(routes)):
      r = routes[i]
      root = -1
      for n in r:
         nodeId = n[2]
         if nodeId != '':
            if not components.contains(nodeId):
               components.insert_set(nodeId)
            if root==-1:
               root = nodeId
            else:
               components.union(root, nodeId)
      # This is a waste to do after each route
      # compress after 1000 routes
      if (i%1000)==0: 
         components.compress()
   components.compress()
   counts={}
   components.count_sets(counts)
   #print counts
   #print components.m
   #print counts
   maxCount = max(counts.values())
   for k,v in counts.iteritems():
      if v==maxCount:
         maxSet = k
         break
   good_routes = []
   for i in range(len(routes)):
      r = routes[i]
      for n in r:
         nodeId = n[2]
         if nodeId != '':
            if components.find(nodeId) == maxSet:
               good_routes.append(r)
               break
   print 'Using', len(good_routes), ' out of', len(routes)
   return good_routes 

def print_route_network(file, routes, name='road_network'):
   print >>file, 'route_network', name
   for r in routes:
      print >>file, 'route'
      for n in r:
         print >>file, ' position',lat_to_string(n[0]), lon_to_string(n[1]),
         if n[2] != '':
            print >>file, 'node_id', n[2]
         else:
            print >>file, ''
      print >>file, 'end_route'
   print >>file, 'end_route_network'

