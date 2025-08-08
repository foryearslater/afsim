# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
"""Extracts routes and route networks from a set of AFSIM files into a shapefile
usage:
       python afsim_to_shape.py afsim_file.txt """
import sys, re
import afpy.shapefile as shapefile
from afpy.afsim_preprocess import preprocess
 
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

def find_routes(text):
   routes=[]
   #routes = re.findall('route.+?end_route', text)
   routeStrings = re.findall(r'route\s.+?end_route\s', text, re.DOTALL)
   print 'found', len(routeStrings), 'routes'
   for r in routeStrings:
      route = []
      if type(r) == type(''):
         for pos in re.findall(r'\s*position\s+[0-9.:]+[nsNS]\s+[0-9.:]+[ewEW]', r):
            lat, lon = pos.strip().split()[1:]
            route.append( (LL_FromString(lat), LL_FromString(lon)) )
         if len(route)>0:
            routes.append(route)
         else:
            print 'empty route'
      else:
         print 'group...'
   return routes 

if __name__ == '__main__':
   routes=[]
   if len(sys.argv)<2:
      print sys.modules[__name__].__doc__
   else:   
      for file in sys.argv[1:]:
         text = ''.join(preprocess(file))
         routes.extend(find_routes(text))
      baseFileName = sys.argv[1].rsplit('.',1)[0]
      shapefile.VmapWriteGeoShape(baseFileName + '.shp', routes)
