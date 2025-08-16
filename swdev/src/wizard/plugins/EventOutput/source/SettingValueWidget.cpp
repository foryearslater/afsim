// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SettingValueWidget.hpp"

#include "CommandValuePair.hpp"
#include "PluginUtil.hpp"
#include "SettingWidget.hpp"
#include "UtQtMemory.hpp"
#include "WsfParseRule.hpp"

namespace wizard
{
namespace EventOutput
{
namespace setting
{
namespace ValueWidget
{

ValueWidget::Type DetermineValueType(const QVector<ValueWidget::Type>& aTypes)
{
   // this catches the Boolean type
   if (aTypes.size() == 1)
   {
      return aTypes[0];
   }
   // simple equation:  MultipleChoice = Constant + UserInput
   // Note:  in the future, this may change; however, this is the only MultipleChoice formula right now...
   else if (aTypes.contains(ValueWidget::Type::cCONSTANT) && aTypes.contains(ValueWidget::Type::cUSER_INPUT_TEXT))
   {
      return ValueWidget::Type::cMULTIPLE_CHOICE;
   }
   else if (aTypes.contains(ValueWidget::Type::cCONSTANT) && aTypes.contains(ValueWidget::Type::cNONE))
   {
      return ValueWidget::Type::cCONSTANT;
   }
   else
   {
      return ValueWidget::Type::cNONE;
   }
};

// determines the default value of the setting
// @param aRule is the WsfParseStruct containing information about the setting
// @param aData is the data of the current setting
// @return the default value of the setting (to be stored in the setting::Data)
const QString* DetermineDefaultValue(const WsfParseStruct& aRule, const Data& aData)
{
   // search the attributes for the command
   auto eventOutputAttributes = aRule.mAttributes;
   for (const auto& attr : eventOutputAttributes)
   {
      if (attr.mName == aData.mCommand.toStdString())
      {
         // search the possible values in the data
         for (const auto& val : aData.mPossibleValues)
         {
            if (val.contains(QString::fromStdString(attr.mDefault)))
            {
               return &val;
            }
         }
      }
   }
   return nullptr;
};

WidgetInterface::WidgetInterface(const setting::Data& aSetting, QWidget* aParent /* = nullptr */)
   : QWidget(aParent)
   , mSetting(aSetting)
{
   // set the object name for convenience (not actually used though...)
   setObjectName(aSetting.mCommand + "::value");
}

void WidgetInterface::Disable()
{
   mValue->setDisabled(true);
   mEnabled = false;
}

void WidgetInterface::Enable()
{
   mValue->setEnabled(true);
   mEnabled = true;
}

Constant::Constant(const setting::Data& aSetting, QWidget* aParent /* = nullptr */)
   : WidgetInterface(aSetting, aParent)
   , mConstantValue(ut::qt::make_qt_ptr<QLabel>(this))
{
   // set the parent pointer and set up the value widget
   mValue = mConstantValue;
   QSignalBlocker block{mConstantValue};
   mConstantValue->setText(aSetting.mPossibleValues[0].split(':')[1]);
   // set up the tooltip because it is important
   mConstantValue->setToolTip(QString("The value of the setting command (") + mConstantValue->text() + ')');
}

Boolean::Boolean(const setting::Data& aSetting, QWidget* aParent /* = nullptr */)
   : WidgetInterface(aSetting, aParent)
   , mBooleanValue(ut::qt::make_qt_ptr<QCheckBox>("true", this))
{
   // set up the tooltip because it is important
   mBooleanValue->setToolTip(QString("The value of the setting command (") +
                             ((mBooleanValue->isChecked()) ? "true" : "false") + ')');
   // set the parent pointer and set up the value widget
   mValue = mBooleanValue;
   QSignalBlocker block{mBooleanValue};
   mBooleanValue->setParent(this);

   // do the necessary connections
   connect(mBooleanValue.data(),
           &QCheckBox::stateChanged,
           [this](const int aState)
           {
              // set up the tooltip because it is important
              mBooleanValue->setToolTip(QString("The value of the setting command (") +
                                        ((mBooleanValue->isChecked()) ? "true" : "false") + ')');
              emit ModifiedByUser();
           });
}

void Boolean::ReadValueP(const editor::token::CommandValuePair* aToken)
{
   PluginUtil::SetBoolCheckBox(mBooleanValue, aToken->GetValue());
}

void Boolean::WriteValueP(editor::token::CommandValuePair* aToken)
{
   PluginUtil::SetBoolToken(*aToken, mBooleanValue->checkState());
}

void Boolean::RestoreDefaultP()
{
   PluginUtil::SetBoolCheckBox(mBooleanValue, mSetting.mDefaultValue->split(':')[1]);
}

MultipleChoice::MultipleChoice(const setting::Data& aSetting, QWidget* aParent /* = nullptr */)
   : WidgetInterface(aSetting, aParent)
   , mMultipleChoiceValue(ut::qt::make_qt_ptr<QComboBox>(this))
{
   // set the parent pointer and set up the value widget
   mValue = mMultipleChoiceValue;
   bool           userDefined{aSetting.mValue.has_value()};
   QSignalBlocker block{mMultipleChoiceValue};
   for (const auto& item : aSetting.mPossibleValues)
   {
      QStringList itemSplit{item.split(':')};
      QString     editType{itemSplit[0]};
      itemSplit.erase(itemSplit.begin());
      QString itemName{itemSplit.join(':')};
      if (editType == "NoEdit")
      {
         mMultipleChoiceValue->addItem(itemName, QVariant{false});
      }
      else if (editType == "Edit")
      {
         mMultipleChoiceValue->addItem((userDefined) ? aSetting.mValue.value() : itemName, QVariant{true});
      }
   }

   // do the necessary connections
   connect(mMultipleChoiceValue.data(),
           QOverload<int>::of(&QComboBox::currentIndexChanged),
           [this](const int aIndex) { emit ModifiedByUser(); });
}

void MultipleChoice::ReadValueP(const editor::token::CommandValuePair* aToken)
{
   auto value = aToken->GetValue();
   if (mMultipleChoiceValue->findText(value) == -1)
   {
      mMultipleChoiceValue->setCurrentIndex(mMultipleChoiceValue->count() - 1);
      mMultipleChoiceValue->setEditText(value);
   }
   mMultipleChoiceValue->setCurrentText(value);
}

void MultipleChoice::WriteValueP(editor::token::CommandValuePair* aToken)
{
   aToken->SetValue(mMultipleChoiceValue->currentText());
}

void MultipleChoice::RestoreDefaultP()
{
   mMultipleChoiceValue->setCurrentIndex(0);
}

UserInput::UserInput(const setting::Data& aSetting, QWidget* aParent /* = nullptr */)
   : WidgetInterface(aSetting, aParent)
   , mUserInputValue(ut::qt::make_qt_ptr<QLineEdit>(this))
{
   // set the parent pointer and set up the value widget
   mValue = mUserInputValue;
   bool           userDefined{aSetting.mValue.has_value()};
   QSignalBlocker block{mValue};
   QStringList    valueSplit{aSetting.mPossibleValues[0].split(':')};
   QString        editType{valueSplit[0]};
   valueSplit.erase(valueSplit.begin());
   if (userDefined)
   {
      mUserInputValue->setText(aSetting.mValue.value());
   }
   else if (editType == "Placeholder")
   {
      mUserInputValue->setPlaceholderText(valueSplit.join(':'));
   }

   // do the necessary connections
   connect(mUserInputValue.data(), &QLineEdit::editingFinished, this, &WidgetInterface::ModifiedByUser);
}

void UserInput::ReadValueP(const editor::token::CommandValuePair* aToken)
{
   mUserInputValue->setText(aToken->GetValue());
}

void UserInput::WriteValueP(editor::token::CommandValuePair* aToken)
{
   aToken->SetValue(mUserInputValue->text());
}

void UserInput::RestoreDefaultP()
{
   mUserInputValue->setText("");
}

} // namespace ValueWidget
} // namespace setting
} // namespace EventOutput
} // namespace wizard
