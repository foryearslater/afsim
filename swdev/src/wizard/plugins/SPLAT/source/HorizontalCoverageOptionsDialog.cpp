// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "HorizontalCoverageOptionsDialog.hpp"

#include <fstream>

#include <QTreeWidget>

#include "HorizontalCoverageTargetRegionWidget.hpp"
#include "OutputFilesWidget.hpp"
#include "OutputVariablesWidget.hpp"
#include "PlatformOptionsWidget.hpp"

SPLAT::HorizontalCoverageOptionsDialog::HorizontalCoverageOptionsDialog(QWidget* aParent)
   : PlotOptionsDialog(aParent)
{
   setWindowTitle("Horizontal Coverage Options");
   Build();
}

void SPLAT::HorizontalCoverageOptionsDialog::Build()
{
   // Note: By calling AddItem() on the QTreeWidgetItems, the mUi.treeWidget will take ownership

   QTreeWidgetItem* targetRegionItem = new QTreeWidgetItem();
   targetRegionItem->setText(0, "Target Region");
   targetRegionItem->setData(0, Qt::UserRole, "TargetRegion");
   AddItem(targetRegionItem);
   AddWidget("TargetRegion", new HorizontalCoverageTargetRegionWidget(this));

   QTreeWidgetItem* outputSelectionItem = new QTreeWidgetItem();
   outputSelectionItem->setText(0, "Output Selection");
   outputSelectionItem->setData(0, Qt::UserRole, "OutputSelection");
   AddItem(outputSelectionItem);

   OutputFilesWidget* outputFilesWidget = new OutputFilesWidget(PlotOptionsWidget::PlotType::cHORIZONTAL_COVERAGE, this);
   QTreeWidgetItem*   outputFilesItem   = new QTreeWidgetItem();
   outputFilesItem->setText(0, "Output Files");
   outputFilesItem->setData(0, Qt::UserRole, "OutputFiles");
   AddItem(outputFilesItem, "", outputSelectionItem);
   AddWidget("OutputFiles", outputFilesWidget);

   // this is so that when "Output Selection" is selected, the Output Files Widget appears
   mWidgetIndexMap["OutputSelection"] = static_cast<int>(mWidgetVec.size() - 1);

   PlatformOptionsWidget* platformOptionsWidget =
      new PlatformOptionsWidget(PlotOptionsWidget::PlotType::cHORIZONTAL_COVERAGE, this);
   QTreeWidgetItem* platformOptionsItem = new QTreeWidgetItem();
   platformOptionsItem->setText(0, "Platform Options");
   platformOptionsItem->setData(0, Qt::UserRole, "PlatformOptions");
   AddItem(platformOptionsItem);
   AddWidget("PlatformOptions", platformOptionsWidget);

   // when modes are updated in the proxy, update the platformOptionsWidget with the new modes
   connect(this, &HorizontalCoverageOptionsDialog::UpdateModes, platformOptionsWidget, &PlatformOptionsWidget::UpdateModes);
   connect(this,
           &HorizontalCoverageOptionsDialog::UpdatePlatformTypes,
           platformOptionsWidget,
           &PlatformOptionsWidget::UpdatePlatformTypes);
   connect(this, &HorizontalCoverageOptionsDialog::CheckValidity, platformOptionsWidget, &PlatformOptionsWidget::CheckValidity);

   // when the name of the .plt file changes in the outputFilesWidget, store it
   connect(outputFilesWidget,
           &OutputFilesWidget::OutputFileChanged,
           this,
           [this](const QString& aFilename) { mOutputFilename = aFilename; });

   connect(this, &HorizontalCoverageOptionsDialog::SetSensorId, outputFilesWidget, &OutputFilesWidget::SetSensorId);
}

void SPLAT::HorizontalCoverageOptionsDialog::WriteData(std::ostream& aStream)
{
   if (aStream.good())
   {
      aStream << "\nhorizontal_coverage\n";

      // Target Platform Type
      if (!mTargetPlatformType.isEmpty())
      {
         aStream << "   target_platform_type " << mTargetPlatformType.toStdString() << "\n";
      }

      // for all Widgets in the Dialog, write their data.
      for (auto widget : mWidgetVec)
      {
         widget->WriteData(aStream);
      }

      aStream << "end_horizontal_coverage\n";
   }
}
