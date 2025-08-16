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
*   File: ResultsModel.hpp
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

#ifndef SCENARIO_ANALYZER_EXPANDING_RESULTS_GROUP_MODEL_HPP
#define SCENARIO_ANALYZER_EXPANDING_RESULTS_GROUP_MODEL_HPP

#include <memory>
#include <vector>

#include <QAbstractItemModel>
#include <QString>
#include <QVector>

#include "ScenarioAnalyzerMessage.hpp"

namespace ScenarioAnalyzer
{
struct ResultsGroupItem
{
   ResultsGroupItem()
      : name(""),
        parent(nullptr),
        expand(false),
        width(0)
   {}

   ResultsGroupItem* findChild(const QString& name);

   QVector<ResultsGroupItem> children;
   QString name;
   ResultsGroupItem* parent;
   bool expand;
   int width;

   ScenarioAnalyzerMessage message;
};

class ExpandingResultsGroupModel : public QAbstractItemModel
{
   public:
      ExpandingResultsGroupModel()
      {
         root.children.clear();
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
      void clear();

      void constructResultsTree(const std::vector<ScenarioAnalyzerMessage>& notes,
                                const std::vector<ScenarioAnalyzerMessage>& results,
                                bool& allowSetTime);

      ResultsGroupItem root;
};
}
#endif