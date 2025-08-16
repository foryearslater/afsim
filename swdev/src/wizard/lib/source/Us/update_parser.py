# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
import os
os.system("coco.exe -namespace Us script.atg")

def fixFile(fn):
   alllines=[]
   for line in open(fn,'r').readlines():
      line=line.replace('L"','"')
      line=line.replace("L'","'")
      line = line.replace('wchar_t', 'cocochar_t')
      alllines.append(line)

   open(fn, 'w').write(''.join(alllines))
fixFile('Parser.cpp')
fixFile('Scanner.cpp')
