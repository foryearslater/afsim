// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2003-2015 The Boeing Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef LAZYCOMBOBOX_HPP
#define LAZYCOMBOBOX_HPP
#include <QComboBox>
#include <QWheelEvent>

#include "UtCallback.hpp"

namespace CommandChain
{
class LazyComboBox : public QComboBox
{
   Q_OBJECT

public:
   LazyComboBox(QWidget* aParent = nullptr)
      : QComboBox(aParent)
   {
      setFocusPolicy(Qt::StrongFocus);
   }

   // Called before the popup is shown to allow filling the combo box with items after a user click
   UtCallbackListN<void()> BeforePopup;

   void showPopup() override
   {
      BeforePopup();
      emit OnBeforePopup();
      QComboBox::showPopup();
   }

signals:
   void OnBeforePopup();

protected:
   // Prevent the mouse wheel from scrolling the combo box when it isn't focus.
   void wheelEvent(QWheelEvent* event) override
   {
      if (!hasFocus())
      {
         event->ignore();
      }
      else
      {
         QComboBox::wheelEvent(event);
      }
   }
};
} // namespace CommandChain
#endif
