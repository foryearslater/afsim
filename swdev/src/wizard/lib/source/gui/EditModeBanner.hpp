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
#ifndef EDITMODEBANNER_HPP
#define EDITMODEBANNER_HPP

#include <memory>

#include <QPushButton>
#include <QWidget>

#include "Signals.hpp"
#include "Vi.hpp"
#include "ui_EditModeBanner.h"

class QUndoStack;

namespace wizard
{
class Undo;

class EditModeBanner : public QWidget
{
   Q_OBJECT
public:
   EditModeBanner(bool aBannerIsForTextEditor, Undo* aUndoPtr);
   ~EditModeBanner() override = default;
   void ShowVisualMode();
   void ShowApplying();

   Ui::EditModeBanner mUi;

protected slots:
   void ApplyClick();
   void RevertClick();
   void EditModeSwitch(Vi::EditMode aEditMode);
   void UndoIndexChanged(int aIndex);

protected:
   bool        mBannerIsForTextEditor;
   QUndoStack* mUndoStackPtr;
};
} // namespace wizard
#endif
