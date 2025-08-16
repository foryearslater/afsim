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

#include "OSMConverterDataContainer.hpp"

#include <QString>

#include "tinyxml2.h"
#include "ui_OSMConverterDialog.h"

namespace
{

const std::unordered_map<int, QString> valueStrings{
   {static_cast<int>(OSMConverterDataContainer::ValueType::cBOOL), "bool "},
   {static_cast<int>(OSMConverterDataContainer::ValueType::cINT), "int "},
   {static_cast<int>(OSMConverterDataContainer::ValueType::cDOUBLE), "double "},
   {static_cast<int>(OSMConverterDataContainer::ValueType::cSTRING), "string "},
   {static_cast<int>(OSMConverterDataContainer::ValueType::cUNITARY), "unitary "}};

}

QString OSMConverterDataContainer::ParseFile(const QString& aFileName)
{
   mNodeMap.clear();
   mWayMap.clear();
   tinyxml2::XMLDocument osmDocument;
   if (osmDocument.LoadFile(qPrintable(aFileName)) == 0)
   {
      tinyxml2::XMLElement* root = osmDocument.FirstChildElement("osm");
      if (root)
      {
         // Iterate through all nodes
         for (tinyxml2::XMLElement* node = root->FirstChildElement("node"); node != nullptr;
              node                       = node->NextSiblingElement("node"))
         {
            // Get the Node id attribute, QString already seems to perform safety checks, no need for separate cases
            QString nodeID = QString::fromUtf8(node->Attribute("id"));
            if (!nodeID.isEmpty())
            {
               unsigned long long mapKey = nodeID.toULongLong();
               if (mNodeMap.find(mapKey) != mNodeMap.end())
               {
                  return ("Please make sure there are no duplicate node ids. The id " + QString::number(mapKey) +
                          " may have already been used.");
               }

               std::unordered_map<QString, QString> mapValues;
               // store all attributes, valid xml will always have a legal attribute name, but the value can be null or empty
               for (const tinyxml2::XMLAttribute* attribute = node->FirstAttribute(); attribute != nullptr;
                    attribute                               = attribute->Next())
               {
                  mapValues.emplace(QString::fromUtf8(attribute->Name()), QString::fromUtf8(attribute->Value()));
               }

               // store all tags that have been defined (auxiliary data): tags should only contain a key and value
               // attribute (k, v)
               for (tinyxml2::XMLElement* childData = node->FirstChildElement("tag"); childData != nullptr;
                    childData                       = childData->NextSiblingElement("tag"))
               {
                  const char* keyName   = childData->Attribute("k");
                  const char* valueName = childData->Attribute("v");
                  if (keyName && valueName)
                  {
                     if (mapValues.find(QString::fromUtf8(keyName)) != mapValues.end())
                     {
                        return ("Please make sure there are no duplicate attributes or tag keys. The key " +
                                QString::fromUtf8(keyName) + " may have already been used in the node with id " +
                                QString::fromUtf8(node->FindAttribute("id")->Name()));
                     }
                     mapValues.emplace(QString::fromUtf8(keyName), QString::fromUtf8(valueName));
                  }
                  else
                  {
                     return ("Please make sure that all tags have a valid k/v (key/value) attribute pair in the way "
                             "with id " +
                             QString::fromUtf8(node->FindAttribute("id")->Name()));
                  }
               }
               mNodeMap.emplace(mapKey, mapValues);
            }
            else
            {
               return ("Please make sure that all nodes have an \"id\" attribute.");
            }
         }

         // Iterate through all ways
         std::unordered_set<unsigned long long> nodesUsed; // used to track duplicates
         for (tinyxml2::XMLElement* way = root->FirstChildElement("way"); way != nullptr;
              way                       = way->NextSiblingElement("way"))
         {
            // Get the Way id attribute
            QString wayID = QString::fromUtf8(way->Attribute("id"));
            if (!wayID.isEmpty())
            {
               unsigned long long mapKey = wayID.toULongLong();
               if (mWayMap.find(mapKey) != mWayMap.end())
               {
                  return ("Please make sure there are no duplicate way ids. The id " + QString::number(mapKey) +
                          " may have already been used.");
               }

               OSMWayData wayData;
               // Find all way attributes
               for (const tinyxml2::XMLAttribute* attribute = way->FirstAttribute(); attribute != nullptr;
                    attribute                               = attribute->Next())
               {
                  wayData.mData.emplace(QString::fromUtf8(attribute->Name()), QString::fromUtf8(attribute->Value()));
               }

               for (tinyxml2::XMLElement* childNode = way->FirstChildElement("nd"); childNode != nullptr;
                    childNode                       = childNode->NextSiblingElement("nd"))
               {
                  QString referenceID = QString::fromUtf8(childNode->Attribute("ref"));
                  if (!referenceID.isEmpty())
                  {
                     if (nodesUsed.find(referenceID.toULongLong()) == nodesUsed.end())
                     {
                        nodesUsed.emplace(referenceID.toULongLong());
                     }
                     else
                     {
                        mIntersections.emplace(referenceID.toULongLong());
                     }
                     wayData.mNodeRefIdList.emplace_back(referenceID.toULongLong());
                  }
               }

               for (tinyxml2::XMLElement* childData = way->FirstChildElement("tag"); childData != nullptr;
                    childData                       = childData->NextSiblingElement("tag"))
               {
                  const char* keyName   = childData->Attribute("k");
                  const char* valueName = childData->Attribute("v");
                  if (keyName && valueName)
                  {
                     if (wayData.mData.find(QString::fromUtf8(keyName)) != wayData.mData.end())
                     {
                        return ("Please make sure there are no duplicate attributes or tag keys. The key " +
                                QString::fromUtf8(keyName) + " may have already been used in the way with id " +
                                QString::fromUtf8(way->FindAttribute("id")->Name()));
                     }
                     wayData.mData.emplace(QString::fromUtf8(keyName), QString::fromUtf8(valueName));
                  }
                  else
                  {
                     return ("Please make sure that all tags have a valid k/v (key/value) attribute pair in the way "
                             "with id " +
                             QString::fromUtf8(way->FindAttribute("id")->Name()));
                  }
               }
               mWayMap.emplace(mapKey, wayData);
            }
            else
            {
               return ("Please make sure that all ways have an \"id\" attribute.");
            }
         }
      }
   }
   else
   {
      return "Could not read the provided file. Please the make sure that the file is valid osm/xml.";
   }
   return "";
}

QString OSMConverterDataContainer::GetRouteNetwork(const std::vector<FilterData>& aNodeTagFilter,
                                                   const std::vector<FilterData>& aWayTagFilter,
                                                   const QString&                 aNetworkName,
                                                   QString&                       aErrorMessage,
                                                   ResultOutput&                  aResultOutput)
{
   aErrorMessage = "";

   for (auto& it : aNodeTagFilter)
   {
      aResultOutput.mNodeTagMatches.emplace(it.mTagKey, 0);
   }

   for (auto& it : aWayTagFilter)
   {
      aResultOutput.mWayTagMatches.emplace(it.mTagKey, 0);
   }

   // QString addition operator requires non-const QStrings
   QString cTAB1 = "   ";
   QString cTAB2 = "      ";
   QString cTAB3 = "         ";
   QString cTAB4 = "            ";
   QString cTAB5 = "               ";

   if (aNetworkName.isEmpty())
   {
      aErrorMessage = "Please provide a name for the route_network.";
      return "";
   }

   QString routeNetwork = "route_network " + aNetworkName + "\n";
   for (auto& wayIt : mWayMap)
   {
      routeNetwork += (cTAB1 + "route\n");
      auto nameData = wayIt.second.mData.find("name");
      if (nameData != wayIt.second.mData.end())
      {
         routeNetwork += (cTAB2 + "name " + nameData->second.simplified().replace(" ", "_") + "\n");
      }

      routeNetwork += (cTAB2 + "navigation\n");
      for (auto& nodeListIt : wayIt.second.mNodeRefIdList)
      {
         auto singleNodeData = mNodeMap.find(nodeListIt);
         if (singleNodeData != mNodeMap.end())
         {
            // Handle Node Position
            auto latitudeIt  = singleNodeData->second.find("lat");
            auto longitudeIt = singleNodeData->second.find("lon");
            if (latitudeIt != singleNodeData->second.end() && longitudeIt != singleNodeData->second.end())
            {
               QString latDirection  = (latitudeIt->second.toDouble() >= 0) ? "n" : "s";
               QString longDirection = (longitudeIt->second.toDouble() >= 0) ? "e" : "w";
               QString latitude      = latitudeIt->second + latDirection;
               QString longitude     = longitudeIt->second + longDirection;
               latitude.remove("-");
               longitude.remove("-");
               routeNetwork += (cTAB3 + "position " + latitude + " " + longitude + "\n");
            }
            else // ERROR: Missing position data
            {
               aErrorMessage = "Could not create route_network. There is missing position data for the node with id " +
                               QString::number(singleNodeData->first);
               return "";
            }

            // Handle Auxiliary Node Data
            QString nodeAuxData = "";
            for (auto& nodeData : singleNodeData->second)
            {
               auto nodeFilterIt =
                  find_if(aNodeTagFilter.begin(),
                          aNodeTagFilter.end(),
                          [nodeData](const FilterData& aNodeTagData)
                          {
                             if (!aNodeTagData.mValue.isEmpty() && !nodeData.second.isEmpty())
                             {
                                return aNodeTagData.mTagKey == nodeData.first && aNodeTagData.mValue == nodeData.second;
                             }
                             return aNodeTagData.mTagKey == nodeData.first && aNodeTagData.mValue.isEmpty();
                          });
               if (nodeFilterIt != aNodeTagFilter.end())
               {
                  auto valueTypeIt = valueStrings.find(static_cast<int>(nodeFilterIt->mValueType));
                  if (valueTypeIt == valueStrings.end())
                  {
                     continue;
                  }

                  QString value = (valueTypeIt->first != static_cast<int>(ValueType::cSTRING)) ?
                                     nodeData.second :
                                     ("\"" + nodeData.second + "\"");
                  if (valueTypeIt->first == static_cast<int>(ValueType::cBOOL))
                  {
                     value = "false";
                     if (!nodeFilterIt->mValue.isEmpty() || (nodeData.second.compare("yes") == 0))
                     {
                        value = "true";
                     }
                  }
                  nodeAuxData += ("\n" + cTAB5 + valueTypeIt->second + nodeFilterIt->mOutputName + " = " + value);
                  aResultOutput.mNodeTagMatches[nodeFilterIt->mTagKey + (nodeFilterIt->mValue.isEmpty() ? "" : ":") +
                                                nodeFilterIt->mValue] += 1;
               }
            }

            // Insert the aux_data block into the waypoint
            if (!nodeAuxData.isEmpty())
            {
               routeNetwork += (cTAB4 + "aux_data" + nodeAuxData + "\n" + cTAB4 + "end_aux_data\n");
            }

            // Deal with intersections
            if (mIntersections.find(singleNodeData->first) != mIntersections.end())
            {
               routeNetwork += (cTAB4 + "node_id " + QString::number(singleNodeData->first) + "\n");
            }
         }
         else // ERROR: missing node definition
         {
            aErrorMessage =
               "Could not create route_network. Could not locate the node with id " + QString::number(nodeListIt);
            return "";
         }
      }
      routeNetwork += (cTAB2 + "end_navigation\n");

      // Handle Auxiliary waypoint/route Data
      QString wayAuxData = "";
      for (const auto& wayData : wayIt.second.mData)
      {
         auto wayFilterIt =
            find_if(aWayTagFilter.begin(),
                    aWayTagFilter.end(),
                    [wayData](const FilterData& aWayTagData)
                    {
                       if (!aWayTagData.mValue.isEmpty() && !wayData.second.isEmpty())
                       {
                          return aWayTagData.mTagKey == wayData.first && aWayTagData.mValue == wayData.second;
                       }
                       return aWayTagData.mTagKey == wayData.first && aWayTagData.mValue.isEmpty();
                    });
         if (wayFilterIt != aWayTagFilter.end())
         {
            auto valueTypeIt = valueStrings.find(static_cast<int>(wayFilterIt->mValueType));
            if (valueTypeIt == valueStrings.end())
            {
               continue;
            }


            QString value = (valueTypeIt->first != static_cast<int>(ValueType::cSTRING)) ? wayData.second :
                                                                                           ("\"" + wayData.second + "\"");
            if (valueTypeIt->first == static_cast<int>(ValueType::cBOOL))
            {
               value = "false";
               if (!wayFilterIt->mValue.isEmpty() || (wayData.second.compare("yes") == 0))
               {
                  value = "true";
               }
            }
            wayAuxData += ("\n" + cTAB3 + valueTypeIt->second + wayFilterIt->mOutputName + " = " + value);
            aResultOutput.mWayTagMatches[wayFilterIt->mTagKey + (wayFilterIt->mValue.isEmpty() ? "" : ":") + wayFilterIt->mValue] +=
               1;
         }
      }

      // Insert the aux_data block into the route
      if (!wayAuxData.isEmpty())
      {
         routeNetwork += (cTAB2 + "aux_data" + wayAuxData + "\n" + cTAB2 + "end_aux_data\n");
      }

      routeNetwork += (cTAB1 + "end_route\n");
   }
   aResultOutput.mRoutesCreated = mWayMap.size();
   routeNetwork += "end_route_network\n";

   return routeNetwork;
}
