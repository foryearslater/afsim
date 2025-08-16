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

#include "ModelImportDockWidget.hpp"

#include <QString>
#include <QStringList>
#include <QWindow>

#include "Environment.hpp"
#include "ModelImportDirectoryReaderThread.hpp"
#include "ModelImportPlugin.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "UtQtMemory.hpp"

enum TreeItemType
{
   cTYPE_ITEM,
   cCATEGORY_ITEM,
   cFILE_ITEM,
   cDIRECTORY_ITEM,
   cALL_ITEM
};

enum TreeItemRole
{
   cTYPE_ROLE = Qt::ItemDataRole::UserRole,
   cPATH_ROLE,
   cINDEX_ROLE
};

// ================================================================================================
//! Returns the path to a file that a QTreeWidgetItem refers to.
//! Refers to a data-base pointer if necessary.
//! @param aItemPtr This is the pointer to the tree item.
//! @param aParseDataPtr This is a pointer to the data-base that the data comes from. Default = nullptr.
//! @returns The path to the file.
static QString PathToItem(const QTreeWidgetItem* aItemPtr, const ModelImport::ParseData* aParseDataPtr = nullptr)
{
   if (aItemPtr->data(0, cTYPE_ROLE) == cTYPE_ITEM && aParseDataPtr)
   {
      return aParseDataPtr->at(aItemPtr->data(0, cINDEX_ROLE).toInt()).PathToFile();
   }
   return aItemPtr->data(0, cPATH_ROLE).toString();
}

// ================================================================================================

ModelImport::DockWidget::DockWidget(QWidget* aParent /*= 0*/, Qt::WindowFlags aWindowFlags /*= 0*/)
   : QDockWidget(aParent, aWindowFlags)
{
   setObjectName("Model Import Beta");
   mUI.setupUi(this);

   mUI.treeWidget->sortItems(0, Qt::SortOrder::AscendingOrder);

   connect(mUI.clearButton,
           &QPushButton::clicked,
           this,
           [this]()
           {
              mUI.searchBar->clear();
              mUI.searchBar->clearEditText();
           });
   connect(mUI.searchBar, &QComboBox::currentTextChanged, this, [this]() { emit SearchModified(); });

   // Imports selected item
   connect(mUI.treeWidget, &QTreeWidget::itemActivated, this, &DockWidget::ItemSelected);
}

// ================================================================================================

void ModelImport::DockWidget::SetPath(const QString& aPath)
{
   if (aPath.isEmpty())
   {
      mUI.pathDisplay->setText("This path is set in the Preferences menu.");
   }
   else
   {
      mUI.pathDisplay->setText(aPath);
   }
}

// ================================================================================================

void ModelImport::DockWidget::DisplayData(const ModelImport::ParseData& aData, const QString& aDisplayMode)
{
   mUI.treeWidget->clear();
   mParseDataPtr = &aData;

   if (aData.empty())
   {
      return;
   }
   if (!mUI.searchBar->currentText().isEmpty())
   {
      DisplaySearchResults(aData, mUI.searchBar->currentText());
   }
   else if (aDisplayMode == "Category List")
   {
      DisplayCategoryList(aData);
   }
   else if (aDisplayMode == "Name List")
   {
      DisplayNameList(aData);
   }
   else if (aDisplayMode == "File Tree")
   {
      DisplayFileTree(aData);
   }
}

// ================================================================================================
// ================================================================================================
//! Allocates and initialized a new tree widget item.
static QTreeWidgetItem* MakeTreeItem(const ModelImport::ParseData& aData, const ModelImport::ItemData& aItem, std::size_t aIndex)
{
   QTreeWidgetItem* treeEntry = ut::qt::make_qt_ptr<QTreeWidgetItem>();
   treeEntry->setText(0, aItem.mName);
   treeEntry->setText(1, aData[aIndex].mFile);
   treeEntry->setText(2, aData[aIndex].mPath);
   treeEntry->setData(0, cTYPE_ROLE, cTYPE_ITEM);
   treeEntry->setData(0, cINDEX_ROLE, QVariant::fromValue(aIndex));
   return treeEntry;
}

// ================================================================================================
//! A helper class for organizing information into a tree for "File Tree" display mode.
//! @note This struct does NOT free any QTreeWidgetItem's that it allocates.
//!       They are expected to be made children of a QTreeWidget.
struct DisplayFileTreeHelper
{
   std::map<QString, DisplayFileTreeHelper> mChildren;
   QTreeWidgetItem*                         mItemPtr = nullptr;

   //! Provides a reference to a child.
   //! Creates the child if it does not already exist.
   //! @param aName This is the name of the child to return.
   //! @returns The specified child.
   DisplayFileTreeHelper& Get(const QString& aName)
   {
      if (mChildren.count(aName) == 0)
      {
         mChildren[aName].mItemPtr = ut::qt::make_qt_ptr<QTreeWidgetItem>(QStringList(aName));
         mChildren[aName].mItemPtr->setData(0, cTYPE_ROLE, cDIRECTORY_ITEM);

         if (mItemPtr) // if not top level
         {
            mItemPtr->addChild(mChildren[aName].mItemPtr);
            mChildren[aName].mItemPtr->setData(0,
                                               cPATH_ROLE,
                                               QVariant::fromValue(ModelImport::Path(PathToItem(mItemPtr), aName)));
         }
         else
         {
            mChildren[aName].mItemPtr->setData(0, cPATH_ROLE, QVariant::fromValue(aName));
         }
      }
      return mChildren[aName];
   }

   //! Provides a reference to a child.
   //! As opposed to Get(), this can be used to get children of children, etc.
   //! @param This is the name of the child to return.
   //! @returns The specified child.
   DisplayFileTreeHelper& GetRecursive(const QString& aName)
   {
      if (!aName.contains('/'))
      {
         return Get(aName);
      }

      int index = aName.indexOf('/');
      return Get(aName.left(index)).GetRecursive(aName.mid(index + 1));
   }
};

// ================================================================================================

void ModelImport::DockWidget::DisplaySearchResults(const ModelImport::ParseData& aData, const QString& aText)
{
   for (std::size_t i = 0; i < aData.size(); i++)
   {
      for (const ModelImport::ItemData& item : aData[i].mDefines)
      {
         if (item.mType == "platform_type" && item.mName.contains(aText, Qt::CaseSensitivity::CaseInsensitive))
         {
            mUI.treeWidget->addTopLevelItem(MakeTreeItem(aData, item, i));
         }
      }
   }
}

// ================================================================================================

void ModelImport::DockWidget::DisplayCategoryList(const ModelImport::ParseData& aData)
{
   std::map<QString, QTreeWidgetItem*> labelMap;
   labelMap["<All>"] = ut::qt::make_qt_ptr<QTreeWidgetItem>(QStringList("<All>"));
   labelMap["<All>"]->setData(0, cTYPE_ROLE, cALL_ITEM);

   // Fill data in map
   for (std::size_t i = 0; i < aData.size(); i++)
   {
      for (const ModelImport::ItemData& item : aData[i].mDefines)
      {
         if (item.mType == "platform_type")
         {
            labelMap["<All>"]->addChild(MakeTreeItem(aData, item, i));

            for (const QString& label : item.mLabels)
            {
               if (labelMap.count(label) == 0)
               {
                  labelMap[label] = ut::qt::make_qt_ptr<QTreeWidgetItem>(QStringList(label));
                  labelMap[label]->setData(0, cTYPE_ROLE, cCATEGORY_ITEM);
               }
               labelMap[label]->addChild(MakeTreeItem(aData, item, i));
            }
         }
      }
   }

   // Move data from map to tree
   for (const auto& item : labelMap)
   {
      // Transfers ownership to mUI.treeWidget
      mUI.treeWidget->addTopLevelItem(item.second);
   }
}

// ================================================================================================

void ModelImport::DockWidget::DisplayNameList(const ModelImport::ParseData& aData)
{
   for (std::size_t i = 0; i < aData.size(); i++)
   {
      for (const ModelImport::ItemData& item : aData[i].mDefines)
      {
         if (item.mType == "platform_type")
         {
            mUI.treeWidget->addTopLevelItem(MakeTreeItem(aData, item, i));
         }
      }
   }
}

// ================================================================================================

void ModelImport::DockWidget::DisplayFileTree(const ModelImport::ParseData& aData)
{
   DisplayFileTreeHelper helper;

   for (std::size_t i = 0; i < aData.size(); i++)
   {
      DisplayFileTreeHelper& dirItem = helper.GetRecursive(aData[i].PathToFile());

      for (const ModelImport::ItemData& item : aData[i].mDefines)
      {
         if (item.mType == "platform_type")
         {
            dirItem.mChildren[item.mName].mItemPtr = MakeTreeItem(aData, item, i);
            dirItem.mItemPtr->addChild(dirItem.mChildren[item.mName].mItemPtr);

            dirItem.mItemPtr->setData(0, cTYPE_ROLE, cFILE_ITEM);
            dirItem.mItemPtr->setData(0, cPATH_ROLE, QVariant::fromValue(aData[i].PathToFile()));
            dirItem.mItemPtr->setData(0, cINDEX_ROLE, QVariant::fromValue(i));
         }
      }
   }

   for (const auto& directory : helper.mChildren)
   {
      mUI.treeWidget->addTopLevelItem(directory.second.mItemPtr);
   }
}

// ================================================================================================

void ModelImport::DockWidget::ItemSelected(QTreeWidgetItem* aSelection, int aColumn)
{
   constexpr int cYES = QMessageBox::StandardButton::Yes;

   emit BeginImport();

   switch (aSelection->data(0, cTYPE_ROLE).toInt())
   {
   case cCATEGORY_ITEM:
      if (cYES == QMessageBox::question(
                     this,
                     QString(),
                     QString("Are you sure you want to import everything in the category \"%1\"?").arg(aSelection->text(0)),
                     QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                     QMessageBox::StandardButton::No))
      {
         for (int i = 0; i < aSelection->childCount(); i++)
            ImportItem(PathToItem(aSelection->child(i), mParseDataPtr));
      }
      break;
   case cFILE_ITEM:
      ImportItem(PathToItem(aSelection));
      break;
   case cDIRECTORY_ITEM:
      if (cYES == QMessageBox::question(this,
                                        QString(),
                                        QString("Are you sure you want to import everything in the directory \"%1\"?")
                                           .arg(PathToItem(aSelection)),
                                        QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                        QMessageBox::StandardButton::No))
      {
         for (const auto& item : *mParseDataPtr)
         {
            if (item.PathToFile().startsWith(PathToItem(aSelection)))
            {
               ImportItem(item.PathToFile());
            }
         }
      }
      break;
   case cALL_ITEM:
      if (cYES == QMessageBox::question(this,
                                        QString(),
                                        QString("Are you sure you want to import the entire repository?"),
                                        QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                                        QMessageBox::StandardButton::No))
      {
         for (const auto& item : *mParseDataPtr)
         {
            ImportItem(item.PathToFile());
         }
      }
      break;
   default:
      emit ImportItem(PathToItem(aSelection, mParseDataPtr));
   }
}
