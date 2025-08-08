# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
import sys, re
 
# "preprocess" the afsim inputs to get a list of lines in all input files
def preprocess(file, getLocations=False):
   path=['.']
   completeFiles = []
   path_vars={}
   def replace_vars(file):
      for var,val in path_vars.iteritems():
         file = file.replace('$('+var+')', val)
         file = file.replace('${'+var+'}', val)
      return file

   def _preprocess(file, lines, locations):
      completeFiles.append(file)
      lineNumber = 0
      foundFile = False
      fullPath = ''
      fileObj=0
      for p in path:
         try:
            fullPath = p+'/'+file
            fileObj = open(fullPath, 'r')
            break
         except:
            fullPath = ''
            pass
      if len(fullPath)==0:
         return
      for l in fileObj.readlines():
         lineNumber+=1
         lines.append(l)
         locations.append( (fullPath, lineNumber) )
         line = l.strip()
         if line.startswith('file_path '):
            path.append(replace_vars(line.split()[1]))
         if line.startswith('define_path_variable'):
            var,val = line.split()[1:]
            path_vars[var]=val
         if line.startswith('include_once ') or line.startswith('include '):
            newFile = line.split()[1]
            newFile = replace_vars(newFile)
            if (newFile not in completeFiles):
               _preprocess(newFile, lines, locations)
   lines=[]
   locations=['.']
   if getLocations: locations = []
   _preprocess(file, lines, locations)
   if getLocations:
      return (lines, locations)
   return lines
