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

#ifndef WIZPOIPROPERTIESWIDGET_HPP
#define WIZPOIPROPERTIESWIDGET_HPP

#include <unordered_map>

#include <QComboBox>
#include <QObject>

#include "ViExport.hpp"
#include "WsfPM_Root.hpp"
#include "annotation/WkfPointOfInterestPropertiesWidget.hpp"

namespace wizard
{
class VI_EXPORT PointOfInterestPropertiesWidget : public Annotation::PointOfInterestPropertiesWidget
{
   Q_OBJECT
public:
   PointOfInterestPropertiesWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~PointOfInterestPropertiesWidget() override = default;

   void SetEntity(vespa::VaEntity* aEntityPtr, bool aCreate) override;
   void PopulateFiles();
   void OverwriteOriginalValues(const PoiInfo& aInfo) override;

   struct PoiChanges
   {
      std::string originalEntityName;
      bool        nameChanged{false};
      bool        positionChanged{false};
      bool        bullseyeChanged{false};
      QString     altString;
      PoiInfo     info;
   };

   std::string GetEntityUiName() const;
   std::string GetOriginalName() const;

signals:
   void ChangesAccepted(const PoiChanges& aChanges);
   void NewPoiAccepted(const std::string& aDestFile, const PoiChanges& aChanges);
   void CreatedNewPoi();
   void NameEdited(const std::string& aName, const QString& aNewName);
   void PositionEdited(const std::string& aName, const vespa::VaPosition& aPosition);

private:
   void AcceptChanges() override;
   void NameOperation() override;
   void PositionOperation(const vespa::VaPosition& aPosition) override;
   void BullseyeOperation(bool aBullseye) override;
   void AssignAltitudeString();

   PoiChanges mChanges;

   QLabel*    mFileLabel;
   QComboBox* mFileComboBox;

   WsfPM_Root                                   mProxyRoot;
   UtCallbackHolder                             mCallbackHolder;
   std::unordered_map<std::string, std::string> mFilePaths;
};
} // namespace wizard

#endif
