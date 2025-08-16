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

#ifndef SPINBOXEVENTFILTER_HPP
#define SPINBOXEVENTFILTER_HPP

#include <QEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QWidget>

namespace PatternVisualizer
{
namespace SpinBoxEventFilter
{
template<typename SpinBoxType, typename T>
static bool Filter(SpinBoxType* aSpinBox, std::function<void(T)> aFunction, QEvent* aEventPtr, bool aCallFuncOnLeave = true)
{
   bool callFunc{false};
   T    value{};

   switch (aEventPtr->type())
   {
   case QEvent::Leave:
      value    = aSpinBox->value();
      callFunc = aCallFuncOnLeave;
      break;

   case QEvent::Wheel:
   {
      int factor{1};
      if (static_cast<QWheelEvent*>(aEventPtr)->delta() < 0)
      {
         factor = -1;
      }
      value    = aSpinBox->value() + factor * aSpinBox->singleStep();
      callFunc = true;
   }
   break;

   case QEvent::KeyRelease:
      switch ((static_cast<QKeyEvent*>(aEventPtr))->key())
      {
      case Qt::Key_Return: //  Note fallthrough
      case Qt::Key_Enter:  //      |
      case Qt::Key_Down:   //      |
      case Qt::Key_Up:     //      V
         value    = aSpinBox->value();
         callFunc = true;
         break;
      }
      break;

   case QEvent::MouseButtonRelease:
      value    = aSpinBox->value();
      callFunc = true;
      break;

   default:
      break;
   }

   if (callFunc)
   {
      aFunction(value);
   }

   return static_cast<QWidget*>(aSpinBox)->eventFilter(aSpinBox, aEventPtr);
}
} // namespace SpinBoxEventFilter
} // namespace PatternVisualizer

#endif // SPINBOXEVENTFILTER_HPP
