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
#ifndef MODELIMPORT_DOCKWIDGET_HPP
#define MODELIMPORT_DOCKWIDGET_HPP

#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QStringList>

#include "ModelImportUtility.hpp"
#include "ui_ModelImportDockWidget.h"

namespace ModelImport
{
struct ItemData;

//! The primary widget for interacting with the user.
class DockWidget : public QDockWidget
{
   Q_OBJECT

public:
   DockWidget(QWidget* aParent = nullptr, Qt::WindowFlags aWindowFlags = Qt::WindowFlags());
   ~DockWidget() override = default;

   //! Updates the primary display with data.
   //! @param aData This is the data to be displayed.
   //! @param aDisplayMode This is the mode the text should be displayed in.
   void DisplayData(const ModelImport::ParseData& aData, const QString& aDisplayMode);

   //! Updates the helper text displayed to the user.
   //! Shows instructional text if the given path is empty.
   //! @param aPath This is the path to be displayed.
   void SetPath(const QString& aPath);

signals:
   void ImportItem(const QString& aPathToFile);
   void BeginImport();

   void SearchModified();

private:
   Ui::ModelImportDockWidget mUI;

   const ModelImport::ParseData* mParseDataPtr = nullptr;

   //! Displays results of a text search.
   //! @param aData This is the data-base the data comes from.
   //! @param aText This is the search pattern.
   void DisplaySearchResults(const ModelImport::ParseData& aData, const QString& aText);

   //! Displays data in "Category List" mode.
   //! @param aData This is the data-base the data comes from.
   void DisplayCategoryList(const ModelImport::ParseData& aData);

   //! Displays data in "Name List" mode.
   //! @param aData This is the data-base the data comes from.
   void DisplayNameList(const ModelImport::ParseData& aData);

   //! Displays data in "File Tree" mode.
   //! @param aData This is the data-base the data comes from.
   void DisplayFileTree(const ModelImport::ParseData& aData);

   //! Processes a user's selection. May emit a signal telling the Plugin to import files.
   //! @param aSelection This is a pointer to the tree widget item the user clicked.
   //! @param aColumn This is a dummy parameter provided for compatibility with the Qt signal that calls this.
   void ItemSelected(QTreeWidgetItem* aSelection, int aColumn);
};
} // namespace ModelImport

#endif
