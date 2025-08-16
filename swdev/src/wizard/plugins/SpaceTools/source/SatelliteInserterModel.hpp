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

#ifndef SATELLITEINSERTERMODEL_HPP
#define SATELLITEINSERTERMODEL_HPP

#include <map>
#include <vector>

#include <QAbstractTableModel>
#include <QJsonDocument>

namespace SpaceTools
{
//! The table model for the Satellite Inserter
class SatelliteInserterModel : public QAbstractTableModel
{
public:
   explicit SatelliteInserterModel(const std::map<QString, QJsonDocument>& aDatabases);
   ~SatelliteInserterModel() override = default;

   int      rowCount(const QModelIndex& aIndex) const override;
   int      columnCount(const QModelIndex& aIndex) const override;
   QVariant data(const QModelIndex& aIndex, int aRole) const override;
   QVariant headerData(int aSection, Qt::Orientation aOrientation, int aRole) const override;

   enum FieldColumn
   {
      cNAME                 = 0,
      cDESIGNATOR           = 1,
      cCOUNTRY              = 2,
      cPLATFORM_TYPE        = 3,
      cORBIT_TYPE           = 4,
      cCONSTELLATION        = 5,
      cFILE                 = 6,
      cNORAD_CATALOG_NUMBER = 7,
      cLAUNCH_DATE          = 8,
      cLAUNCH_SITE          = 9,
      cRADAR_CROSS_SECTION  = 10,
      cOPERATIONAL_STATUS   = 11,
      cDEFINITION_TYPE      = 12,
      cDATABASE             = 13,
      cLAST_COLUMN          = 14
   };

private:
   void SetExistingData(const std::multimap<QString, int>& aInsertedPlatforms, const QSet<QString>& aInsertedDesignators);

   //! The data that is displayed in the viewer.
   std::vector<std::map<int, QString>> mData;
};
} // namespace SpaceTools
#endif
