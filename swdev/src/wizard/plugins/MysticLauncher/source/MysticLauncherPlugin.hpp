// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef MYSTICLAUNCHERPLUGIN_HPP
#define MYSTICLAUNCHERPLUGIN_HPP

#include <QProcess>

#include "FileTypeCache.hpp"
#include "Plugin.hpp"

namespace MysticLauncher
{
class FileDetector : public wizard::FileTypeCache::FileDetector
{
public:
   const std::string cHEADER;
   FileDetector()
      : cHEADER("\0\0WSF_PIPE\n", 11)
   {
   }
   std::string DetectFileType(const QFileInfo& aFileInfo, const std::string& aFileHeader) override;
};
class Plugin : public wizard::Plugin
{
public:
   Plugin(const QString& aPluginName, const size_t aUniqueId);
   ~Plugin() override;

protected:
private:
   void ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu);
   void ProcessErrorCB(QProcess::ProcessError error);
   void Startup();

   FileDetector mDetector;
   QFileInfo    mFileInfo;
};
} // namespace MysticLauncher
#endif
