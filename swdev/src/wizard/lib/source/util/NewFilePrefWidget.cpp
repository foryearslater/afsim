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

#include "NewFilePrefWidget.hpp"

wizard::NewFilePrefWidget::NewFilePrefWidget(QWidget* aParent)
   : PrefWidgetT<NewFilePrefObject>(aParent)
{
   mUi.setupUi(this);
   mMacroHelpDialogUi.setupUi(&mMacroHelpDialog);
   mMacroHelpDialog.setWindowFlag(Qt::WindowType::WindowStaysOnTopHint, true);
   connect(mMacroHelpDialogUi.okButton, &QPushButton::clicked, &mMacroHelpDialog, &QDialog::close);

   connect(mUi.macroHelpButton, &QPushButton::clicked, &mMacroHelpDialog, &QDialog::open);

   mUi.addButton->setIcon(QIcon::fromTheme("add"));
   mUi.removeButton->setIcon(QIcon::fromTheme("remove"));

   connect(mUi.addButton, &QToolButton::clicked, this, &NewFilePrefWidget::AddItem);
   connect(mUi.removeButton, &QToolButton::clicked, this, &NewFilePrefWidget::RemoveItem);

   connect(mUi.nameLineEdit,
           &QLineEdit::editingFinished,
           [this]()
           {
              SaveCurrentItem();
              mCurrentItem = mUi.nameLineEdit->text().toStdString();
              UpdateComboBox();
           });
   connect(mUi.comboBox,
           QOverload<const QString&>::of(&QComboBox::activated),
           [this](const QString& aSelected) { GoTo(aSelected.toStdString()); });
}

void wizard::NewFilePrefWidget::ReadPreferenceData(const NewFilePrefData& aPrefData)
{
   mTemporary.SetPreferences(aPrefData);

   const HeadingData* first = mTemporary.First();
   if (first != nullptr)
   {
      mCurrentItem = first->mName;
   }

   UpdateComboBox();
   ReadCurrentItem();
}

void wizard::NewFilePrefWidget::WritePreferenceData(NewFilePrefData& aPrefData)
{
   SaveCurrentItem();
   aPrefData = mTemporary.GetPreferences();
}

void wizard::NewFilePrefWidget::ReadCurrentItem()
{
   const HeadingData* heading = mTemporary.GetHeading(mCurrentItem);
   if (heading != nullptr)
   {
      mUi.commentCheckBox->setChecked(heading->mEnableComment);
      mUi.macroCheckBox->setChecked(heading->mEnableMacros);
      mUi.nameLineEdit->setText(QString::fromStdString(heading->mName));
      mUi.headingTextEdit->setPlainText(QString::fromStdString(heading->mText));
   }
}

void wizard::NewFilePrefWidget::SaveCurrentItem()
{
   HeadingData* heading = mTemporary.GetHeading(mCurrentItem);
   if (heading != nullptr)
   {
      heading->mEnableComment = mUi.commentCheckBox->isChecked();
      heading->mEnableMacros  = mUi.macroCheckBox->isChecked();
      heading->mName          = mUi.nameLineEdit->text().toStdString();
      heading->mText          = mUi.headingTextEdit->toPlainText().toStdString();
   }
}

bool wizard::NewFilePrefWidget::ValidateName() const
{
   if (mUi.nameLineEdit->text().isEmpty())
   {
      QMessageBox::critical(nullptr, "Invalid Name", "Name field cannot be left empty.");
      mUi.comboBox->setCurrentText(QString::fromStdString(mCurrentItem));
      return false;
   }

   int nameCollisions = 0;
   for (const auto& heading : mTemporary.Data())
   {
      if (heading.mName == mUi.nameLineEdit->text().toStdString())
      {
         nameCollisions++;
      }
   }

   if (nameCollisions > 1)
   {
      QMessageBox::critical(nullptr, "Invalid Name", "Heading profile names must be unique.");
      mUi.comboBox->setCurrentText(QString::fromStdString(mCurrentItem));
      return false;
   }

   return true;
}

void wizard::NewFilePrefWidget::GoTo(const std::string& aIndex)
{
   if (ValidateName())
   {
      SaveCurrentItem();
      mCurrentItem = aIndex;
      ReadCurrentItem();
   }
}

void wizard::NewFilePrefWidget::AddItem()
{
   if (mTemporary.GetHeadingCount() == 0 || ValidateName())
   {
      SaveCurrentItem();
      mTemporary.AddHeading();
      mCurrentItem.clear();
      UpdateComboBox();
      ReadCurrentItem();
   }
}

void wizard::NewFilePrefWidget::RemoveItem()
{
   mTemporary.RemoveHeading(mCurrentItem);
   const auto* first = mTemporary.First();
   if (first != nullptr)
   {
      mCurrentItem = first->mName;
   }
   else
   {
      mCurrentItem.clear();
   }
   UpdateComboBox();
   ReadCurrentItem();
}

void wizard::NewFilePrefWidget::UpdateComboBox()
{
   mUi.comboBox->clear();
   mTemporary.SortList();

   for (const auto& heading : mTemporary.Data())
   {
      mUi.comboBox->addItem(QString::fromStdString(heading.mName));
   }

   mUi.comboBox->setCurrentText(QString::fromStdString(mCurrentItem));

   const bool isEmpty = (mUi.comboBox->count() == 0);

   mUi.comboBox->setDisabled(isEmpty);
   mUi.removeButton->setDisabled(isEmpty);
   mUi.headingTextEdit->setDisabled(isEmpty);
   mUi.nameLineEdit->setDisabled(isEmpty);

   ReadCurrentItem();
}
