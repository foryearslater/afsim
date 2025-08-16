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

#ifndef TEXTSOURCECACHE_HPP
#define TEXTSOURCECACHE_HPP

#include <map>
#include <set>
#include <string>

#include "UtPath.hpp"
#include "Util.hpp"
#include "WsfParseSourceProvider.hpp"

class QsciDocument;
class UtTextDocument;
class WsfParseNode;

namespace wizard
{
class ProjectWorkspace;
class TextSource;

//! The source cache provides in-memory access to all files referenced in the project.
//! This improves speed, and allows us to react to changes without saving to the file.
class VI_EXPORT TextSourceCache
{
public:
   explicit TextSourceCache(wizard::ProjectWorkspace* aWorkspacePtr);
   ~TextSourceCache();
   TextSource* GetSource(const UtPath& aPath, bool aRead = true);
   TextSource* FindSource(const UtPath& aPath, bool aRead = true);
   TextSource* FindSource(const std::string& aPath, bool aRead = true);
   TextSource* FindSource(UtTextDocument* aParseSourcePtr);
   typedef std::map<std::string, TextSource*, wizard::UtilFileStringCompare> SourceMap;
   const SourceMap&                                                          GetSources() const { return mSources; }

   void                  DeleteSource(TextSource* aSourcePtr);
   void                  Clear();
   std::set<TextSource*> mNewSources;

protected:
   wizard::ProjectWorkspace* mWorkspacePtr;
   SourceMap                 mSources;
};

//! Provides sources to the WsfParser from the TextSourceCache instead of the files directly
class CacheSourceProvider : public WsfParseSourceProvider
{
public:
   explicit CacheSourceProvider(TextSourceCache* aCachePtr)
      : mAllowNewSources(true)
      , mCachePtr(aCachePtr)
   {
   }

   bool             mAllowNewSources;
   TextSourceCache* mCachePtr;
   UtTextDocument*  FindSource(const UtPath& aPath, bool aReadAccess) override;
   UtTextDocument*  CreateSource(const UtPath& aPath) override;
};
} // namespace wizard

#endif
