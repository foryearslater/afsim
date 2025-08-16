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
*   File: SelectCheckModel.hpp
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

#ifndef SCENARIO_ANALYZER_SELECT_CHECK_GROUP_MODEL_HPP
#define SCENARIO_ANALYZER_SELECT_CHECK_GROUP_MODEL_HPP

#include <QAbstractItemModel>
#include <QString>
#include <QVector>

namespace ScenarioAnalyzer
{
struct CheckTreeItem
{
   QString name;
   QString title;
   QVector<CheckTreeItem> children;
   // The number of direct children that are selected, including itself.
   int selectedCount;
   bool valid;
   CheckTreeItem* parent;
};

class SelectCheckGroupModel : public QAbstractItemModel
{

   public:
      SelectCheckGroupModel() :
         QAbstractItemModel()
      {
         root.children.clear();
         root.selectedCount = 0;
         root.valid = true;
         root.parent = nullptr;
      };

      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& parent) const override;
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      bool setData(const QModelIndex& index, const QVariant& value, int role) override;

      void refreshRoot();
      void selectRecursive(QModelIndex index);
      void deselectRecursive(QModelIndex index);
      void clear();

      CheckTreeItem root;
};
}
#endif