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

#include "VerticalMapOptionsDialog.hpp"

#include <fstream>

#include <QTreeWidget>

#include "OutputFilesWidget.hpp"
#include "OutputVariablesWidget.hpp"
#include "PlatformOptionsWidget.hpp"
#include "VerticalMapTargetRegionWidget.hpp"

SPLAT::VerticalMapOptionsDialog::VerticalMapOptionsDialog(QWidget* aParent)
   : PlotOptionsDialog(aParent)
{
   setWindowTitle("Vertical Map Options");
   Build();
}

void SPLAT::VerticalMapOptionsDialog::Build()
{
   // Note: By calling AddItem() on the QTreeWidgetItems, the mUi.treeWidget will take ownership

   QTreeWidgetItem* targetRegionItem = new QTreeWidgetItem();
   targetRegionItem->setText(0, "Target Region");
   targetRegionItem->setData(0, Qt::UserRole, "TargetRegion");
   AddItem(targetRegionItem);
   AddWidget("TargetRegion", new VerticalMapTargetRegionWidget(this));

   QTreeWidgetItem* outputSelectionItem = new QTreeWidgetItem();
   outputSelectionItem->setText(0, "Output Selection");
   outputSelectionItem->setData(0, Qt::UserRole, "OutputSelection");
   AddItem(outputSelectionItem);

   OutputVariablesWidget* variablesWidget       = new OutputVariablesWidget(this);
   QTreeWidgetItem*       variableSelectionItem = new QTreeWidgetItem();
   variableSelectionItem->setText(0, "Variable Selection");
   variableSelectionItem->setData(0, Qt::UserRole, "Variables");
   AddItem(variableSelectionItem, "", outputSelectionItem);
   AddWidget("Variables", variablesWidget);

   // this is so that when "Output Selection" is selected, the Variables Widget appears
   mWidgetIndexMap["OutputSelection"] = static_cast<int>(mWidgetVec.size() - 1);

   OutputFilesWidget* outputFilesWidget = new OutputFilesWidget(PlotOptionsWidget::PlotType::cVERTICAL_MAP, this);
   QTreeWidgetItem*   outputFilesItem   = new QTreeWidgetItem();
   outputFilesItem->setText(0, "Output Files");
   outputFilesItem->setData(0, Qt::UserRole, "OutputFiles");
   AddItem(outputFilesItem, "", outputSelectionItem);
   AddWidget("OutputFiles", outputFilesWidget);

   PlatformOptionsWidget* platformOptionsWidget =
      new PlatformOptionsWidget(PlotOptionsWidget::PlotType::cVERTICAL_MAP, this);
   QTreeWidgetItem* platformOptionsItem = new QTreeWidgetItem();
   platformOptionsItem->setText(0, "Platform Options");
   platformOptionsItem->setData(0, Qt::UserRole, "PlatformOptions");
   AddItem(platformOptionsItem);
   AddWidget("PlatformOptions", platformOptionsWidget);

   // when required_jamming_power is selected as a variable, certain options becomes available in the platformOptionsWidget
   connect(variablesWidget,
           &OutputVariablesWidget::RequiredJammingPowerPressed,
           platformOptionsWidget,
           &PlatformOptionsWidget::RequiredJammingPowerPressed);

   // when modes are updated in the proxy, update the platformOptionsWidget with the new modes
   connect(this, &VerticalMapOptionsDialog::UpdateModes, platformOptionsWidget, &PlatformOptionsWidget::UpdateModes);
   connect(this,
           &VerticalMapOptionsDialog::UpdatePlatformTypes,
           platformOptionsWidget,
           &PlatformOptionsWidget::UpdatePlatformTypes);
   connect(this, &VerticalMapOptionsDialog::CheckValidity, platformOptionsWidget, &PlatformOptionsWidget::CheckValidity);

   // when the name of the .plt file changes in the outputFilesWidget, store it
   connect(outputFilesWidget,
           &OutputFilesWidget::GnuplotFileChanged,
           this,
           [this](const QString& aFilename) { mGnuplotFilename = aFilename; });

   connect(this, &VerticalMapOptionsDialog::SetSensorId, outputFilesWidget, &OutputFilesWidget::SetSensorId);
}

void SPLAT::VerticalMapOptionsDialog::WriteData(std::ostream& aStream)
{
   if (aStream.good())
   {
      aStream << "\nvertical_map\n";

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

      aStream << "end_vertical_map\n";
   }
}
