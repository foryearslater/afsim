// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "DialogBuilderMainDialog.hpp"

#include <QAction>
#include <QDrag>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>

#include "DialogBuilderScriptCreator.hpp"

WkDialogBuilder::MainDialog::MainDialog(QWidget* parent)
   : QDialog(parent)
   , mScriptToMoveValid(false)
   , mScriptToMoveName("")
   , mScriptToMoveRow(0)
   , mScriptToMoveColumn(0)
{
   mUi.setupUi(this);

   QWidget* widget = new QWidget();
   mUi.scrollArea->setWidget(widget);
   mScrollAreaLayout = new QGridLayout();
   widget->setLayout(mScrollAreaLayout);

   for (auto& i : ButtonSizeToStringMap)
   {
      mUi.buttonSizeComboBox->addItem(i.second);
   }

   // Hide the dockableCheckBox until the feature is fully implemented.
   mUi.dockableCheckBox->setVisible(false);

   connect(mUi.buttonSizeComboBox,
           QOverload<int>::of(&QComboBox::currentIndexChanged),
           this,
           [this] { PopulateScrollArea(mUi.rowSpinBox->value(), mUi.columnSpinBox->value()); });

   // When accepted, make sure the name is a valid choice
   connect(mUi.buttonBox,
           &QDialogButtonBox::accepted,
           [this]
           {
              if (mUi.nameLineEdit->text().isEmpty())
              {
                 QMessageBox::warning(this, "Empty Name", "A dialog must have a name.");
              }
              else if (mBannedNames.contains(QString(mUi.nameLineEdit->text()).remove(QRegularExpression("\\s"))))
              {
                 QMessageBox::warning(this,
                                      "Name Conflict",
                                      "The selected dialog name is already in use. Please pick a new name.");
              }
              else
              {
                 QString errorMsg;
                 if (VerifyButtonValidity(errorMsg))
                 {
                    QDialog::accept();
                 }
                 else
                 {
                    QMessageBox::warning(this, "Button Error", errorMsg);
                 }
              }
           });
   connect(mUi.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

   // When columns change, update the number of columns displayed, changing this can not remove scripts.
   connect(mUi.columnSpinBox,
           QOverload<int>::of(&QSpinBox::valueChanged),
           [this](int i)
           {
              int cols = GetMinColumns();
              if (i < cols)
              {
                 mUi.columnSpinBox->setValue(cols);
                 PopulateScrollArea(mUi.rowSpinBox->value(), cols);
              }
              else
              {
                 PopulateScrollArea(mUi.rowSpinBox->value(), i);
              }
           });
   // When row change, update the number of row displayed, changing this can not remove scripts.
   connect(mUi.rowSpinBox,
           QOverload<int>::of(&QSpinBox::valueChanged),
           [this](int i)
           {
              int rows = GetMinRows();
              if (i < rows)
              {
                 mUi.rowSpinBox->setValue(rows);
                 PopulateScrollArea(rows, mUi.columnSpinBox->value());
              }
              else
              {
                 PopulateScrollArea(i, mUi.columnSpinBox->value());
              }
           });
}

bool WkDialogBuilder::MainDialog::Execute(DialogDef& aDef)
{
   Populate(aDef);
   if (exec() == QDialog::Accepted)
   {
      GetDialog(aDef);
      return true;
   }
   return false;
}

void WkDialogBuilder::MainDialog::ButtonContextMenuRequested(QPoint pos)
{
   auto* button = dynamic_cast<QPushButton*>(sender());
   if (button)
   {
      int row = button->property("row").toInt();
      int col = button->property("col").toInt();

      int index = GetScriptIndex(row, col);
      if (index != -1)
      {
         const QString& scriptName = mScripts.at(index).displayName;

         // Create Popup menu
         QMenu    popup(this);
         QAction* moveAction = new QAction("Move " + scriptName);
         popup.addAction(moveAction);
         // Create the handers for the move action
         connect(moveAction,
                 &QAction::triggered,
                 this,
                 [this, scriptName, row, col]
                 {
                    mScriptToMoveName   = scriptName;
                    mScriptToMoveValid  = true;
                    mScriptToMoveRow    = row;
                    mScriptToMoveColumn = col;
                 });

         if (mScriptToMoveValid)
         {
            QString text = mScriptToMoveName;
            if (text == "")
            {
               text = "<empty name>";
            }
            QAction* swapAction = new QAction("Swap with " + text);
            popup.addAction(swapAction);
            // Create the handers for the swap action
            connect(swapAction,
                    &QAction::triggered,
                    this,
                    [this, scriptName, row, col]
                    {
                       int index1 = GetScriptIndex(row, col);
                       int index2 = GetScriptIndex(mScriptToMoveRow, mScriptToMoveColumn);
                       if (index1 != -1 && index2 != -1)
                       {
                          SwapScriptLocations(mScripts[index1], mScripts[index2]);
                       }
                       // Update the scroll area
                       // PopulateScrollArea(mUi.rowSpinBox->value(), mUi.columnSpinBox->value());
                       // Reset the Move Script Command once the script is swapped
                       mScriptToMoveValid = false;
                    });
         }
         QAction* deleteAction = new QAction("Remove " + scriptName);
         popup.addAction(deleteAction);
         // Create the handers for the remove action
         connect(deleteAction,
                 &QAction::triggered,
                 this,
                 [this, row, col]
                 {
                    int index = GetScriptIndex(row, col);
                    if (index != -1)
                    {
                       mScripts.erase(mScripts.begin() + index);
                       // Update the scroll area
                       PopulateScrollArea(mUi.rowSpinBox->value(), mUi.columnSpinBox->value());
                    }
                    // Reset the Move Script Command on a Delete
                    mScriptToMoveValid = false;
                 });

         // Execute the menu
         popup.exec(button->mapToGlobal(pos));
      }
      else // No script is defined for this button (it is a "+")
      {
         // Check to see if there is a script available to be moved
         if (mScriptToMoveValid)
         {
            QString text = mScriptToMoveName;
            if (text == "")
            {
               text = "<empty name>";
            }
            // Create Popup menu
            QMenu    popup(this);
            QAction* moveAction = new QAction("Move " + text + " to here");
            popup.addAction(moveAction);

            // Create the handers for the move action
            connect(moveAction,
                    &QAction::triggered,
                    this,
                    [this, row, col]
                    {
                       int index = GetScriptIndex(mScriptToMoveRow, mScriptToMoveColumn);
                       if (index != -1)
                       {
                          mScripts[index].col = col;
                          mScripts[index].row = row;
                       }

                       // Update the scroll area
                       PopulateScrollArea(mUi.rowSpinBox->value(), mUi.columnSpinBox->value());
                       // Reset the Move Script Command once the script is moved
                       mScriptToMoveValid = false;
                    });

            // Execute the menu
            popup.exec(button->mapToGlobal(pos));
         }
      }
   }
}

void WkDialogBuilder::MainDialog::HandleButtonClicked()
{
   auto* button = qobject_cast<QPushButton*>(sender());
   if (button)
   {
      int row = button->property("row").toInt();
      int col = button->property("col").toInt();

      // Create temp script with the correct row and col
      ScriptDefinition scriptDef = ScriptDefinition();
      scriptDef.col              = col;
      scriptDef.row              = row;

      // Find the script that is at the selected location
      bool scriptExist = false;
      for (const auto& i : mScripts)
      {
         if (i.row == row && i.col == col)
         {
            // Launch Edit script dialog with the proper script
            scriptDef   = i;
            scriptExist = true;
            break;
         }
      }

      // Launch a Script Creator to the user may modify the script
      ScriptCreator sc(this);
      // Return true if accepted (false if canceled)
      if (sc.Execute(scriptDef))
      {
         if (scriptExist)
         {
            // Find the script that was modified and update it
            for (auto& i : mScripts)
            {
               if (i.row == row && i.col == col)
               {
                  i = scriptDef;
                  break;
               }
            }
         }
         else
         {
            // If the script exists, then we already updated it since it was a reference.
            // If the script did not exist, then we must add it to the definitions.
            mScripts.push_back(scriptDef);
         }

         PopulateScriptButton(button, scriptDef);
      }
   }
}

void WkDialogBuilder::MainDialog::GetDialog(DialogDef& aDef)
{
   aDef.name       = mUi.nameLineEdit->text();
   aDef.cols       = mUi.columnSpinBox->value();
   aDef.rows       = mUi.rowSpinBox->value();
   aDef.dockable   = mUi.dockableCheckBox->isChecked();
   aDef.buttonSize = (ButtonSize)mUi.buttonSizeComboBox->currentIndex();
   aDef.scripts    = mScripts;
}

void WkDialogBuilder::MainDialog::Populate(const DialogDef& aDef)
{
   mScripts = aDef.scripts;

   mUi.nameLineEdit->setText(aDef.name);
   mUi.columnSpinBox->setValue(aDef.cols);
   mUi.rowSpinBox->setValue(aDef.rows);
   mUi.dockableCheckBox->setChecked(aDef.dockable);
   mUi.buttonSizeComboBox->setCurrentIndex(aDef.buttonSize);
   PopulateScrollArea(aDef.rows, aDef.cols);
}

void WkDialogBuilder::MainDialog::PopulateScrollArea(int aRows, int aCols)
{
   // Clear out buttons
   for (auto i : mScrollAreaButtons)
   {
      delete i;
   }
   mScrollAreaButtons.clear();

   // Create the new buttons
   for (int i = 0; i < aRows; ++i)
   {
      for (int j = 0; j < aCols; ++j)
      {
         auto* button = new QPushButton("+");
         button->setProperty("row", i);
         button->setProperty("col", j);
         connect(button, &QPushButton::clicked, this, &MainDialog::HandleButtonClicked);
         button->setContextMenuPolicy(Qt::CustomContextMenu);
         connect(button, &QPushButton::customContextMenuRequested, this, &MainDialog::ButtonContextMenuRequested);
         ApplyButtonSize(button, (ButtonSize)mUi.buttonSizeComboBox->currentIndex());
         mScrollAreaLayout->addWidget(button, i, j);
         mScrollAreaButtons.append(button);
         button->installEventFilter(this);
         button->setAcceptDrops(true);

         for (auto script : mScripts)
         {
            if (script.row == i && script.col == j)
            {
               PopulateScriptButton(button, script);
               break;
            }
         }
      }
   }
}

void WkDialogBuilder::MainDialog::PopulateScriptButton(QPushButton* aButton, const ScriptDefinition& aDef)
{
   // Update the button with the correct Icon if specified
   ApplyButtonNameAndIcon(aButton, aDef.displayName, aDef.icon);

   if (!aDef.useDefaultColor)
   {
      ApplyButtonColor(aButton, aDef.color);
   }
}

int WkDialogBuilder::MainDialog::GetMinRows()
{
   int row = 0;
   for (auto i : mScripts)
   {
      if (i.row > row)
      {
         row = i.row;
      }
   }
   // zero based so add 1, to get number of rows
   ++row;
   return row;
}

int WkDialogBuilder::MainDialog::GetMinColumns()
{
   int col = 0;
   for (auto i : mScripts)
   {
      if (i.col > col)
      {
         col = i.col;
      }
   }
   // zero based so add 1, to get number of columns
   ++col;
   return col;
}

void WkDialogBuilder::MainDialog::SwapScriptLocations(ScriptDefinition& aFirst, ScriptDefinition& aSecond)
{
   int row     = aFirst.row;
   int col     = aFirst.col;
   aFirst.row  = aSecond.row;
   aFirst.col  = aSecond.col;
   aSecond.row = row;
   aSecond.col = col;

   SwapButtonLocations(aFirst.row, aFirst.col, aSecond.row, aSecond.col);
}

void WkDialogBuilder::MainDialog::MoveScriptLocation(ScriptDefinition& aDefinition, int aRow, int aCol)
{
   // Find if a script exist in aRow, aCol. If so, call SwapScriptLocations instead.
   bool validMove = true;
   for (auto& script : mScripts)
   {
      if (script.row == aRow && script.col == aCol)
      {
         SwapScriptLocations(aDefinition, script);
         validMove = false;
      }
   }

   if (validMove)
   {
      SwapButtonLocations(aDefinition.row, aDefinition.col, aRow, aCol);
      aDefinition.row = aRow;
      aDefinition.col = aCol;
   }
}

void WkDialogBuilder::MainDialog::SwapButtonLocations(int aRow1, int aCol1, int aRow2, int aCol2)
{
   QPushButton* button1 = nullptr;
   QPushButton* button2 = nullptr;
   for (auto& button : mScrollAreaButtons)
   {
      if (button->property("row") == aRow1 && button->property("col") == aCol1)
      {
         button1 = button;
      }
      else if (button->property("row") == aRow2 && button->property("col") == aCol2)
      {
         button2 = button;
      }

      if (button1 && button2)
      {
         break;
      }
   }

   if (button1 && button2)
   {
      mScrollAreaLayout->addWidget(button1, aRow2, aCol2);
      button1->setProperty("row", aRow2);
      button1->setProperty("col", aCol2);
      mScrollAreaLayout->addWidget(button2, aRow1, aCol1);
      button2->setProperty("row", aRow1);
      button2->setProperty("col", aCol1);
   }
}

int WkDialogBuilder::MainDialog::GetScriptIndex(int aRow, int aCol)
{
   int index = 0;
   for (auto& i : mScripts)
   {
      if (i.col == aCol && i.row == aRow)
      {
         return index;
      }
      index++;
   }
   return -1;
}

bool WkDialogBuilder::MainDialog::eventFilter(QObject* watched, QEvent* event)
{
   if (event->type() == QEvent::MouseButtonPress)
   {
      QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
      if (mouseEvent && mouseEvent->button() == Qt::LeftButton)
      {
         mDragStartPos = mouseEvent->pos();
      }
   }
   if (event->type() == QEvent::MouseMove)
   {
      QMouseEvent* moveEvent = dynamic_cast<QMouseEvent*>(event);
      if (moveEvent)
      {
         if ((moveEvent->pos() - mDragStartPos).manhattanLength() < QApplication::startDragDistance())
         {
            return false;
         }
         QDrag*     drag     = new QDrag(this);
         QMimeData* mimeData = new QMimeData;

         int i = watched->property("row").toInt();
         int j = watched->property("col").toInt();
         mimeData->setText(QString::number(i) + "," + QString::number(j));
         drag->setMimeData(mimeData);

         QWidget* widget = dynamic_cast<QWidget*>(watched);
         if (widget)
         {
            QPixmap pixmap = widget->grab();
            drag->setHotSpot(QPoint(pixmap.width() / 2, pixmap.height() / 2));
            drag->setPixmap(pixmap);
         }

         drag->exec(Qt::CopyAction | Qt::MoveAction);
      }
   }
   else if (event->type() == QEvent::DragEnter)
   {
      QDragEnterEvent* dragEvent = dynamic_cast<QDragEnterEvent*>(event);
      if (dragEvent)
      {
         dragEvent->acceptProposedAction();
      }
   }
   else if (event->type() == QEvent::Drop)
   {
      QDropEvent* dropEvent = dynamic_cast<QDropEvent*>(event);
      if (dropEvent && dropEvent->mimeData()->hasFormat("text/plain"))
      {
         dropEvent->acceptProposedAction();

         QString     text = dropEvent->mimeData()->text();
         QStringList list = text.split(",");
         if (list.size() == 2)
         {
            int x1     = list.at(0).toInt();
            int y1     = list.at(1).toInt();
            int index1 = GetScriptIndex(x1, y1);

            int x2     = watched->property("row").toInt();
            int y2     = watched->property("col").toInt();
            int index2 = GetScriptIndex(x2, y2);

            // If both scripts were valid, swap them
            if (index1 != -1 && index2 != -1)
            {
               SwapScriptLocations(mScripts[index1], mScripts[index2]);
            }
            // If only script 1 was valid (other script location is a "+")
            else if (index1 != -1)
            {
               MoveScriptLocation(mScripts[index1], x2, y2);
            }
            // If only script 2 was valid (other script location is a "+")
            else if (index2 != -1)
            {
               MoveScriptLocation(mScripts[index2], x1, y1);
            }
            mScriptToMoveValid = false;
         }
      }
   }
   return false;
}

bool WkDialogBuilder::MainDialog::VerifyButtonValidity(QString& aErrorMsg)
{
   QSet<QString> nameSet;
   for (auto& buttonIt : mScrollAreaButtons)
   {
      QLabel* name = buttonIt->findChild<QLabel*>();
      if (name)
      {
         QString nameText = name->objectName().remove(QRegularExpression("\\s"));
         if (nameText.isEmpty())
         {
            aErrorMsg = "Defined buttons cannot have an empty name field.";
            return false;
         }
         else
         {
            if (nameSet.contains(nameText))
            {
               aErrorMsg = "Button names must be unique within the dialog (with whitespace ignored). Another button "
                           "already uses the name \'" +
                           nameText + "\'.";
               return false;
            }
            nameSet.insert(nameText);
         }
      }
   }
   return true;
}
