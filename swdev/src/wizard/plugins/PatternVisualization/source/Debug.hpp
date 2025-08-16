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

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <QString>

namespace PatternVisualizer
{
namespace Debug
{
// Causes the application to launch the debugger at the current point.
void DebuggerBreak();

// Shows a non-fatal error message to the user.  The user will be able
// to break into the debugger, continue and ignore the error, or
// terminate the application.  The parameter errorMessageHtml can contain
// HTML formatting.
void ShowError(const QString& aErrorMessageHtml);

// Shows a fatal error message to the user.  The user will be able
// to break into the debugger or terminate the program.  The parameter
// errorMessageHtml can contain HTML formatting.
void ShowFatalError(const QString& aErrorMessageHtml);
} // namespace Debug
} // namespace PatternVisualizer

#endif
