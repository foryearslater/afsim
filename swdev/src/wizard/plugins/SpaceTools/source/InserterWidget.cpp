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

#include "InserterWidget.hpp"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QStringList>

#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "SatelliteInserterModel.hpp"
#include "SatelliteInserterPrefWidget.hpp"

// FilterModel Functions
//----------------------------------------------------------------------------
SpaceTools::FilterModel::FilterModel(QObject* aParentPtr, int aColumn, int aType, const QVariant& aValue)
   : QSortFilterProxyModel(aParentPtr)
   , mColumn(aColumn)
   , mType(aType)
   , mValue(aValue)
   , mTypeFiltered(false)
{
}

SpaceTools::FilterModel::FilterModel(QObject* aParentPtr)
   : QSortFilterProxyModel(aParentPtr)
   , mColumn(-1)
   , mType(0)
   , mValue(QVariant())
   , mTypeFiltered(false)
{
}

//! Decides if the row fits the current filter
//!
//! @param aSourceRow The row that is being checked
//! @param aSourceParent Not used
//! @returns True if the data fits the filter, false otherwise.
bool SpaceTools::FilterModel::filterAcceptsRow(int aSourceRow, const QModelIndex& aSourceParent) const
{
   bool         accepted = true;
   QVariant     left     = sourceModel()->data(sourceModel()->index(aSourceRow, mColumn), Qt::DisplayRole);
   unsigned int pIdx     = sourceModel()->data(sourceModel()->index(aSourceRow, mColumn), Qt::UserRole).toInt();
   if (!Compare(left, mType, mValue, pIdx))
   {
      accepted = false;
   }
   return accepted;
}

bool SpaceTools::FilterModel::lessThan(const QModelIndex& aLeftSource, const QModelIndex& aRightSource) const
{
   QString leftString  = sourceModel()->data(aLeftSource).toString();
   QString rightString = sourceModel()->data(aRightSource).toString();

   return QString::localeAwareCompare(leftString, rightString) < 0;
}

//! Gets the header data for the table view
//!
//! @param aSection The section of data desired
//! @param aOrientation The orientation of the header
//! @param aRole The role of the data
//! @returns The name of the header for the section or section number if name is undefined.
QVariant SpaceTools::FilterModel::headerData(int aSection, Qt::Orientation aOrientation, int aRole) const
{
   QVariant retVal;
   if (mTypeFiltered && (aOrientation == Qt::Horizontal) && (aRole == Qt::DisplayRole))
   {
      retVal = data(index(0, aSection), Qt::ToolTipRole);
   }
   else
   {
      retVal = QSortFilterProxyModel::headerData(aSection, aOrientation, aRole);
   }
   return retVal;
}

bool SpaceTools::FilterModel::Compare(const QVariant& aLeft,
                                      int             aCompType,
                                      const QVariant& aRight,
                                      unsigned int    aPlatformIndex) const
{
   bool compare = true;
   switch (aCompType)
   {
   case (SpaceTools::InserterWidget::FilterType::cEQUAL):
      compare = aLeft == aRight;
      break;
   case (SpaceTools::InserterWidget::FilterType::cNOT_EQUAL):
      compare = aLeft != aRight;
      break;
   case (SpaceTools::InserterWidget::FilterType::cCONTAINS):
      compare = aLeft.toString().toLower().contains(aRight.toString().toLower());
      break;
   }
   return compare;
}


// InserterWidget Functions
//----------------------------------------------------------------------------
SpaceTools::InserterWidget::InserterWidget(QWidget* aParentPtr, PrefObject* aPrefObjectPtr)
   : QWidget(aParentPtr)
   , mIdManager(0)
   , mPrefObjectPtr(aPrefObjectPtr)
   , mHandlerPtr(ut::make_unique<SatelliteInserterHandler>())
{
   mUI.setupUi(this);

   mUI.mRuleWidget->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(mUI.mRuleWidget, &QWidget::customContextMenuRequested, this, &SpaceTools::InserterWidget::RuleContextMenu);

   QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
   sizePolicy1.setHorizontalStretch(0);
   sizePolicy1.setVerticalStretch(1);
   sizePolicy1.setHeightForWidth(mUI.mRuleWidget->sizePolicy().hasHeightForWidth());
   mUI.mRuleWidget->setSizePolicy(sizePolicy1);

   mUI.mSatelliteView->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(mUI.mSatelliteView, &QWidget::customContextMenuRequested, this, &SpaceTools::InserterWidget::TableItemContextMenu);

   QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
   sizePolicy2.setHorizontalStretch(0);
   sizePolicy2.setVerticalStretch(3);
   sizePolicy2.setHeightForWidth(mUI.mSatelliteView->sizePolicy().hasHeightForWidth());
   mUI.mSatelliteView->setSizePolicy(sizePolicy2);

   connect(mUI.mInsertButton, &QPushButton::clicked, [this]() { InsertSatellites(); });
   connect(mPrefObjectPtr, &PrefObject::UpdateTable, this, &InserterWidget::UpdateTable);
   connect(mUI.mOptionsButton, &QPushButton::clicked, this, &InserterWidget::InsertOptions);

   connect(mUI.mSatelliteView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &InserterWidget::Sort);
   mUI.mSatelliteView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

   SetModel(new SatelliteInserterModel(mPrefObjectPtr->GetCheckedDatabases()));

   mUI.mSatelliteView->setColumnHidden(SatelliteInserterModel::FieldColumn::cFILE, true);
}

void SpaceTools::InserterWidget::OnShow()
{
   SetInsertFile();
   DisableInsertButton();
}

void SpaceTools::InserterWidget::AddFilterRule(QListWidgetItem* aItemPtr)
{
   unsigned int id   = mIdManager++;
   int          col  = aItemPtr->data(Qt::UserRole).toInt();
   int          comp = aItemPtr->data(Qt::UserRole + 1).toInt();
   QVariant     val  = aItemPtr->data(Qt::UserRole + 2);
   aItemPtr->setData(Qt::UserRole + 3, id);

   SpaceTools::FilterModel* modelPtr = new SpaceTools::FilterModel(this, col, comp, val);
   if (comp == SpaceTools::InserterWidget::FilterType::cEQUAL)
   {
      if (mUI.mSatelliteView->model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString() == "type")
      {
         modelPtr->SetTypeFiltered(true);
      }
   }
   modelPtr->setSourceModel(mUI.mSatelliteView->model());
   SetModel(modelPtr);
   mUI.mRuleWidget->addItem(aItemPtr);
   mFilterModels[id] = modelPtr;
}

void SpaceTools::InserterWidget::DisableInsertButton()
{
   ClearSelection();
   mUI.mInsertButton->setEnabled(false);
}

void SpaceTools::InserterWidget::ClearRules()
{
   SetModel(mSourceModelPtr);
   for (auto&& it : mFilterModels)
   {
      delete it.second;
   }
   mFilterModels.clear();
   mUI.mRuleWidget->clear();
   mUI.mSatelliteView->setSortingEnabled(false);
   ClearSelection();
}

void SpaceTools::InserterWidget::ClearSelection()
{
   mUI.mSatelliteView->clearSelection();
}

void SpaceTools::InserterWidget::CreateFilter(const QVariant& aData, int aColumn, int aFilterType)
{
   if (aFilterType == SpaceTools::InserterWidget::FilterType::cUNSET ||
       aFilterType == SpaceTools::InserterWidget::FilterType::cCONTAINS)
   {
      int t = aFilterType;
      if (aFilterType == SpaceTools::InserterWidget::FilterType::cUNSET)
      {
         t = SpaceTools::InserterWidget::FilterType::cEQUAL;
      }
      QVariant d = aData;

      if (QDialog::Accepted == GetFilter(aColumn, t, d))
      {
         QListWidgetItem* itemPtr = new QListWidgetItem(GetRuleText(aColumn, t, d), mUI.mRuleWidget);
         itemPtr->setData(Qt::UserRole, aColumn);
         itemPtr->setData(Qt::UserRole + 1, t);
         itemPtr->setData(Qt::UserRole + 2, d);
         AddFilterRule(itemPtr);
      }
   }
   else
   {
      QListWidgetItem* itemPtr  = new QListWidgetItem(GetRuleText(aColumn, aFilterType, aData), mUI.mRuleWidget);
      int              typeEnum = aFilterType;

      itemPtr->setData(Qt::UserRole, aColumn);
      itemPtr->setData(Qt::UserRole + 1, typeEnum);
      itemPtr->setData(Qt::UserRole + 2, aData);
      AddFilterRule(itemPtr);
   }
}

void SpaceTools::InserterWidget::EditRule(QListWidgetItem* aItemPtr)
{
   unsigned int             id      = aItemPtr->data(Qt::UserRole + 3).toUInt();
   int                      col     = aItemPtr->data(Qt::UserRole).toInt();
   int                      comp    = aItemPtr->data(Qt::UserRole + 1).toInt();
   QVariant                 val     = aItemPtr->data(Qt::UserRole + 2);
   SpaceTools::FilterModel* rulePtr = mFilterModels[id];
   if (GetFilter(col, comp, val) == QDialog::Accepted)
   {
      rulePtr->SetComparatorAndValue(comp, val);
      aItemPtr->setText(GetRuleText(col, comp, val));
      aItemPtr->setData(Qt::UserRole + 1, comp);
      aItemPtr->setData(Qt::UserRole + 2, val);
   }
   EnableInsert(QItemSelection(), QItemSelection());
}

//! Enable the insert insert button
void SpaceTools::InserterWidget::EnableInsert(const QItemSelection& aSelected, const QItemSelection& aDeselected)
{
   mUI.mInsertButton->setEnabled(!mUI.mSatelliteView->selectionModel()->selectedRows().empty());
}

QDialog::DialogCode SpaceTools::InserterWidget::GetFilter(const int aColumn, int& aFilterType, QVariant& aValue)
{
   QDialog rule;
   rule.setWindowTitle("Filter rule");
   rule.setLayout(new QVBoxLayout(&rule));
   rule.layout()->addWidget(
      new QLabel("Show if value in column " + QString::number(aColumn + 1) + " " + GetTypeText(aFilterType) + ":"));
   QLineEdit* lePtr = new QLineEdit(&rule);
   lePtr->setText(aValue.toString());
   rule.layout()->addWidget(lePtr);
   QDialogButtonBox* bbPtr = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &rule);
   rule.layout()->addWidget(bbPtr);
   connect(bbPtr, &QDialogButtonBox::accepted, &rule, &QDialog::accept);
   connect(bbPtr, &QDialogButtonBox::rejected, &rule, &QDialog::reject);
   QDialog::DialogCode retval = (QDialog::DialogCode)rule.exec();
   if (retval == QDialog::Accepted)
   {
      aValue = lePtr->text();
   }
   return retval;
}

QString SpaceTools::InserterWidget::GetRuleText(const int aColumn, const int aFilterType, const QVariant& aValue)
{
   return "C[" + QString::number(aColumn + 1) + "] " + GetTypeText(aFilterType) + " " + aValue.toString();
}

QString SpaceTools::InserterWidget::GetTypeText(int aFilterType)
{
   QString typeText;
   switch (aFilterType)
   {
   case (SpaceTools::InserterWidget::FilterType::cEQUAL):
      typeText = "=";
      break;
   case (SpaceTools::InserterWidget::FilterType::cNOT_EQUAL):
      typeText = "!=";
      break;
   case (SpaceTools::InserterWidget::FilterType::cCONTAINS):
      typeText = "contains";
      break;
   default:
      typeText = "?";
      break;
   }
   return typeText;
}

void SpaceTools::InserterWidget::InsertOptions()
{
   QFileDialog fd(this);
   fd.setFileMode(QFileDialog::AnyFile);
   fd.setNameFilter(QString("Text (*.txt);; All Files(*.*)"));
   if (fd.exec())
   {
      SetInsertFile(fd.selectedFiles().at(0));
   }
}

void SpaceTools::InserterWidget::InsertSatellites()
{
   mUI.mInsertButton->setEnabled(false); // disable button to wait for proxy to update. will be enabled on selection change
   mHandlerPtr->InsertSatellites(mUI.mSatelliteView->model(), mUI.mSatelliteView->selectionModel()->selectedRows());
}

void SpaceTools::InserterWidget::LoadFilter()
{
   QDialog filterChooser;
   filterChooser.setWindowTitle("Load Filter");
   filterChooser.setLayout(new QVBoxLayout());

   FilterListWidget* lwPtr = new FilterListWidget(&filterChooser, mPrefObjectPtr);
   filterChooser.layout()->addWidget(lwPtr);
   QDialogButtonBox* bbPtr =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &filterChooser);
   filterChooser.layout()->addWidget(bbPtr);
   connect(bbPtr, &QDialogButtonBox::accepted, &filterChooser, &QDialog::accept);
   connect(bbPtr, &QDialogButtonBox::rejected, &filterChooser, &QDialog::reject);
   int result = filterChooser.exec();
   if (result == QDialog::Accepted)
   {
      auto* current = lwPtr->currentItem();
      if (current && !current->text().isEmpty())
      {
         auto& filters = mPrefObjectPtr->GetSavedFilters();
         auto  it      = filters.find(current->text());
         if (it != filters.end())
         {
            LoadFilter(it->second);
         }
      }
   }
}

void SpaceTools::InserterWidget::LoadFilter(const std::vector<PrefData::Filter>& aFilter)
{
   ClearRules();
   for (auto&& it : aFilter)
   {
      QListWidgetItem* itemPtr = new QListWidgetItem(GetRuleText(it.mColumn, it.mComp, it.mVar), mUI.mRuleWidget);
      itemPtr->setData(Qt::UserRole, it.mColumn);
      itemPtr->setData(Qt::UserRole + 1, it.mComp);
      itemPtr->setData(Qt::UserRole + 2, it.mVar);
      AddFilterRule(itemPtr);
   }
}

void SpaceTools::InserterWidget::RemoveRule(QListWidgetItem* aItemPtr)
{
   unsigned int             id           = aItemPtr->data(Qt::UserRole + 3).toUInt();
   SpaceTools::FilterModel* rulePtr      = mFilterModels[id];
   QAbstractItemModel*      childRulePtr = rulePtr->sourceModel();
   // for every filtered rule, if its source if 'rule', set it's source to 'childRule'
   bool found = false;
   for (auto&& it : mFilterModels)
   {
      if (it.second->sourceModel() == rulePtr)
      {
         it.second->setSourceModel(childRulePtr);
         found = true;
      }
   }
   if (!found) // if it wasn't attached to another rule, it must be attached to the table
   {
      SetModel(childRulePtr);
   }
   delete rulePtr;
   mFilterModels.erase(id);
   delete aItemPtr;
   DisableInsertButton();
}

void SpaceTools::InserterWidget::RuleContextMenu(const QPoint& aPoint)
{
   QListWidgetItem* itemPtr = mUI.mRuleWidget->itemAt(aPoint);
   QMenu*           menuPtr = new QMenu(this);
   if (itemPtr != nullptr)
   {
      QAction* removeRule = new QAction("Remove Rule", menuPtr);
      connect(removeRule, &QAction::triggered, [this, itemPtr]() { RemoveRule(itemPtr); });
      menuPtr->addAction(removeRule);
      QAction* editRule = new QAction("Edit Rule", menuPtr);
      connect(editRule, &QAction::triggered, [this, itemPtr]() { EditRule(itemPtr); });
      menuPtr->addAction(editRule);
   }
   QAction* saveFilterPtr = new QAction("Save Filter", menuPtr);
   connect(saveFilterPtr, &QAction::triggered, [this]() { SaveFilter(); });
   menuPtr->addAction(saveFilterPtr);
   QAction* loadFilterPtr = new QAction("Load Filter", menuPtr);
   connect(loadFilterPtr, &QAction::triggered, [this]() { LoadFilter(); });
   menuPtr->addAction(loadFilterPtr);
   QAction* clearRulesPtr = new QAction("Clear Rules", menuPtr);
   connect(clearRulesPtr, &QAction::triggered, [this]() { ClearRules(); });
   menuPtr->addAction(clearRulesPtr);
   menuPtr->exec(mUI.mRuleWidget->mapToGlobal(aPoint));
}

void SpaceTools::InserterWidget::SaveFilter()
{
   QString filterName = QInputDialog::getText(this, "Input Filter Name", "Name");
   if (!filterName.isEmpty())
   {
      std::vector<PrefData::Filter> filter;

      std::transform(mFilterModels.begin(),
                     mFilterModels.end(),
                     std::back_inserter(filter),
                     [](std::pair<int, FilterModel*> it)
                     { return PrefData::Filter(it.second->GetColumn(), it.second->GetType(), it.second->GetValue()); });

      mPrefObjectPtr->SaveFilter(filterName, filter);
   }
}

//! Sets the view's model to the given model pointer.
//!
//! @param aModelPtr The model to set
//! @param aSetSource Bool on whether the model needs to be set as the source or not.
void SpaceTools::InserterWidget::SetModel(QAbstractItemModel* aModelPtr, bool aSetSource)
{
   if (aSetSource)
   {
      mSourceModelPtr = aModelPtr;
   }
   mUI.mSatelliteView->setModel(aModelPtr);
   DisableInsertButton();
   connect(mUI.mSatelliteView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &InserterWidget::EnableInsert);
}

void SpaceTools::InserterWidget::SetInsertFile(const QString& aFile, bool aReset)
{
   if (aReset)
   {
      mHandlerPtr->SetInsertFile("");
   }
   else
   {
      mHandlerPtr->SetInsertFile(aFile);
   }
   UtPath wd(wizard::Project::Instance()->WorkingDirectory());
   auto   relPath = wd.GetRelativePath(mHandlerPtr->GetInsertFile().toStdString()).GetNormalizedPath();
   mUI.mFileLabel->setText(QString::fromStdString(relPath));
}

void SpaceTools::InserterWidget::TableItemContextMenu(const QPoint& aPoint)
{
   QModelIndex idx  = mUI.mSatelliteView->indexAt(aPoint);
   QVariant    data = mUI.mSatelliteView->model()->data(idx, Qt::DisplayRole);
   int         col  = idx.column();

   QMenu*              menuPtr         = new QMenu(this);
   QAbstractItemModel* activeFilterPtr = mUI.mSatelliteView->model();
   QString             column          = activeFilterPtr->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
   QString             value           = activeFilterPtr->data(idx, Qt::DisplayRole).toString();
   QAction*            showOnlyPtr     = new QAction("Show when column: " + column + " = " + value, menuPtr);
   connect(showOnlyPtr,
           &QAction::triggered,
           [=]() { CreateFilter(data, col, SpaceTools::InserterWidget::FilterType::cEQUAL); });
   menuPtr->addAction(showOnlyPtr);
   QAction* hideAllPtr = new QAction("Hide when column: " + column + " = " + value, menuPtr);
   connect(hideAllPtr,
           &QAction::triggered,
           [=]() { CreateFilter(data, col, SpaceTools::InserterWidget::FilterType::cNOT_EQUAL); });
   menuPtr->addAction(hideAllPtr);
   QAction* filterActionPtr = new QAction("Create \"Contains\" Filter", menuPtr);
   connect(filterActionPtr,
           &QAction::triggered,
           [=]() { CreateFilter(data, col, SpaceTools::InserterWidget::FilterType::cCONTAINS); });
   menuPtr->addAction(filterActionPtr);
   menuPtr->exec(mUI.mSatelliteView->mapToGlobal(aPoint));
}

//! Updates the table with the new model
void SpaceTools::InserterWidget::UpdateTable()
{
   SetModel(new SatelliteInserterModel(mPrefObjectPtr->GetCheckedDatabases()), true);
   ClearRules();
}

void SpaceTools::InserterWidget::Sort(int aColumn, Qt::SortOrder aSortOrder)
{
   auto modelPtr = mFilterModels[mIdManager];
   if (modelPtr == nullptr)
   {
      mUI.mSatelliteView->setSortingEnabled(true);
      modelPtr = new FilterModel(this);
      modelPtr->setSourceModel(mUI.mSatelliteView->model());
      modelPtr->sort(aColumn, aSortOrder);
      SetModel(modelPtr);
   }
}
