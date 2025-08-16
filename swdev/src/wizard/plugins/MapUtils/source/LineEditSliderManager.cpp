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

#include "LineEditSliderManager.hpp"

#include <QSignalBlocker>

#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"

namespace MapUtils
{
namespace PluginUtil
{

LineEditSliderManager::LineEditSliderManager(QAbstractSlider*          aSliderPtr,
                                             QLineEdit*                aLineEditPtr,
                                             const Type                aType,
                                             const bool                aRelative,
                                             const DirectionCharacters aDirChars) noexcept
   : QObject()
   , mLineEditPtr(aLineEditPtr)
   , mSliderPtr(aSliderPtr)
   , mType(aType)
   , mRelative(aRelative)
   , mDirChars(aDirChars)
{
}

void LineEditSliderManager::Initialize(const double aPropertyValue, const int aPropertyUnit) noexcept
{
   Disconnect();
   SetManagerValueDouble(aPropertyValue);
   SetManagerUnitInt(aPropertyUnit);
   Connect();
}

void LineEditSliderManager::Initialize(const QString& aPropertyValue, const int aPropertyUnit) noexcept
{
   Disconnect();
   SetManagerValueString(aPropertyValue);
   SetManagerUnitInt(aPropertyUnit);
   Connect();
}

void LineEditSliderManager::Initialize(const double aPropertyValue, const QString& aPropertyUnit) noexcept
{
   Disconnect();
   SetManagerValueDouble(aPropertyValue);
   SetManagerUnitString(aPropertyUnit);
   Connect();
}

void LineEditSliderManager::Initialize(const QString& aPropertyValue, const QString& aPropertyUnit) noexcept
{
   Disconnect();
   SetManagerValueString(aPropertyValue);
   SetManagerUnitString(aPropertyUnit);
   Connect();
}

void LineEditSliderManager::SetLineEdit(QLineEdit* aLineEditPtr) noexcept
{
   Disconnect();
   mLineEditPtr = aLineEditPtr;
   Connect();
}

void LineEditSliderManager::SetSlider(QAbstractSlider* aSliderPtr) noexcept
{
   Disconnect();
   mSliderPtr = aSliderPtr;
   Connect();
}

void LineEditSliderManager::SetManagerValueDouble(const double aPropertyValue) noexcept
{
   QString propertyValueAndUnit;
   if (mRelative)
   {
      mValueDouble         = aPropertyValue;
      mValueString         = QString::number(aPropertyValue, cFLOATING_POINT);
      propertyValueAndUnit = mValueString + ' ' + mUnitString;
   }
   else
   {
      mValueDouble = aPropertyValue;
      mValueString = ConvertDoubleToDMS(QString::number(aPropertyValue, cFLOATING_POINT));
   }
   // set the values of the components
   SetLineEditValue(propertyValueAndUnit);
   SetSliderValue(static_cast<int>(mValueDouble));
   // emit the appropriate signals
   EmitValueChangedSignals();
}

double LineEditSliderManager::GetManagerValueDouble() const noexcept
{
   return mValueDouble;
}

void LineEditSliderManager::SetManagerValueString(const QString& aPropertyValueAndUnit) noexcept
{
   if (mRelative)
   {
      auto   pvauSplit = ParseValueAndUnit(aPropertyValueAndUnit);
      bool   success;
      double val = pvauSplit.mValue.toDouble(&success);
      if (success)
      {
         mValueDouble = val;
         mValueString = pvauSplit.mValue;
      }
   }
   else
   {
      QString valStr = ConvertDMS_ToDouble(aPropertyValueAndUnit);
      bool    success;
      double  val = valStr.toDouble(&success);
      if (success)
      {
         mValueDouble = val;
         mValueString = ConvertDoubleToDMS(valStr);
      }
   }
   // set the values of the components
   SetLineEditValue(aPropertyValueAndUnit);
   SetSliderValue(static_cast<int>(mValueDouble));
   // emit the appropriate signals
   EmitValueChangedSignals();
}

QString LineEditSliderManager::GetManagerValueString() const noexcept
{
   return mValueString;
}

void LineEditSliderManager::SetManagerUnitInt(const int aUnits) noexcept
{
   if (mRelative)
   {
      if (mType == Type::cANGLE)
      {
         mUnitInt    = aUnits;
         mUnitString = QString::fromStdString(UtUnitAngle::FindUnitName(aUnits));
      }
      else if (mType == Type::cLENGTH)
      {
         mUnitInt    = aUnits;
         mUnitString = QString::fromStdString(UtUnitLength::FindUnitName(aUnits));
      }
      else
      {
         // the relative units were not recognized
         mUnitInt    = -1;
         mUnitString = "";
      }
      // set the values of the components
      SetLineEditValue(mValueString + ' ' + mUnitString);
      SetSliderValue(static_cast<int>(mValueDouble));
      // emit the appropriate signals
      EmitUnitChangedSignals();
      EmitValueChangedSignals();
   }
   else
   {
      // units do not make sense for an absolute property
      mUnitInt    = -1;
      mUnitString = "";
   }
}

int LineEditSliderManager::GetManagerUnitInt() const noexcept
{
   return mUnitInt;
}

void LineEditSliderManager::SetManagerUnitString(const QString& aPropertyValueAndUnit) noexcept
{
   auto pvauSplit = ParseValueAndUnit(aPropertyValueAndUnit);
   if (mRelative)
   {
      if (mType == Type::cANGLE)
      {
         mUnitInt    = UtUnitAngle::ReadUnit(pvauSplit.mUnit.toStdString());
         mUnitString = pvauSplit.mUnit;
      }
      else if (mType == Type::cLENGTH)
      {
         mUnitInt    = UtUnitLength::ReadUnit(pvauSplit.mUnit.toStdString());
         mUnitString = pvauSplit.mUnit;
      }
      else
      {
         // the relative units were not recognized
         mUnitInt    = -1;
         mUnitString = "";
      }
      // set the values of the components
      SetLineEditValue(aPropertyValueAndUnit);
      SetSliderValue(static_cast<int>(mValueDouble));
      // emit the appropriate signals
      EmitUnitChangedSignals();
      EmitValueChangedSignals();
   }
   else
   {
      // units do not make sense for an absolute property
      mUnitInt    = -1;
      mUnitString = "";
   }
}

QString LineEditSliderManager::GetManagerUnitString() const noexcept
{
   return mUnitString;
}

void LineEditSliderManager::SetRelative(const bool aRelative) noexcept
{
   mRelative = aRelative;
}

bool LineEditSliderManager::GetRelative() const noexcept
{
   return mRelative;
}

void LineEditSliderManager::SetRange(const RangeD& aRange) noexcept
{
   mValueRange = aRange;
}

LineEditSliderManager::RangeD LineEditSliderManager::GetRange() const noexcept
{
   return mValueRange;
}

void LineEditSliderManager::SetDefaultRange(const int aUnits, const RangeD& aRange) noexcept
{
   if (aUnits != -1)
   {
      mValueRange = aRange;
      SetLineEditRange(aUnits);
      SetSliderRange();
   }
}

bool LineEditSliderManager::IsFormatValid() noexcept
{
   switch (mType)
   {
   case Type::cANGLE:
      return dynamic_cast<UtQtAngleValueEdit*>(mLineEditPtr)->IsValid();
   case Type::cLATITUDE:
      return dynamic_cast<UtQtLatPosLineEdit*>(mLineEditPtr)->IsValid();
   case Type::cLENGTH:
      return dynamic_cast<UtQtLengthValueEdit*>(mLineEditPtr)->IsValid();
   case Type::cLONGITUDE:
      return dynamic_cast<UtQtLonPosLineEdit*>(mLineEditPtr)->IsValid();
   default:
      return false;
   }
}

QString LineEditSliderManager::GetLineEditValueAndUnitString() const noexcept
{
   switch (mType)
   {
   case Type::cANGLE:
   {
      UtQtAngleValueEdit* angleLE_Ptr = dynamic_cast<UtQtAngleValueEdit*>(mLineEditPtr);
      if (angleLE_Ptr->IsValid())
      {
         return angleLE_Ptr->text();
      }
   }
   case Type::cLATITUDE:
   {
      UtQtLatPosLineEdit* latLE_Ptr = dynamic_cast<UtQtLatPosLineEdit*>(mLineEditPtr);
      if (latLE_Ptr->IsValid())
      {
         return latLE_Ptr->text();
      }
   }
   case Type::cLENGTH:
   {
      UtQtLengthValueEdit* lengthLE_Ptr = dynamic_cast<UtQtLengthValueEdit*>(mLineEditPtr);
      if (lengthLE_Ptr->IsValid())
      {
         return lengthLE_Ptr->text();
      }
   }
   case Type::cLONGITUDE:
   {
      UtQtLonPosLineEdit* lonLE_Ptr = dynamic_cast<UtQtLonPosLineEdit*>(mLineEditPtr);
      if (lonLE_Ptr->IsValid())
      {
         return lonLE_Ptr->text();
      }
   }
   default:
      return "";
   }
}

LineEditSliderManager::ValueAndUnitSS LineEditSliderManager::GetLineEditValueAndUnitStringPair() const noexcept
{
   QString pvauStr = GetLineEditValueAndUnitString();
   if (!pvauStr.isEmpty())
   {
      return ParseValueAndUnit(pvauStr);
   }
   return {"", ""};
}

LineEditSliderManager::ValueAndUnitDI LineEditSliderManager::GetLineEditValueAndUnitDoubleIntPair() const noexcept
{
   return {GetLineEditValueDouble(), GetLineEditUnitInt()};
}

QString LineEditSliderManager::GetLineEditValueString() const noexcept
{
   ValueAndUnitSS pvauSplitSS = GetLineEditValueAndUnitStringPair();
   return pvauSplitSS.mValue;
}

double LineEditSliderManager::GetLineEditValueDouble() const noexcept
{
   QString valStr = GetLineEditValueString();
   bool    valSuccess;
   double  val = valStr.toDouble(&valSuccess);
   if (valSuccess)
   {
      return val;
   }
   return 0.0;
}

QString LineEditSliderManager::GetLineEditUnitString() const noexcept
{
   ValueAndUnitSS pvauSplitSS = GetLineEditValueAndUnitStringPair();
   return pvauSplitSS.mUnit;
}

int LineEditSliderManager::GetLineEditUnitInt() const noexcept
{
   ValueAndUnitSS pvauSplitSS = GetLineEditValueAndUnitStringPair();
   std::string    unitStr     = pvauSplitSS.mUnit.toStdString();
   switch (mType)
   {
   case Type::cANGLE:
      return UtUnitAngle::ReadUnit(unitStr);
   case Type::cLENGTH:
      return UtUnitLength::ReadUnit(unitStr);
   case Type::cLATITUDE:
      // use default
   case Type::cLONGITUDE:
      // use default
   default:
      return -1;
   }
}

void LineEditSliderManager::Connect() noexcept
{
   if (mLineEditPtr != nullptr)
   {
      connect(mLineEditPtr, &QLineEdit::textChanged, this, &LineEditSliderManager::HandleLineEdit);
   }
   if (mSliderPtr != nullptr)
   {
      connect(mSliderPtr, &QAbstractSlider::valueChanged, this, &LineEditSliderManager::HandleSlider);
   }
}

void LineEditSliderManager::Disconnect() noexcept
{
   if (mLineEditPtr != nullptr)
   {
      disconnect(mLineEditPtr, &QLineEdit::textChanged, this, &LineEditSliderManager::HandleLineEdit);
   }
   if (mSliderPtr != nullptr)
   {
      disconnect(mSliderPtr, &QAbstractSlider::valueChanged, this, &LineEditSliderManager::HandleSlider);
   }
}

void LineEditSliderManager::HandleSlider(const int aPropertyValue) noexcept
{
   // each type can have a specific way of handling the slider
   switch (mType)
   {
   case Type::cANGLE:
      break;
   case Type::cLATITUDE:
   case Type::cLONGITUDE:
      break;
   case Type::cLENGTH:
      // use default
   default:
      return;
   }

   // the slider was changed, so change the line edit
   QString propertyValueAndUnit;

   double tempVal = static_cast<double>(aPropertyValue);
   if (AreDifferent(tempVal, mValueDouble))
   {
      // set the value
      mValueDouble = tempVal;
      if (mType != Type::cLENGTH)
      {
         Clamp(mValueDouble);
      }
      mValueString = QString::number(mValueDouble, cFLOATING_POINT);
      if (mRelative)
      {
         propertyValueAndUnit = mValueString + ' ' + mUnitString;
      }
      else
      {
         propertyValueAndUnit = ConvertDoubleToDMS(mValueString);
      }
      SetLineEditValue(propertyValueAndUnit);
      // emit the appropriate signals
      EmitValueChangedSignals();
      EmitValueEditedSignals();

      // emit the ValidFormat signal (for the status label)
      emit ValidFormat();
   }
}

void LineEditSliderManager::HandleLineEdit(const QString& aPropertyValue) noexcept
{
   QSignalBlocker block(mSliderPtr);
   // the line edit was changed, so change the slider
   QString val;
   QString unit;
   if (IsFormatValid())
   {
      // emit the ValidFormat signal (for the status label)
      emit ValidFormat();

      if (!mRelative)
      {
         val = ConvertDMS_ToDouble(aPropertyValue);
      }
      else
      {
         val  = GetLineEditValueString();
         unit = GetLineEditUnitString();
      }
      bool   valSuccess;
      double tempVal = val.toDouble(&valSuccess);
      if (valSuccess && AreDifferent(tempVal, mValueDouble))
      {
         // set the value
         mValueDouble = tempVal;
         if (mType != Type::cLENGTH)
         {
            Clamp(mValueDouble);
         }
         mValueString = QString::number(mValueDouble, cFLOATING_POINT);
         SetSliderValue(static_cast<int>(mValueDouble));
         // emit the appropriate signals
         EmitValueChangedSignals();
         EmitValueEditedSignals();
      }
      int  tempUnit    = GetLineEditUnitInt();
      bool unitSuccess = tempUnit != -1;
      if (mRelative && unitSuccess && AreDifferent<int>(tempUnit, mUnitInt))
      {
         // set the unit
         mUnitInt    = tempUnit;
         mUnitString = unit;
         // emit the appropriate signals
         EmitUnitChangedSignals();
      }
   }
   else
   {
      // emit the InvalidFormat signal (for the status label)
      emit InvalidFormat();
   }
}

QString LineEditSliderManager::ConvertDMS_ToDouble(const QString& aStrDMS) noexcept
{
   // the direction character must be at the end of the value
   int dirCharLoc = aStrDMS.indexOf(mDirChars.mPosChar) + aStrDMS.indexOf(mDirChars.mNegChar) + 1;
   if (dirCharLoc != aStrDMS.size() - 1)
   {
      return aStrDMS;
   }

   QString dmsStr = aStrDMS;
   int     sign   = 0;
   // determine the sign from the direction character
   if (dmsStr[dirCharLoc] == mDirChars.mNegChar)
   {
      sign = -1;
   }
   else
   {
      sign = 1;
   }
   // remove the direction character before parsing the DMS
   dmsStr.remove(dirCharLoc, 1);

   QStringList  dmsList = dmsStr.split(":");
   unsigned int deg     = 0;
   unsigned int min     = 0;
   double       secFrac = 0.0;
   double       val     = 0.0;
   if (!dmsList.empty())
   {
      bool degSuccess;
      deg = dmsList[0].toInt(&degSuccess);
      if (degSuccess)
      {
         val += static_cast<double>(deg);
         if (dmsList.size() > 1)
         {
            bool minSuccess;
            min = dmsList[1].toInt(&minSuccess);
            if (minSuccess)
            {
               val += static_cast<double>(min / 60.0);
               if (dmsList.size() > 2)
               {
                  bool secFracSuccess;
                  secFrac = dmsList[2].toDouble(&secFracSuccess);
                  if (secFracSuccess)
                  {
                     val += static_cast<double>(secFrac / 3600.0);
                  }
               }
            }
         }
      }
   }
   val *= sign;
   return QString::number(val, cFLOATING_POINT);
}

QString LineEditSliderManager::ConvertDoubleToDMS(const QString& aStrDouble) noexcept
{
   if (aStrDouble.indexOf(":") != -1)
   {
      return aStrDouble;
   }

   bool         success;
   double       degFrac = aStrDouble.toDouble(&success);
   int          sign    = (degFrac < 0.0) ? -1 : 1;
   unsigned int deg     = 0;
   double       minFrac = 0.0;
   unsigned int min     = 0;
   double       secFrac = 0.0;
   unsigned int sec     = 0;
   char         dirChar = '_';
   if (success)
   {
      deg     = static_cast<unsigned int>(sign * degFrac);
      minFrac = (sign * degFrac - deg) * 60.0;
      min     = static_cast<unsigned int>((sign * degFrac - deg) * 60.0);
      secFrac = ((sign * degFrac - deg) * 60.0 - min) * 60.0;
      sec     = static_cast<unsigned int>(((sign * degFrac - deg) * 60.0 - min) * 60.0);
      if (sign == -1)
      {
         dirChar = mDirChars.mNegChar;
      }
      else
      {
         dirChar = mDirChars.mPosChar;
      }
   }

   // construct a string list based on user preferences for DMS format
   QStringList dmsList;
   switch (wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetLatLonFormatting())
   {
   case UtAngle::FmtD + 8:
      dmsList.push_back(QString::number(sign * degFrac, cFLOATING_POINT, 8) + dirChar);
      break;
   case UtAngle::FmtD + 6:
      dmsList.push_back(QString::number(sign * degFrac, cFLOATING_POINT, 6) + dirChar);
      break;
   case UtAngle::FmtDM + 6:
      dmsList.push_back(QString::number(deg));
      dmsList.push_back(QString::number(minFrac, cFLOATING_POINT, 6) + dirChar);
      break;
   case UtAngle::FmtDM + 4:
      dmsList.push_back(QString::number(deg));
      dmsList.push_back(QString::number(minFrac, cFLOATING_POINT, 4) + dirChar);
      break;
   case UtAngle::FmtDMS + 4:
      dmsList.push_back(QString::number(deg));
      dmsList.push_back(QString::number(min));
      dmsList.push_back(QString::number(secFrac, cFLOATING_POINT, 4) + dirChar);
      break;
   case UtAngle::FmtDMS + 2:
      dmsList.push_back(QString::number(deg));
      dmsList.push_back(QString::number(min));
      dmsList.push_back(QString::number(secFrac, cFLOATING_POINT, 2) + dirChar);
      break;
   case UtAngle::FmtDMS:
      dmsList.push_back(QString::number(deg));
      dmsList.push_back(QString::number(min));
      dmsList.push_back(QString::number(sec) + dirChar);
      break;
   }
   // assemble the DMS string
   return dmsList.join(":");
}


bool LineEditSliderManager::SetLineEditRange(const int aUnits) noexcept
{
   if (mLineEditPtr != nullptr)
   {
      std::string unit;
      switch (mType)
      {
      case Type::cANGLE:
         unit = UtUnitAngle::FindUnitName(aUnits);
         static_cast<UtQtAngleValueEdit*>(mLineEditPtr)
            ->SetBounds<UtAngleValue>(UtAngleValue(mValueRange.mMinimum, unit), UtAngleValue(mValueRange.mMaximum, unit));
         break;
      case Type::cLATITUDE:
         static_cast<UtQtLatPosLineEdit*>(mLineEditPtr)
            ->SetBounds(UtLatPos(mValueRange.mMinimum), UtLatPos(mValueRange.mMaximum));
         break;
      case Type::cLENGTH:
         unit = UtUnitLength::FindUnitName(aUnits);
         static_cast<UtQtLengthValueEdit*>(mLineEditPtr)
            ->SetBounds<UtLengthValue>(UtLengthValue(mValueRange.mMinimum, unit),
                                       UtLengthValue(mValueRange.mMaximum, unit));
         break;
      case Type::cLONGITUDE:
         static_cast<UtQtLonPosLineEdit*>(mLineEditPtr)
            ->SetBounds(UtLonPos(mValueRange.mMinimum), UtLonPos(mValueRange.mMaximum));
         break;
      }
   }
   return false;
}

void LineEditSliderManager::SetSliderRange() noexcept
{
   if (mSliderPtr != nullptr)
   {
      mSliderPtr->setRange(mValueRange.mMinimum, mValueRange.mMaximum);
   }
}

LineEditSliderManager::ValueAndUnitSS LineEditSliderManager::ParseValueAndUnit(const QString& aPropertyValueAndUnit) const noexcept
{
   ValueAndUnitSS pvau;
   if (!mRelative)
   {
      pvau.mValue = aPropertyValueAndUnit;
      pvau.mUnit  = -1;
   }
   else
   {
      double             value;
      std::string        unit;
      std::istringstream pvauStream(aPropertyValueAndUnit.toStdString());
      pvauStream >> value >> unit;
      if (pvauStream.good() || (!pvauStream.fail() && pvauStream.eof()))
      {
         pvau.mValue = QString::number(value, cFLOATING_POINT);
         pvau.mUnit  = QString::fromStdString(unit);
      }
   }
   return pvau;
}

void LineEditSliderManager::Clamp(double& aPropertyValue) noexcept
{
   aPropertyValue = UtMath::Limit(aPropertyValue, mValueRange.mMinimum, mValueRange.mMaximum);
}

void LineEditSliderManager::SetLineEditValue(const QString& aPropertyValueAndUnit) noexcept
{
   if (mLineEditPtr != nullptr)
   {
      Disconnect();
      int cursor = mLineEditPtr->cursorPosition();
      if (!aPropertyValueAndUnit.isEmpty())
      {
         mLineEditPtr->setText(aPropertyValueAndUnit);
      }
      else
      {
         mLineEditPtr->setText(mValueString + ' ' + mUnitString);
      }
      mLineEditPtr->setCursorPosition(cursor);
      Connect();
   }
}

void LineEditSliderManager::SetSliderValue(const int aPropertyValue) noexcept
{
   if (mSliderPtr != nullptr)
   {
      Disconnect();
      mSliderPtr->setValue(static_cast<int>(mValueDouble));
      Connect();
   }
}

void LineEditSliderManager::EmitUnitChangedSignals() noexcept
{
   emit UnitChanged();
   emit UnitChangedInt(mUnitInt);
   emit UnitChangedString(mUnitString);
}

void LineEditSliderManager::EmitValueChangedSignals(const QString& aPropertyValue /* = "" */) noexcept
{
   emit ValueChanged();
   emit ValueChangedDouble(mValueDouble);
   if (!aPropertyValue.isEmpty())
   {
      // for relative property
      emit ValueChangedString(aPropertyValue);
   }
   else
   {
      // for absolute property
      emit ValueChangedString(mValueString);
   }
}

void LineEditSliderManager::EmitValueEditedSignals(const QString& aPropertyValue /* = "" */) noexcept
{
   emit ValueEdited();
   emit ValueEditedDouble(mValueDouble);
   if (!aPropertyValue.isEmpty())
   {
      emit ValueEditedString(aPropertyValue);
   }
   else
   {
      emit ValueEditedString(mValueString);
   }
}

} // namespace PluginUtil
} // namespace MapUtils
