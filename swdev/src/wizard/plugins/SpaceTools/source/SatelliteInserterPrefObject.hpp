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
#ifndef SATELLITEINSERTERPREFOBJECT_HPP
#define SATELLITEINSERTERPREFOBJECT_HPP

#include <map>
#include <vector>

#include <QJsonDocument>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "UtPath.hpp"
#include "WkfPrefObject.hpp"

namespace SpaceTools
{
struct PrefData
{
   PrefData();

   struct Filter
   {
      Filter(int aColumn, int aComp, const QVariant& aVar)
         : mColumn(aColumn)
         , mComp(aComp)
         , mVar(aVar)
      {
      }

      int      mColumn;
      int      mComp;
      QVariant mVar;
   };

   struct File
   {
      File()                 = default;
      File(const File& aSrc) = default;

      File(const QString& aPath, bool aChecked = true)
         : mPath(aPath)
         , mChecked(aChecked)
      {
      }

      QString mPath{""};
      bool    mChecked{true};
   };

   struct Database
   {
      Database() = default;
      Database(const QJsonDocument& aDatabase, const QString& aPath, bool aChecked = true)
         : mDatabase(aDatabase)
         , mFile(File(aPath, aChecked))
      {
      }

      QJsonDocument mDatabase;
      File          mFile;
   };

   std::map<QString, std::vector<Filter>> mSavedFilters;
   std::vector<Database>                  mLoadedDatabases;
   std::vector<File>                      mTLE_Sets;
};

class PrefObject : public wkf::PrefObjectT<PrefData>
{
   Q_OBJECT

public:
   explicit PrefObject(QObject* aParentPtr = nullptr);

   void SaveFilter(const QString& aName, const std::vector<PrefData::Filter>& aFilter);
   void SaveTLE_Sets(const std::vector<SpaceTools::PrefData::File>& aTLE_SetList);
   void DeleteFilter(const QString& aName);

   //! Get the saved filters from the current preferences
   const std::map<QString, std::vector<PrefData::Filter>>& GetSavedFilters() const
   {
      return mCurrentPrefs.mSavedFilters;
   }

   std::map<QString, QJsonDocument> GetCheckedDatabases();
   QStringList                      GetCheckedTLE_Sets();

signals:
   void UpdateTable();
   void SaveTLE_List();

private:
   void     Apply() override;
   PrefData ReadSettings(QSettings& aSettings) const override;
   void     SaveSettingsP(QSettings& aSettings) const override;
};
} // namespace SpaceTools

Q_DECLARE_METATYPE(SpaceTools::PrefData)
Q_DECLARE_METATYPE(SpaceTools::PrefData::Database)
Q_DECLARE_METATYPE(SpaceTools::PrefData::File)
#endif
