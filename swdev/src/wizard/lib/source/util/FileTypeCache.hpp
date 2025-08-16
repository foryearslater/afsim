// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef FILETYPECACHE_HPP
#define FILETYPECACHE_HPP

#include <map>
#include <string>
#include <vector>

#include <QDateTime>
#include <QString>

#include "ViExport.hpp"

class QFileInfo;

namespace wizard
{
// Determines file types and caches the information
class VI_EXPORT FileTypeCache
{
public:
   // Provides pluggable file detection
   class VI_EXPORT FileDetector
   {
   public:
      virtual ~FileDetector() {}
      virtual std::string DetectFileType(const QFileInfo& aFileInfo, const std::string& aFileHeader) = 0;
   };

   std::string        GetFileType(const QFileInfo& aFileInfo);
   static std::string DetermineFileType(const QFileInfo& aFileInfo);
   static void        RegisterDetector(FileDetector* aDetector);
   static void        UnregisterDetector(FileDetector* aDetector);
   static void        SetFileTypeIcon(const std::string& aFileType, const std::string& aFileIcon);
   static std::string GetFileTypeIcon(const std::string& aFileType);

private:
   struct FileInfo
   {
      QDateTime   mLastModified;
      long long   mFileSize;
      std::string mFileType;
   };

   typedef std::map<QString, FileInfo>       CacheMap;
   CacheMap                                  mCache;
   static std::vector<FileDetector*>         mDetectors;
   static std::map<std::string, std::string> mFileTypeIcons;
};
} // namespace wizard
#endif
