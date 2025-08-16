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


#include "ToolPrefObject.hpp"

#include <QFileInfo>
#include <QTextStream>

#include "UtStringUtil.hpp"
#include "UtTextDocument.hpp"
#include "Utml.hpp"

wizard::ToolPrefData::ToolPrefData()
{
   QFile file(":/external_tools_search.txt");
   file.open(QFile::ReadOnly | QFile::Text);
   QTextStream stream(&file);
   QString     text = stream.readAll();
   file.close();
   std::stringstream ss(text.toStdString());
   UtTextDocument    configFile(ss);
   UtmlParser        parser(configFile);
   UtmlObject        config = UtmlObject::makeContainer("root");
   parser.Parse(config);
#ifdef _WIN32
   std::string thisSystem = "windows";
#else
   std::string thisSystem = "linux";
#endif
   for (int i = 0; i < config.GetObjectCount(); ++i)
   {
#undef GetObject // Avoid conflict with Windows macro
      UtmlObject obj = config.GetObject(i);
      if (obj.GetType() == "Tool")
      {
         if (obj.PropertyValue("system", thisSystem) != thisSystem)
         {
            continue;
         }
         std::string              search = obj.PropertyValue("search_paths", "");
         std::vector<std::string> lines;
         UtStringUtil::Parse(search, lines, '\n');
         for (std::string line : lines)
         {
            UtStringUtil::TrimWhiteSpace(line);
            if (line.empty())
            {
               continue;
            }
            UtPath exePath(line);
            if (exePath.IsFile())
            {
               QString     alias     = obj.PropertyValue("id", "").c_str();
               QString     path      = exePath.GetSystemPath().c_str();
               QString     cmdline   = obj.PropertyValue("command_line", "").c_str();
               QString     working   = obj.PropertyValue("working_directory", "").c_str();
               QStringList fileTypes = QString::fromStdString(obj.PropertyValue("file_extensions", "")).split(',');
               mToolList.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(alias),
                                 std::forward_as_tuple(alias, path, cmdline, working, fileTypes));
               break;
            }
         }
      }
   }
}

wizard::ToolPrefObject::ToolPrefObject(QObject* parent /*= nullptr*/)
   : wkf::PrefObjectT<ToolPrefData>(parent, cNAME)
{
}

void wizard::ToolPrefObject::Apply()
{
   emit Changed();
}

wizard::ToolPrefData wizard::ToolPrefObject::ReadSettings(QSettings& aSettings) const
{
   ToolPrefData pData;
   if (!aSettings.contains("tools/size"))
   {
      pData.mToolList = mDefaultPrefs.mToolList;
   }
   else
   {
      int simCount = aSettings.beginReadArray("tools");
      for (int i = 0; i < simCount; ++i)
      {
         aSettings.setArrayIndex(i);
         QString     alias     = aSettings.value("alias").toString();
         QString     path      = aSettings.value("path").toString();
         QString     cmdLine   = aSettings.value("cmdLine").toString();
         QString     working   = aSettings.value("working").toString();
         QStringList fileTypes = aSettings.value("fileTypes").toStringList();

         if (!alias.isEmpty())
         {
            pData.mToolList[alias] = ToolDef(alias, path, cmdLine, working, fileTypes);
         }
      }
      aSettings.endArray();
   }
   return pData;
}

void wizard::ToolPrefObject::SaveSettingsP(QSettings& aSettings) const
{
   int i = 0;
   aSettings.beginWriteArray("tools");
   for (auto&& it : mCurrentPrefs.mToolList)
   {
      if (!it.second.mAlias.isEmpty())
      {
         aSettings.setArrayIndex(i++);
         aSettings.setValue("alias", it.second.mAlias);
         aSettings.setValue("path", it.second.mPath);
         aSettings.setValue("cmdLine", it.second.mCmdLine);
         aSettings.setValue("working", it.second.mWorking);
         aSettings.setValue("fileTypes", it.second.mExtensions);
      }
   }
   aSettings.endArray();
}

const wizard::ToolDef wizard::ToolPrefObject::FindTool(const QString& aName) const
{
   std::map<QString, ToolDef>::const_iterator it = mCurrentPrefs.mToolList.find(aName);
   if (it != mCurrentPrefs.mToolList.end())
   {
      return it->second;
   }
   else
   {
      return ToolDef();
   }
}

const std::map<QString, wizard::ToolDef>& wizard::ToolPrefObject::Tools() const
{
   return mCurrentPrefs.mToolList;
}
