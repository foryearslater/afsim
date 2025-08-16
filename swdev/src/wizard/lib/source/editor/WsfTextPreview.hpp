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
#ifndef WSFTEXTPREVIEW_HPP
#define WSFTEXTPREVIEW_HPP

#include <QPlainTextEdit>

#include "TextStyler.hpp"
#include "UtCallback.hpp"
#include "UtTextDocument.hpp"
#include "ViExport.hpp"


namespace wizard
{
class Project;

//! Displays a syntax-highlighted snippet of an WSF file
class VI_EXPORT WsfTextPreview : public QPlainTextEdit, public TextStyler
{
public:
   explicit WsfTextPreview(QWidget* aParent);
   ~WsfTextPreview() override;

   void SetText(Project* aProjectPtr, UtTextDocumentRange aRange);

   void  HighlightRange(UtTextRange& aRange);
   void  HighlightLine(size_t aLine);
   QSize sizeHint() const override;
   void  SetTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt) override;
   void  MergeTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt) override;

private:
   void SetTextResolve();

   void UpdateStyle();
   void ApplyHighlight();

   void                         SetScenario(Project* aProjectPtr);
   UtCallbackN<void()>*         mParseCompleteCallbackPtr;
   bool                         mStyleComplete;
   Project*                     mProjectPtr;
   UtTextDocumentRange          mRange;
   UtTextRange                  mHighlightRange;
   size_t                       mHighlightLine;
   std::vector<QTextCharFormat> mFormatChanges;
   // Make sure we don't reenter StyleLines()
};
} // namespace wizard

#endif
