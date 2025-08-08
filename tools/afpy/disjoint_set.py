# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
class DisjointSet:
   def __init__(self):
      self.m = {}
   def insert_set(self, t):
      self.m[t] = t
   def find(self,t):
      root = self.m[t]
      if root == t: return root
      return self.find(root)
   def union(self,a,b):
      aRoot = self.find(a)
      bRoot = self.find(b)
      self.m[aRoot] = bRoot
   def contains(self,t): return t in self.m
   def compress(self):
      for k,v in self.m.iteritems():
         if k!=v:
            root = self.find(k)
            parent = k
            while parent != root:
               pp = self.m[parent]
               parent = self.m[parent]
               self.m[parent] = root
   def count_sets(self,counts):
      for k,v in self.m.iteritems():
         if v not in counts:
            counts[v] = 0
         counts[v] = counts[v]+1

