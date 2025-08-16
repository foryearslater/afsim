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
#include "WkPlatformDataDockWidget.hpp"

#include <QIcon>
#include <QMenu>

#include "UtQtGL2DPlot.hpp"
#include "VaUtils.hpp"
#include "WkPlotUpdater.hpp"
#include "WkPlottingWidget.hpp"
#include "WkPlugin.hpp"
#include "WkfDefines.hpp"
#include "WkfEnvironment.hpp"
#include "WkfFactory.hpp"
#include "WkfMainWindow.hpp"
#include "WkfPluginManager.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitTypes.hpp"
#include "WkfViewer.hpp"
#include "WkfVtkEnvironment.hpp"
#include "data_ring/WkfRingDataDialog.hpp"

warlock::PlatformDataDockWidget::PlatformDataDockWidget(QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/)
   : wkf::PlatformDataDockWidget(parent, f)
   , mContextMenuItemPlugin(nullptr)
{
}

void warlock::PlatformDataDockWidget::PlotActionHandler()
{
   // When continuing to record plottingwidget data after exiting the window, need to check if data is already being
   // recorded for that platform before creating a new widget
   QMainWindow*  centralWidget = wkfEnv.GetMainWindow()->centralWidget();
   const QString plotType      = sender()->property("dataType").toString();

   // either matches the object name of existing widget or becomes title for new widget
   const QString newTitle = "Plot: " + mContextMenuItemId + " " + plotType;

   auto* plotWidget = centralWidget->findChild<PlottingWidget*>(newTitle);
   if (!plotWidget)
   {
      plotWidget = vaEnv.GetFactory()->CreateDockWidget<PlottingWidget>("PlottingWidget", centralWidget);
      assert(plotWidget);

      const QString xAxis = "Time";

      // Find the updater
      if (mContextMenuItemPlugin)
      {
         std::unique_ptr<warlock::PlotUpdater> updaterPtr =
            mContextMenuItemPlugin->GetPlotUpdater(mContextMenuItemId, plotType, mItemContext);
         if (updaterPtr != nullptr)
         {
            plotWidget->SetUpdater(std::move(updaterPtr));
         }
      }

      plotWidget->setWindowTitle(newTitle);
      plotWidget->GetPlot()->SetTitle(mContextMenuItemId + " " + plotType);
      plotWidget->GetPlot()->SetSeriesName(mContextMenuItemId + " " + plotType);
      plotWidget->SetBaseLabelXAxis(xAxis);
      plotWidget->SetBaseLabelYAxis(plotType);

      // assign object name to be checked for later
      plotWidget->setObjectName(newTitle);
   }

   plotWidget->show();
}

void warlock::PlatformDataDockWidget::AddRingActionHandler()
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario != nullptr)
   {
      wkf::Platform* platform = scenario->FindPlatformByName(mContextMenuItemId.toStdString());
      if (platform != nullptr)
      {
         vespa::VaViewer* viewer = vaEnv.GetStandardViewer();
         if (viewer != nullptr)
         {
            wkf::RingDataDialog* dialog   = new wkf::RingDataDialog(wkfEnv.GetMainWindow());
            QString              dataName = sender()->property("dataType").toString();

            // Initialize the maximum or minimum value to 200% of the current value.
            double value = sender()->property("initialValue").toDouble();
            value *= 2;
            value > 0 ? dialog->SetMaxValue(value) : dialog->SetMinValue(value);

            // Determine the unit to display in the dialog.
            QString unit;
            if (dataName == "Latitude" || dataName == "Longitude")
            {
               unit = " degrees";
            }
            else
            {
               auto updater = wkfEnv.GetUpdater(dataName, platform->GetName());
               if (updater != nullptr)
               {
                  unit = updater->GetValueString();
                  unit = unit.remove(QRegExp("\\W^\\/"));
                  unit = unit.remove(QRegExp("\\."));
                  unit = unit.remove(QRegExp("[0]"));
                  unit = unit.prepend(" ");
               }
            }
            dialog->SetUnit(unit);

            // Create a ring attachment when the dialog is accepted.
            connect(dialog,
                    &QDialog::accepted,
                    [&]()
                    {
                       wkf::AttachmentDataRing* attachment =
                          vespa::make_attachment<wkf::AttachmentDataRing>(*platform, viewer);

                       auto updater = wkfEnv.GetUpdater(dataName, platform->GetName());
                       if (updater != nullptr)
                       {
                          attachment->SetUpdater(std::move(updater));
                          attachment->SetLevel(platform->FindAttachmentsOfType<wkf::AttachmentDataRing>().size());
                          attachment->SetMinValue(dialog->GetMinValue());
                          attachment->SetMaxValue(dialog->GetMaxValue());
                          attachment->SetMinColor(dialog->GetMinColor());
                          attachment->SetMaxColor(dialog->GetMaxColor());
                          attachment->SetClampedToBase(dialog->GetClamped());
                       }
                    });

            dialog->exec();
         }
      }
   }
}

void warlock::PlatformDataDockWidget::RemoveRingActionHandler()
{
   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario != nullptr)
   {
      wkf::Platform* platform = scenario->FindPlatformByName(mContextMenuItemId.toStdString());
      if (platform != nullptr)
      {
         int removedRingLevel = 0;
         for (auto& ring : platform->FindAttachmentsOfType<wkf::AttachmentDataRing>())
         {
            if (ring->GetUpdater()->GetUpdaterName() == sender()->property("dataType").toString())
            {
               removedRingLevel = ring->GetLevel();
               platform->RemoveAttachment(ring->GetUniqueId());
            }
         }

         // Lower each remaining ring.
         if (removedRingLevel != 0)
         {
            for (auto& ring : platform->FindAttachmentsOfType<wkf::AttachmentDataRing>())
            {
               if (ring->GetLevel() > removedRingLevel)
               {
                  ring->SetLevel(ring->GetLevel() - 1);
               }
            }
         }
      }
   }
}

void warlock::PlatformDataDockWidget::BuildMenuForItem(QMenu& aMenu, QTreeWidgetItem& aItem)
{
   auto* wkItem = dynamic_cast<wkf::PlatformDataTreeItem*>(&aItem);

   if (wkItem != nullptr && wkItem->Plottable())
   {
      // Store the plugin responsible for the QTreeWidgetItem, so we can call it for a plot updater
      mContextMenuItemPlugin = dynamic_cast<Plugin*>(GetPluginFromItem(wkItem));
      // If the plugin did not provide an ID Field for the QTreeWidgetItem, return the platform name as the Identifier.
      mContextMenuItemId = wkItem->data(0, wkf::ID_ROLE).toString();
      if (mContextMenuItemId.isEmpty())
      {
         mContextMenuItemId = QString::fromStdString(mPlatformName);
      }

      QAction* action = new QAction(QIcon::fromTheme("plot"), "Plot " + aItem.text(0), &aMenu);
      if (aItem.parent() != nullptr && aItem.parent()->text(0) == "Aux Data")
      {
         mItemContext = 1;
      }
      else
      {
         mItemContext = 0;
      }

      action->setProperty("dataType", aItem.text(0));

      connect(action, &QAction::triggered, this, &PlatformDataDockWidget::PlotActionHandler);
      aMenu.addAction(action);

      AddRingItem(aMenu, aItem);
   }
}

void warlock::PlatformDataDockWidget::AddRingItem(QMenu& aMenu, QTreeWidgetItem& aItem)
{
   bool ringExists = false;

   wkf::Scenario* scenario = vaEnv.GetStandardScenario();
   if (scenario != nullptr)
   {
      wkf::Platform* platform = scenario->FindPlatformByName(mContextMenuItemId.toStdString());
      if (platform != nullptr)
      {
         for (const auto& ring : platform->FindAttachmentsOfType<wkf::AttachmentDataRing>())
         {
            wkf::Updater* updater = ring->GetUpdater();
            if (updater != nullptr)
            {
               if (updater->GetUpdaterName() == aItem.text(0))
               {
                  ringExists = true;
                  break;
               }
            }
         }
      }
   }

   if (ringExists)
   {
      QAction* ringAction = new QAction(QIcon::fromTheme("data_ring"), "Remove " + aItem.text(0) + " Ring", &aMenu);
      ringAction->setProperty("dataType", aItem.text(0));
      connect(ringAction, &QAction::triggered, this, &PlatformDataDockWidget::RemoveRingActionHandler);
      aMenu.addAction(ringAction);
   }
   else
   {
      QAction* ringAction = new QAction(QIcon::fromTheme("data_ring"), "Add " + aItem.text(0) + " Ring", &aMenu);
      ringAction->setProperty("dataType", aItem.text(0));

      double initialValue = 0;
      if (aItem.text(0) == "Latitude")
      {
         wkf::Platform* platform = scenario->FindPlatformByName(mContextMenuItemId.toStdString());
         if (platform != nullptr)
         {
            initialValue = platform->GetPosition().GetLat();
         }
      }
      else if (aItem.text(0) == "Longitude")
      {
         wkf::Platform* platform = scenario->FindPlatformByName(mContextMenuItemId.toStdString());
         if (platform != nullptr)
         {
            initialValue = platform->GetPosition().GetLon();
         }
      }
      else
      {
         QString valueStr = aItem.text(1).split(" ").first();
         initialValue     = valueStr.toDouble();
      }
      ringAction->setProperty("initialValue", initialValue);

      connect(ringAction, &QAction::triggered, this, &PlatformDataDockWidget::AddRingActionHandler);
      aMenu.addAction(ringAction);
   }
}
