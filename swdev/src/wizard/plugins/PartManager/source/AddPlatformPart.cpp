// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "AddPlatformPart.hpp"

#include <QTextBlock>

#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "Project.hpp"
#include "Undo.hpp"
#include "UtQt.hpp"
#include "WkfEnvironment.hpp"
#include "WsfPM_MilRoot.hpp"

PartManager::AddPlatformPart::AddPlatformPart(const WsfPProxyNode& node, QWidget* parent /*= nullptr*/)
   : QDialog(parent)
   , mPlatform(node)
{
   mUi.setupUi(this);
   setAttribute(Qt::WA_DeleteOnClose);

   const WsfPM_MilRoot root(mPlatform.GetProxy());
   mTypeObjectMaps[mUi.sensorRadioButton]    = root.sensorTypes();
   mTypeObjectMaps[mUi.commRadioButton]      = root.commTypes();
   mTypeObjectMaps[mUi.weaponRadioButton]    = root.weaponTypes();
   mTypeObjectMaps[mUi.processorRadioButton] = root.processorTypes();
   mTypeObjectMaps[mUi.moverRadioButton]     = root.moverTypes();
   mTypeObjectMaps[mUi.fuelRadioButton]      = root.fuelTypes();

   setWindowTitle(QString::fromStdString("Add part to " + mPlatform.GetName()));

   PrepopulateForm(mUi.typeButtonGroup->checkedButton());

   connect(mUi.typeButtonGroup,
           static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked),
           this,
           &AddPlatformPart::PrepopulateForm);
   connect(mUi.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &AddPlatformPart::deleteLater);
   connect(mUi.buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &AddPlatformPart::AddPart);
}

//! Fills the combo box (and the "Part name" line edit in the case of Mover and
//! Fuel) based on the user's radio button selection.
void PartManager::AddPlatformPart::PrepopulateForm(QAbstractButton* button)
{
   WsfPM_ObjectMap&           typeObjMap = mTypeObjectMaps[button];
   std::vector<WsfPProxyNode> types;

   typeObjMap.GetAllValues(types);
   std::sort(types.begin(), types.end());

   // Fill the combo box with specific type names.
   mUi.specPartTypeComboBox->clear();
   for (auto&& type : types)
   {
      QString typeName = QString::fromStdString(type.GetName());
      mUi.specPartTypeComboBox->addItem(typeName, QVariant::fromValue<WsfPProxyNode>(type));
   }

   // Mover and Fuel have pre-determined names. Set the name and disable the
   // line edit widget so the user can't change it.
   if ((button == mUi.moverRadioButton) || (button == mUi.fuelRadioButton))
   {
      const std::string name = (button == mUi.moverRadioButton) ? "mover" : "fuel";

      mUi.partNameLineEdit->setDisabled(true);
      mUi.partNameLineEdit->setText(QString::fromStdString(name));
   }
   else
   {
      mUi.partNameLineEdit->clear();
      mUi.partNameLineEdit->setDisabled(false);
   }
}

//! Try to add the part specified by the user in the GUI to the platform.
void PartManager::AddPlatformPart::AddPart()
{
   if (!mUi.partNameLineEdit->text().isEmpty())
   {
      QVariant            data        = mUi.specPartTypeComboBox->itemData(mUi.specPartTypeComboBox->currentIndex());
      const WsfPProxyNode newPartType = data.value<WsfPProxyNode>();
      const QString       newPartName = mUi.partNameLineEdit->text();
      const QString       platName    = QString::fromStdString(mPlatform.GetName());

      wizard::UndoContext undoCtx(QString("Add %1 to %2").arg(newPartName).arg(platName));

      QString partTypeStr   = "";
      bool    duplicatePart = mPlatform.FindPart(newPartName.toStdString()).IsValid();
      if (mUi.sensorRadioButton->isChecked())
      {
         partTypeStr = "sensor";
      }
      else if (mUi.weaponRadioButton->isChecked())
      {
         partTypeStr = "weapon";
      }
      else if (mUi.moverRadioButton->isChecked())
      {
         partTypeStr   = "mover";
         duplicatePart = mPlatform.FindPart(partTypeStr.toStdString()).IsValid();
      }
      else if (mUi.commRadioButton->isChecked())
      {
         partTypeStr = "comm";
      }
      else if (mUi.processorRadioButton->isChecked())
      {
         partTypeStr = "processor";
      }
      else if (mUi.fuelRadioButton->isChecked())
      {
         partTypeStr   = "fuel";
         duplicatePart = mPlatform.FindPart(partTypeStr.toStdString()).IsValid();
      }

      if (!duplicatePart)
      {
         QString specificPartType = mUi.specPartTypeComboBox->currentText();

         QTextCursor         attributeLoc = GetEditor()->textCursor();
         UtTextDocumentRange attribRange  = mPlatform.GetSourceDocumentRange();
         if (attribRange.Valid())
         {
            QString tabSpace   = "";
            int     indentSize = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
            for (int i = 0; i < indentSize; ++i)
            {
               tabSpace.append(" ");
            }
            QString newPartBlock = "";
            if (partTypeStr == "mover" || partTypeStr == "fuel")
            {
               newPartBlock = "\n" + tabSpace + "add " + partTypeStr + " " + specificPartType + "\n" + tabSpace +
                              "end_" + partTypeStr + "\n";
            }
            else
            {
               newPartBlock = "\n" + tabSpace + "add " + partTypeStr + " " + newPartName + " " + specificPartType +
                              "\n" + tabSpace + "end_" + partTypeStr + "\n";
            }
            // insert at bottom of platform def
            QTextCursor bottomCursor = GetEditor()->textCursor();
            bottomCursor.setPosition(static_cast<int>(attribRange.GetEnd()));
            bottomCursor.setPosition(bottomCursor.block().previous().position());
            bottomCursor.movePosition(QTextCursor::EndOfLine);

            bottomCursor.insertText(newPartBlock);
            GetEditor()->setTextCursor(bottomCursor);
            GetEditor()->textCursor().movePosition(QTextCursor::StartOfLine);
            GetEditor()->textCursor().clearSelection();
            // since a new line was added, the text range of the platform needs to be updated...
            int curPos = GetEditor()->textCursor().blockNumber();
            GetEditor()->GoToLine(curPos - 1);
         }
         deleteLater();
      }
      else
      {
         UtQtShowSimpleMsg("Part name conflict",
                           "A part named <b>" + newPartName + "</b> already exists on this platform.",
                           "Please choose a different name or remove the existing part.",
                           this);
      }
   }
   else
   {
      UtQtShowSimpleMsg("Name required", "Please enter a name for the part.", "", this);
   }
}

wizard::Editor* PartManager::AddPlatformPart::GetEditor()
{
   UtTextDocumentRange attrTextRange = mPlatform.GetSourceDocumentRange();
   std::string         fileName      = attrTextRange.mSource->GetFilePath().GetNormalizedPath();
   return wizard::Project::Instance()->GotoFile(fileName);
}
