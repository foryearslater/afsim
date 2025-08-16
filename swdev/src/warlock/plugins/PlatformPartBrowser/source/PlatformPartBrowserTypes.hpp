// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016-2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef PLATFORMPARTBROWSERTYPES_HPP
#define PLATFORMPARTBROWSERTYPES_HPP

#include <map>
#include <string>

#include <QTreeWidgetItem>
#include <QVariant>

#include "UtTypeTraits.hpp"

class WsfPlatformPart;

namespace WkPlatformPartBrowser
{
// PartType is used for determining the icon to use for the platform part.
// These types are arranged so that the more specific types are after the more general types.
// For example, ePLATFORMPART < eARTICULATEDPART < eSENSOR < eRADAR
// This is so that the icon reflects the most specific type possible
enum PartType
{
   ePLATFORMPART,
   eARTICULATEDPART,
   eMOVER_LAND,
   eMOVER_AIR,
   eMOVER_SURFACE,
   eMOVER_SUBSURFACE,
   eMOVER_SPACE,
   ePROCESSOR,
   eFUEL,
   eSENSOR,
   eCOMM,
   eWEAPON,
   eWEAPON_EXPLICIT,
   eRADAR
};

static const std::map<PartType, QString> sPartTypeToIcon = {{ePLATFORMPART, "box_house"},
                                                            {eARTICULATEDPART, "box_house"},
                                                            {eMOVER_LAND, "tank"},
                                                            {eMOVER_AIR, "airplane"},
                                                            {eMOVER_SURFACE, "boat"},
                                                            {eMOVER_SUBSURFACE, "submarine"},
                                                            {eMOVER_SPACE, "satellite"},
                                                            {ePROCESSOR, "gears"},
                                                            {eFUEL, "gas"},
                                                            {eSENSOR, "camera"},
                                                            {eCOMM, "hand_radio"},
                                                            {eWEAPON, "gun"},
                                                            {eWEAPON_EXPLICIT, "bomb"},
                                                            {eRADAR, "radar"}};

enum PropertyName
{
   eDEBUG,
   eON,
   eOPERATIONAL,
   eBROKEN,
   eROLL,
   ePITCH,
   eYAW,
   eTILT,
   eSLEWMODE,
   eCURRENTMODE,
   eMODES,
   eRECEIVERS,
   eTRANSMITTERS,
   eTRACKS,
   eRCVRCOUNT,
   eXMTRCOUNT,
   eBANDWIDTH,
   eNOISEPOWER,
   eDETECTIONTHRESHOLD,
   ePEAKPOWER,
   eAVERAGEPOWER,
   eFREQUENCY,
   ePULSEWIDTH,
   ePRI, // pulse repetition interval
   eRANGELIMITSMIN,
   eRANGELIMITSMAX,
   eELEVATIONFOVMIN,
   eELEVATIONFOVMAX,
   eAZIMUTHFOVMIN,
   eAZIMUTHFOVMAX,
};

static const std::map<PropertyName, QString> sPropertyNameToString = {
   {eDEBUG, "Debug"},
   {eON, "On"},
   {eOPERATIONAL, "Operational"},
   {eBROKEN, "Broken"},
   {eROLL, "Roll"},
   {ePITCH, "Pitch"},
   {eYAW, "Yaw"},
   {eTILT, "Tilt"},
   {eSLEWMODE, "Slew Mode"},
   {eCURRENTMODE, "Current Mode"},
   {eMODES, "Modes"},
   {eTRACKS, "Tracks"},
   {eRECEIVERS, "Receiver"},
   {eTRANSMITTERS, "Transmitter"},
   {eBANDWIDTH, "Bandwidth"},
   {eNOISEPOWER, "Noise Power"},
   {eDETECTIONTHRESHOLD, "Detection Threshold"},
   {ePEAKPOWER, "Peak Power"},
   {eAVERAGEPOWER, "Average Power"},
   {eFREQUENCY, "Frequency"},
   {ePULSEWIDTH, "Pulse Width"},
   {ePRI, "Pulse Repetition Interval"},
   {eRANGELIMITSMIN, "Range, Min"},
   {eRANGELIMITSMAX, "Range, Max"},
   {eELEVATIONFOVMIN, "Elevation, Min"},
   {eELEVATIONFOVMAX, "Elevation, Max"},
   {eAZIMUTHFOVMIN, "Azimuth, Min"},
   {eAZIMUTHFOVMAX, "Azimuth, Max"},
};
} // namespace WkPlatformPartBrowser

namespace std
{
template<>
struct hash<WkPlatformPartBrowser::PropertyName>
{
   size_t operator()(const WkPlatformPartBrowser::PropertyName& x) const
   {
      using type = ut::underlying_type_t<WkPlatformPartBrowser::PropertyName>;
      return hash<type>()(static_cast<type>(x));
   }
};
} // namespace std

namespace WkPlatformPartBrowser
{
using PropertyMap = std::map<PropertyName, QVariant>;
using Property    = std::pair<PropertyName, QVariant>;

class PlatformPart
{
public:
   PlatformPart(unsigned int aUniqueId, const std::string& aName, const std::string& aUserType)
      : mUniqueId(aUniqueId)
      , mName(aName)
      , mUserType(aUserType)
      , mType(ePLATFORMPART)
   {
   }

   bool operator==(const PlatformPart& aRhs) const
   {
      return (mName == aRhs.mName) && (mUserType == aRhs.mUserType) && (mUniqueId == aRhs.mUniqueId);
   }

   unsigned int GetUniqueId() const { return mUniqueId; }

   const std::string& GetName() const { return mName; }
   void               SetName(const std::string& aName) { mName = aName; }

   const std::string& GetUserType() const { return mUserType; }

   const PartType& GetType() const { return mType; }
   void            SetType(const PartType& aType) { mType = aType; }

   const PropertyMap& GetProperties() const { return mProperties; }

   QVariant FindProperty(const PropertyName& aPropertyName) const;

   bool UpdateProperty(const PropertyName& aName, const QVariant& aValue);

   bool AddProperty(const PropertyName& aName, const QVariant& aValue);

protected:
   unsigned int mUniqueId;
   std::string  mName;
   std::string  mUserType; // User Defined type
   PartType     mType;
   PropertyMap  mProperties;
};

using PlatformPartList = std::vector<PlatformPart>;

} // namespace WkPlatformPartBrowser

Q_DECLARE_METATYPE(WkPlatformPartBrowser::Property);

#endif // !PLATFORMPARTBROWSERTYPES_HPP
