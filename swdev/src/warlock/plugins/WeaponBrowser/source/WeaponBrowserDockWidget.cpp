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

#include "WeaponBrowserDockWidget.hpp"

#include "UtMemory.hpp"
#include "WeaponBrowserSimCommands.hpp"
#include "WkPermissions.hpp"
#include "WkfEnvironment.hpp"
#include "WkfScenario.hpp"
#include "WkfUnitsObject.hpp"
#include "WkfVtkEnvironment.hpp"

WkWeaponBrowser::WeaponCommandWidget::WeaponCommandWidget(QWidget* aParent)
   : QWidget(aParent)
   , mTrackButton(new wkf::TrackSelector)
   , mTrackEditor(new QLineEdit)
   , mQuantityEditor(new QLineEdit)
   , mFireButton(new QPushButton("Fire"))
   , mDeleteButton(new QPushButton("X"))
   , mTrackLabel(new QLabel("Track:"))
   , mQuantityLabel(new QLabel("Quantity:"))
{
   mTrackButton->setIconSize(QSize(16, 16));
   mTrackEditor->setStyleSheet("border: 1px solid gray");
   mQuantityEditor->setStyleSheet("border: 1px solid gray");
   mQuantityLabel->setFixedWidth(mQuantityLabel->fontMetrics().width(mQuantityLabel->text()));
   mTrackLabel->setFixedWidth(mTrackLabel->fontMetrics().width(mTrackLabel->text()));
   mFireButton->setFixedWidth(36);
   mDeleteButton->setFixedWidth(3 * mDeleteButton->fontMetrics().width("X"));
   mTrackEditor->setMinimumWidth(20);
   mQuantityEditor->setMinimumWidth(30);

   mFireButton->setToolTip("Issues a Firing command.");
   mDeleteButton->setToolTip("Removes the target.");

   mTrackEditor->setValidator(new QIntValidator(1, std::numeric_limits<int>::max(), this));
   mQuantityEditor->setValidator(new QIntValidator(1, std::numeric_limits<int>::max(), this));

   QHBoxLayout* cellLayout = new QHBoxLayout;
   cellLayout->addWidget(mTrackLabel);
   cellLayout->addWidget(mTrackEditor);
   cellLayout->addWidget(mTrackButton);
   cellLayout->addWidget(mQuantityLabel);
   cellLayout->addWidget(mQuantityEditor);
   cellLayout->addWidget(mFireButton);
   cellLayout->addWidget(mDeleteButton);
   cellLayout->setContentsMargins(0, 0, 0, 0);
   setLayout(cellLayout);

   // default palette for this widget
   mDefaultEditStyle = mQuantityEditor->palette();
}

WkWeaponBrowser::DockWidget::DockWidget(SimInterface*   aSimInterface,
                                        DataContainer&  aDataContainer,
                                        QWidget*        aParent,
                                        Qt::WindowFlags aFlags)
   : QDockWidget(aParent, aFlags)
   , mDataContainer(aDataContainer)
   , mSimInterface(aSimInterface)
{
   mUi.setupUi(this);
   connect(&wkfEnv, &wkf::Environment::PlatformOfInterestChanged, this, &DockWidget::PlatformOfInterestChanged);
   connect(&mDataContainer, &DataContainer::RelevantDataChanged, this, &DockWidget::Update);
   connect(wkfEnv.GetPreferenceObject<wkf::UnitsObject>(),
           &wkf::UnitsObject::LengthUnitChanged,
           this,
           &DockWidget::LengthUnitChanged);
   Update();
}

// Update() is re-called in other places to maintain data encapsulation (no reference to widget data). It also resets
// the geometry of the widget
void WkWeaponBrowser::DockWidget::Update()
{
   QObject::disconnect(mExpandConnection);
   QObject::disconnect(mCollapseConnection);
   mUi.mTreeWidget->clear();
   std::vector<WeaponMapData> displayData = mDataContainer.GetWidgetData();
   mUi.mTreeWidget->setHeaderLabel(QString::fromStdString(mSimInterface->GetPlatformOfInterest()));
   mUi.mTreeWidget->headerItem()->setText(1, "");
   mUi.mTreeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
   mUi.mTreeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeMode::Fixed);
   mUi.mTreeWidget->header()->resizeSection(1, 50);
   mUi.mTreeWidget->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

   // iterate over all weapon types for this platform
   for (const auto& typeIt : displayData)
   {
      QString weaponStr =
         QString::fromStdString(typeIt.mName) + "   (remaining: " + QString::number(typeIt.mQuantityRemaining);

      // Add the No-Escape distance to the output string
      if (typeIt.mNoEscapeDistance.has_value())
      {
         // Change the value to the default of meters
         auto value    = typeIt.mNoEscapeDistance.value() * UtMath::cM_PER_FT;
         auto noEscape = wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetFormattedValue(wkf::ValueType::cLENGTH, value);
         weaponStr += ", No Escape: " + noEscape;
      }

      // Add the No-Maneuver distance to the output string
      if (typeIt.mNoManeuverDistance.has_value())
      {
         // Change the value to the default of meters
         auto value = typeIt.mNoManeuverDistance.value() * UtMath::cM_PER_FT;
         auto noManeuverStr =
            wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetFormattedValue(wkf::ValueType::cLENGTH, value);
         weaponStr += ", No Maneuver: " + noManeuverStr;
      }

      weaponStr += ")";
      QTreeWidgetItem* weapon = new QTreeWidgetItem(QStringList(weaponStr));
      weapon->setToolTip(0, "Weapon Type: " + QString::fromStdString(typeIt.mType));
      QFrame*          groupFrame  = new QFrame(mUi.mTreeWidget);
      QVBoxLayout*     verticalBox = new QVBoxLayout();
      QTreeWidgetItem* commandItem = new QTreeWidgetItem();
      mUi.mTreeWidget->addTopLevelItem(weapon);

      const std::vector<FiringData>& data = typeIt.mWeaponCommands;
      for (unsigned int i = 0; i < data.size(); ++i)
      {
         AddCommand(verticalBox, i, typeIt.mName);
      }

      AddPlusButton(weapon, typeIt.mName, data);
      verticalBox->setContentsMargins(0, 0, 8, 0);
      groupFrame->setLayout(verticalBox);
      weapon->addChild(commandItem);
      mUi.mTreeWidget->setItemWidget(commandItem, 0, groupFrame);

      // Expand/collapse the drop-down menu
      if (typeIt.mExpand)
      {
         mUi.mTreeWidget->expandItem(weapon);
      }
      else
      {
         mUi.mTreeWidget->collapseItem(weapon);
      }
      // Palette will not get applied if this code excerpt is used from the AddCommand function (does the default theme
      // for widgets get applied after? Maybe when widget is set in the treeWidget?) Handles firing button validity and
      // text colors when the widget is populated through the selection of a platform
      QList<WeaponCommandWidget*> commands = groupFrame->findChildren<WeaponCommandWidget*>();
      for (const auto& it : commands)
      {
         if (it->mTrackEditor->text().toInt() == 0)
         {
            it->mFireButton->setEnabled(false);
         }
         if (it->mQuantityEditor->text().toDouble() == 0)
         {
            it->mFireButton->setEnabled(false);
         }
         else if (typeIt.mQuantityRemaining < it->mQuantityEditor->text().toDouble())
         {
            QPalette palette = it->mQuantityEditor->palette();
            palette.setColor(QPalette::Text, QColor(Qt::red));
            it->mQuantityEditor->setPalette(palette);
            it->mFireButton->setEnabled(false);
         }
      }
   }

   // Handle expansion/collapsing of drop-down menus by saving state and reopening when the platform is reselected based
   // on the previous state
   const std::string& platformName    = mDataContainer.GetPlatformOfInterest();
   auto               handleExpansion = [&, platformName, displayData](QTreeWidgetItem* aItem, bool aExpand)
   {
      auto weaponName = aItem->text(0).split(" ").at(0).toStdString();
      mDataContainer.MenuExpansionUpdate(platformName, weaponName, aExpand);
   };

   mCollapseConnection = connect(mUi.mTreeWidget,
                                 &QTreeWidget::itemCollapsed,
                                 this,
                                 [handleExpansion](QTreeWidgetItem* aItem) { handleExpansion(aItem, false); });
   mExpandConnection   = connect(mUi.mTreeWidget,
                               &QTreeWidget::itemExpanded,
                               this,
                               [handleExpansion](QTreeWidgetItem* aItem) { handleExpansion(aItem, true); });
}

void WkWeaponBrowser::DockWidget::AddCommand(QVBoxLayout* aLayout, unsigned int aIndex, const std::string& aName)
{
   WeaponCommandWidget*                        command      = new WeaponCommandWidget();
   std::vector<WkWeaponBrowser::WeaponMapData> weaponVector = mDataContainer.GetWidgetData();
   const auto                                  mapDataIt    = std::find_if(weaponVector.begin(),
                                       weaponVector.end(),
                                       [aName](const WeaponMapData& arg) { return aName == arg.mName; });
   if (mapDataIt != weaponVector.end())
   {
      // Quantity Selection
      command->mQuantityEditor->setText(QString::number(mapDataIt->mWeaponCommands.at(aIndex).quantity));
      double quantityRemaining = mapDataIt->mQuantityRemaining;
      connect(command->mQuantityEditor,
              &QLineEdit::textChanged,
              this,
              [&, quantityRemaining, command, aName, aIndex](const QString& aText)
              {
                 mDataContainer.UpdateWeaponQueuedQuantity(aText, aName, aIndex);
                 // set the text color red and disable firing when there is an invalid quantity
                 if (quantityRemaining < aText.toDouble())
                 {
                    QPalette palette = command->mQuantityEditor->palette();
                    palette.setColor(QPalette::Text, QColor(Qt::red));
                    command->mQuantityEditor->setPalette(palette);
                    command->mFireButton->setEnabled(false);
                 }
                 else
                 {
                    command->mQuantityEditor->setPalette(command->mDefaultEditStyle);
                    // both track and quantity must be valid for firing to be enabled
                    if (aText.toDouble() > 0 && command->mTrackEditor->text().toInt() > 0)
                    {
                       command->mFireButton->setEnabled(true);
                    }
                    else
                    {
                       command->mFireButton->setEnabled(false);
                    }
                 }
              });

      // Track Selection, FiringData initializes track to 0.
      // We don't want to populate weapon command's Track property when a new command is added.
      // Only after the user has selected a track or typed a track into the box.
      if (mapDataIt->mWeaponCommands.at(aIndex).trackIndex == 0)
      {
         command->mTrackEditor->setText("");
      }
      else
      {
         command->mTrackEditor->setText(QString::number(mapDataIt->mWeaponCommands.at(aIndex).trackIndex));
      }

      connect(command->mTrackButton,
              &wkf::TrackSelector::Selected,
              this,
              [&, command](wkf::Track* aTrack) { TrackSelected(command, aTrack); });
      connect(command->mTrackEditor,
              &QLineEdit::textChanged,
              this,
              [&, quantityRemaining, command, aName, aIndex](const QString& aText)
              {
                 mDataContainer.UpdateWeaponTrack(aText, aName, aIndex);
                 // disable firing when there is a track <= 0, and enable when both quantity and track are valid
                 double firingAmount = command->mQuantityEditor->text().toDouble();
                 if (aText.toInt() > 0 && firingAmount > 0 && quantityRemaining >= firingAmount)
                 {
                    command->mFireButton->setEnabled(true);
                 }
                 else
                 {
                    command->mFireButton->setEnabled(false);
                 }
              });

      // Fire Command
      connect(command->mFireButton,
              &QPushButton::clicked,
              this,
              [&, command, quantityRemaining, aName, aIndex]()
              {
                 auto* platformPtr =
                    vaEnv.GetStandardScenario()->FindPlatformByName(mSimInterface->GetPlatformOfInterest());
                 if (platformPtr)
                 {
                    if (!warlock::HasPermissionToControlPlatform(*platformPtr))
                    {
                       QMessageBox::warning(nullptr,
                                            "Weapon not Fired",
                                            "User does not have permissions to modify platform " +
                                               QString::fromStdString(platformPtr->GetName()) + ".");
                    }
                    else
                    {
                       double quantity = command->mQuantityEditor->text().toDouble();
                       int    track    = command->mTrackEditor->text().toInt();
                       // only attempt to fire if command is valid
                       if (quantityRemaining >= quantity && track > 0)
                       {
                          mSimInterface->AddSimCommand(
                             ut::make_unique<WkWeaponBrowser::FireCommand>(mSimInterface->GetPlatformOfInterest(),
                                                                           aName,
                                                                           track,
                                                                           quantity));
                          mDataContainer.RemoveWeaponCommand(aName, aIndex);
                          Update();
                       }
                    }
                 }
              });

      // Delete Command
      connect(command->mDeleteButton,
              &QPushButton::clicked,
              this,
              [&, aName, aIndex]()
              {
                 mDataContainer.RemoveWeaponCommand(aName, aIndex);
                 Update();
              });

      aLayout->insertWidget(aIndex, command);
   }
}

void WkWeaponBrowser::DockWidget::AddPlusButton(QTreeWidgetItem*               aWeaponPtr,
                                                const std::string&             aName,
                                                const std::vector<FiringData>& aData)
{
   // cancels track selector tool if it is active.
   QList<WeaponCommandWidget*> commands = mUi.mTreeWidget->findChildren<WeaponCommandWidget*>();
   for (const auto& it : commands)
   {
      it->mTrackButton->CancelSelection();
   }

   QPushButton* addCommandButton = new QPushButton("+");
   addCommandButton->setFixedWidth(50);
   addCommandButton->setToolTip("Add a new weapon command");

   mUi.mTreeWidget->setItemWidget(aWeaponPtr, 1, addCommandButton);
   connect(addCommandButton,
           &QPushButton::clicked,
           [&, aName]()
           {
              mDataContainer.AddWeaponCommand(aName);
              Update();
           });
}

void WkWeaponBrowser::DockWidget::PlatformOfInterestChanged(wkf::Platform* aPlatform)
{
   if (aPlatform)
   {
      mDataContainer.SetPlatformOfInterest(aPlatform->GetName());
      mSimInterface->SetPlatformOfInterest(aPlatform->GetName());
   }
   else
   {
      mDataContainer.SetPlatformOfInterest("");
      mSimInterface->SetPlatformOfInterest("");
      Update();
   }
}

void WkWeaponBrowser::DockWidget::TrackSelected(WeaponCommandWidget* aRow, wkf::Track* aTrack)
{
   if (aRow)
   {
      if (aTrack)
      {
         aRow->mTrackEditor->setText(QString::number(aTrack->GetTrackId().GetLocalTrackId()));
      }
      else
      {
         aRow->mTrackEditor->setText("");
      }
   }
}
