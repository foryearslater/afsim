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

#ifndef MAP_UTILS_ROTATE_SCENARIO_DIALOG_HPP
#define MAP_UTILS_ROTATE_SCENARIO_DIALOG_HPP

#include <QDialog>
#include <QSize>

#include "LineEditSliderManager.hpp"
#include "PluginUtil.hpp"
#include "ScenarioTransformation.hpp"
#include "ScenarioTransformationDialogInterface.hpp"
#include "ScenarioTransformationIO_Types.hpp"
#include "VaFwd.hpp"
#include "ui_RotateScenarioDialog.h"

namespace MapUtils
{
namespace PluginUtil
{
class Message;
}

namespace ScenarioTransformation
{
//! @brief namespace for helper objects, functions, and variables for use in RotateScenario::Dialog
namespace RotateScenario
{

//! @brief denotes the menu item for the RotateScenario::Dialog mode selection menu
enum class Menu
{
   cSELECT_A_MODE    = 0, //!< the default, "invalid" mode
   cROTATE_SCENARIO  = 2, //!< rotation mode for entire scenario
   cROTATE_SELECTION = 3, //!< rotation mode for selection
};

//! @brief denotes the mode selection menu in RotateScenario::Dialog
static QStringList cROTATE_MENU{"Select a Mode", "", "Rotate Scenario", "Rotate Selection"};

//! @brief class definition of RotateScenario::Dialog
//! @par Purpose:
//!    This dialog allows the user to rotate an entire scenario or a selection of objects on the Map.
//! @par How to use:
//!    Either use the line edit or the dial to rotate the scenario/selection of objects.
//!    The line edit allows the user to input the angle (value and unit).
//!    The slider allows the user to input the angle as an integral value.
//! @par Change modes:
//!    Use the combo box to change modes.
//!    The available modes are:
//! - "Rotate Scenario" allows the user to rotate the scenario by a specific amount.
//! - "Rotate Selection" allows the user to rotate the selection by a specific amount.
//! @note A more detailed help text can be found <a href="../../documentation/html/docs/wkf_plugin/wiz_rotate.html">here</a>.
class Dialog final : public DialogInterface
{
   Q_OBJECT

public:
   using LineEditSliderManager = MapUtils::PluginUtil::LineEditSliderManager;
   using TransformationType    = MapUtils::ScenarioTransformation::Type;
   using DirectionCharacters   = MapUtils::ScenarioTransformation::IO_Types::DirectionCharacters;

   //! @brief constructs a RotateScenario::Dialog
   //! @param aParent is the parent QWidget
   //! @param aFlags is variable that describes the properties and appearance of the dialog
   //! @param aCurrentMode is the mode with which to start up the dialog
   //! @param aShowHelpButton determines whether or not to show the "Help" button
   //! @param aShowModeMenu determines whether or not to show the mode menu (combo box)
   Dialog(QWidget* aParent, Qt::WindowFlags aFlags, const Menu aCurrentMode, const bool aShowHelpButton, const bool aShowModeMenu);

   //! @brief destructs a RotateScenario::Dialog
   ~Dialog() override = default;

protected slots:
   //! @brief overridden slot from DialogInterface that executes when "Okay" button is pressed
   void accept() override;

   //! @brief overridden slot from DialogInterface that executes when RotateScenario::Dialog is closed
   void reject() override;

   //! @brief executes when the angle line edit or slider is modified
   //! @param aPropertyValueAndUnit is the property value and unit as a string
   //! @pre the LineEditSliderManager acquired the data
   //! @pre the acquired data has a proper format and is in-range
   void AngleModified(const QString& aPropertyValueAndUnit);

   //! @brief executed when the property unit in the line edit changes
   //! @param aPropertyUnit is the string representation of the unit to which to convert the property
   void UnitsModified(const QString& aPropertyUnit);

   //! @brief executed when the user changes modes via the combo box
   //! @param aState the mode to which the user switched
   void ModeChanged(const int aState);

protected:
   Dialog()                       = default;
   Dialog(const Dialog& /*aSrc*/) = default;
   Dialog(Dialog&& /*aSrc*/)      = default;

   Dialog& operator=(const Dialog& /*aSrc*/) = default;
   Dialog& operator=(Dialog&& /*aSrc*/) = default;

private:
   //! @brief it resets the ghost scenario/selection back to the original state
   void ResetP() override;

   //! @brief determines whether or not there is a "need" to apply rotation
   //! @return whether or not it is necessary to apply rotation
   //! @note prevents unnecessary calls to ApplyRotation
   bool NeedToApplyRotation();

   //! @brief performs the rotation of the ghost scenario/selection
   //! @note this rotation acts as a "temporary" rotation, as it only acts on the ghost scenario/selection
   void ApplyRotation();

   //! @brief initializes the UI components that need initialization
   //! @note for RotateScenario::Dialog, the components needing initialization are:
   //! - the line edit status label
   //! - the angle units
   //! - the angle manager, which initializes:
   //!    - the line edit
   //!    - the dial
   void InitializeComponents() override;

   //! @name routines to initialize the UI components
   //@{

   //! @brief initializes the label
   void InitializeLabel();

   //! @brief initializes the units of the variables that use units
   //! @param aUnit is the value with which to initialize the variables
   void InitializeUnits(const UtUnitAngle::BaseUnit aUnit);

   //@}

   //! @brief enables certain UI components
   void UI_Enable() override;

   //! @brief disables certain UI components
   void UI_Disable() override;

   //! @brief denotes the RotateScenario::Dialog GUI
   Ui::RotateScenarioDialog mUi{};

   //! @brief denotes the help text RotateScenario::Dialog
   static const Message cROTATE_HELP_TEXT;

   //! @name variables for the UI behavior
   //@{

   //! @brief signal manager for the line edit and slider
   LineEditSliderManager mAngleManager;

   //! @brief variable to keep track of the previous combo box mode
   //! @note to prevent confusion with magic numbers, this is a Menu enum
   Menu mPreviousMode{Menu::cSELECT_A_MODE};

   //@}

   //! @name variables for storing the values for rotation
   //! @todo allow for DMS format in the future
   //@{

   //! @brief denotes the current angle value
   //! @note this is the value from the line edit
   UtAngleValue mCurrentAngle{0.0};

   //! @brief denotes the previous angle value
   UtAngleValue mPreviousAngle{0.0};

   //@}
};

} // namespace RotateScenario
} // namespace ScenarioTransformation
} // namespace MapUtils

#endif // MAP_UTILS_ROTATE_SCENARIO_DIALOG_HPP
