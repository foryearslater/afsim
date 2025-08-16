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

#ifndef OSMCONVERTERDATACONTAINER_HPP
#define OSMCONVERTERDATACONTAINER_HPP

#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <QHash>
#include <QString>

namespace std
{

template<>
struct hash<QString>
{
   std::size_t operator()(const QString& aString) const { return qHash(aString); }
};

} // namespace std

class OSMConverterDataContainer
{
public:
   enum class ValueType
   {
      cBOOL    = 0,
      cINT     = 1,
      cDOUBLE  = 2,
      cSTRING  = 3,
      cUNITARY = 4
   };

   struct FilterData
   {
      QString   mTagKey;
      QString   mValue;
      QString   mOutputName;
      ValueType mValueType;
   };

   struct ResultOutput
   {
      size_t                    mRoutesCreated;
      std::map<QString, size_t> mWayTagMatches;
      std::map<QString, size_t> mNodeTagMatches;
   };

   struct OSMWayData
   {
      std::vector<unsigned long long>      mNodeRefIdList; // Reference Id
      std::unordered_map<QString, QString> mData;
   };

   OSMConverterDataContainer() = default;
   QString ParseFile(const QString& aFileName);
   QString GetRouteNetwork(const std::vector<FilterData>& aNodeTagFilter,
                           const std::vector<FilterData>& aWayTagFilter,
                           const QString&                 aNetworkName,
                           QString&                       aErrorMessage,
                           ResultOutput&                  aResultOutput);
   size_t  GetNumberOfRoutes() const { return mWayMap.size(); }

private:
   // OSM Id for nodes are stored as a 64 bit integer number >= 1. The wiki doesn't say anything about way id, so assume the same.
   std::unordered_map<unsigned long long, std::unordered_map<QString, QString>> mNodeMap; // Key: Node Id, Value: data
                                                                                          // (lat, lon, aux, etc.)
   std::unordered_map<unsigned long long, OSMWayData> mWayMap; // Key: Way Id,  Value: Node list and data
   std::unordered_set<unsigned long long>             mIntersections;
};

#endif
