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

#ifndef ERRORDIALOG_HPP
#define ERRORDIALOG_HPP

#include "ui_ErrorDialog.h"

class QTextDocument;

namespace PatternVisualizer
{
class ErrorDialog : public QDialog
{
   Q_OBJECT

public:
   ErrorDialog(QTextDocument* aErrorMessagePtr, const QString& aApplicationTitle, bool aIsFatal, QWidget* aParentPtr = nullptr);

private:
   Ui::ErrorDialog mUI;
};
} // namespace PatternVisualizer

#endif
