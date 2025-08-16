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

#ifndef WIZRINGPROPERTIESWIDGET_HPP
#define WIZRINGPROPERTIESWIDGET_HPP

#include "ViExport.hpp"
#include "WsfPM_Root.hpp"
#include "annotation/WkfRingPropertiesWidget.hpp"

namespace wizard
{
class VI_EXPORT RingPropertiesWidget : public Annotation::RingPropertiesWidget
{
   Q_OBJECT
public:
   RingPropertiesWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
   ~RingPropertiesWidget() override = default;

   struct UnitStrings
   {
      UnitStrings() {}
      UnitStrings(const QString& aAdditionalRingRadius,
                  const QString& aBaseRadius,
                  const QString& aCenterAngle,
                  const QString& aWidth,
                  const QString& aRadialRate)
         : additionalRingRadius(aAdditionalRingRadius)
         , baseRadius(aBaseRadius)
         , centerAngle(aCenterAngle)
         , width(aWidth)
         , radialRate(aRadialRate)
      {
      }

      QString additionalRingRadius;
      QString baseRadius;
      QString centerAngle;
      QString width;
      QString radialRate;
   };

   struct RingChanges
   {
      std::string                          originalRingName;
      std::string                          entityName;
      bool                                 newRing;
      bool                                 entityContext;
      bool                                 nameChanged{false};
      bool                                 baseRadiusChanged{false};
      bool                                 additionalRingsChanged{false};
      bool                                 additionalRingRadiusChanged{false};
      bool                                 centerAngleChanged{false};
      bool                                 widthChanged{false};
      bool                                 radialRateChanged{false};
      bool                                 ringColorChanged{false};
      bool                                 radialColorChanged{false};
      bool                                 ringLineWidthChanged{false};
      bool                                 radialLineWidthChanged{false};
      bool                                 showRangeLabelsChanged{false};
      bool                                 showAngleLabelsChanged{false};
      bool                                 rangeLabelColorChanged{false};
      bool                                 angleLabelColorChanged{false};
      bool                                 alignPlatformChanged{false};
      wkf::AttachmentRangeRing::Properties properties;
      UnitStrings                          unitStrings;
   };

   int  GetCurrentNameIndex();
   void SetCurrentNameIndex(int aIndex);
   void SetEntity(vespa::VaEntity* aEntityPtr, const RingFlags& aFlags) override;
   void OverwriteOriginalValues(const wkf::AttachmentRangeRing::Properties& aInfo) override;

signals:
   void ChangesAccepted(const RingChanges& aChanges);
   void NewPoiRingAccepted(const std::string& aDestFile,
                           const RingChanges& aChanges,
                           const std::string& aName,
                           vespa::VaPosition& aPos);
   void NewRingAccepted(const std::string& aDestFile, const RingChanges& aChanges);

private:
   void                            CheckDefaults();
   void                            AcceptChangesP() override;
   void                            PopulateFiles(vespa::VaEntity* aEntityPtr);
   std::unordered_set<std::string> GetRingList() const override;

   void SetCenterRadius() override;
   void SetAdditionalRadii() override;
   void SetNumberAdditonalRings(const int aValue) override;
   void SetCenterAngle() override;
   void SetAngleWidth() override;
   void SetRadialRate() override;
   void RingColorUpdated(const QColor& aColor) override;
   void RadialColorUpdated(const QColor& aColor) override;
   void SetRingLineWidth(const int aValue) override;
   void SetRadialLineWidth(const int aValue) override;
   void RangeLabelsToggled(bool aState) override;
   void AngleLabelsToggled(bool aState) override;
   void RangeLabelColorUpdated(const QColor& aColor) override;
   void AngleLabelColorUpdated(const QColor& aColor) override;
   void SetAlignWithPlatform(bool aState) override;
   void NameChanged(const QString& aName) override;
   void SelectionChanged(int aIndex) override;

   QLabel*    mFileLabel;
   QComboBox* mFileComboBox;

   RingChanges                                  mChanges;
   WsfPM_Root                                   mProxyRoot;
   UtCallbackHolder                             mCallbackHolder;
   std::unordered_map<std::string, std::string> mFilePaths;
};
} // namespace wizard

#endif
