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

#ifndef MAP_UTILS_TRANSLATE_SCENARIO_DIALOG_HPP
#define MAP_UTILS_TRANSLATE_SCENARIO_DIALOG_HPP

#include <QDialog>
#include <QSize>

#include "LineEditSliderManager.hpp"
#include "PluginUtil.hpp"
#include "ScenarioTransformation.hpp"
#include "ScenarioTransformationDialogInterface.hpp"
#include "ScenarioTransformationIO_Types.hpp"
#include "UtLatPos.hpp"
#include "UtLength.hpp"
#include "UtLonPos.hpp"
#include "UtOptional.hpp"
#include "ui_TranslateScenarioDialog.h"

namespace MapUtils
{
namespace PluginUtil
{
class Message;
}
namespace ScenarioTransformation
{
//! @brief namespace for helper objects, functions, and variables for use in TranslateScenario::Dialog
namespace TranslateScenario
{

//! @brief enumeration for TranslateScenario::Dialog's menu combo box
enum class Menu
{
   cSELECT_A_MODE          = 0, //!< the default, "invalid" mode
   cTRANSLATE_SCENARIO_TO  = 2, //!< absolute translation mode for entire scenario
   cTRANSLATE_SCENARIO_BY  = 3, //!< relative translation mode for entire scenario
   cTRANSLATE_SELECTION_TO = 4, //!< absolute translation mode for selection
   cTRANSLATE_SELECTION_BY = 5  //!< relative translation mode for selection
};

//! @brief enumeration for TranslateScenario::Dialog's line edit stack widgets
enum class Page
{
   cDEFAULT  = 0, //!< the default page
   cABSOLUTE = 1, //!< page for absolute translation
   cRELATIVE = 2  //!< page for relative translation
};

//! @brief denotes the mode selection menu in TranslateScenario::Dialog
static QStringList cTRANSLATE_MENU{"Select a Mode",
                                   "",
                                   "Translate Scenario to...",
                                   "Translate Scenario by...",
                                   "Translate Selection to...",
                                   "Translate Selection by..."};

//! @brief class definition of TranslateScenario::Dialog
//! @par Purpose:
//!    This dialog allows the user to translate an entire scenario or a selection of objects on the Map.
//! @par How to use:
//!    Either use the line edits or the sliders to translate the scenario/selection of objects.
//!    The line edits allows the user to input the location (in latitude/longitude) or the distance (value and unit).
//!    The slider allows the user to input the degree of the location.
//!  @par Change modes:
//!    Use the combo box to change modes.
//! - "Translate Scenario to..." allows the user to translate the scenario to a specific location.
//! - "Translate Selection to..." allows the user to translate the selection of objects to a specific location.
//! - "Translate Scenario by..." allows the user to translate the scenario by a specific distance.
//! - "Translate Selection by..." allows the user to translate the selection of objects by a specific distance.
//! @note A more detailed help text can be found <a href="../../documentation/html/docs/wkf_plugin/wiz_translate.html">here</a>.
class Dialog final : public DialogInterface
{
   Q_OBJECT

public:
   using LineEditSliderManager = MapUtils::PluginUtil::LineEditSliderManager;
   using TransformationType    = MapUtils::ScenarioTransformation::Type;
   using DirectionCharacters   = MapUtils::ScenarioTransformation::IO_Types::DirectionCharacters;

   //! @brief constructs a TranslateScenario::Dialog
   //! @param aParent is the parent QWidget
   //! @param aFlags is variable that describes the properties and appearance of the dialog
   //! @param aCurrentMode is the mode with which to start up the dialog
   //! @param aShowHelpButton determines whether or not to show the "Help" button
   //! @param aShowModeMenu determines whether or not to show the mode menu (combo box)
   Dialog(QWidget* aParent, Qt::WindowFlags aFlags, const Menu aCurrentMode, const bool aShowHelpButton, const bool aShowModeMenu);

   //! @brief destructs a TranslateScenario::Dialog
   ~Dialog() override = default;

protected slots:
   //! @brief overridden slot from DialogInterface that executes when "Okay" button is pressed
   void accept() override;

   //! @brief overridden slot from DialogInterface that executes when RotateScenario::Dialog is closed
   void reject() override;

   //! @brief executes when the latitude line edit or slider is modified
   //! @param aPropertyValue is the property value as a string
   //! @pre the LineEditSliderManager acquired the data
   //! @pre the acquired data has a proper format and is in-range
   void LatitudeModified(const QString& aPropertyValue);

   //! @brief executes when the longitude line edit or slider is modified
   //! @param aPropertyValue is the property value as a string
   //! @pre the LineEditSliderManager acquired the data
   //! @pre the acquired data has a proper format and is in-range
   void LongitudeModified(const QString& aPropertyValue);

   //! @brief executes when the vertical length line edit is modified
   //! @param aPropertyValue is the property value as a string
   //! @pre the LineEditSliderManager acquired the data
   //! @pre the acquired data has a proper format and is in-range
   void VerticalLengthModified(const QString& aPropertyValue);

   //! @brief executes when the horizontal length line edit is modified
   //! @param aPropertyValue is the property value as a string
   //! @pre the LineEditSliderManager acquired the data
   //! @pre the acquired data has a proper format and is in-range
   void HorizontalLengthModified(const QString& aPropertyValue);

   //! @brief executed when the property unit in the vertical line edit changes
   //! @param aPropertyUnit is the string representation of the unit to which to convert the property
   void UpdateVerticalUnits(const QString& aPropertyUnit);

   //! @brief executed when the property unit in the horizontal line edit changes
   //! @param aPropertyUnit is the string representation of the unit to which to convert the property
   void UpdateHorizontalUnits(const QString& aPropertyUnit);

   //! @brief executed when the user changes modes via the combo box
   //! @param aState the mode to which the user switched
   void ModeChanged(const int aState);

protected:
   Dialog()                       = default;
   Dialog(const Dialog& /*aSrc*/) = default;
   Dialog(Dialog&& /*aSrc*/)      = default;

   Dialog& operator=(const Dialog& /*aSrc*/) = default;
   Dialog& operator=(Dialog&& /*aSrc*/) = default;

   //! @brief resets the ghost scenario/selection back to the original state
   void ResetP() override;

   //! @brief performs the absolute latitudinal translation of the scenario/selection of entities
   //! @param aLatitude is the latitude to which to translate the latitude
   //! @note this translation acts as a "temporary" translation, as it only acts on the ghost scenario/selection
   void TranslateLatitudeAbsolute(const UtLatPos aLatitude);

   //! @brief performs the relative latitudinal translation of the scenario/selection of entities
   //! @param aVerticalLength is the length by which to translate the latitude
   //! @note this translation acts as a "temporary" translation, as it only acts on the ghost scenario/selection
   void TranslateLatitudeRelative(const UtLengthValue aVerticalLength);

   //! @brief performs the absolute longitudinal translation of the scenario/selection of entities
   //! @param aLongitude is the longitude to which to translate the longitude
   //! @note this translation acts as a "temporary" translation, as it only acts on the ghost scenario/selection
   void TranslateLongitudeAbsolute(const UtLonPos aLongitude);

   //! @brief performs the relative longitudinal translation of the scenario/selection of entities
   //! @param aHorizontalLength is the length by which to translate the longitude
   //! @note this translation acts as a "temporary" translation, as it only acts on the ghost scenario/selection
   void TranslateLongitudeRelative(const UtLengthValue aHorizontalLength);

private:
   //! @brief initializes the UI components that need initialization
   //! @note for TranslateScenario::Dialog, the components needing initialization are:
   //! - the line edit labels
   //! - the line edit status labels
   //! - (in relative mode) the length units
   //! - (in absolute mode) the (latitude/longitude) managers, which initializes:
   //!    - the (latitude/longitude) line edit
   //!    - the (latitude/longitude) slider
   //! - (in relative mode) the (vertical/horizontal) managers, which initializes:
   //!    - the (vertical/horizontal) line edit
   void InitializeComponents() override;

   //! @name routines to initialize the UI components
   //@{

   //! @brief initializes the labels
   void InitializeLabels();

   //! @brief initializes the signal manager
   //! @note this initializes the managers based on the type of translation ("absolute" or "relative")
   void InitializeManagers();

   //! @brief initializes the units
   //! @param aUnit is the value with which to initialize the variables
   //! @note the parameter is used only for the relative properties
   void InitializeUnits(const UtUnitLength::BaseUnit aUnit);

   //@}

   bool WithinBounds(const UtLatPos aLatitude) const;

   //! @brief determines whether or not there is a "need" to apply translation
   //! @return whether or not it is necessary to apply translation
   //! @note prevents unnecessary calls to ApplyTranslation
   bool NeedToApplyTranslation() const;

   //! @brief applies the translation to the ghost scenario/selection
   void ApplyTranslation();

   //! @brief enables certain UI components
   void UI_Enable() override;

   //! @brief disables certain UI components
   void UI_Disable() override;

   //! @brief does necessary changes to the UI view for absolute translation modes
   void SwitchToAbsolute();

   //! @brief does necessary changes to the UI view for relative translation modes
   void SwitchToRelative();

   //! @brief denotes the TranslateScenario::Dialog GUI
   Ui::TranslateScenarioDialog mUi{};

   //! @brief denotes the help text of TranslateScenario::Dialog
   static const Message cTRANSLATE_HELP_TEXT;

   //! @name variables for the UI behavior
   //@{

   //! @brief signal manager for the latitude line edit and slider
   LineEditSliderManager mLatitudeManager;

   //! @brief signal manager for the longitude line edit and slider
   LineEditSliderManager mLongitudeManager;

   //! @brief signal manager for the latitude line edit and slider
   LineEditSliderManager mVerticalManager;

   //! @brief signal manager for the longitude line edit and slider
   LineEditSliderManager mHorizontalManager;

   //! @brief variable to keep track of the previous combo box mode
   //! @note to prevent confusion with magic numbers, this is a Menu enum
   Menu mPreviousMode{Menu::cSELECT_A_MODE};

   //! @brief denotes whether or not the dialog is in "relative" translation mode
   bool mRelative;

   //! @brief denotes the translation mode
   //! @note initialized to cRELATIVE_XYA
   //! @todo add preference to allow for cRELATIVE_LLA
   TranslationMode mTranslateMode{TranslationMode::cRELATIVE_XYA};

   //! @brief denotes the latitude/longitude format
   UtAngle::Fmt mAbsoluteFormat;

   //@}

   //! @name variables for storing the values for translation
   //@{

   //! @brief denotes the previous position of the centroid of the scenario/selection
   vespa::VaPosition mPreviousPosition{};

   //! @brief denotes the current position of the centroid of the scenario/selection
   vespa::VaPosition mCurrentPosition{};

   //! @name variables for relative translation
   //@{

   //! @brief denotes the previous vertical length
   UtLengthValue mPreviousVerticalLength{};

   //! @brief denotes the previous horizontal length
   UtLengthValue mPreviousHorizontalLength{};

   //@}

   //! @name variables to cap the translation (to prevent the "pole bug")

   //! @brief denotes the maximum latitude and longitude
   vespa::VaPosition mMaximumPosition{};

   //! @brief denotes the maximum latitude and longitude
   vespa::VaPosition mMinimumPosition{};

   //! @brief denotes the maximum vertical length
   UtLengthValue mMaximumLength{};

   //! @brief denotes the minimum vertical length
   UtLengthValue mMinimumLength{};

   //@}
};

} // namespace TranslateScenario
} // namespace ScenarioTransformation
} // namespace MapUtils

#endif // MAP_UTILS_TRANSLATE_SCENARIO_DIALOG_HPP
