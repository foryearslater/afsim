// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
/*******************************************************************************
*
*   File: SelectCheckModel.cpp
*
*   Classification: UNCLASSIFIED
*
*   Details:
*
*   Contract Number:
*   Contractor Name: Radiance Technologies
*   Address:         350 Wynn Drive
*                    Huntsville, AL 35805
*
*   Abstract:
*
*******************************************************************************/

#include "SelectCheckModel.hpp"

#include <QBrush>

// virtual
Qt::ItemFlags ScenarioAnalyzer::SelectCheckGroupModel::flags(const QModelIndex& index) const
{
   //TODO ItemIsTristate is deprecated in favor of ItemIsAutoTristate in Qt 5.8
   auto item = static_cast<CheckTreeItem*>(index.internalPointer());
   if (item->valid)
   {
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsTristate;
   }
   else
   {
      return 0;
   }
}

// virtual
QVariant ScenarioAnalyzer::SelectCheckGroupModel::headerData(int, Qt::Orientation, int) const
{
   return QVariant();
}

// virtual
int ScenarioAnalyzer::SelectCheckGroupModel::rowCount(const QModelIndex& parent) const
{
   if (parent == QModelIndex())
   {
      return 1;
   }

   auto parentItem = static_cast<CheckTreeItem*>(parent.internalPointer());
   return parentItem->children.size();
}

// virtual
int ScenarioAnalyzer::SelectCheckGroupModel::columnCount(const QModelIndex& /* parent */) const
{
   return 1;
}

// virtual
QModelIndex ScenarioAnalyzer::SelectCheckGroupModel::index(int row, int column, const QModelIndex& parent) const
{
   if (parent == QModelIndex())
   {
      return this->createIndex(0, 0, (void*)&this->root);
   }

   auto parentItem = static_cast<CheckTreeItem*>(parent.internalPointer());
   CheckTreeItem* item = &parentItem->children[row];
   return this->createIndex(row, column, item);
}

// virtual
QModelIndex ScenarioAnalyzer::SelectCheckGroupModel::parent(const QModelIndex& index) const
{
   auto item = static_cast<CheckTreeItem*>(index.internalPointer());
   CheckTreeItem* parentItem = item->parent;
   if (parentItem == nullptr)
   {
      return QModelIndex();
   }
   else if (parentItem == &(this->root))
   {
      return this->createIndex(0, 0, parentItem);
   }
   else
   {
      CheckTreeItem* grandparentItem = parentItem->parent;
      int parentRow = (int)(parentItem - &grandparentItem->children[0]);
      return this->createIndex(parentRow, 0, parentItem);
   }
}

// virtual
QVariant ScenarioAnalyzer::SelectCheckGroupModel::data(const QModelIndex& index, int role) const
{
   auto item = static_cast<CheckTreeItem*>(index.internalPointer());
   switch (role)
   {
      case Qt::ItemDataRole::DisplayRole:
         return QVariant(item->title);
      case Qt::ItemDataRole::ForegroundRole:
      {
         if (!item->valid)
         {
            return QVariant(QBrush(Qt::gray));
         }
      }
      case Qt::CheckStateRole:
         if (item->selectedCount == 0)
         {
            return Qt::Unchecked;
         }
         else if (item->selectedCount == item->children.size() + 1)
         {
            return Qt::Checked;
         }
         else
         {
            return Qt::PartiallyChecked;
         }
   }
   return QVariant();
}

void ScenarioAnalyzer::SelectCheckGroupModel::selectRecursive(QModelIndex index)
{
   auto item = static_cast<CheckTreeItem*>(index.internalPointer());
   if (item->valid)
   {
      for (int i = 0; i < item->children.size(); ++i)
      {
         auto childItem = &(item->children[i]);
         QModelIndex index = this->createIndex(i, 0, childItem);
         this->selectRecursive(index);
      }
      item->selectedCount = item->children.size() + 1;
      this->dataChanged(index, index, QVector<int> {Qt::CheckStateRole});
   }
}

void ScenarioAnalyzer::SelectCheckGroupModel::deselectRecursive(QModelIndex index)
{
   auto item = static_cast<CheckTreeItem*>(index.internalPointer());
   for (int i = 0; i < item->children.size(); ++i)
   {
      auto childItem = &(item->children[i]);
      QModelIndex index = this->createIndex(i, 0, childItem);
      this->deselectRecursive(index);
   }
   item->selectedCount = 0;
   this->dataChanged(index, index, QVector<int> {Qt::CheckStateRole});
}


// virtual
bool ScenarioAnalyzer::SelectCheckGroupModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
   if (role != Qt::CheckStateRole)
   {
      return false;
   }

   auto item = static_cast<CheckTreeItem*>(index.internalPointer());
   // update the selected state of all child and parent nodes
   bool selected = value == Qt::Checked;
   if (selected)
   {
      this->selectRecursive(index);

      QModelIndex idx = index;
      for (;;)
      {
         idx = idx.parent();
         if (idx == QModelIndex())
         {
            break;
         }
         item = static_cast<CheckTreeItem*>(idx.internalPointer());
         ++item->selectedCount;

         if (item->selectedCount == item->children.size())
         {
            ++item->selectedCount;
         }

         this->dataChanged(idx, idx, QVector<int> {Qt::CheckStateRole});

         if (item->selectedCount != item->children.size() + 1)
         {
            break;
         }
      }
   }
   else
   {
      this->deselectRecursive(index);

      QModelIndex idx = index;
      for (;;)
      {
         idx = idx.parent();
         if (idx == QModelIndex())
         {
            break;
         }
         item = static_cast<CheckTreeItem*>(idx.internalPointer());
         --item->selectedCount;

         if (item->selectedCount == item->children.size())
         {
            --item->selectedCount;
         }

         this->dataChanged(idx, idx, QVector<int> {Qt::CheckStateRole});

         if (item->selectedCount != item->children.size() - 1)
         {
            break;
         }
      }
   }

   return true;
}

void ScenarioAnalyzer::SelectCheckGroupModel::refreshRoot()
{
   this->beginResetModel();
   this->endResetModel();
}

void ScenarioAnalyzer::SelectCheckGroupModel::clear()
{
   this->beginResetModel();
   this->root.children.clear();
   this->root.selectedCount = 0;
   this->endResetModel();
}
