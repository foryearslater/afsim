// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef PLATFORMDATATREEWIDGETITEM_HPP
#define PLATFORMDATATREEWIDGETITEM_HPP

#include "UtUnitTypes.hpp"
#include "WkfUnitTypes.hpp"

namespace wizard
{
template<class T>
class ValidatedUnitTreeWidgetItem : public wkf::UnitTreeWidgetItemT<T>
{
public:
   ValidatedUnitTreeWidgetItem(QTreeWidgetItem* aParent, const QString& aLabelName)
      : wkf::UnitTreeWidgetItemT<T>(aParent, aLabelName)
   {
   }

   void setData(int column, int role, const QVariant& value) override
   {
      QStringList tokenList = value.toString().split(" ");
      if (tokenList.size() == 0)
      {
         return;
      }

      // Latitude and Longitude validity are checked elsewhere
      if (std::is_same<wkf::Latitude, T>::value || std::is_same<wkf::Longitude, T>::value)
      {
         QTreeWidgetItem::setData(column, role, value);
         return;
      }

      std::map<std::string, int> unitStrings;
      if (std::is_same<wkf::Acceleration, T>::value)
      {
         unitStrings = UtUnitAcceleration::mUnitStrings;
      }
      else if (std::is_same<wkf::Altitude, T>::value)
      {
         unitStrings = UtUnitLength::mUnitStrings;
      }
      else if (std::is_same<wkf::Angle, T>::value)
      {
         unitStrings = UtUnitAngle::mUnitStrings;
      }
      else if (std::is_same<wkf::AngularRate, T>::value)
      {
         unitStrings = UtUnitAngularRate::mUnitStrings;
      }
      else if (std::is_same<wkf::AngularAcceleration, T>::value)
      {
         unitStrings = UtUnitAngularAcceleration::mUnitStrings;
      }
      else if (std::is_same<wkf::Force, T>::value)
      {
         unitStrings = UtUnitForce::mUnitStrings;
      }
      else if (std::is_same<wkf::Frequency, T>::value)
      {
         unitStrings = UtUnitFrequency::mUnitStrings;
      }
      else if (std::is_same<wkf::Length, T>::value)
      {
         unitStrings = UtUnitLength::mUnitStrings;
      }
      else if (std::is_same<wkf::Mass, T>::value)
      {
         unitStrings = UtUnitMass::mUnitStrings;
      }
      else if (std::is_same<wkf::MassTransfer, T>::value)
      {
         unitStrings = UtUnitMassTransfer::mUnitStrings;
      }
      else if (std::is_same<wkf::Power, T>::value)
      {
         unitStrings = UtUnitPower::mUnitStrings;
      }
      else if (std::is_same<wkf::PowerDB, T>::value)
      {
         unitStrings = UtUnitPowerDB::mUnitStrings;
      }
      else if (std::is_same<wkf::Pressure, T>::value)
      {
         unitStrings = UtUnitPressure::mUnitStrings;
      }
      else if (std::is_same<wkf::Ratio, T>::value)
      {
         unitStrings = UtUnitRatio::mUnitStrings;
      }
      else if (std::is_same<wkf::Speed, T>::value)
      {
         unitStrings = UtUnitSpeed::mUnitStrings;
      }
      else if (std::is_same<wkf::Time, T>::value)
      {
         unitStrings = UtUnitTime::mUnitStrings;
      }
      else if (std::is_same<wkf::Torque, T>::value)
      {
         unitStrings = UtUnitTorque::mUnitStrings;
      }
      else
      {
         return;
      }

      bool isDouble = false;
      tokenList[0].toDouble(&isDouble);
      if (isDouble)
      {
         // If second token is a valid unit, use it
         if (tokenList.size() >= 2 && unitStrings.count(tokenList[1].toStdString()) != 0)
         {
            QTreeWidgetItem::setData(column, role, value);
         }
         // If no second token, or second token is invalid, use previous unit.
         else
         {
            QStringList prevTokenList = wkf::UnitTreeWidgetItemT<T>::text(column).split(" ");
            if (prevTokenList.size() >= 2)
            {
               QTreeWidgetItem::setData(column, role, QVariant(tokenList[0] + " " + prevTokenList[1]));
            }
         }
      }
   }

   void SetUnitText(const QString& aText) override
   {
      mUnitChanged = true;
      setData(1, 0, QVariant(aText));
      mUnitChanged = false;
   }

   bool GetUnitChanged() const { return mUnitChanged; }

private:
   // True if last setData call was through SetUnitText
   bool mUnitChanged = false;
};

// SPECIALIZED TREEWIDGETITEMs
using ValidatedAltitudeTreeWidgetItem = ValidatedUnitTreeWidgetItem<wkf::Altitude>;
using ValidatedAngleTreeWidgetItem    = ValidatedUnitTreeWidgetItem<wkf::Angle>;
} // namespace wizard
#endif // PLATFORMDATATREEWIDGETITEM_HPP
