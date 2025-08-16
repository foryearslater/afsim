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
#include "WkScriptTypes.hpp"

#include <sstream>

#include "UtLLAPos.hpp"
#include "UtScriptClass.hpp"
#include "WsfGeoPoint.hpp"
#include "WsfPlatform.hpp"
#include "WsfTrack.hpp"

namespace warlock
{
namespace script
{
DataType DataTypeFromString(const QString& aTypeStr)
{
   static const std::map<QString, DataType> stringToType = {{"int", cINTEGER},
                                                            {"void", cVOID},
                                                            {"double", cDOUBLE},
                                                            {"string", cSTRING},
                                                            {"bool", cBOOL},
                                                            {"WsfGeoPoint", cWSF_GEOPOINT},
                                                            {"WsfPlatform", cWSF_PLATFORM},
                                                            {"WsfSensor", cWSF_SENSOR},
                                                            {"WsfWeapon", cWSF_WEAPON},
                                                            {"WsfComm", cWSF_COMM},
                                                            {"WsfTrack", cWSF_TRACK},
                                                            {"Array<int>", cARRAY_INTEGER},
                                                            {"Array<double>", cARRAY_DOUBLE},
                                                            {"Array<string>", cARRAY_STRING},
                                                            {"Array<bool>", cARRAY_BOOL},
                                                            {"Array<WsfGeoPoint>", cARRAY_WSF_GEOPOINT},
                                                            {"Array<WsfPlatform>", cARRAY_WSF_PLATFORM},
                                                            {"Array<WsfTrack>", cARRAY_WSF_TRACK},
                                                            {"Array<WsfSensor>", cARRAY_WSF_SENSOR},
                                                            {"Array<WsfWeapon>", cARRAY_WSF_WEAPON},
                                                            {"Array<WsfComm>", cARRAY_WSF_COMM}};

   auto it = stringToType.find(aTypeStr);
   if (it != stringToType.end())
   {
      return it->second;
   }
   return cUNKNOWN;
}

DataType BaseDataTypeFromString(const QString& aTypeStr)
{
   DataType type = DataTypeFromString(aTypeStr);
   if (IsArrayType(type))
   {
      return (DataType)(type & ~cARRAY_MASK);
   }
   return type;
}

QStringList UtScriptDataToString(const UtScriptData& aData)
{
   switch (aData.GetType())
   {
   case ut::script::Data::Type::cBOOL:
   {
      if (aData.GetBool())
      {
         return QStringList() << "true";
      }
      else
      {
         return QStringList() << "false";
      }
   }
   case ut::script::Data::Type::cDOUBLE:
      return QStringList() << QString::number(aData.GetDouble());
   case ut::script::Data::Type::cINT:
      return QStringList() << QString::number(aData.GetInt());
   case ut::script::Data::Type::cSTRING:
      return QStringList() << QString::fromStdString(aData.GetString());
   case ut::script::Data::Type::cPOINTER: // WsfGeoPoint, WsfPlatform, etc; and Arrays
   {
      if (aData.GetPointer()->GetScriptClass()->GetClassName() == "WsfGeoPoint")
      {
         WsfGeoPoint* geoPoint = static_cast<WsfGeoPoint*>(aData.GetPointer()->GetAppObject());

         std::stringstream lla;
         lla << UtLLAPos(geoPoint->GetLat(), geoPoint->GetLon(), geoPoint->GetAlt());

         return QStringList() << QString::fromStdString(lla.str());
      }
      else if (aData.GetPointer()->GetScriptClass()->GetClassName() == "WsfPlatform")
      {
         WsfPlatform* platformPtr = static_cast<WsfPlatform*>(aData.GetPointer()->GetAppObject());
         return QStringList() << QString::fromStdString(platformPtr->GetName());
      }
      else if (aData.GetPointer()->GetScriptClass()->GetClassName() == "WsfTrack")
      {
         WsfTrack* trackPtr = static_cast<WsfTrack*>(aData.GetPointer()->GetAppObject());

         std::stringstream track;
         track << trackPtr->GetTrackId();

         return QStringList() << QString::fromStdString(track.str());
      }
      else if (aData.GetPointer()->GetScriptClass()->GetClassName() == "Array<int>")
      {
         QStringList intList;

         UtScriptDataList* intValues = static_cast<UtScriptDataList*>(aData.GetPointer()->GetAppObject());

         for (const auto& i : *intValues)
         {
            intList << QString::number(i.GetInt());
         }
         return intList;
      }
      else if (aData.GetPointer()->GetScriptClass()->GetClassName() == "Array<double>")
      {
         QStringList doubleList;

         UtScriptDataList* doubleValues = static_cast<UtScriptDataList*>(aData.GetPointer()->GetAppObject());

         for (const auto& d : *doubleValues)
         {
            doubleList << QString::number(d.GetDouble());
         }
         return doubleList;
      }
      else if (aData.GetPointer()->GetScriptClass()->GetClassName() == "Array<bool>")
      {
         QStringList boolList;

         UtScriptDataList* boolValues = static_cast<UtScriptDataList*>(aData.GetPointer()->GetAppObject());

         for (const auto& b : *boolValues)
         {
            boolList << QString::number(b.GetBool());
         }
         return boolList;
      }
      else if (aData.GetPointer()->GetScriptClass()->GetClassName() == "Array<string>")
      {
         QStringList stringList;

         UtScriptDataList* stringValues = static_cast<UtScriptDataList*>(aData.GetPointer()->GetAppObject());

         for (const auto& s : *stringValues)
         {
            stringList << QString::fromStdString(s.GetString());
         }
         return stringList;
      }
      else if (aData.GetPointer()->GetScriptClass()->GetClassName() == "Array<WsfGeoPoint>")
      {
         QStringList geoPointList;

         UtScriptDataList* geoPoints = static_cast<UtScriptDataList*>(aData.GetPointer()->GetAppObject());

         for (const auto& g : *geoPoints)
         {
            WsfGeoPoint*      geoPoint = static_cast<WsfGeoPoint*>(g.GetPointer()->GetAppObject());
            std::stringstream lla;
            lla << UtLLAPos(geoPoint->GetLat(), geoPoint->GetLon(), geoPoint->GetAlt());

            geoPointList << QString::fromStdString(lla.str());
         }
         return geoPointList;
      }
      else if (aData.GetPointer()->GetScriptClass()->GetClassName() == "Array<WsfPlatform>")
      {
         QStringList platformList;

         UtScriptDataList* platforms = static_cast<UtScriptDataList*>(aData.GetPointer()->GetAppObject());

         for (const auto& p : *platforms)
         {
            WsfPlatform* platformPtr = static_cast<WsfPlatform*>(p.GetPointer()->GetAppObject());

            platformList << QString::fromStdString(platformPtr->GetName());
         }
         return platformList;
      }
      else if (aData.GetPointer()->GetScriptClass()->GetClassName() == "Array<WsfTrack>")
      {
         QStringList trackList;

         UtScriptDataList* tracks = static_cast<UtScriptDataList*>(aData.GetPointer()->GetAppObject());

         for (const auto& t : *tracks)
         {
            WsfTrack* trackPtr = static_cast<WsfTrack*>(t.GetPointer()->GetAppObject());

            std::stringstream track;
            track << trackPtr->GetTrackId();

            trackList << QString::fromStdString(track.str());
         }
         return trackList;
      }
   }
   default:
      return QStringList() << "<unknown-type>";
   }
}
} // namespace script
} // namespace warlock
