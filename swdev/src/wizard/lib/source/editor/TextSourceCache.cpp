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

#include "TextSourceCache.hpp"

#include <QPlainTextDocumentLayout>
#include <QTextBlock>

#include "ProjectWorkspace.hpp"
#include "TextSource.hpp"
#include "UtQt.hpp"

wizard::TextSourceCache::TextSourceCache(wizard::ProjectWorkspace* aProjectPtr)
   : mWorkspacePtr(aProjectPtr)
{
}

wizard::TextSourceCache::~TextSourceCache()
{
   Clear();
}

wizard::TextSource* wizard::TextSourceCache::GetSource(const UtPath& aPath, bool aRead)
{
   const std::string& pathStr(aPath.GetSystemPath());
   TextSource*        src      = FindSource(pathStr, false);
   bool               srcAdded = false;

   if (src == nullptr)
   {
      src      = new TextSource(mWorkspacePtr, aPath);
      srcAdded = true;
   }

   if (aRead)
   {
      src->SetFlags(src->GetFlags() | TextSource::cINCLUDED);
      if (!src->IsLoaded() && !src->Load())
      {
         delete src;
         src = nullptr;
      }
   }

   if (srcAdded && src)
   {
      mSources[pathStr] = src;
      mNewSources.insert(src);
   }

   return src;
}

wizard::TextSource* wizard::TextSourceCache::FindSource(const std::string& aPath, bool aRead)
{
   SourceMap::iterator iter = mSources.find(aPath);

   if (iter != mSources.end())
   {
      TextSource* sourcePtr = iter->second;
      if (aRead && !sourcePtr->IsLoaded())
      {
         sourcePtr->Load();
      }
      return sourcePtr;
   }

   return nullptr;
}

wizard::TextSource* wizard::TextSourceCache::FindSource(const UtPath& aPath, bool aRead)
{
   return FindSource(aPath.GetSystemPath(), aRead);
}

wizard::TextSource* wizard::TextSourceCache::FindSource(UtTextDocument* aParseSourcePtr)
{
   return static_cast<TextSource*>(aParseSourcePtr);
}

void wizard::TextSourceCache::DeleteSource(TextSource* aSourcePtr)
{
   mNewSources.erase(aSourcePtr);
   mSources.erase(aSourcePtr->GetSystemPath());
   delete aSourcePtr;
}

void wizard::TextSourceCache::Clear()
{
   mNewSources.clear();
   SourceMap sources = GetSources();
   mSources.clear();
   for (SourceMap::iterator i = sources.begin(); i != sources.end(); ++i)
   {
      delete i->second;
   }
}


UtTextDocument* wizard::CacheSourceProvider::FindSource(const UtPath& aPath, bool aRead)
{
   TextSource* src = mCachePtr->FindSource(aPath, aRead);
   if (src)
   {
      if (!src->IsDeleted())
      {
         return src->GetSource();
      }
      else
      {
         if (mAllowNewSources && aPath.Stat() == UtPath::cFILE)
         {
            src->ReadSource();
            if (!src->IsDeleted())
            {
               return src->GetSource();
            }
         }
         return nullptr;
      }
   }
   if (mAllowNewSources && aPath.Stat() == UtPath::cFILE)
   {
      TextSource* src = mCachePtr->GetSource(aPath, aRead);
      if (src)
      {
         return src->GetSource();
      }
   }
   return nullptr;
}

UtTextDocument* wizard::CacheSourceProvider::CreateSource(const UtPath& aPath)
{
   // assert(FindSource(aPath, false) == 0);
   return mCachePtr->GetSource(aPath);
}
