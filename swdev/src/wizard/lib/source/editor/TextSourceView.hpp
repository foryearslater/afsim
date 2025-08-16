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

#ifndef TEXTSOURCEVIEW_HPP
#define TEXTSOURCEVIEW_HPP

#include "Editor.hpp"
#include "TextSource.hpp"

namespace wizard
{
class Editor;

class TextSourceView
{
public:
   TextSourceView(Editor* aEditorPtr, TextSource* aSourcePtr);
   wizard::Editor* mEditorPtr;
   TextSource*     mSourcePtr;
};
} // namespace wizard

#endif
