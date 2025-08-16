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

#ifndef WIZMAPLAYERDOCKWIDGET_HPP
#define WIZMAPLAYERDOCKWIDGET_HPP

#include "FileTypeCache.hpp"
#include "WkfMapLayerDockWidget.hpp"

namespace wizard
{
class MapLayerFileDetector : public FileTypeCache::FileDetector
{
public:
   const std::string cLE_TIFF_HEADER{"II\x2A\0"};
   const std::string cBE_TIFF_HEADER{"MM\0\x2A"};
   MapLayerFileDetector()           = default;
   ~MapLayerFileDetector() override = default;
   std::string DetectFileType(const QFileInfo& aFileInfo, const std::string& aFileHeader) override;
};
// this UI allows the user to add layers to their maps
// layers may be shape or image files
class MapLayerDockWidget : public wkf::MapLayerDockWidget
{
public:
   MapLayerDockWidget(QWidget* aParent = nullptr, Qt::WindowFlags aFlags = Qt::WindowFlags());
   ~MapLayerDockWidget() override;

   void ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu);

private:
   // we override the implementation in wkf::MapLayerDockWidget so that we
   // don't interpret drags into the map view... wizard handles these through
   // the FileTypeCache
   void DragEnter(QDragEnterEvent* aEvent) override;

   // we override the implementation in wkf::MapLayerDockWidget so that we
   // don't interpret drags into the map view... wizard handles these through
   // the FileTypeCache
   void DragDrop(QDropEvent* aEvent) override;

   void                 CreateLayerShape(const QString& aFilename);
   void                 CreateLayerTiff(const QString& aFilename);
   MapLayerFileDetector mDetector;
};
} // namespace wizard

#endif
