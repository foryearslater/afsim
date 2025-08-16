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

#ifndef SCENARIO_TRANSFORMATION_DIALOG_INTERFACE_HPP
#define SCENARIO_TRANSFORMATION_DIALOG_INTERFACE_HPP

#include <QComboBox>
#include <QDialog>

#include "DialogMenuAndButtonsWidget.hpp"
#include "Message.hpp"
#include "PluginUtil.hpp"

namespace MapUtils
{
class Plugin;

namespace ScenarioTransformation
{

//! @brief base class of the MapUtils::ScenarioTransformation dialogs, providing common functionality
class DialogInterface : public QDialog
{
   Q_OBJECT

public:
   using Message    = MapUtils::Message;
   using WhatButton = DialogMenuAndButtonsWidget::WhatButton;

   //! @brief destructs a ScenarioTransformation::DialogInterface
   virtual ~DialogInterface() = default;

signals:
   //! @brief signal to notify connected objects that a MapUtils::ScenarioTransformation dialog has been closed
   void Closed();

protected slots:
   void accept() override = 0;

   //! @note this needs to be pure virtual, so that the OnForceClose slot can call the respective override
   void reject() override = 0;

   //! @brief executes when the MapUtils::ForceClose signal is fired
   //! @param aDialogPtr is the dialog that is being force closed
   virtual void OnForceClose(DialogInterface* aDialogPtr);

   //! @brief applies the changes from the ghost scenario/selection to the real scenario/selection
   //! @note this is a "permanent" transformation, as it acts on the real scenario/selection.
   //!    (it is not exactly permanent, since the changes can be undone in wizard)
   virtual void ApplyPressed();

   //! @brief resets the ghost scenario/selection back to the original state
   virtual void ResetPressed();

   //! @brief displays a brief version of the AFSIM help text for a MapUtils::ScenarioTransformation dialog
   virtual void HelpPressed() { mHelpTextPtr->Display(); }

protected:
   DialogInterface() = default;

   //! @brief constructs a ScenarioTransformation::DialogInterface
   //! @param aParent is the parent QWidget
   //! @param aFlags is variable that describes the properties and appearance of the widget
   //! @param aHelpText is a reference to the MapUtils::Message containing the help text
   DialogInterface(QWidget* aParent, Qt::WindowFlags aFlags, const Message* aHelpText);

   DialogInterface(const DialogInterface& /*aSrc*/) = default;
   DialogInterface(DialogInterface&& /*aSrc*/)      = default;

   DialogInterface& operator=(const DialogInterface& /*aSrc*/) = default;
   DialogInterface& operator=(DialogInterface&& /*aSrc*/) = default;

   //! @brief helper method for the accept slot
   //! @note only called when the "Okay" button is pressed
   void AcceptP();

   //! @brief helper method for the reject slot
   //! @note only called when the "Cancel" button or the "X" is pressed
   void RejectP();

   //! @brief helper method for the ResetPressed slot
   virtual void ResetP() = 0;

   //! @brief routine to execute before the mode switch
   void BeforeModeSwitch();

   //! @brief routine to execute after the mode switch
   void AfterModeSwitch();

   //! @brief initializes the components of the dialog
   virtual void InitializeComponents() = 0;

   //! @brief disables certain UI components
   virtual void UI_Disable() = 0;

   //! @brief enables certain UI components
   virtual void UI_Enable() = 0;

   //! @brief default handler of exceptions thrown in a MapUtils::ScenarioTransformation dialog
   //! @param aProblem is a pointer to the thrown exception
   //! @note see MapUtils::Message for more information about MapUtils exception handling
   virtual void HandleMessage(const Message* aProblem);

   //! @brief defaults to a transformation mode
   //! @tparam MODE is the type of MapUtils::ScenarioTransformation dialog mode menu item
   //! @pre the MODE type must be convertible to an integer
   //! @param aMode is the mode to which to default
   //! @param aMenuPtr is a pointer to the mode combo box of the particular MapUtils::ScenarioTransformation dialog
   template<typename MODE>
   void DefaultToMode(const MODE aMode, QComboBox* aMenuPtr)
   {
      static_assert(!std::is_convertible<MODE, int>::value,
                    "The \"MODE\" template parameter must be convertible to an int.");

      aMenuPtr->setCurrentIndex(static_cast<int>(aMode));
   }

   //! @brief control variable to denote if in a "selection" mode
   bool mSelectionOnly;

   //! @brief variable to denote what entities to ghost
   PluginUtil::WhatEntities mEntitiesToGhost;

   //! @name control variables for various reasons
   //@{

   //! @brief denotes what button was pressed
   //! @note needed to control behavior of various routines
   WhatButton mClickedButton;

   //! @brief denotes whether or not a problem occurred
   bool mDidAProblemOccur{false};

   //@}

   //! @brief denotes the initial position of the centroid of the scenario/selection
   //! @note for Rotate, this is the center of rotation (COR)
   //! @todo make the COR an user-definable position (issue #1602)
   //! @todo update this to be the Centroid type (issue #1995)
   vespa::VaPosition mCentroid;

   //! @brief denotes the help text
   //! @note references the derived class's help text
   const Message* mHelpTextPtr;
};

} // namespace ScenarioTransformation
} // namespace MapUtils

#endif // SCENARIO_TRANSFORMATION_DIALOG_INTERFACE_HPP
