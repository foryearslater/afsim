// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "SimControllerToolbarWidgets.hpp"

#include <functional>

#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QStyleOptionSlider>
#include <QStylePainter>
#include <QTimer>
#ifdef _WIN32
#include <QWinThumbnailToolBar>
#include <QWinThumbnailToolButton>
#endif

#include "SimControllerPrefObject.hpp"
#include "SimControllerSimCommands.hpp"
#include "SimControllerSimInterface.hpp"
#include "UtMath.hpp"
#include "UtMemory.hpp"
#include "WkRunManager.hpp"
#include "WkSimEnvironment.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"

WkSimController::ClockRateSpinBox::ClockRateSpinBox(QWidget* parent)
   : QDoubleSpinBox(parent)
{
   mResetActionPtr = new wkf::Action(QIcon::fromTheme("reset-small"), "Reset to x1.00", this);
   connect(mResetActionPtr, &QAction::triggered, this, &ClockRateSpinBox::Reset);

   connect(this,
           static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
           this,
           &ClockRateSpinBox::ValueChanged);
}

void WkSimController::ClockRateSpinBox::ValueChanged(double value)
{
   if (value != 1.0)
   {
      lineEdit()->addAction(mResetActionPtr, QLineEdit::TrailingPosition);
   }
   else
   {
      lineEdit()->removeAction(mResetActionPtr);
   }
}

void WkSimController::ClockRateSpinBox::Reset()
{
   // KLUDGE: Don't call setValue here as it will invoke the ValueChanged slot, which will remove the reset action
   // from the QLineEdit. This causes a crash in Qt, so instead delay the call to setValue using a single-shot
   // QTimer.
   QTimer::singleShot(100, [this]() { setValue(1.0); });
}

WkSimController::ClockRateSlider::ClockRateSlider(QWidget* parent)
   : QSlider(parent)
{
}

void WkSimController::ClockRateSlider::paintEvent(QPaintEvent* ev)
{
   QStylePainter      p(this);
   QStyleOptionSlider opt;
   initStyleOption(&opt);

   QRect handle = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

   // draw tick marks
   // do this manually because they are very badly behaved with style sheets
   int interval = tickInterval();
   if (interval == 0)
   {
      interval = pageStep();
   }

   if (tickPosition() != NoTicks)
   {
      for (int i = minimum(); i <= maximum(); i += interval)
      {
         int x = round((double)((double)((double)(i - minimum()) / (double)(maximum() - minimum())) *
                                   (double)(width() - handle.width()) +
                                (double)(handle.width() / 2.0))) -
                 1;
         int h = (i - minimum()) % 5 == 0 ? 4 : 2;

         if (tickPosition() == TicksBothSides || tickPosition() == TicksAbove)
         {
            int y = rect().top();
            p.drawLine(x, y, x, y + h);
         }
         if (tickPosition() == TicksBothSides || tickPosition() == TicksBelow)
         {
            int y = rect().bottom();
            p.drawLine(x, y, x, y - h);
         }
      }
   }

   // draw the slider (this is basically copy/pasted from QSlider::paintEvent)
   opt.subControls = QStyle::SC_SliderGroove;
   p.drawComplexControl(QStyle::CC_Slider, opt);

   // draw the slider handle
   opt.subControls = QStyle::SC_SliderHandle;
   p.drawComplexControl(QStyle::CC_Slider, opt);
}
