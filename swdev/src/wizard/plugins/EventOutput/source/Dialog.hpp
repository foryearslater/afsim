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

#ifndef EVENT_OUTPUT_DIALOG_INTERFACE_HPP
#define EVENT_OUTPUT_DIALOG_INTERFACE_HPP

#include <vector>

#include <QDialog>
#include <QPointer>
#include <QString>

#include "UtCallbackN.hpp"
#include "ui_EventOutputWriterDialog.h"

namespace wizard
{

class Editor;

namespace EventOutput
{
namespace editor
{

class Editor;

namespace tokenizer
{

struct BlockTags;
class Tokenizer;

} // namespace tokenizer
} // namespace editor

namespace setting
{

class Widget;

namespace ValueWidget
{

enum class Type;

}

} // namespace setting

namespace event
{

class EventsGroupBoxWidget;

}

//! Provides the capability of adding or editing (csv_)event_output blocks in AFSIM input files
//! @par Purpose:
//!   This widget allows one to edit event_output blocks.
//! @par How to use the Settings tabs:
//!   Toggle the Add? check box to add a particular setting to the block.
//!   Change the value of the setting with the widget in the Value column.
//! @par How to use the Events tabs:
//!   Check the disable check box to write disable {event} to the block.
//!   Check the enable check box to write enable {event} to the block.>
//!   <strong>Note:</strong> Both of the check boxes unchecked does not write the event to the block.
//! @par Documentation:
//!   Right-click the setting or event command to bring up a context menu.
//!   Clicking the context menu item will bring open the documentation for that command in a web browser.
//!   <strong>Note:</strong> The links work best on Chrome and Firefox.  Edge and Internet Explorer seem to have issues
//!   with it...
//! @note A more detailed help text can be found <a
//! href="../../documentation/html/docs/wkf_plugin/wiz_event_output.html">here</a>
class Dialog : public QDialog
{
   Q_OBJECT
public:
   //! Denotes the tab in the EventOutput::Dialog's tab widget
   enum class Page
   {
      cDUMMY,             //!< denotes a dummy tab
      cCSV_EVENTS_LIST,   //!< denotes the csv_event_output events list
      cCSV_SETTINGS_LIST, //!< denotes the csv_event_output settings list
      cEVT_EVENTS_LIST,   //!< denotes the event_output events list
      cEVT_SETTINGS_LIST  //!< denotes the event_output settings list
   };

   //! Constructs a Dialog
   //! @param aShowHelpButton denotes whether or not to show the Help button
   //! @param aParent is a pointer to the parent widget
   //! @param aFlags denotes the flags that describe the look-and-feel of the window
   Dialog(bool aShowHelpButton, QWidget* aParent, Qt::WindowFlags aFlags);
   //! Destructs a Dialog
   ~Dialog() override;

signals:
   //! Signal to notify connected objects that the Dialog is being closed
   void Closed();

private slots:
   //! @name related to the tab widget
   //@{

   //! Slot executed when the CSV check box is checked/unchecked
   //! @param aState is the state of the CSV check box
   void OnCSV_Selected(int aState);
   //! Slot executed when the EVT check box is checked/unchecked
   //! @param aState is the state of the EVT check box
   void OnEVT_Selected(int aState);

   //@}

   //! @name related to signals in the DialogButtonsWidget
   //@{

   //! Slot executed when the Restore Defaults button is clicked
   void OnRestoreDefaults();
   //! Slot executed when the Okay button is clicked
   void OnOkayClick();
   //! Slot executed when the Cancel button is clicked
   void OnCancelClick();
   //! Slot executed when the Help button is clicked
   void OnHelpClick();

   //@}

   //! @name slots associated with the PluinUtil::Signals signalton
   //@{

   //! Slot executed when the Signals::AcquireSettings signal is emitted
   //! @param aWidget is a "return value parameter" denoting the setting::Widget requested by the emitter object
   //! @param aType is the type of setting::Widget to acquire for the emitter object (as a string)
   //! @note currently, the Signals::AcquireSettings signal is emitted only from the tokenizer::Tokenizer
   void OnAcquireSettings(setting::Widget*& aWidget, const QString& aType);
   //! Slot executed when the Signals::AcquireCommandInputType signal is emitted
   //! @param aInputType is a "return value parameter" denoting the ValueWidget::Type requested by the emitter object
   //! @param aCommand is the name of the setting whose ValueWidget::Type the emitter object requested (as a string)
   //! @note currently, the Signals::AcquireCommandInputType signal is emitted only from the tokenizer::Tokenizer
   void OnAcquireCommandInputType(setting::ValueWidget::Type& aInputType, const QString& aCommand);

   //@}

private:
   //! @name related to the tab widget
   //@{

   //! Disables the csv_event_output settings and events tabs
   void DisableCSV_EventOutput();
   //! Enables the csv_event_output settings and events tabs
   void EnableCSV_EventOutput();
   //! Disables the event_output settings and events tabs
   void DisableEVT_EventOutput();
   //! Enables the event_output settings and events tabs
   void EnableEVT_EventOutput();

   //@}

   //! Executed when a wizard::Editor closes
   //! @param aEditorPtr is a pointer to the wizard::Editor that is closing
   void OnEditorClosed(wizard::Editor* aEditorPtr);

   //! @name related to loading/saving (csv_)event_output
   //@{

   //! Slot executed when the current file changes
   //! @param aFilePath is the path (plus name) of the file whose (csv_)event_output blocks needs loading
   void LoadEventOutput(const QString& aFilePath);

   //! Converts the (csv_)event_output token::Tokens into a string stored in each tokenizer::Tokenizer of the current
   //! editor::Editor
   //! @param aPromptUser denotes whether or not to prompt the user about "stashing (caching)" changes
   void CacheEventOutput(bool aPromptUser);

   //@}

   //! The user interface of the EventOutput::Dialog
   Ui::EventOutputWriterDialog mUi;
   //! Whether or not the dummy tab is in the widget
   //! @note this variable serves as the offset when disabling/enabling widgets
   //! @note 0 means in the widget, -1 means not in the widget
   int mDummyTabInWidget{0};

   //! The map of source files
   std::map<QString, editor::Editor> mSourceFiles;
   //! The current file
   editor::Editor* mCurrentFile{nullptr};
   //! The history of accessing source files
   QStringList mFileHistory;
   //! Holds necessary non-Qt callbacks
   std::unique_ptr<UtCallbackN<void(wizard::Editor*)>> mEditorClosedCB;
};

} // namespace EventOutput
} // namespace wizard

#endif // EVENT_OUTPUT_DIALOG_INTERFACE_HPP
