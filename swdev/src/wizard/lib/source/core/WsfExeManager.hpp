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

#ifndef WSFEXEMANAGER_HPP
#define WSFEXEMANAGER_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <QObject>
#include <QSet>

#include "UtCallback.hpp"
#include "UtCallbackHolder.hpp"
#include "ViExport.hpp"
#include "WsfParseDefinitions.hpp"
#include "WsfStyler.hpp"
#include "XML_Context.hpp"

class QComboBox;
class QSettings;
class UtPath;

//! convenience macro to access the WsfExeManager
#define wizExeMgr wizard::WsfExeManager::GetInstance()

namespace wizard
{
class WsfExe;

class VI_EXPORT WsfExeManager : public QObject
{
public:
   static WsfExeManager& GetInstance();

   WsfExeManager();
   ~WsfExeManager() override;

   void LoadSettings(QSettings& aSettings);
   void SaveSettings(QSettings& aSettings);

   WsfExe*                     Find(const UtPath& aPath) const;
   WsfExe*                     Add(const UtPath& aPath);
   void                        SetDefault(int aIndex);
   WsfExe*                     GetDefaultExe();
   WsfExe*                     GetNoExe();
   WsfStyler*                  GetStyler() { return mWsfStylerPtr.get(); }
   void                        Remove(WsfExe* aExePtr);
   void                        Refresh();
   bool                        Refresh(WsfExe* aExePtr);
   const std::vector<WsfExe*>& GetExes() const { return mExes; }
   WsfExe*                     FindByPath(const std::string& aPath);
   WsfExe*                     FindByDescriptiveName(const std::string& aName);
   WsfExe*                     FindByApplicationName(const std::string& aName);
   WsfExe* FindBest(const std::string& aApplicationName, const std::string& aVersion, const std::string& aPath);

   bool MoveDown(WsfExe* aExePtr);
   bool MoveUp(WsfExe* aExePtr);

   std::shared_ptr<WsfParseDefinitions> GetDefaultParseDefinitions();

   WsfExe* FindFromXML(XML_Context& aContext);

   int                            GetIndex(WsfExe* aExePtr);
   static UtCallbackListN<void()> WsfExeListUpdated;

   static void GetDefaultGrammar(WsfParseDefinitions& parseDefinitions, std::string& fullGrammarText);
protected slots:
   void WsfExePicked(int);

protected:
   bool IgnoreExecutable(const std::string& aFileName);
   void LoadDefaultGrammar();
   void Clear();

   std::vector<WsfExe*>       mExes;
   WsfExe*                    mNoExePtr;
   std::unique_ptr<WsfStyler> mWsfStylerPtr;
   int                        mDefaultExeIndex;
   UtCallbackHolder           mCallbacks;
   bool                       mAutoLoad;
   QSet<QString>              mFileNamesToIgnore;
};
} // namespace wizard
#endif
