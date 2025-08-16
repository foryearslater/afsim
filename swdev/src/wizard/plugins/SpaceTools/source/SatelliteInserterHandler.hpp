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

#ifndef SATELLITEINSERTERHANDLER_HPP
#define SATELLITEINSERTERHANDLER_HPP

#include <map>
#include <tuple>
#include <vector>

#include <QAbstractItemModel>
#include <QSet>
#include <QString>

#include "UtPath.hpp"

namespace SpaceTools
{
class SatelliteInserterHandler
{
public:
   SatelliteInserterHandler()  = default;
   ~SatelliteInserterHandler() = default;

   //! Gets the path of the current insert file
   const QString& GetInsertFile() const { return mInsertFile; }
   void           SetInsertFile(const QString& aFile);
   void           InsertSatellites(const QAbstractItemModel* aModelPtr, const QModelIndexList& aIndexList);

private:
   static std::map<QString, std::pair<UtPath, std::vector<QString>>> GetInsertPlatforms(const QAbstractItemModel* aModelPtr,
                                                                                        const QModelIndexList& aIndexList);
   static QString                                                    FindDefinitions(const QString&        aFile,
                                                                                     const UtPath&         aBasePath,
                                                                                     std::vector<QString>& aPlatforms,
                                                                                     QSet<QString>&        aInvalidFilesList);

   static const QSet<QString> GetDesignators();

   QString mInsertFile;
};
} // namespace SpaceTools
#endif
