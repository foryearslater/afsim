// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#include "SatelliteInserterPrefObject.hpp"

#include "SpaceToolsUtil.hpp"
#include "UtPath.hpp"
#include "WkfEnvironment.hpp"

namespace // {anonymous}
{
std::vector<QJsonDocument> GetDefaultDatabases(const UtPath& aBasePath, std::vector<QString>& aPath)
{
   std::vector<QJsonDocument> retval{};
   if (aBasePath.IsDirectory())
   {
      std::vector<QString> defaultDatabases{"/satellite.json", "/satcat.json"};
      for (const auto& path : defaultDatabases)
      {
         aPath.push_back(QString::fromStdString(aBasePath.GetNormalizedPath()) + path);
         retval.push_back(SpaceTools::Util::GetDatabase(aPath.back()));
      }
   }
   return retval;
}
} // namespace

SpaceTools::PrefData::PrefData()
{
   UtPath                     basePath(wkfEnv.GetDemosDir().toStdString() + "/satellite_demos");
   std::vector<QString>       databasePath;
   std::vector<QJsonDocument> databases = GetDefaultDatabases(basePath, databasePath);
   for (size_t i = 0; i < databases.size(); ++i)
   {
      if (!databases[i].isNull())
      {
         mLoadedDatabases.emplace_back(databases[i], databasePath[i]);
      }
   }
}

SpaceTools::PrefObject::PrefObject(QObject* aParentPtr)
   : wkf::PrefObjectT<PrefData>(aParentPtr, "SatelliteInserter")
{
}

//! Saves the current filter
//!
//! @param aName The name of the filter being saved
//! @param aFilter The filter being saved
void SpaceTools::PrefObject::SaveFilter(const QString& aName, const std::vector<PrefData::Filter>& aFilter)
{
   mCurrentPrefs.mSavedFilters[aName] = aFilter;
}

//! Saves the current set of TLE files
//!
//! @param aTLE_SetList The list of TLE files being saved
void SpaceTools::PrefObject::SaveTLE_Sets(const std::vector<SpaceTools::PrefData::File>& aTLE_SetList)
{
   mCurrentPrefs.mTLE_Sets = aTLE_SetList;
}

//! Deletes the filter with the given name
//!
//! @param aName The name of the filter to be deleted
void SpaceTools::PrefObject::DeleteFilter(const QString& aName)
{
   mCurrentPrefs.mSavedFilters.erase(aName);
}

//! Get the checked databases from the current loaded databases
//!
//! @returns The checked databases from the current loaded databases
std::map<QString, QJsonDocument> SpaceTools::PrefObject::GetCheckedDatabases()
{
   std::map<QString, QJsonDocument> checkedDatabases;
   for (const auto& database : mCurrentPrefs.mLoadedDatabases)
   {
      if (database.mFile.mChecked)
      {
         UtPath path(database.mFile.mPath.toStdString());
         auto   file            = QString::fromStdString(path.GetNormalizedPath());
         checkedDatabases[file] = database.mDatabase;
      }
   }
   return checkedDatabases;
}

//! Get the checked TLE files from the loaded TLE files
//!
//! @returns The checked TLE files from the loaded files
QStringList SpaceTools::PrefObject::GetCheckedTLE_Sets()
{
   QStringList checkedSets;
   for (const auto& set : mCurrentPrefs.mTLE_Sets)
   {
      if (set.mChecked)
      {
         checkedSets << set.mPath;
      }
   }
   return checkedSets;
}

void SpaceTools::PrefObject::Apply()
{
   emit UpdateTable();
}

SpaceTools::PrefData SpaceTools::PrefObject::ReadSettings(QSettings& aSettings) const
{
   PrefData pData;
   if (aSettings.contains("filters/size"))
   {
      pData.mSavedFilters.clear();
      int filterCount = aSettings.beginReadArray("filters");
      for (int i = 0; i < filterCount; ++i)
      {
         aSettings.setArrayIndex(i);
         QString fname     = aSettings.value("name").toString();
         int     ruleCount = aSettings.beginReadArray("rules");
         for (int j = 0; j < ruleCount; ++j)
         {
            aSettings.setArrayIndex(j);
            int      col   = aSettings.value("column").toInt();
            int      type  = aSettings.value("type").toInt();
            QVariant value = aSettings.value("value");
            pData.mSavedFilters[fname].emplace_back(col, type, value);
         }
         aSettings.endArray();
      }
      aSettings.endArray();
   }
   else
   {
      pData.mSavedFilters = mDefaultPrefs.mSavedFilters;
   }

   if (aSettings.contains("loaded_databases/size"))
   {
      pData.mLoadedDatabases.clear();
      int loadedDatabaseCount = aSettings.beginReadArray("loaded_databases");
      for (int i = 0; i < loadedDatabaseCount; ++i)
      {
         aSettings.setArrayIndex(i);
         QString            path    = aSettings.value("path").toString();
         bool               checked = aSettings.value("checked").toBool();
         PrefData::Database database(Util::GetDatabase(path), path, checked);
         pData.mLoadedDatabases.emplace_back(database);
      }
      aSettings.endArray();
   }
   else
   {
      pData.mLoadedDatabases = mDefaultPrefs.mLoadedDatabases;
   }

   if (aSettings.contains("tle_sets/size"))
   {
      pData.mTLE_Sets.clear();
      int tleSetCount = aSettings.beginReadArray("tle_sets");
      for (int i = 0; i < tleSetCount; ++i)
      {
         aSettings.setArrayIndex(i);
         QString        path    = aSettings.value("path").toString();
         bool           checked = aSettings.value("checked").toBool();
         PrefData::File file(path, checked);
         pData.mTLE_Sets.emplace_back(file);
      }
      aSettings.endArray();
   }
   else
   {
      pData.mTLE_Sets = mDefaultPrefs.mTLE_Sets;
   }
   return pData;
}

void SpaceTools::PrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.beginWriteArray("filters", static_cast<int>(mCurrentPrefs.mSavedFilters.size()));
   int i = 0;
   for (auto&& it : mCurrentPrefs.mSavedFilters)
   {
      aSettings.setArrayIndex(i);
      aSettings.setValue("name", it.first);
      aSettings.beginWriteArray("rules", static_cast<int>(it.second.size()));
      int j = 0;
      for (auto&& jt : it.second)
      {
         aSettings.setArrayIndex(j);
         aSettings.setValue("column", jt.mColumn);
         aSettings.setValue("type", jt.mComp);
         aSettings.setValue("value", jt.mVar);
         ++j;
      }
      aSettings.endArray();
      ++i;
   }
   aSettings.endArray();

   aSettings.beginWriteArray("loaded_databases", static_cast<int>(mCurrentPrefs.mLoadedDatabases.size()));
   i = 0;
   for (auto&& it : mCurrentPrefs.mLoadedDatabases)
   {
      aSettings.setArrayIndex(i);
      aSettings.setValue("path", it.mFile.mPath);
      aSettings.setValue("checked", it.mFile.mChecked);
      ++i;
   }
   aSettings.endArray();

   aSettings.beginWriteArray("tle_sets", static_cast<int>(mCurrentPrefs.mTLE_Sets.size()));
   i = 0;
   for (auto&& it : mCurrentPrefs.mTLE_Sets)
   {
      aSettings.setArrayIndex(i);
      aSettings.setValue("path", it.mPath);
      aSettings.setValue("checked", it.mChecked);
      ++i;
   }
   aSettings.endArray();
}
