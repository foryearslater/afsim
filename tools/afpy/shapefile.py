# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************
import struct

def WriteProjectionFile(aFileName):
   open(aFileName,'w').write("GEOGCS[\"Geographic Coordinate System\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137,298.257223560493]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]]")

def ComputeFileLength(features):
   #length = 100
   length = 50
   for f in features:
      length += 4 + (22 + 2 * 1) + (10 * len(f))
   return length

def WriteMainFileHeader(aOut, aFileLength, aMBR):
   fileCode = 9994;
   aOut.write(struct.pack('>7I', fileCode,0,0,0,0,0,aFileLength))
   aOut.write(struct.pack('2I4d4d', 1000, 3, aMBR[1], aMBR[0], aMBR[3], aMBR[2], 0,0,0,0))

def WriteMainFileContentHeader(aOut, aRecordNumber, aContentLength):
   aOut.write(struct.pack('>II', aRecordNumber, aContentLength))
def ReadMainFileContentHeader(aFile):
   return struct.unpack('>II', aFile.read(8))

def WritePolyLineRecord(aOut, points, mbr):
   shapeType = 3
   aOut.write(struct.pack('I', shapeType))
   aOut.write(struct.pack('4d', mbr[1], mbr[0], mbr[3], mbr[2]))
   aOut.write(struct.pack('3I', 1, len(points), 0))
   for p in points:
      aOut.write(struct.pack('2d', p[1], p[0]))
def ReadPolyLineRecord(aFile):
   shapeType = 3
   struct.unpack('I', aFile.read(4))
   mbr = struct.unpack('4d', aFile.read(8*4))
   ele, pointCount, zero = struct.unpack('3I', aFile.read(4*3))
   points = []
   for p in range(pointCount):
      pts = struct.unpack('2d', aFile.read(8*2))
      points.append( (float(pts[0]), float(pts[1])) )
   return points
def GetMBRs(features):
   mbrs=[]
   for f in features:
      if len(f)>0:
         lats = [p[0] for p in f]
         lons = [p[1] for p in f]
         mbrs.append([min(lats), min(lons), max(lats), max(lons)])
   return ([min([p[0] for p in mbrs]), min([p[1] for p in mbrs]), max([p[2] for p in mbrs]), max([p[3] for p in mbrs])], mbrs)

def VmapWriteGeoShape(aFileName, features, writeDbf = False):
   i=0
   # Remove empty features
   while i < len(features):
      if len(features[i])<1:
         del features[i]
      else:
         i+=1
   mbr, mbrs = GetMBRs(features)
   projFileName = aFileName.rsplit('.',1)[0]
   WriteProjectionFile(projFileName + ".prj");
   ofs = open(aFileName, 'wb')
   # Create the header
   fileLength = ComputeFileLength(features)
   #print fileLength
   WriteMainFileHeader(ofs, fileLength, mbr)
   numParts = 1
   i = 0
   indexing = []
   for f in features:
      # content length in 16-bit words. 
      contentLength = (22 + 2 * numParts) + (8 * len(f))

      # Store index for .shx file
      indexing.append( (ofs.tell() / 2, contentLength) )

      # Write the main file content header
      WriteMainFileContentHeader(ofs, i, contentLength)

      # Write a polyline record
      WritePolyLineRecord(ofs, f, mbrs[i])
      i+=1

   def write_shx():
      shxFile = open(projFileName + '.shx', 'wb')
      # shx file is 8 bytes per record + 100 byte header
      shxFileSize = 100 + 8 * len(indexing)
      WriteMainFileHeader(shxFile, shxFileSize / 2, mbr)
      for i in indexing:
         shxFile.write(struct.pack('>ii', i[0], i[1]))

   def write_dbf():
      try:
         from dbfpy import dbf
      except:
         print 'Skipping .dbf output.  Install dbfpy to output dbf file.'
         return
      db=dbf.Dbf(projFileName + '.dbf', new=True)
      #db.addField( ('NAME', 'C', 15) )
      for f in features:
         rec = db.newRecord()
         #rec['NAME'] = 'hi' + str(len(f))
         rec.store()
      db.close()   

   write_shx()
   write_dbf()
   print 'Done.'

# Reads the .prj file, and determines the projection of the data in the .shp file.
def GetProjection(aPrjFileName):
   text = open(aPrjFileName,'r').read()
   projType = text[:6]
   return projType

def ReadShapefile(filename):
   fnStart = filename.split('.')[0]
   proj = GetProjection(fnStart + '.prj')
   if proj != 'GEOGCS':
      print('Projection not supported: ', proj)
      return
   file = open(filename, 'rb')
   file.seek(100) # 100 byte header discarded
   polylines = []
   while 1:
      try:
         number, length = ReadMainFileContentHeader(file)
         polylines.append(ReadPolyLineRecord(file))
      except:
         break
   return polylines
