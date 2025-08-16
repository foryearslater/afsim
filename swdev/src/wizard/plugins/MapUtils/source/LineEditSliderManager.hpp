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

#ifndef LINE_EDIT_SLIDER_MANAGER_HPP
#define LINE_EDIT_SLIDER_MANAGER_HPP

#include <cmath>
#include <utility>

#include <QAbstractSlider>
#include <QLineEdit>
#include <QObject>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "Message.hpp"
#include "PluginUtil.hpp"
#include "ScenarioTransformation.hpp"
#include "ScenarioTransformationIO_Types.hpp"
#include "UtQtUnitLineEdit.hpp"
#include "UtQtUnitValueEdit.hpp"

namespace MapUtils
{
namespace PluginUtil
{

//! @brief manages the interactions between a line edit and a slider
//! @par details
//!    For the ScenarioTransformation dialogs, the line edits and the sliders have to modify each other.
//!    The line edit has two signals that trigger when the line edit is modified:  textChanged and textEdited.
//!    The slider has two signals that trigger when the slider is modified:  valueChanged and sliderMoved.
//!    The signals textChanged, valueChanged, and sliderMoved trigger if the user interacts with the component
//!    or the program modifies the component.  However, textEdited ONLY triggers if the user interacts with the
//!    component. It would seem that the signals would trigger an infinite loop, but eventually the cycle breaks when
//!    QAbstractSlider::setValue, as the slider ignores calls to that function if it has the same value that is passed.
//!    Nevertheless, it causes subroutines that are called by the slots to execute unnecessarily.
//!    This object blocks the signals of one component when updating the other component, thus breaking the cycle.
//!    To keep similar behavior, it contains two signals:  ValueChanged and ValueEdited.
//!    ValueChanged triggers when the user OR the program modifies either the line edit or the slider.
//!    ValueEdited triggers ONLY when the user modifies either the line edit or the slider.
class LineEditSliderManager : public QObject
{
   Q_OBJECT

public:
   using DirectionCharacters = MapUtils::ScenarioTransformation::IO_Types::DirectionCharacters;
   using Type                = ScenarioTransformation::Type;
   using RangeD              = MapUtils::ScenarioTransformation::IO_Types::RangeD;
   using ValueAndUnitDI      = MapUtils::ScenarioTransformation::IO_Types::ValueAndUnitDI;
   using ValueAndUnitSS      = MapUtils::ScenarioTransformation::IO_Types::ValueAndUnitSS;

   //! @brief constructs the manager
   //! @param aSliderPtr is a pointer to the slider
   //! @param aLineEditPtr is a pointer to the line edit
   //! @param aType is the ScenarioTransformation::Type occurring
   //! @param aRelative is whether or not the transformation is dealing with a relative property (in particular, angle
   //! and length)
   //! @param aDirChars determines the positive and negative characters for an absolute property (in particular,
   //! latitude and longitude)
   explicit LineEditSliderManager(QAbstractSlider*          aSliderPtr,
                                  QLineEdit*                aLineEditPtr,
                                  const Type                aType,
                                  const bool                aRelative,
                                  const DirectionCharacters aDirChars) noexcept;
   LineEditSliderManager(LineEditSliderManager&& /* aSrc */) = default;

   //! @brief destructs a LineEditSliderManager
   ~LineEditSliderManager() override = default;

   LineEditSliderManager& operator=(LineEditSliderManager&& /* aSrc */) = default;

   //! @name the initialization routines
   //@{

   //! @brief initializes the manager's property value and unit
   //! @param aPropertyValue is the property value
   //! @param aPropertyUnit is the property unit
   void Initialize(const double aPropertyValue, const int aPropertyUnit) noexcept;

   //! @brief initializes the manager's property value and unit
   //! @param aPropertyValue is the property value
   //! @param aPropertyUnit is the property unit
   void Initialize(const QString& aPropertyValue, const int aPropertyUnit) noexcept;

   //! @brief initializes the manager's property value and unit
   //! @param aPropertyValue is the property value
   //! @param aPropertyUnit is the property unit
   void Initialize(const double aPropertyValue, const QString& aPropertyUnit) noexcept;

   //! @brief initializes the manager's property value and unit
   //! @param aPropertyValue is the property value
   //! @param aPropertyUnit is the property unit
   void Initialize(const QString& aPropertyValue, const QString& aPropertyUnit) noexcept;

   //@}

   //! @brief sets the pointer to the line edit
   //! @param aLineEditPtr is a pointer to the line edit
   void SetLineEdit(QLineEdit* aLineEditPtr) noexcept;

   //! @brief sets the pointer to the slider
   //! @param aSliderPtr is a pointer to the line edit
   void SetSlider(QAbstractSlider* aSliderPtr) noexcept;

   //! @brief sets the property value
   //! @param aPropertyValue is the value of the property in double format
   void SetManagerValueDouble(const double aPropertyValue) noexcept;

   //! @brief gets the property value
   //! @return the value of the property in double format
   double GetManagerValueDouble() const noexcept;

   //! @brief sets the property value
   //! @param aPropertyValueAndUnit is the value of the property in string format
   void SetManagerValueString(const QString& aPropertyValueAndUnit) noexcept;

   //! @brief gets the property value
   //! @return the value of the property in string format
   QString GetManagerValueString() const noexcept;

   //! @brief sets the property units
   //! @param aUnit is the units of the property in int format
   void SetManagerUnitInt(const int aUnit) noexcept;

   //! @brief gets the property units
   //! @return the units of the property in int format
   int GetManagerUnitInt() const noexcept;

   //! @brief sets the property units
   //! @param aPropertyValueAndUnit is the units of the property in string format
   void SetManagerUnitString(const QString& aPropertyValueAndUnit) noexcept;

   //! @brief gets the property units
   //! @return the units of the property in string format
   QString GetManagerUnitString() const noexcept;

   //! @brief sets whether or not the property is relative
   //! @param aRelative is whether or not the property is relative
   void SetRelative(const bool aRelative) noexcept;

   //! @brief gets whether or not the property is relative
   //! @return whether or not the property is relative
   bool GetRelative() const noexcept;

   //! @brief sets the range of the property
   //! @param aRange is the property range (minimum and maximum)
   //! @note if initializing, do not use this
   void SetRange(const RangeD& aRange) noexcept;

   //! @brief gets the range of the property
   //! @return the property range (minimum and maximum)
   RangeD GetRange() const noexcept;

   //! @brief sets the line edit and slider range
   //! @param aUnit is the property units
   //! @param aRange is the property range (minimum and maximum)
   //! @note this routine is used by the ScenarioTransformation dialogs for initialization
   void SetDefaultRange(const int aUnit, const RangeD& aRange) noexcept;

   //! @brief determines whether or not the property value and unit format is valid
   //! @return whether or not the format is valid
   bool IsFormatValid() noexcept;

   //! @brief acquires the property value and unit
   //! @return the value and unit in string format
   QString GetLineEditValueAndUnitString() const noexcept;

   //! @brief acquires the property value and unit
   //! @return the value and unit in ValueAndUnitSS format
   ValueAndUnitSS GetLineEditValueAndUnitStringPair() const noexcept;

   //! @brief acquires the property value and unit
   //! @return the value and unit in ValueAndUnitDI format
   ValueAndUnitDI GetLineEditValueAndUnitDoubleIntPair() const noexcept;

   //! @brief acquires the property value
   //! @return the value in string format
   QString GetLineEditValueString() const noexcept;

   //! @brief acquires the property value
   //! @return the value in string format
   double GetLineEditValueDouble() const noexcept;

   //! @brief acquires the property unit
   //! @return the units in string format
   QString GetLineEditUnitString() const noexcept;

   //! @brief acquires the property unit
   //! @return the units in string format
   int GetLineEditUnitInt() const noexcept;

signals:
   //! @brief signal to notify that the units of the line edit were changed
   void UnitChanged();

   //! @brief signal to notify that the units of the line edit were changed
   //! @param aPropertyUnit is the units in int format
   //! @note this signal emits if and only if in relative mode AND if the units actually changed
   void UnitChangedInt(const int aPropertyUnit);

   //! @brief signal to notify that the units of the line edit were changed
   //! @param aPropertyUnit is the units in string format
   //! @note this signal emits if and only if in relative mode AND if the units actually changed
   void UnitChangedString(const QString& aPropertyUnit);

   //! @brief signal to notify that the value of the line edit or the slider changed
   //! @note the change is EITHER by the user OR by the program
   void ValueChanged();

   //! @brief signal to notify that the value of the line edit or the slider changed
   //! @param aPropertyValue is the value in double format
   //! @note the change is EITHER by the user OR by the program
   void ValueChangedDouble(const double aPropertyValue);

   //! @brief signal to notify that the value of the line edit or the slider changed
   //! @param aPropertyValue is the value in string format
   //! @note the change is EITHER by the user OR by the program
   void ValueChangedString(const QString& aPropertyValue);

   //! @brief signal to notify that the value of the line edit or the slider changed
   //! @note the change is by the user ONLY
   void ValueEdited();

   //! @brief signal to notify that the value of the line edit or the slider changed
   //! @param aPropertyValue is the value in double format
   //! @note the change is by the user ONLY
   void ValueEditedDouble(const double aPropertyValue);

   //! @brief signal to notify that the value of the line edit or the slider changed
   //! @param aPropertyValue is the value in string format
   //! @note the change is by the user ONLY
   void ValueEditedString(const QString& aPropertyValue);

   //! @brief signal to notify that the format is invalid
   //! @note this signal is intended for a status label
   void InvalidFormat();

   //! @brief signal to notify that the format is valid
   //! @note this signal is intended for a status label
   void ValidFormat();

   //! @brief signal to notify the connected object to handle a message
   //! @param aProblem is the message to handle
   void HandleMessage(const Message* aProblem);

protected:
   LineEditSliderManager()                                        = delete;
   LineEditSliderManager(const LineEditSliderManager& /* aSrc */) = delete;

   LineEditSliderManager& operator=(const LineEditSliderManager& /* aSrc */) = delete;
   ;

private:
   //! @brief connects the appropriate signals to the appropriate slots
   void Connect() noexcept;

   //! @brief disconnects the appropriate signals from the appropriate slots
   void Disconnect() noexcept;

   //! @brief handles the slider when the QAbstractSlider::valueChanged signal is emitted
   //! @param aPropertyValue is the current value of the slider
   void HandleSlider(const int aPropertyValue) noexcept;

   //! @brief handles the line edit when the QLineEdit::textChanged signal is emitted
   //! @param aPropertyValue is the current value of the line edit
   void HandleLineEdit(const QString& aPropertyValue) noexcept;

   //! @brief converts a property from degree, minute, second (DMS) format to decimal format
   //! @param aStrDMS is a string representing the value to convert
   QString ConvertDMS_ToDouble(const QString& aStrDMS) noexcept;

   //! @brief converts a property from decimal format to degree, minute, second (DMS) format
   //! @param aStrDouble is a string representing the value to convert
   QString ConvertDoubleToDMS(const QString& aStrDouble) noexcept;

   //! @brief sets the range of the line edit
   //! @param aUnit is the units for which to acquire the range
   //! @return whether or not it set the range
   bool SetLineEditRange(const int aUnit) noexcept;

   //! @brief sets the range of the slider
   void SetSliderRange() noexcept;

   //! @brief parses the value and unit, ignoring all whitespace (leading, separating, and trailing)
   //! @param aPropertyValueAndUnit is the property value and unit
   //! @pre the format of aPropertyValueAndUnit is valid
   ValueAndUnitSS ParseValueAndUnit(const QString& aPropertyValueAndUnit) const noexcept;

   //! @brief clamps the value between the maximum and minimum
   //! @param aPropertyValue is the value to clamp
   void Clamp(double& aPropertyValue) noexcept;

   //! @brief sets the value of the line edit
   //! @param aPropertyValueAndUnit is the property value and unit with which to set the line edit
   //! @pre the slider was modified
   //! @note will block the signals of the slider before modifying the slider value
   void SetLineEditValue(const QString& aPropertyValueAndUnit) noexcept;

   //! @brief sets the value of the slider
   //! @param aPropertyValue is the property value with which to set the slider
   //! @pre the line edit was modified
   //! @note will block the signals of the line edit before modifying the line edit value
   void SetSliderValue(const int aPropertyValue) noexcept;

   //! @brief emits the UnitChanged signals
   void EmitUnitChangedSignals() noexcept;

   //! @brief emits the ValueChanged signals
   //! @param aPropertyValue is an optional string that contains the value with which to emit the signals
   void EmitValueChangedSignals(const QString& aPropertyValue = "") noexcept;

   //! @brief emits the ValueEdited signals
   //! @param aPropertyValue is an optional string that contains the value with which to emit the signals
   void EmitValueEditedSignals(const QString& aPropertyValue = "") noexcept;

   //! @name variables set on construction
   //@{

   //! @brief the line edit to manage
   QLineEdit* mLineEditPtr{nullptr};

   //! @brief the slider to manage
   QAbstractSlider* mSliderPtr{nullptr};

   //! @brief the type of transformation
   Type mType;

   //! @brief whether or not it is a relative property
   bool mRelative;

   //! @brief the direction characters for an absolute property
   DirectionCharacters mDirChars;

   //@}

   //! @brief variables set on initialization
   //@{

   //! @brief the property value in double format
   double mValueDouble;

   //! @brief the property value in string format
   QString mValueString;

   //! @brief the property units in int format
   int mUnitInt;

   //! @brief the property units in string format
   QString mUnitString;

   //! @brief the property range (minimum and maximum)
   RangeD mValueRange;

   //@}

   //! @brief denotes floating point format for QString::number when converting from double
   //! @note floating point IS NOT scientific notation
   static const char cFLOATING_POINT = 'f';
};

} // namespace PluginUtil
} // namespace MapUtils

#endif // LINE_EDIT_SLIDER_MANAGER_HPP
