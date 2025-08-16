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
#ifndef COLORUTILS_HPP
#define COLORUTILS_HPP

#include "Editor.hpp"
// Platform Browser namespace.
namespace ColorUtils
{
std::string IdentifierAtPosition(wizard::Editor* aEditorPtr, size_t aPos, size_t& aStartingPos, bool aScriptOnly = true);

QColor      GetColorFromString(const std::string& aColorStr);
std::string GetColorString(wizard::Editor* aEditorPtr, const size_t& aStartPos);
bool        IsValidColor(const std::string& aColorStr);
} // namespace ColorUtils

#endif
