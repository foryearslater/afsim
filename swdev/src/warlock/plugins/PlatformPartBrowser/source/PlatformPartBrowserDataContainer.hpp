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

#ifndef PLATFORMPARTBROWSERDATACONTAINER_HPP
#define PLATFORMPARTBROWSERDATACONTAINER_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include <QObject>

#include "PlatformPartBrowserTypes.hpp"

namespace WkPlatformPartBrowser
{
class DataContainer : public QObject
{
   Q_OBJECT

public:
   void               SetPlatformOfInterest(const std::string& aPlatformName);
   const std::string& GetPlatformOfInterest() const { return mPlatformOfInterest; }

   void SetParts(const std::string& aPlatformName, const PlatformPartList& aPlatformParts);

   const std::vector<PlatformPart>& GetParts() { return mPlatformParts; }

   void UpdateProperty(unsigned int aUniqueId, PropertyName aPropertyName, const QVariant& aVariant);

   void UpdateParts(std::vector<PlatformPart>& aPlatformParts);

   void CreateTreeWidgetItems(QTreeWidget* aTreeWidget);

signals:
   void CheckBoxToggled(const std::string& aPartName, PropertyName aPropertyName, bool aChecked);
   void PlatformPartsSet(const QString& aPlatformName, const PlatformPartList& aPartList);

private:
   enum SpecialItemTypes
   {
      eCHECKBOX = Qt::UserRole
   };

   QList<QTreeWidgetItem*> CreateItem(const PlatformPart& aPart, const Property& aProperty);

   void UpdateItem(unsigned int aUniqueId, PropertyName aPropertyName, const QVariant& aVariant);

   void AddCheckBoxToTreeWidgetItem(QTreeWidget*        aTreeWidget,
                                    QTreeWidgetItem*    aItem,
                                    const PlatformPart& aPart,
                                    PropertyName        aPropertyName);

   std::string                                                                                 mPlatformOfInterest;
   PlatformPartList                                                                            mPlatformParts;
   std::unordered_map<unsigned int, std::unordered_map<PropertyName, QList<QTreeWidgetItem*>>> mTreeWidgets;
};
} // namespace WkPlatformPartBrowser

#endif