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

#ifndef DIALOG_BUTTONS_WIDGET_HPP
#define DIALOG_BUTTONS_WIDGET_HPP

#include <QDialogButtonBox>

namespace wizard
{
namespace EventOutput
{

//! Widget containing the buttons used in the EventOutput::Dialog
//! @par details
//!    This widget provides a standard set of buttons used in the EventOutput::Dialog.
//!    the buttons are:
//! - Okay (for applying changes and closing the dialog)
//! - Cancel (for ignoring changes and closing the dialog)
//! - Help (for displaying help text)
//! - Restore Defaults (for resetting the state of components)
class DialogButtonsWidget : public QDialogButtonBox
{
   Q_OBJECT
public:
   //! Constructs a DialogButtonsWidget
   //! @param aParent is a pointer to the parent widget
   DialogButtonsWidget(QWidget* aParent = nullptr);
   //! Destructs a DialogButtonsWidget
   ~DialogButtonsWidget() override = default;

   //! @brief hides the "Help" button
   void HideHelpButton();

   //! @brief shows the "Help" button
   void ShowHelpButton();

signals:
   //! Signal to notify connected objects that the user clicked the "Okay" button
   void OkayClicked();
   //! Signal to notify connected objects that the user clicked the "Cancel" button
   void CancelClicked();
   //! Signal to notify connected objects that the user clicked the "Help" button
   void HelpClicked();
   //! Signal to notify connected objects that the user clicked the "Restore Defaults" button
   void RestoreDefaultsClicked();

private:
   //! Denotes whether or not the "Help" button is hidden
   bool mHelpButtonHidden = false;
};

} // namespace EventOutput
} // namespace wizard

#endif // DIALOG_BUTTONS_WIDGET_HPP
