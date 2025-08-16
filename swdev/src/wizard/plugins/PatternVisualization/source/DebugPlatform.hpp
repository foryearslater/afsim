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

#ifndef DEBUGPLATFORM_HPP
#define DEBUGPLATFORM_HPP

#include <QString>
#include <QTextDocument>

namespace PatternVisualizer
{
namespace Debug
{
// Causes the application to launch the debugger at the current point.
void DebuggerBreak();

// Implements the platform specific fallback for displaying an error
// message even when Qt isn't initialized.
void FallbackShowMessage(QTextDocument* aErrorMessage, const QString& aApplicationTitle, bool aIsFatal);
} // namespace Debug
} // namespace PatternVisualizer

#endif
