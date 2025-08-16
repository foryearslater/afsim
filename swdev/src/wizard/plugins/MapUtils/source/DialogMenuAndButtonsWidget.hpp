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

#ifndef MAP_UTILS_DIALOG_MENU_AND_BUTTONS_WIDGET_HPP
#define MAP_UTILS_DIALOG_MENU_AND_BUTTONS_WIDGET_HPP

#include <QWidget>

#include "ui_DialogMenuAndButtonsWidget.h"

namespace MapUtils
{
namespace ScenarioTransformation
{

//! @brief widget containing the menu and buttons common to the MapUtils::ScenarioTransformation dialogs
//! @par details
//!    This widget provides a standard set of components used by the MapUtils::ScenarioTransformation dialogs.
//!    it provides a menu in the form of a combo box and provides three standard buttons.
//!    the buttons are:
//! - Okay (for applying changes and closing the dialog)
//! - Cancel (for undoing temporary changes and closing the dialog)
//! - Apply (for applying changes)
//! - Reset (for undoing temporary changes)
//! - Help (for displaying help text)
//! @note Not all of the buttons are available.  They are available in two different groups
//! - Okay and Cancel
//! - Apply and Reset
//! @note The "Help" button is exempt to the above note.
class DialogMenuAndButtonsWidget : public QWidget
{
   Q_OBJECT

public:
   //! @brief denotes what button was clicked
   enum class WhatButton
   {
      cNONE,   //!< denotes no button was clicked
      cOKAY,   //!< denotes the "Okay" button was clicked
      cCANCEL, //!< denotes the "Cancel" button was clicked
      cAPPLY,  //!< denotes the "Apply" button was clicked
      cRESET,  //!< denotes the "Reset" button was clicked
      cHELP    //!< denotes the "Help" button was clicked
   };

   //! @brief constructs a DialogMenuAndButtonsWidget
   //! @param aParent is the parent widget
   //! @param aFlags is the flags describing the look-and-feel of the window
   DialogMenuAndButtonsWidget(QWidget* aParent, Qt::WindowFlags aFlags = Qt::WindowFlags());
   DialogMenuAndButtonsWidget(DialogMenuAndButtonsWidget&& /*aSrc*/) = default;

   //! @brief destructs a DialogMenuAndButtonsWidget
   ~DialogMenuAndButtonsWidget() override = default;

   DialogMenuAndButtonsWidget& operator=(DialogMenuAndButtonsWidget&& /*aSrc*/) = default;

   //! @brief constructs the combo box menu
   //! @param aMenuItemList is the item list to which to create the combo box menu
   //! @note an empty string for an item means a separator
   void ConstructMenu(const QStringList& aMenuItemList);

   //! @name convenient accessor methods
   //@{

   //! @brief accesses the combo box menu
   UtQtComboBox* Menu() { return mUi.mMenu; }

   //! @brief accesses the "Okay" button
   QPushButton* Okay() { return mUi.mOkay; }

   //! @brief accesses the "Cancel" button
   QPushButton* Cancel() { return mUi.mCancel; }

   //! @brief accesses the "Apply" button
   QPushButton* Apply() { return mUi.mApply; }

   //! @brief accesses the "Reset" button
   QPushButton* Reset() { return mUi.mReset; }

   //! @brief accesses the "Help" button
   QPushButton* Help() { return mUi.mHelp; }

   //@}

   //! @name functions controlling the visibility of the buttons and menu
   //@{

   //! @brief switches the UI view to use "Okay" and "Cancel" buttons
   //! @post only the "Okay" and "Cancel" buttons are shown
   //! @note this does not apply to the "Help" button as it has its own Show/Hide functions
   void UseOkayAndCancel();

   //! @brief switches the UI view to use "Apply" and "Reset" buttons
   //! @post only the "Apply" and "Reset" buttons are shown
   //! @note this does not apply to the "Help" button as it has its own Show/Hide functions
   void UseApplyAndReset();

   //! @brief hides the combo box menu
   void HideMenu();

   //! @brief shows the combo box menu
   void ShowMenu();

   //! @brief hides the "Help" button
   void HideHelpButton();

   //! @brief shows the "Help" button
   void ShowHelpButton();

   //@}

protected:
   DialogMenuAndButtonsWidget()                                           = delete;
   DialogMenuAndButtonsWidget(const DialogMenuAndButtonsWidget& /*aSrc*/) = delete;

   DialogMenuAndButtonsWidget& operator=(const DialogMenuAndButtonsWidget& /*aSrc*/) = delete;

private:
   //! @brief denotes the menu and buttons widget GUI for MapUtils::ScenarioTransformation dialogs
   Ui::DialogMenuAndButtonsWidget mUi;

   //! @name convenient control variables
   //@{

   //! @brief denotes whether or not the "Okay" and "Cancel" buttons are used
   bool mUsingOkayAndCancel = false;

   //! @brief denotes whether or not the "Apply" and "Reset" buttons are used
   bool mUsingApplyAndReset = false;

   //! @brief denotes whether or not the "Menu" combo box is hidden
   bool mMenuHidden = false;

   //! @brief denotes whether or not the "Help" button is hidden
   bool mHelpButtonHidden = false;

   //@}
};

} // namespace ScenarioTransformation
} // namespace MapUtils

#endif // MAP_UTILS_DIALOG_MENU_AND_BUTTONS_WIDGET_HPP
