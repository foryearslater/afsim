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

#ifndef RANGERING_DOCKWIDGET_HPP
#define RANGERING_DOCKWIDGET_HPP

#include <QDockWidget>

// EXERCISE 1 TASK 4a
// Add an include for code generated from the .ui file
// PLACE YOUR CODE HERE

namespace Training
{
   //! This represents the specific dockable widget associated with our plugin.
   class DockWidget : public QDockWidget
   {
      // This line is required for Qt signals to be properly emitted from this class.
      Q_OBJECT

   public:

      DockWidget(QWidget*        aParent      = nullptr,
                 Qt::WindowFlags aWindowFlags = Qt::WindowFlags());

      ~DockWidget() override = default;

   private:
      // EXERCISE 1 TASK 4b
      // Declare a member variable for the UI element defined in "ui_TrainingDockWidget.h"
      // PLACE YOUR CODE HERE

   };
}

#endif
