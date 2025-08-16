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

#include "WizMapLayerDockWidget.hpp"

#include <QFileInfo>
#include <QMenu>

#include "Signals.hpp"
#include "UtoCmeTerrain.hpp"
#include "UtoViewer.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"

wizard::MapLayerDockWidget::MapLayerDockWidget(QWidget* aParent, Qt::WindowFlags aFlags)
   : wkf::MapLayerDockWidget(aParent, aFlags)
{
   wizard::FileTypeCache::RegisterDetector(&mDetector);

   wizard::FileTypeCache::SetFileTypeIcon("tiff", "globe");
   wizard::FileTypeCache::SetFileTypeIcon("shp", "zone");

   connect(wizSignals, &wizard::Signals::ActionsForFileRequested, this, &MapLayerDockWidget::ActionsForFileRequested);
}

wizard::MapLayerDockWidget::~MapLayerDockWidget()
{
   wizard::FileTypeCache::UnregisterDetector(&mDetector);
}

void wizard::MapLayerDockWidget::ActionsForFileRequested(const QFileInfo& aFileInfo, QMenu* aMenu)
{
   if ((wizard::FileTypeCache::DetermineFileType(aFileInfo)) == "tiff")
   {
      connect(aMenu->addAction(QIcon::fromTheme("globe"), "Open as map layer"),
              &QAction::triggered,
              [this, aFileInfo]() { CreateLayerTiff(aFileInfo.absoluteFilePath()); });
   }
   if ((wizard::FileTypeCache::DetermineFileType(aFileInfo)) == "shp")
   {
      connect(aMenu->addAction(QIcon::fromTheme("zone"), "Open as map layer"),
              &QAction::triggered,
              [this, aFileInfo]() { CreateLayerShape(aFileInfo.absoluteFilePath()); });
   }
}

void wizard::MapLayerDockWidget::DragEnter(QDragEnterEvent* aEvent)
{
   // this may be called internally from the widget, or externally from the map display
   // since wizard has its own means of handling file actions (FileTypeCache) we should make
   // sure only the internal stuff is handled
   if (rect().contains(aEvent->pos()))
   {
      wkf::MapLayerDockWidget::DragEnter(aEvent);
   }
}

void wizard::MapLayerDockWidget::DragDrop(QDropEvent* aEvent)
{
   // this may be called internally from the widget, or externally from the map display
   // since wizard has its own means of handling file actions (FileTypeCache) we should make
   // sure only the internal stuff is handled
   if (rect().contains(aEvent->pos()))
   {
      wkf::MapLayerDockWidget::DragDrop(aEvent);
   }
}

void wizard::MapLayerDockWidget::CreateLayerShape(const QString& aFilename)
{
   wkf::Viewer* v = vaEnv.GetStandardViewer();
   if (nullptr != v)
   {
      UtoCmeTerrain* terrain = dynamic_cast<UtoCmeTerrain*>(&v->GetViewer()->World().Terrain());
      if (terrain != nullptr)
      {
         wkf::MapLayerDockWidget::CreateLayerShape(aFilename, *terrain);
      }
   }
}

void wizard::MapLayerDockWidget::CreateLayerTiff(const QString& aFilename)
{
   wkf::Viewer* v = vaEnv.GetStandardViewer();
   if (nullptr != v)
   {
      UtoCmeTerrain* terrain = dynamic_cast<UtoCmeTerrain*>(&v->GetViewer()->World().Terrain());
      if (terrain != nullptr)
      {
         wkf::MapLayerDockWidget::CreateLayerTiff(aFilename, *terrain);
      }
   }
}

std::string wizard::MapLayerFileDetector::DetectFileType(const QFileInfo& aFileInfo, const std::string& aFileHeader)
{
   QString suffix = aFileInfo.suffix();
   if ((suffix == "tif") || (suffix == "tiff"))
   {
      if (aFileHeader.size() > cLE_TIFF_HEADER.size() &&
          aFileHeader.compare(0, cLE_TIFF_HEADER.size(), cLE_TIFF_HEADER) == 0)
      {
         return "tiff";
      }
      else if (aFileHeader.size() > cBE_TIFF_HEADER.size() &&
               aFileHeader.compare(0, cBE_TIFF_HEADER.size(), cBE_TIFF_HEADER) == 0)
      {
         return "tiff";
      }
   }
   else if (suffix == "shp")
   {
      return "shp";
   }
   return std::string();
}
