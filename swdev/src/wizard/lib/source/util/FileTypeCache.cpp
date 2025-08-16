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

#include "FileTypeCache.hpp"

#include <QFile>
#include <QFileInfo>
#include <QRegExp>

std::vector<wizard::FileTypeCache::FileDetector*> wizard::FileTypeCache::mDetectors;
std::map<std::string, std::string>                wizard::FileTypeCache::mFileTypeIcons;

std::string wizard::FileTypeCache::GetFileType(const QFileInfo& aFileInfo)
{
   CacheMap::const_iterator i = mCache.find(aFileInfo.absoluteFilePath());
   if (i != mCache.end())
   {
      const FileInfo& info = i->second;
      if (info.mFileSize == aFileInfo.size() && info.mLastModified == aFileInfo.lastModified())
      {
         return info.mFileType;
      }
   }

   FileInfo& info     = mCache[aFileInfo.absoluteFilePath()];
   info.mLastModified = aFileInfo.lastModified();
   info.mFileSize     = (long long)aFileInfo.size();
   info.mFileType     = DetermineFileType(aFileInfo);
   return info.mFileType;
}

std::string wizard::FileTypeCache::DetermineFileType(const QFileInfo& aFileInfo)
{
   std::string fileType;
   QFile       file(aFileInfo.absoluteFilePath());
   if (file.open(QFile::ReadOnly))
   {
      const int         readLen = 100; // Needs to be >= the size of the largest header.
      const QByteArray& header  = file.read(readLen);

      std::string hdr(header.begin(), header.end());
      for (size_t i = 0; i < mDetectors.size() && fileType.empty(); ++i)
      {
         fileType = mDetectors[i]->DetectFileType(aFileInfo, hdr);
      }
      if (!fileType.empty())
      {
         return fileType;
      }
   }

   return fileType;
}

void wizard::FileTypeCache::UnregisterDetector(FileDetector* aDetector)
{
   mDetectors.erase(std::find(mDetectors.begin(), mDetectors.end(), aDetector));
}

void wizard::FileTypeCache::RegisterDetector(FileDetector* aDetector)
{
   mDetectors.push_back(aDetector);
}

std::string wizard::FileTypeCache::GetFileTypeIcon(const std::string& aFileType)
{
   std::string                                  icon;
   std::map<std::string, std::string>::iterator i = mFileTypeIcons.find(aFileType);
   if (i != mFileTypeIcons.end())
   {
      icon = i->second;
   }
   return icon;
}

// Create a mapping from the string returned by DetermineFileType/GetFileType
// and the icon used to represent that file type
void wizard::FileTypeCache::SetFileTypeIcon(const std::string& aFileType, const std::string& aFileIcon)
{
   mFileTypeIcons[aFileType] = aFileIcon;
}
