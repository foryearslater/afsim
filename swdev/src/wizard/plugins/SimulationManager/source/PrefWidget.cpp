// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "PrefWidget.hpp"

#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QRadioButton>

#include "PrefObject.hpp"
#include "RunEnvManager.hpp"
#include "UtPath.hpp"
#include "UtProcess.hpp"
#include "Util.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

namespace
{
namespace AppDetailsTable
{
enum class Row
{
   name = 0,
   path,
   version,
   exe,
   date,
   profilingOut,
   cliParams,
   count // always last
};

enum class Col
{
   label = 0,
   value,
   count // always last
};
} // namespace AppDetailsTable

std::set<QString> GetColumnStrings(const int column, const QTableWidget* widget)
{
   std::set<QString> strings;
   for (int row = 0; true; ++row)
   {
      const auto* item = widget->item(row, column);
      if (!item)
      {
         return strings;
      }
      strings.insert(item->text());
   }
}

} // namespace

SimulationManager::PrefWidget::PrefWidget(QWidget* parent /*= nullptr*/)
   : PrefWidgetT<PrefObject>(parent)
   , mItemRemoveFlag(false)
{
   mUi.setupUi(this);
   mUi.simList->horizontalHeader()->setMaximumSectionSize(200);

   mUi.mAdd->setIcon(QIcon::fromTheme("add"));
   mUi.mRemove->setIcon(QIcon::fromTheme("remove"));

   connect(mUi.mAdd, &QPushButton::clicked, this, &PrefWidget::Add);
   connect(mUi.mRemove, &QPushButton::clicked, this, &PrefWidget::Remove);
   mUi.simList->setSelectionMode(QAbstractItemView::NoSelection);
}

void SimulationManager::PrefWidget::showEvent(QShowEvent* aEvent)
{
   // This is necessary because the pref widget isn't constructed until after settings are loaded into pref object
   ReadPreferenceData(mPrefObjectPtr->GetPreferences());
}

void SimulationManager::PrefWidget::ReadPreferenceData(const PrefData& aPrefData)
{
   mUi.simList->clear();
   mUi.simList->setRowCount(0);

   mUi.simDetails->clear();
   for (auto&& it : aPrefData.mSimList)
   {
      SimItem* item = new SimItem(mUi.simList,
                                  it.second.mName,
                                  it.second.mPath,
                                  it.second.mVersion,
                                  it.second.mExeName,
                                  it.second.mDate,
                                  it.second.mProfilingOutputPath,
                                  it.second.mExtraCommandLineArgs,
                                  it.second.mBuiltIn,
                                  it.second.mGood,
                                  it.second.mSupportsProfiling);
      connect(item->GetRadioButton(), &QRadioButton::toggled, [this, item](bool aChecked) {
         if (aChecked)
         {
            PopulateAppDetails(item);
         }
      });

      if ((it.second.mGood) && (it.first == aPrefData.mActiveSim))
      {
         item->GetRadioButton()->setChecked(true);
      }
   }
}

void SimulationManager::PrefWidget::WritePreferenceData(PrefData& aPrefData)
{
   aPrefData.mSimList.clear();

   int  c     = mUi.simList->rowCount();
   bool found = false;
   for (int i = 0; i < c; ++i)
   {
      SimItem* item                    = static_cast<SimItem*>(mUi.simList->item(i, 1));
      bool     active                  = item->GetRadioButton()->isEnabled();
      aPrefData.mSimList[item->Name()] = SimData(item->Path(),
                                                 item->Name(),
                                                 item->BuiltIn() ? "" : item->Version(),
                                                 item->ExeName(),
                                                 item->DateModified(),
                                                 item->ProfilingOutput(),
                                                 item->ExtraCLIArgs(),
                                                 active,
                                                 item->BuiltIn(),
                                                 item->SupportsProfiling());

      if (item->GetRadioButton()->isChecked())
      {
         aPrefData.mActiveSim = item->Name();
         found                = true;
      }
   }
   if (!found)
   {
      aPrefData.mActiveSim = "";
   }
}

void SimulationManager::PrefWidget::Add()
{
#ifdef _WIN32
   QString file = QFileDialog::getOpenFileName(this, "Choose AFSIM executable", QString(), "Executable (*.exe);;");
#else
   QString file = QFileDialog::getOpenFileName(this, "Choose AFSIM executable", QString(), "Executable (*);;");
#endif
   if (!file.isNull())
   {
      ut::optional<SimData> data = QueryExecutableIdeOutput(file);
      if (data.has_value())
      {
         QString alias = data->mName + " " + data->mVersion;
         alias         = GetPreferenceObject()->FindAvailableAlias(alias, GetColumnStrings(1, mUi.simList));
         SimItem* item = new SimItem(mUi.simList,
                                     alias,
                                     data->mPath,
                                     data->mVersion,
                                     data->mExeName,
                                     data->mDate,
                                     "",
                                     "",
                                     data->mBuiltIn,
                                     data->mGood,
                                     data->mSupportsProfiling);
         connect(item->GetRadioButton(), &QRadioButton::toggled, [this, item](bool aChecked) {
            if (aChecked)
            {
               PopulateAppDetails(item);
            }
         });
      }
      else
      {
         QMessageBox::warning(this, "Non-AFSIM executable", file + " does not appear to be an AFSIM executable.");
      }
   }
   // else user canceled open dialog.  Do nothing.
}

void SimulationManager::PrefWidget::Remove()
{
   mItemRemoveFlag = true;

   int row = mUi.simList->currentRow();
   if (row >= 0)
   {
      SimItem* item = dynamic_cast<SimItem*>(mUi.simList->item(row, 1));
      if (item)
      {
         if (item->BuiltIn())
         {
            mItemRemoveFlag = false;
            return;
         }
         if (item->GetRadioButton()->isChecked())
         {
            mUi.simDetails->clear();
         }
      }
      mUi.simList->removeRow(row);
   }

   mItemRemoveFlag = false;
}

SimulationManager::SimItem::SimItem(QTableWidget*      aParent,
                                    const QString&     aName,
                                    const QString&     aPath,
                                    const QString&     aVersion,
                                    const QString&     aExeName,
                                    const QString&     aDate,
                                    const QString&     aProfilingOutput,
                                    const QString&     aCliArgs,
                                    bool               aBuiltIn,
                                    bool               aGood,
                                    ut::optional<bool> aSupportsProfiling)
   : QTableWidgetItem(aName)
{
   const int r = aParent->rowCount();
   aParent->setRowCount(r + 1);
   mRadioButtonPtr = new QRadioButton(aParent);
   aParent->setCellWidget(r, 0, mRadioButtonPtr);
   aParent->setItem(r, 1, this);
   SetName(aName);
   SetPath(aPath);
   SetVersion(aVersion);
   SetBuiltIn(aBuiltIn);
   SetExeName(aExeName);
   SetDateModified(aDate);
   SetProfilingOutput(aProfilingOutput);
   SetExtraCLIArgs(aCliArgs);
   SetSupportsProfiling(aSupportsProfiling);

   UtPath p = UtPath::GetExePath();
   p.Up();
   p += aPath.toStdString();

   QString wizVer = wizard::RunEnvManager::GetVersionString(false).c_str();

   if (aBuiltIn)
   {
      setData(Qt::ToolTipRole, aName + '\n' + p.GetSystemPath().c_str());
      aParent->setItem(r, 2, new QTableWidgetItem(wizVer));
      SetVersion(wizVer);
   }
   else
   {
      setData(Qt::ToolTipRole, aName + '\n' + aPath);
      aParent->setItem(r, 2, new QTableWidgetItem(aVersion));
   }
   if (!aGood)
   {
      aParent->item(r, 1)->setTextColor(Qt::gray);
      aParent->item(r, 2)->setTextColor(Qt::gray);
      mRadioButtonPtr->setEnabled(false);
   }
   aParent->resizeColumnsToContents();
}

void SimulationManager::PrefWidget::PopulateAppDetails(SimItem* aItem)
{
   disconnect(mUi.simDetails, &QTableWidget::cellChanged, this, &PrefWidget::CellValueChanged);
   disconnect(mUi.simDetails,
              SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
              this,
              SLOT(DetailsItemDoubleClicked(QTableWidgetItem*)));

   using Row = AppDetailsTable::Row;
   using Col = AppDetailsTable::Col;

   // simDetails dialog box setup
   mUi.simDetails->setRowCount(static_cast<int>(Row::count));
   mUi.simDetails->setColumnCount(static_cast<int>(Col::count));
   mUi.simDetails->verticalHeader()->setVisible(false);
   mUi.simDetails->horizontalHeader()->setVisible(false);
   mUi.simDetails->horizontalHeader()->setStretchLastSection(true);

   // Left column setup with non-editable labels
   QTableWidgetItem* nameLabel = new QTableWidgetItem("Name");
   nameLabel->setFlags(nameLabel->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* pathLabel = new QTableWidgetItem("Path");
   pathLabel->setFlags(pathLabel->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* versionLabel = new QTableWidgetItem("Version");
   versionLabel->setFlags(versionLabel->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* exeLabel = new QTableWidgetItem("Filename");
   exeLabel->setFlags(exeLabel->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* dateLabel = new QTableWidgetItem("Date Modified");
   dateLabel->setFlags(dateLabel->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* profilingOutLabel = new QTableWidgetItem("Profiling Output Path");
   profilingOutLabel->setFlags(profilingOutLabel->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* commandLineParamLabel = new QTableWidgetItem("Extra arguments (Advanced)");
   commandLineParamLabel->setFlags(commandLineParamLabel->flags() & ~Qt::ItemIsEditable);

   // Add items to table
   mUi.simDetails->setItem(static_cast<int>(Row::name), static_cast<int>(Col::label), nameLabel);
   mUi.simDetails->setItem(static_cast<int>(Row::path), static_cast<int>(Col::label), pathLabel);
   mUi.simDetails->setItem(static_cast<int>(Row::version), static_cast<int>(Col::label), versionLabel);
   mUi.simDetails->setItem(static_cast<int>(Row::exe), static_cast<int>(Col::label), exeLabel);
   mUi.simDetails->setItem(static_cast<int>(Row::date), static_cast<int>(Col::label), dateLabel);
   mUi.simDetails->setItem(static_cast<int>(Row::profilingOut), static_cast<int>(Col::label), profilingOutLabel);
   mUi.simDetails->setItem(static_cast<int>(Row::cliParams), static_cast<int>(Col::label), commandLineParamLabel);

   // Gets the path of the executable
   UtPath p = UtPath::GetExePath();
   p.Up();
   p += aItem->Path().toStdString();
   QFileInfo* file = new QFileInfo(p.GetSystemPath().c_str());

   // Right column setup with data pertaining to the selected application
   QTableWidgetItem* nameItem = new QTableWidgetItem(aItem->Name());
   if (aItem->BuiltIn())
   {
      nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
   }

   // The following items are not editable
   QTableWidgetItem* pathItem = new QTableWidgetItem(file->absoluteFilePath());
   pathItem->setFlags(pathItem->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* versionItem = new QTableWidgetItem(aItem->Version());
   versionItem->setFlags(versionItem->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* exeItem = new QTableWidgetItem(file->fileName());
   exeItem->setFlags(exeItem->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* dateItem = new QTableWidgetItem(file->lastModified().toString());
   dateItem->setFlags(dateItem->flags() & ~Qt::ItemIsEditable);

   QTableWidgetItem* profilingOutItem  = new QTableWidgetItem(aItem->ProfilingOutput());
   const auto        supportsProfiling = aItem->SupportsProfiling();

   if (!supportsProfiling.has_value())
   {
      // We query this on setting read if it doesn't exist.  If we get here it's likely the executable
      // path is wrong.
      const QString msg("Profiling support is unknown.  Remove and re-add the executable.");
      profilingOutItem->setText(msg);
      profilingOutItem->setToolTip(msg);
      profilingOutItem->setFlags(profilingOutItem->flags() & ~Qt::ItemIsEditable);
   }
   else if (!*supportsProfiling)
   {
      // Check for support above built-in so built-ins without support can be label "not supported"
      const QString msg("Executable does not support profiling.");
      profilingOutItem->setText(msg);
      profilingOutItem->setToolTip(msg);
      profilingOutItem->setFlags(profilingOutItem->flags() & ~Qt::ItemIsEditable);
   }
   else if (aItem->BuiltIn())
   {
      const QString msg("Profiling unavailable for default executable. Add a new executable entry to enable.");
      profilingOutItem->setText(msg);
      profilingOutItem->setToolTip(msg);
      profilingOutItem->setFlags(profilingOutItem->flags() & ~Qt::ItemIsEditable);
   }
   else
   {
      connect(mUi.simDetails,
              SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
              SLOT(DetailsItemDoubleClicked(QTableWidgetItem*)));
   }


   QTableWidgetItem* cliParamsItem = new QTableWidgetItem(aItem->ExtraCLIArgs());


   // Add items to table
   mUi.simDetails->setItem(static_cast<int>(Row::name), static_cast<int>(Col::value), nameItem);
   mUi.simDetails->setItem(static_cast<int>(Row::path), static_cast<int>(Col::value), pathItem);
   mUi.simDetails->setItem(static_cast<int>(Row::version), static_cast<int>(Col::value), versionItem);
   mUi.simDetails->setItem(static_cast<int>(Row::exe), static_cast<int>(Col::value), exeItem);
   mUi.simDetails->setItem(static_cast<int>(Row::date), static_cast<int>(Col::value), dateItem);
   mUi.simDetails->setItem(static_cast<int>(Row::profilingOut), static_cast<int>(Col::value), profilingOutItem);
   mUi.simDetails->setItem(static_cast<int>(Row::cliParams), static_cast<int>(Col::value), cliParamsItem);


   // Resizes the table to fit its container
   mUi.simDetails->resizeColumnsToContents();

   // The only editable item in the simDetails table is the nameItem. This connects any change
   // to nameItem to a method, NameChanged, that updates the respective SimItem.
   connect(mUi.simDetails, &QTableWidget::cellChanged, this, &PrefWidget::CellValueChanged);
}

void SimulationManager::PrefWidget::DetailsItemDoubleClicked(QTableWidgetItem* item)
{
   if (item && item->row() == static_cast<int>(AppDetailsTable::Row::profilingOut) &&
       item->column() == static_cast<int>(AppDetailsTable::Col::value))
   {
      const QString selectedPath = QFileDialog::getSaveFileName(this, "Profiling output path");
      if (!selectedPath.isNull())
      {
         item->setText(selectedPath);
      }
      // else user canceled, do nothing.
   }
}

// Called when the name of the executable is changed in the details dialog.
void SimulationManager::PrefWidget::CellValueChanged(int row, int col)
{
   //**NOTE**
   // when an item is removed from the table CellValueChanged is called
   // itemRemoveFlag is true when an item is being removed from the table
   // if an item is being removed from the table we don't need to update the name
   // as that would cause the wrong items name to be updated.
   if (col == static_cast<int>(AppDetailsTable::Col::value) && !mItemRemoveFlag)
   {
      // Find the element in the simList whose value was just changed
      QString newValue = mUi.simDetails->item(row, col)->text();
      for (int i = 0; i < mUi.simList->rowCount(); ++i)
      {
         SimItem* item = dynamic_cast<SimItem*>(mUi.simList->item(i, 1));
         if (item && item->GetRadioButton()->isChecked())
         {
            if (row == static_cast<int>(AppDetailsTable::Row::name))
            {
               item->setText(newValue);
               item->SetName(newValue);
            }
            else if (row == static_cast<int>(AppDetailsTable::Row::profilingOut) && item->SupportsProfiling())
            {
               item->SetProfilingOutput(newValue);
            }
            else if (row == static_cast<int>(AppDetailsTable::Row::cliParams))
            {
               item->SetExtraCLIArgs(newValue);
            }
            mUi.simList->resizeColumnsToContents();
            break;
         }
      }
   }
}
