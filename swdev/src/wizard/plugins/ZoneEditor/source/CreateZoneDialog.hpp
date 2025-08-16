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

#ifndef CREATEZONEDIALOG_HPP
#define CREATEZONEDIALOG_HPP

#include <QDialog>

#include "UtTextDocument.hpp"
#include "VaPosition.hpp"
#include "WsfPProxy.hpp"
#include "ui_CreateZoneDialog.h"
#include "zone_browser/WkfZoneBrowserDataContainer.hpp"

namespace ZoneEditor
{
class CreateZoneDialog : public QDialog
{
public:
   CreateZoneDialog(wkf::ZoneBrowserDataContainer& aDataContainer,
                    WsfPProxy&                     aProxy,
                    const vespa::VaPosition&       aInitialPosition,
                    QWidget*                       aParentPtr = nullptr);
   ~CreateZoneDialog() override = default;

private:
   void closeEvent(QCloseEvent* aEventPtr) override;

   void CreateZone();
   void ZoneTypeChanged(const QString& aText);
   void ShowGeometricCommands(bool aShow);
   void ShowPolygonalCommands(bool aShow);

   void AddPointToTree(double aLatitude, double aLongitude, double aAltitude);
   void DisplayPointData(QTreeWidgetItem* aItemPtr, QTreeWidgetItem* aPreviousItemPtr);
   void CreateTreeWidgetContextMenu(const QPoint& aPoint);
   void PopulatePointTree();

   wkf::CommonZoneVariables     GetCommonVariables();
   wkf::CircularZoneVariables   GetCircularVariables();
   wkf::EllipticalZoneVariables GetEllipticalVariables();
   wkf::SphericalZoneVariables  GetSphericalVariables();
   wkf::PolygonalZoneVariables  GetPolygonalVariables();
   void                         UpdatePreview();

   std::string GetRelativePath(const UtTextDocument* aDocPtr);

   void PopulateFileList();
   void PopulateFileComboBox();
   void PopulatePlatformComboBox();
   void PopulateZoneComboBox();

   // Used to sort the file list in alphabetical order.
   static bool SortAlphabetical(const std::string& aLHS, const std::string& aRHS) { return aLHS < aRHS; };

   bool                                             mIsClosing;
   std::vector<std::string>                         mFileList;
   std::vector<wkf::ZonePointStruct>                mPointsList;
   std::map<QTreeWidgetItem*, wkf::ZonePointStruct> mPointsMap;

   wkf::ZoneBrowserDataContainer& mDataContainer;
   WsfPProxy&                     mProxy;
   vespa::VaPosition              mPosition;

   Ui::CreateZoneDialog mUi;
};
} // namespace ZoneEditor

#endif // !CREATEZONEDIALOG_HPP
