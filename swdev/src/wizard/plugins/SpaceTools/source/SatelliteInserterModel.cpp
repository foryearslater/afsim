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

#include "SatelliteInserterModel.hpp"

#include <map>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

#include "UtCast.hpp"
#include "UtPath.hpp"

SpaceTools::SatelliteInserterModel::SatelliteInserterModel(const std::map<QString, QJsonDocument>& aDatabases)
{
   mData.clear();
   std::multimap<QString, int> insertedPlatforms;
   QSet<QString>               insertedDesignators;
   for (const auto& database : aDatabases)
   {
      auto obj = database.second.object();
      auto arr = obj["platforms"].toArray();
      for (int i = 0; i < arr.size(); ++i)
      {
         std::map<int, QString> platform;
         auto                   plat = arr.at(i).toObject();

         for (int j = 0; j < cLAST_COLUMN; ++j)
         {
            auto header  = headerData(j, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
            header       = header.toLower().replace(' ', '_');
            QString data = plat[header].toString();
            if (j == cDATABASE)
            {
               data = database.first;
            }
            else if (j == cDESIGNATOR && !data.isEmpty())
            {
               insertedPlatforms.insert(std::pair<QString, int>(data, i));
               insertedDesignators.insert(data);
            }
            platform[j] = data;
         }
         mData.push_back(platform);
      }
   }
   SetExistingData(insertedPlatforms, insertedDesignators);
}

void SpaceTools::SatelliteInserterModel::SetExistingData(const std::multimap<QString, int>& aInsertedPlatforms,
                                                         const QSet<QString>&               aInsertedDesignators)
{
   for (const auto& designator : aInsertedDesignators)
   {
      if (aInsertedPlatforms.count(designator) > 1)
      {
         auto  range     = aInsertedPlatforms.equal_range(designator);
         auto& firstRow  = range.first;
         int   row1      = firstRow->second;
         auto  secondRow = ++firstRow;

         // First run through sets the first row with all information

         for (auto& nextIt = secondRow; nextIt != range.second; ++nextIt)
         {
            int row2 = nextIt->second;
            for (int j = 0; j < cLAST_COLUMN; ++j)
            {
               if (mData[row1].at(j).isEmpty() && !mData[row2].at(j).isEmpty())
               {
                  mData[row1].at(j) = mData[row2].at(j);
               }
            }
         }

         // Second run through fills in missing data for rows
         for (auto& nextIt = secondRow; nextIt != range.second; ++nextIt)
         {
            for (int j = 0; j < cLAST_COLUMN; ++j)
            {
               int row2 = nextIt->second;
               if (mData[row2].at(j).isEmpty() && !mData[row1].at(j).isEmpty())
               {
                  mData[row2].at(j) = mData[row1].at(j);
               }
            }
         }
      }
   }
}

//! The current row count of the data
//!
//! @param aIndex Not used
//! @returns The number of rows
int SpaceTools::SatelliteInserterModel::rowCount(const QModelIndex& aIndex) const
{
   return ut::cast_to_int(mData.size());
}


//! The current column count of the data
//!
//! @param aIndex Not used
//! @returns The number of columns
int SpaceTools::SatelliteInserterModel::columnCount(const QModelIndex& aIndex) const
{
   return cLAST_COLUMN;
}

//! Gets the header data for the model
//!
//! @param aSection The section of data desired
//! @param aOrientation The orientation of the header
//! @param aRole The role of the data
//! @returns The name of the header for the section or section number if name is undefined.
QVariant SpaceTools::SatelliteInserterModel::headerData(int aSection, Qt::Orientation aOrientation, int aRole) const
{
   QVariant header;
   if (aRole == Qt::DisplayRole)
   {
      if (aOrientation == Qt::Horizontal)
      {
         switch (aSection)
         {
         case FieldColumn::cCONSTELLATION:
            header = "Constellation";
            break;
         case FieldColumn::cCOUNTRY:
            header = "Country";
            break;
         case FieldColumn::cDESIGNATOR:
            header = "Designator";
            break;
         case FieldColumn::cNAME:
            header = "Name";
            break;
         case FieldColumn::cORBIT_TYPE:
            header = "Orbit Type";
            break;
         case FieldColumn::cDEFINITION_TYPE:
            header = "Definition Type";
            break;
         case FieldColumn::cPLATFORM_TYPE:
            header = "Platform Type";
            break;
         case FieldColumn::cFILE:
            header = "File";
            break;
         case FieldColumn::cDATABASE:
            header = "Database";
            break;
         case FieldColumn::cNORAD_CATALOG_NUMBER:
            header = "Norad Catalog Number";
            break;
         case FieldColumn::cLAUNCH_DATE:
            header = "Launch Date";
            break;
         case FieldColumn::cLAUNCH_SITE:
            header = "Launch Site";
            break;
         case FieldColumn::cRADAR_CROSS_SECTION:
            header = "Radar Cross Section";
            break;
         case FieldColumn::cOPERATIONAL_STATUS:
            header = "Operational Status";
            break;
         default:
            header = aSection + 1;
            break;
         }
      }
   }
   return header;
}

//! Gets the data from the model
//!
//! @param aIndex The index of the data to return
//! @param aRole The role of the data to return
//! @returns The data at the given row and column. If data doesn't exist or index is invalid, return empty QVariant
QVariant SpaceTools::SatelliteInserterModel::data(const QModelIndex& aIndex, int aRole) const
{
   QVariant data;
   if (aIndex.isValid() && static_cast<size_t>(aIndex.row()) < mData.size())
   {
      auto it = mData[aIndex.row()].find(aIndex.column());
      if (it != mData[aIndex.row()].end())
      {
         if (aRole == Qt::DisplayRole || aRole == Qt::UserRole)
         {
            data = it->second;
            if (aRole == Qt::DisplayRole && aIndex.column() == FieldColumn::cDATABASE)
            {
               UtPath file{data.toString().toStdString()};
               data = QString::fromStdString(file.GetFileName());
            }
         }
      }
   }
   return data;
}
