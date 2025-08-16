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

#ifndef EVENT_OUTPUT_PLUGIN_UTIL_HPP
#define EVENT_OUTPUT_PLUGIN_UTIL_HPP

#include <QCheckBox>
#include <QPointer>
#include <QString>

#include "EventWidget.hpp"
#include "SettingValueWidget.hpp"
#include "UtMemory.hpp"

#define evtOutSignals wizard::EventOutput::PluginUtil::mSignals.get()

namespace wizard
{
namespace EventOutput
{
namespace editor
{
namespace token
{

struct CommandValuePair;

}
} // namespace editor

namespace setting
{

class Widget;

namespace ValueWidget
{

enum class Type;

}
} // namespace setting

namespace PluginUtil
{

//! Signals for EventOutput::Plugin
//! @par details
//!   This object contains signals used by non-QObjects to acquire information from QObjects via the
//!   EventOutput::Dialog. it is also a signalton (pun intended).
struct Signals final : public QObject
{
   Q_OBJECT
public:
   //! Constructs a Signals
   //! @param aParent is a pointer to the parent object
   Signals(QObject* aParent = nullptr);
   //! Destructs a Signals
   ~Signals() override;

signals:
   //! Signal to notify the connected object that the emitter objects would like to acquire a setting::Widget of a
   //! particular type
   //! @param aWidget is the "return value parameter" of the setting::Widget requested by the emitter object
   //! @param aType is the type of the setting::Widget requested by the emitter object
   void AcquireSettings(setting::Widget*& aWidget, const QString& aType);
   //! Signal to notify the connected object that the emitter objects would like to acquire a ValueWidget::Type of a
   //! particular command
   //! @param aInputType is the type of value widget requested by the emitter object
   //! @param aCommand is the name of the property whose value widget type the emitter object requested
   void AcquireCommandInputType(setting::ValueWidget::Type& aInputType, const QString& aCommand);

private:
   //! The instance pointer to prevent multiple signaltons
   static Signals* mInstancePtr;
};

extern std::unique_ptr<Signals> mSignals;

//! Set the value of a check box based on a string == comparison
//! @param aCheckBoxPtr is a pointer to the check box to set
//! @param aValue is the value to test
//! @param aCheckedValue is the identifier for the true value
void SetBoolCheckBox(QCheckBox* aCheckBoxPtr, const QString& aValue, const QString& aCheckedValue = "true");
//! Set the value of a BooleanCheckBoxWidget based on a string == comparison
//! @param aBooleanCheckBoxPtr is a pointer to the BooleanCheckBoxWidget to set
//! @param aValue is the boolean value
//! @param aFalseValue is the identifier for the false value
//! @param aTrueValue is the identifier for the true value
bool SetBoolCheckBox(BooleanCheckBoxWidget* aBooleanCheckBoxPtr,
                     const QString&         aValue,
                     const QString&         aFalseValue = "false",
                     const QString&         aTrueValue  = "true");

//! Set the value of a command-value pair representing a boolean value
//! @param aToken is a reference to the command-value pair to set
//! @param aValue is the boolean value
//! @param aFalseValue is the identifier for the false value
//! @param aTrueValue is the identifier for the true value
void SetBoolToken(editor::token::CommandValuePair& aToken,
                  bool                             aValue,
                  const QString&                   aFalseValue = "false",
                  const QString&                   aTrueValue  = "true");
//! Set the value of a command-value pair representing a boolean value
//! @param aToken is a reference to the command-value pair to set
//! @param aValue is the boolean value
//! @param aFalseValue is the identifier for the false value
//! @param aTrueValue is the identifier for the true value
void SetBoolToken(editor::token::CommandValuePair& aToken,
                  const QString&                   aValue,
                  const QString&                   aFalseValue = "false",
                  const QString&                   aTrueValue  = "true");
//! Set the value of a command-value pair representing a boolean value
//! @param aToken is a reference to the command-value pair to set
//! @param aState is the overall EventRadioWidget state
//! @param aFalseValue is the identifier for the false value
//! @param aTrueValue is the identifier for the true value
bool SetBoolToken(editor::token::CommandValuePair& aToken,
                  event::Value                     aState,
                  const QString&                   aFalseValue = "false",
                  const QString&                   aTrueValue  = "true");

} // namespace PluginUtil
} // namespace EventOutput
} // namespace wizard

#endif // EVENT_OUTPUT_PLUGIN_UTIL_HPP
