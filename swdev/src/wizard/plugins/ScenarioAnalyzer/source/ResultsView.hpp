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
*   File: ResultsView.hpp
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

#ifndef SCENARIO_ANALYZER_RESULTS_TREE_VIEW_HPP
#define SCENARIO_ANALYZER_RESULTS_TREE_VIEW_HPP

#include <QStyledItemDelegate>
#include <QTreeView>

namespace ScenarioAnalyzer
{
struct ResultsGroupItem;

class ResultsItemDelegate : public QStyledItemDelegate
{
   public:
      ResultsItemDelegate() : QStyledItemDelegate() {};
      void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
      //QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};


class ResultsTreeView : public QTreeView
{
      Q_OBJECT

   public:
      explicit ResultsTreeView(QWidget* parent = 0);
      ~ResultsTreeView() {}

      ResultsGroupItem* resultsGroupItemAtIndex(const QModelIndex& index);
      void expandSomeChildren(const QModelIndex& index); // helper for customExpand()

   public slots:
      void displayContextMenu(const QPoint& point);
      void goToLocation(const QString& locationType, const QString& locationName);
      void customExpand();
};
}
#endif
