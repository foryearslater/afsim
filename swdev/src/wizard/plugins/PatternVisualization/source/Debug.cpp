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

#include "Debug.hpp"

#include <memory>

#include <QClipboard>
#include <QDateTime>

#include "DebugPlatform.hpp"
#include "ErrorDialog.hpp"
#include "UtMemory.hpp"

namespace PatternVisualizer
{
namespace Debug
{

std::unique_ptr<QTextDocument> CreateErrorMessage(const QString& aErrorMessageHtml)
{
   std::unique_ptr<QTextDocument> document = ut::make_unique<QTextDocument>();
   document->setHtml(aErrorMessageHtml);

   QTextCursor cursor(document.get());

   QTextCharFormat charFormat;
   charFormat.setFontPointSize(7.5);
   charFormat.setForeground(QBrush{Qt::GlobalColor::darkGray});

   cursor.movePosition(QTextCursor::End);
   cursor.insertBlock();
   cursor.setBlockFormat(QTextBlockFormat{});
   cursor.setBlockCharFormat(charFormat);

   cursor.insertText(QString{"\n%1 v%2\n%3"}.arg(QApplication::applicationDisplayName(),
                                                 QApplication::applicationVersion(),
                                                 QDateTime::currentDateTime().toString(Qt::RFC2822Date)));

   return document;
}

void ShowErrorImpl(const QString& aErrorMessageHtml, bool aIsFatal)
{
   // Note, we cannot assume that Qt is fully initialized and ready to
   // go at this point.  We have to be careful what Qt services we use.
   // Utility classes such as QString and QTextDocument are safe, but
   // anything creating a widget is not safe to use.

   std::unique_ptr<QTextDocument> errorMessage = CreateErrorMessage(aErrorMessageHtml);

   if (qApp)
   {
      // We have a QApplication, so it's safe to use the clipboard and
      // create widgets.
      QClipboard* clipboard = QApplication::clipboard();
      clipboard->setText(errorMessage->toPlainText());

      ErrorDialog dialog{errorMessage.get(), QApplication::applicationDisplayName(), aIsFatal};
      dialog.exec();
   }
   else
   {
      // No QApplication, so use a platform-specific fallback case.
      FallbackShowMessage(errorMessage.get(), QApplication::applicationDisplayName(), aIsFatal);
   }
}

void ShowError(const QString& aErrorMessageHtml)
{
   ShowErrorImpl(aErrorMessageHtml, false);
}

void ShowFatalError(const QString& aErrorMessageHtml)
{
   ShowErrorImpl(aErrorMessageHtml, true);
}

} // namespace Debug
} // namespace PatternVisualizer
