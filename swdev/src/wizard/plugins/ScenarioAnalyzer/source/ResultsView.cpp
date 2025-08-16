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
*   File: ResultsView.cpp
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


#include <string>

#include <QHeaderView>
#include <QPainter>
#include <QMessageBox>
#include <QMenu>


#include "FindReferencesResultDialog.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "ResultsModel.hpp"
#include "ResultsView.hpp"
#include "WsfParseNode.hpp"

void ScenarioAnalyzer::ResultsItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
   painter->save();
   QStyledItemDelegate::paint(painter, option, index);
   painter->restore();

   // store width to be accessed from sizeHint
   auto model = const_cast<QAbstractItemModel*>(index.model());
   model->setData(index, option.rect.width(), Qt::UserRole + 1);
}

ScenarioAnalyzer::ResultsTreeView::ResultsTreeView(QWidget* parent) : QTreeView(parent)
{
   this->setContextMenuPolicy(Qt::CustomContextMenu);
   this->setTextElideMode(Qt::TextElideMode::ElideNone);
   QObject::connect(this, &QAbstractItemView::customContextMenuRequested, this, &ResultsTreeView::displayContextMenu);
}

void ScenarioAnalyzer::ResultsTreeView::displayContextMenu(const QPoint& point)
{
   QModelIndex index = indexAt(point);

   auto item = static_cast<ResultsGroupItem*>(index.internalPointer());
   const std::vector<ScenarioFileLocation>& locations = item->message.GetScenarioFileLocations();

   QMenu popup;
   for (const auto& location : locations)
   {
      QString locationType = location.mType.c_str();
      QString locationName = location.mName.c_str();

      if (location.mType.back() == 's')
      {
         locationType = location.mType.substr(0, location.mType.size() - 1).c_str();
      }
      QAction* action = popup.addAction("Go to " + locationType + " " + locationName);
      QObject::connect(action, &QAction::triggered, this, [this, locationType, locationName] { goToLocation(locationType, locationName); });
   }
   popup.exec(QCursor::pos());
}

void ScenarioAnalyzer::ResultsTreeView::goToLocation(const QString& locationType, const QString& locationName)
{
   wizard::Project* project = wizard::ProjectWorkspace::Instance()->GetProject();
   wizard::ParseResults* parseResults = project->WaitForParseResults();

   std::string locNameStr = locationName.toLocal8Bit().data();
   std::string locTypeStr = locationType.toLocal8Bit().data();

   std::vector<WsfParseNode*> definitions;
   parseResults->FindDefinitions(locNameStr, locTypeStr, definitions);

   std::vector<UtTextDocumentRange> locations;
   if (!definitions.empty())
   {
      for (const WsfParseNode* def : definitions)
      {
         locations.push_back(def->mValue);
      }
   }

   if (locations.size() > 1)
   {
      wizard::FindReferencesResultDialog* dlg = new wizard::FindReferencesResultDialog(this);
      QString                       label("Find References: \"");
      label += locNameStr.c_str();
      label += '"';
      dlg->Show(project, locations, label);
   }
   else if (locations.size() == 1)
   {
      project->GotoRange(locations[0]);
   }
   else
   {
      QMessageBox::information(this, tr("Find References"), tr("No reference found."));
   }

}

void ScenarioAnalyzer::ResultsTreeView::expandSomeChildren(const QModelIndex& index)
{
   auto model = dynamic_cast<ExpandingResultsGroupModel*>(this->model());
   if (model)
   {
      int children = model->rowCount(index);
      for (int i = 0; i != children; ++i)
      {
         QModelIndex childIndex = model->index(i, 0, index);
         QVariant expandVar = model->data(childIndex, Qt::UserRole + 3); // gets bool item->expand
         bool expand = expandVar.toBool();
         if (expand)
         {
            this->expand(childIndex);
            expandSomeChildren(childIndex);
         }
      }
   }
}

void ScenarioAnalyzer::ResultsTreeView::customExpand()
{
   auto model = dynamic_cast<ExpandingResultsGroupModel*>(this->model());
   if (model)
   {
      QModelIndex rootIndex = this->rootIndex();
      expandSomeChildren(rootIndex);
   }
}