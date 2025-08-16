// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef WkScriptTypes_hpp
#define WkScriptTypes_hpp

#include "warlock_core_export.h"

#include <map>
#include <string>
#include <vector>

#include <QList>
#include <QVariant>

#include "UtScriptData.hpp"

namespace warlock
{
namespace script
{
// Context in which the script is valid
enum WARLOCK_CORE_EXPORT Context
{
   GLOBAL,
   PLATFORM
};

struct WARLOCK_CORE_EXPORT Data
{
   // Script name, if this is non-empty, then text should be empty
   std::string name;
   // Script text, if this is non-empty, then name should be empty.
   // The script must be of the form:
   //   void dostuff(int x)\n
   //      writeln("Hello", x); \n
   //   end_script
   std::string              text;
   std::vector<std::string> argNames;
   std::vector<std::string> argTypes;
   std::vector<int>         argSizes;
   std::string              returnType;
   Context                  context;
   bool                     clampPlatformSelection;
};
// Map of script name to Script Data
using DataMap = std::map<std::string, Data>;

enum
{
   cARRAY_MASK = 0x100
};

enum DataType
{
   cUNKNOWN,
   cVOID,
   cINTEGER,
   cDOUBLE,
   cSTRING,
   cBOOL,
   cWSF_PLATFORM,
   cWSF_SENSOR,
   cWSF_WEAPON,
   cWSF_COMM,
   cWSF_TRACK,
   cWSF_GEOPOINT,
   cARRAY_INTEGER      = cARRAY_MASK | cINTEGER,
   cARRAY_DOUBLE       = cARRAY_MASK | cDOUBLE,
   cARRAY_STRING       = cARRAY_MASK | cSTRING,
   cARRAY_BOOL         = cARRAY_MASK | cBOOL,
   cARRAY_WSF_PLATFORM = cARRAY_MASK | cWSF_PLATFORM,
   cARRAY_WSF_SENSOR   = cARRAY_MASK | cWSF_SENSOR,
   cARRAY_WSF_WEAPON   = cARRAY_MASK | cWSF_WEAPON,
   cARRAY_WSF_COMM     = cARRAY_MASK | cWSF_COMM,
   cARRAY_WSF_TRACK    = cARRAY_MASK | cWSF_TRACK,
   cARRAY_WSF_GEOPOINT = cARRAY_MASK | cWSF_GEOPOINT
};

struct WARLOCK_CORE_EXPORT Filter
{
   QString type{""};
   QString value{""};
   bool    isInclusive{true};
   bool    isFiltered{false};
};

struct WARLOCK_CORE_EXPORT Argument
{
   std::string type;
   QVariant    data;
};

struct WARLOCK_CORE_EXPORT ReturnValue
{
   std::string  platformName;
   std::string  scriptName;
   UtScriptData returnValue;
};

struct WARLOCK_CORE_EXPORT Instance
{
   std::string     platformName;
   Data            script;
   QList<Argument> args;
   QList<Filter>   filters;
   ReturnValue     returnValue;
};

WARLOCK_CORE_EXPORT QStringList UtScriptDataToString(const UtScriptData& aData);
WARLOCK_CORE_EXPORT DataType    DataTypeFromString(const QString& aTypeStr);
WARLOCK_CORE_EXPORT DataType    BaseDataTypeFromString(const QString& aTypeStr);

inline bool IsArrayType(DataType aType)
{
   return (aType & cARRAY_MASK) != 0;
};
inline bool IsArrayType(const std::string& aTypeStr)
{
   return IsArrayType(DataTypeFromString(QString::fromStdString(aTypeStr)));
}
inline bool IsValidType(const std::string& aTypeStr)
{
   return DataTypeFromString(QString::fromStdString(aTypeStr)) != cUNKNOWN;
}
inline bool IsVoidType(const std::string& aTypeStr)
{
   return DataTypeFromString(QString::fromStdString(aTypeStr)) == cVOID;
}
inline bool IsSupportedPlatformPartType(const std::string& aTypeStr)
{
   return (aTypeStr == "WsfSensor" || aTypeStr == "WsfWeapon" || aTypeStr == "WsfComm" ||
           aTypeStr == "Array<WsfSensor>" || aTypeStr == "Array<WsfWeapon>" || aTypeStr == "Array<WsfComm>");
}

inline QDataStream& operator<<(QDataStream& aStream, const Argument& aArg)
{
   aStream << QString::fromStdString(aArg.type);
   aStream << aArg.data;
   return aStream;
}

inline QDataStream& operator>>(QDataStream& aStream, Argument& aArg)
{
   QString temp;
   aStream >> temp >> aArg.data;
   aArg.type = temp.toStdString();
   return aStream;
}

inline QDataStream& operator<<(QDataStream& aStream, const Filter& aArg)
{
   aStream << aArg.type;
   aStream << aArg.value;
   aStream << aArg.isFiltered;
   aStream << aArg.isInclusive;
   return aStream;
}

inline QDataStream& operator>>(QDataStream& aStream, Filter& aArg)
{
   QString type;
   QString value;

   aStream >> aArg.type >> aArg.value >> aArg.isFiltered >> aArg.isInclusive;

   return aStream;
}
} // namespace script
} // namespace warlock

Q_DECLARE_METATYPE(warlock::script::ReturnValue);
Q_DECLARE_METATYPE(warlock::script::Filter);

#endif
