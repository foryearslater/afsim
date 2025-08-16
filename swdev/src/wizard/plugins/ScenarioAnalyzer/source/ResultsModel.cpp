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
*   File: ResultsModel.cpp
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


#include <memory>
#include <string>
#include <vector>

#include <QApplication>
#include <QPalette>
#include <QBrush>

#include "ResultsModel.hpp"
#include "ScenarioAnalyzerUtilities.hpp"

ScenarioAnalyzer::ResultsGroupItem* ScenarioAnalyzer::ResultsGroupItem::findChild(const QString& name)
{
   return std::find_if(children.begin(), children.end(), [&name](ResultsGroupItem & child) { return child.name == name; });
}

// virtual
Qt::ItemFlags ScenarioAnalyzer::ExpandingResultsGroupModel::flags(const QModelIndex& /* index */) const
{
   return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

// virtual
QVariant ScenarioAnalyzer::ExpandingResultsGroupModel::headerData(int, Qt::Orientation, int) const
{
   return QVariant();
}

// virtual
int ScenarioAnalyzer::ExpandingResultsGroupModel::rowCount(const QModelIndex& parent) const
{
   if (parent == QModelIndex())
   {
      return 1;
   }

   auto parentItem = static_cast<ResultsGroupItem*>(parent.internalPointer());
   return parentItem->children.size();
}

// virtual
int ScenarioAnalyzer::ExpandingResultsGroupModel::columnCount(const QModelIndex& /* parent */) const
{
   return 1;
}

// virtual
QModelIndex ScenarioAnalyzer::ExpandingResultsGroupModel::index(int row, int column, const QModelIndex& parent) const
{
   if (parent == QModelIndex())
   {
      return this->createIndex(0, 0, (void*)&this->root);
   }

   auto parentItem = static_cast<ResultsGroupItem*>(parent.internalPointer());
   ResultsGroupItem* item = &parentItem->children[row];
   return this->createIndex(row, column, item);
}

// virtual
QModelIndex ScenarioAnalyzer::ExpandingResultsGroupModel::parent(const QModelIndex& index) const
{
   auto item = static_cast<ResultsGroupItem*>(index.internalPointer());
   ResultsGroupItem* parentItem = item->parent;
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
      ResultsGroupItem* grandparentItem = parentItem->parent;
      int parentRow = (int)(parentItem - &grandparentItem->children[0]);
      return this->createIndex(parentRow, 0, parentItem);
   }
}

// virtual
QVariant ScenarioAnalyzer::ExpandingResultsGroupModel::data(const QModelIndex& index, int role) const
{
   auto item = static_cast<ResultsGroupItem*>(index.internalPointer());
   if (item)
   {
      switch (role)
      {
         case Qt::ItemDataRole::DisplayRole:
            return QVariant(item->name);
         case Qt::ItemDataRole::ForegroundRole:
            if (item->message.IsSuccessfulResult()) // PASS
            {
               QBrush greenText(QColor(37, 124, 52));
               return QVariant(greenText);
            }
            else if (item->message.GetSeverity() == ScenarioAnalyzerMessage::cWARNING) // WARNING
            {
               QBrush yellowText(QColor(180, 133, 36));
               return QVariant(yellowText);
            }
            else if (item->message.GetSeverity() == ScenarioAnalyzerMessage::cERROR) // ERROR
            {
               QBrush redText(QColor(201, 51, 38));
               return QVariant(redText);
            }
            else
            {
               // Session note or other group item- use default palette for text
               return QVariant(QApplication::palette().text());
            }

         case Qt::UserRole + 1:
            return QVariant(item->width);
         case Qt::UserRole + 2:
            return QVariant("DEPRECATED");
         case Qt::UserRole + 3:
            return QVariant(item->expand);
      }
   }
   return QVariant();
}

// virtual
// only to be used for storing item's width, which will be accessed by the custom styled item delegate for sizeHint()
bool ScenarioAnalyzer::ExpandingResultsGroupModel::setData(const QModelIndex&  index, const QVariant& value, int role)
{
   if (role != Qt::UserRole + 1)
   {
      return false;
   }
   auto item = static_cast<ResultsGroupItem*>(index.internalPointer());
   if (item && value.canConvert<int>() && value.toInt() > 0)
   {
      item->width = value.toInt();
   }
   return false;
}

void ScenarioAnalyzer::ExpandingResultsGroupModel::refreshRoot()
{
   this->beginResetModel();
   this->endResetModel();
}

void ScenarioAnalyzer::ExpandingResultsGroupModel::clear()
{
   this->beginResetModel();
   this->root.children.clear();
   this->endResetModel();
}

void ScenarioAnalyzer::ExpandingResultsGroupModel::constructResultsTree(const std::vector<ScenarioAnalyzerMessage>& notes,
                                                                        const std::vector<ScenarioAnalyzerMessage>& results,
                                                                        bool& allowSetTime)
{
   clear();

   // Important NOTE: We are reserving the size of the children for each of these group items,
   //                 because we store pointers to to them. Because they are stored in a vector,
   //                 if we do not reserve the space upfront, the location of the data may move,
   //                 as std::vector must be stored in contiguous memory space.

   int numTypes = 2;
   root.expand = true;
   root.children.reserve(numTypes);

   root.children.push_back(ResultsGroupItem());
   ResultsGroupItem* allCheckResults = &root.children.back();
   allCheckResults->parent = &root;
   allCheckResults->children.reserve(static_cast<int>(results.size()));
   allCheckResults->name = QString("Check Results");
   allCheckResults->expand = true;

   root.children.push_back(ResultsGroupItem());
   ResultsGroupItem* allSessionNotes = &root.children.back();
   allSessionNotes->parent = &root;
   allSessionNotes->children.reserve(static_cast<int>(notes.size()));
   allSessionNotes->name = QString("Session Notes");
   allSessionNotes->expand = true;

   // Create items for results
   for (const auto& result : results)
   {
      QString suiteName = QString(result.GetSuiteName().c_str());
      ResultsGroupItem* suiteItem = allCheckResults->findChild(suiteName);

      // Create a new suiteItem if it doesn't already exist
      if (suiteItem == allCheckResults->children.end())
      {
         allCheckResults->children.push_back(ResultsGroupItem());
         suiteItem = &(allCheckResults->children.back());
         suiteItem->children.reserve(ScenarioAnalyzerMessage::cNUM_SEVERITIES);
         suiteItem->name = suiteName;
         suiteItem->parent = allCheckResults;
         suiteItem->expand = true;
         suiteItem->message = result;
      }

      // Rather than storing by raw severity level, we want to show
      // the result of the test- which isn't the same! We'll go ahead and change "note" to "pass",
      // since we know session notes are stored in a separate container.
      QString severityStr = QString(ScenarioAnalyzerMessage::SeverityString(result.GetSeverity()).c_str());
      if (severityStr == ScenarioAnalyzerMessage::SeverityString(ScenarioAnalyzerMessage::cNOTE).c_str())
      {
         severityStr = "PASS";
      }
      ResultsGroupItem* severityItem = suiteItem->findChild(severityStr);

      // Create a new severityItem if it doesn't already exist
      if (severityItem == suiteItem->children.end())
      {
         suiteItem->children.push_back(ResultsGroupItem());
         severityItem = &(suiteItem->children.back());
         severityItem->children.reserve(static_cast<int>(results.size()));
         severityItem->name = severityStr;
         severityItem->parent = suiteItem;
         severityItem->expand = true;
         severityItem->message = result;
      }

      QString checkName = QString(result.GetCheckName().c_str());
      ResultsGroupItem* checkNameItem = severityItem->findChild(checkName);

      // Create a new checkItem if it doesn't already exist
      if (checkNameItem == severityItem->children.end())
      {
         severityItem->children.push_back(ResultsGroupItem());
         checkNameItem = &(severityItem->children.back());
         checkNameItem->children.reserve(static_cast<int>(results.size()));
         checkNameItem->name = checkName;
         checkNameItem->parent = severityItem;
         checkNameItem->expand = true;
         checkNameItem->message = result;
      }

      // If not a simple PASS result, add the detailed message
      if (!result.IsSuccessfulResult())
      {
         checkNameItem->children.push_back(ResultsGroupItem());
         ResultsGroupItem* messageItem = &(checkNameItem->children.back());
         messageItem->children.reserve(static_cast<int>(results.size()));
         messageItem->name = result.GetDetails().c_str();
         messageItem->parent = checkNameItem;
         messageItem->children.clear();
         messageItem->message = result;
      }
   }

   // Create items for notes
   // For notes, suite name will be SESSION_NOTE
   // check name will be "category"
   // and details will hold details
   for (const auto& note : notes)
   {
      // Decide if we should give user the option to adjust time at which checks are run
      // TODO: Why on earth are we doing this here, and why is there no shared file for
      //       this category ? (If string changes in wsf_scenario_analyzer, it won't
      //       get updated here...)
      QString category = note.GetCheckName().c_str();
      if (category == "Missing Platforms")
      {
         allowSetTime = true;
      }

      ResultsGroupItem* categoryItem = allSessionNotes->findChild(category);
      if (categoryItem == allSessionNotes->children.end())
      {
         // create a new warning category for this warning message if necessary
         allSessionNotes->children.push_back(ResultsGroupItem());
         categoryItem = &(allSessionNotes->children.back());
         categoryItem->children.reserve(static_cast<int>(notes.size()));
         categoryItem->name = category;
         categoryItem->parent = allSessionNotes;
         categoryItem->message = note;
         // leave severityLevel blank
      }

      QString details = note.GetDetails().c_str();
      ResultsGroupItem* detailItem = categoryItem->findChild(details);
      if (detailItem == categoryItem->children.end())
      {
         categoryItem->children.push_back(ResultsGroupItem());
         detailItem = &(categoryItem->children.back());
         detailItem->name = details;
         detailItem->parent = categoryItem;
         detailItem->children.clear();
         detailItem->message = note;
      }
   }

   // once results tree is constructed, modify names of severity items for check results
   // to also display number of checks appearing in the category
   for (int i = 0, sz = allCheckResults->children.size(); i != sz; ++i)
   {
      ResultsGroupItem* suite = &(allCheckResults->children[i]);
      for (int j = 0, ch = suite->children.size(); j != ch; ++j)
      {
         ResultsGroupItem* severity = &(suite->children[j]);
         std::string nameAndCount = (severity->name).toLocal8Bit().constData();
         nameAndCount += " (" + std::to_string(severity->children.size()) + ")";
         severity->name = QString::fromLocal8Bit(nameAndCount.c_str());
      }
   }
}