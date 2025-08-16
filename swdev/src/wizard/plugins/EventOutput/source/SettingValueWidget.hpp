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

#ifndef EVENT_OUTPUT_VALUE_WIDGETS_HPP
#define EVENT_OUTPUT_VALUE_WIDGETS_HPP

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QWidget>

#include "UtQtUiPointer.hpp"

class WsfParseStruct;

namespace wizard
{
namespace EventOutput
{

class BooleanCheckBoxWidget;

namespace editor
{
namespace token
{

struct CommandValuePair;

}
} // namespace editor

namespace setting
{

struct Data;

//! Namespace for objects, functions, and variables related to the value widget of a setting::Item
namespace ValueWidget
{

//! Represents a value widget's input type
enum class Type
{
   cNONE,            //!< denotes no input method for the value widget
   cCONSTANT,        //!< denotes a pre-defined, immutable input for the value widget
   cBOOLEAN,         //!< denotes boolean input for the value widget
   cMULTIPLE_CHOICE, //!< denotes a list of inputs for the value widget
   cUSER_INPUT_TEXT  //!< denotes a user-defined, mutable input for the value widget
};

// determines the ValueWidget::Type of the setting
// @param aTypes is a list of ValueWidget::Types
// @return the overall ValueWidget::Type
ValueWidget::Type DetermineValueType(const QVector<ValueWidget::Type>& aTypes);

// determines the default value of the setting
// @param aRule is the WsfParseStruct containing information about the setting
// @param aData is the data of the current setting
// @return the default value of the setting (to be stored in the setting::Data)
const QString* DetermineDefaultValue(const WsfParseStruct& aRule, const Data& aData);

//! Represents a device to modify a setting::Item's value
//! @par details
//!   A value widget is a term used for the device used to modify the value of a setting::Item.
//!   this class provides the common interface for a value widget.
//! @see setting::Item for more information.
//! @note this hierarchy uses the Non-virtual Interface pattern
class WidgetInterface : public QWidget
{
   Q_OBJECT
public:
   //! Constructs a WidgetInterface
   //! @param aSetting is the setting::Data the value widget references
   //! @param aParent is a pointer to the parent widget
   WidgetInterface(const setting::Data& aSetting, QWidget* aParent = nullptr);
   //! Destructs a WidgetInterface
   ~WidgetInterface() override = default;

   //! Get the value widget
   //! @return a pointer to the value widget (immutable)
   //! @note this method is shadowed by the derived classes to get the actual value widget
   const QWidget* GetWidget() const { return mValue.data(); }
   //! Get the value widget
   //! @return a pointer to the value widget (mutable)
   virtual QWidget* GetWidget() { return mValue; }

   //! @name methods for editing the property information
   //! @note the property for the value widget is the value of the setting
   //@{

   //! Read the setting value information
   //! @param aToken is the setting command-value pair from which to read the setting value
   void ReadValue(const editor::token::CommandValuePair* aToken) { ReadValueP(aToken); }
   //! Read the setting value information
   //! @param aToken is the setting command-value pair to which to write the setting value
   void WriteValue(editor::token::CommandValuePair* aToken) { WriteValueP(aToken); }
   //! Restores the default state of the setting value
   //! @note this does not edit any information in the back-end
   void RestoreDefault() { RestoreDefaultP(); }

   //@}

   //! Disables the widget
   void Disable();
   //! Determines whether or not the widget is disabled
   //! @return whether or not the widget is disabled
   bool IsDisabled() const { return !mEnabled; }
   //! Enables the widget
   void Enable();
   //! Determines whether or not the widget is enabled
   //! @return whether or not the widget is enabled
   bool IsEnabled() const { return mEnabled; }
   //! Determines whether or not the widget is editable
   //! @return whether or not the widget is editable
   bool IsEditable() const { return mValue->isEnabled() && IsEditableP(); }

signals:
   //! Signal to notify connected objects that the user modified the value widget
   void ModifiedByUser();

protected:
   //! The value widget
   QPointer<QWidget> mValue;
   //! The setting::Data reference
   const setting::Data& mSetting;

private:
   //! @note the private, virtual interface
   //@{

   //! Read the setting value information
   //! @param aToken is the setting command-value pair from which to read the setting value
   virtual void ReadValueP(const editor::token::CommandValuePair* aToken) = 0;
   //! Read the setting value information
   //! @param aToken is the setting command-value pair to which to write the setting value
   virtual void WriteValueP(editor::token::CommandValuePair* aToken) = 0;
   //! Restores the default state of the setting value
   //! @note this does not edit any information in the back-end
   virtual void RestoreDefaultP() = 0;

   //! Determines whether or not the widget is editable
   //! @return whether or not the widget is editable
   //! @note this can be different for derived classes
   virtual bool IsEditableP() const = 0;

   //@}

   //! Denotes whether or not the widget is enabled
   bool mEnabled{true};
};

//! Represents the Constant value widget
//! @par details
//!   The Constant value widget has a pre-defined value that cannot be changed by the user.
//!   of course, it can be changed by the programmer, but that is not advised.
//!   this is represented by the use of a QLabel.
class Constant : public WidgetInterface
{
public:
   //! Constructs a Constant
   //! @param aSetting is the setting::Data the value widget references
   //! @param aParent is a pointer to the parent widget
   Constant(const setting::Data& aSetting, QWidget* aParent = nullptr);
   //! Destructs a Constant
   ~Constant() override = default;

   //! Get the value widget
   //! @return a pointer to the value widget
   QLabel* GetWidget() override { return mConstantValue; }

private:
   //! @note the private, virtual interface
   //@{

   //! Read the setting value information
   //! @param aToken is the setting command-value pair from which to read the setting value
   void ReadValueP(const editor::token::CommandValuePair* aToken) override {}
   //! Read the setting value information
   //! @param aToken is the setting command-value pair to which to write the setting value
   void WriteValueP(editor::token::CommandValuePair* aToken) override {}
   //! Restores the default state of the setting value
   //! @note this does not edit any information in the back-end
   void RestoreDefaultP() override {}

   //! Determines whether or not the widget is editable
   //! @return whether or not the widget is editable
   bool IsEditableP() const override { return false; }

   //@}

   //! The value widget
   ut::qt::UiPointer<QLabel> mConstantValue;
};

//! Represents the Boolean value widget
//! @par details
//!   The Boolean value widget has a value that can be false or true.
//!   this is represented by the use of a QCheckBox.
class Boolean : public WidgetInterface
{
public:
   //! Constructs a Boolean
   //! @param aSetting is the setting::Data the value widget references
   //! @param aParent is a pointer to the parent widget
   Boolean(const setting::Data& aSetting, QWidget* aParent = nullptr);
   //! Destructs a Boolean
   ~Boolean() override = default;

   //! Get the value widget
   //! @return a pointer to the value widget
   QCheckBox* GetWidget() override { return mBooleanValue; }

private:
   //! @note the private, virtual interface
   //@{

   //! Read the setting value information
   //! @param aToken is the setting command-value pair from which to read the setting value
   void ReadValueP(const editor::token::CommandValuePair* aToken) override;
   //! Read the setting value information
   //! @param aToken is the setting command-value pair to which to write the setting value
   void WriteValueP(editor::token::CommandValuePair* aToken) override;
   //! Restores the default state of the setting value
   //! @note this does not edit any information in the back-end
   void RestoreDefaultP() override;

   //! Determines whether or not the widget is editable
   //! @return whether or not the widget is editable
   bool IsEditableP() const override { return false; }

   //@}

   //! The value widget
   ut::qt::UiPointer<QCheckBox> mBooleanValue;
};

//! Represents the MultipleChoice value widget
//! @par details
//!   The MutlipleChoice value widget has a value that be chosen from a list of values.
//!   this is represented by the use of a QComboBox.
class MultipleChoice : public WidgetInterface
{
public:
   //! Constructs a MultipleChoice
   //! @param aSetting is the setting::Data the value widget references
   //! @param aParent is a pointer to the parent widget
   MultipleChoice(const setting::Data& aSetting, QWidget* aParent = nullptr);
   //! Destructs a MultipleChoice
   ~MultipleChoice() override = default;

   //! Get the value widget
   //! @return a pointer to the value widget
   QComboBox* GetWidget() override { return mMultipleChoiceValue; }

private:
   //! @note the private, virtual interface
   //@{

   //! Read the setting value information
   //! @param aToken is the setting command-value pair from which to read the setting value
   void ReadValueP(const editor::token::CommandValuePair* aToken) override;
   //! Read the setting value information
   //! @param aToken is the setting command-value pair to which to write the setting value
   void WriteValueP(editor::token::CommandValuePair* aToken) override;
   //! Restores the default state of the setting value
   //! @note this does not edit any information in the back-end
   void RestoreDefaultP() override;

   //! Determines whether or not the widget is editable
   //! @return whether or not the widget is editable
   bool IsEditableP() const override { return mMultipleChoiceValue->currentData().toBool(); }

   //@}

   //! The value widget
   ut::qt::UiPointer<QComboBox> mMultipleChoiceValue;
};

//! Represents the UserInput value widget
//! @par details
//!   The UserInput value widget has a value that is user-defined and mutable.
//!   this is represented by the use of a QLineEdit.
class UserInput : public WidgetInterface
{
public:
   //! Constructs a UserInput
   //! @param aSetting is the setting::Data the value widget references
   //! @param aParent is a pointer to the parent widget
   UserInput(const setting::Data& aSetting, QWidget* aParent = nullptr);
   //! Destructs a UserInput
   ~UserInput() override = default;

   //! Get the value widget
   //! @return a pointer to the value widget
   QLineEdit* GetWidget() override { return mUserInputValue; }

private:
   //! @note the private, virtual interface
   //@{

   //! Read the setting value information
   //! @param aToken is the setting command-value pair from which to read the setting value
   void ReadValueP(const editor::token::CommandValuePair* aToken) override;
   //! Read the setting value information
   //! @param aToken is the setting command-value pair to which to write the setting value
   void WriteValueP(editor::token::CommandValuePair* aToken) override;
   //! Restores the default state of the setting value
   //! @note this does not edit any information in the back-end
   void RestoreDefaultP() override;

   //! Determines whether or not the widget is editable
   //! @return whether or not the widget is editable
   bool IsEditableP() const override { return mUserInputValue->isEnabled(); }

   //@}

   //! The value widget
   ut::qt::UiPointer<QLineEdit> mUserInputValue;
};

} // namespace ValueWidget
} // namespace setting
} // namespace EventOutput
} // namespace wizard

#endif // EVENT_OUTPUT_VALUE_WIDGETS_HPP
