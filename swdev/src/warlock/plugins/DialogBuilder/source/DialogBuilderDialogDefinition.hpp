// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2017 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef DialogBuilderDialogDefinition_hpp
#define DialogBuilderDialogDefinition_hpp

#include <map>
#include <string>

#include <QApplication>
#include <QColor>
#include <QDataStream>
#include <QFile>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPalette>
#include <QPushButton>
#include <QString>
#include <QVariant>

#include "UtScriptData.hpp"
#include "WkScriptSimInterface.hpp"

namespace WkDialogBuilder
{
// Mapping of the enumeration to strings for consistent displays
static const std::map<warlock::script::Context, QString> ScriptContextToStringMap = {{warlock::script::GLOBAL, "global"},
                                                                                     {warlock::script::PLATFORM,
                                                                                      "platform"}};

// All the information to define the Script as it appears in the dialog.
struct ScriptDefinition
{
   QString displayName{};
   bool    useDefaultColor{true};
   QColor  color{qApp->palette().color(QPalette::Button)};
   QString icon{};
   QString link{};
   QString scriptName{};
   QString platformType{};
   bool    clampPlatformSelection{false};
   bool    promptUser{false};

   warlock::script::Context         context{warlock::script::GLOBAL};
   QList<warlock::script::Argument> args{};
   QList<warlock::script::Filter>   filters{};

   int row{0};
   int col{0};
};
using ScriptDefinitions = QVector<ScriptDefinition>;

static QDataStream& operator<<(QDataStream& stream, const ScriptDefinition& def)
{
   // clang-format off
      stream << def.col
             << def.color
             << def.context
             << def.displayName
             << def.icon
             << def.platformType
             << def.promptUser
             << def.row
             << def.scriptName
             << def.useDefaultColor
             << def.args
             << def.filters
             << def.link
             << def.clampPlatformSelection;
   // clang-format on
   return stream;
}

static QDataStream& operator>>(QDataStream& stream, ScriptDefinition& def)
{
   int context;
   // clang-format off
      stream >> def.col
             >> def.color
             >> context
             >> def.displayName
             >> def.icon
             >> def.platformType
             >> def.promptUser
             >> def.row
             >> def.scriptName
             >> def.useDefaultColor
             >> def.args
             >> def.filters
             >> def.link
             >> def.clampPlatformSelection;
   // clang-format on
   def.context = (warlock::script::Context)context;
   return stream;
}

enum ButtonSize
{
   THIN,
   SMALL_SQUARE,
   LARGE_SQUARE,
   SMALL_TALL,
   LARGE_TALL
};

static const QSize cSMALL_SQUARE_SIZE = QSize(65, 65);
static const QSize cLARGE_SQUARE_SIZE = QSize(100, 100);
static const QSize cSMALL_TALL_SIZE   = QSize(60, 80);
static const QSize cLARGE_TALL_SIZE   = QSize(90, 120);
static const int   cTHIN_HEIGHT       = 22;

static const std::map<ButtonSize, QString> ButtonSizeToStringMap = {{THIN, "thin"},
                                                                    {SMALL_SQUARE, "square (small)"},
                                                                    {LARGE_SQUARE, "square (large)"},
                                                                    {SMALL_TALL, "tall (small)"},
                                                                    {LARGE_TALL, "tall (large)"}};
inline void                                ApplyButtonSize(QPushButton* aButton, ButtonSize aSize)
{
   switch (aSize)
   {
   case SMALL_SQUARE:
   {
      aButton->setFixedSize(cSMALL_SQUARE_SIZE);
      aButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      break;
   }
   case LARGE_SQUARE:
   {
      aButton->setFixedSize(cLARGE_SQUARE_SIZE);
      aButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      break;
   }
   case SMALL_TALL:
   {
      aButton->setFixedSize(cSMALL_TALL_SIZE);
      aButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      break;
   }
   case LARGE_TALL:
   {
      aButton->setFixedSize(cLARGE_TALL_SIZE);
      aButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      break;
   }
   case THIN:
   default:
   {
      aButton->setMinimumSize(QSize(0, 0));
      aButton->setFixedHeight(cTHIN_HEIGHT);
      aButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
      break;
   }
   }
}

inline void ApplyButtonNameAndIcon(QPushButton* aButton, const QString& aName, const QString& aIcon)
{
   // clear the existing widgets from the layout
   auto clearLayout = aButton->layout();
   if (clearLayout)
   {
      while (auto item = clearLayout->takeAt(0))
      {
         delete item->widget();
         delete item;
      }
      delete clearLayout;
   }

   // for dialog formatting purposes, use labels and layouts instead of setting text or icons directly on the button
   auto    layout = new QHBoxLayout(aButton);
   QLabel* label;
   if (QFile(aIcon).exists())
   {
      QPixmap pixmap(aIcon);
      label = new QLabel(aButton);
      label->setPixmap(pixmap.scaled(aButton->width(), aButton->height(), Qt::KeepAspectRatio));
      layout->setContentsMargins(0, 0, 0, 0);
   }
   else
   {
      label = new QLabel(aName, aButton);
      // the THIN button size has a fixed height of 22
      if (aButton->height() > cTHIN_HEIGHT)
      {
         label->setWordWrap(true);
      }
      // without margins, text appears to go a little bit outside of the button
      layout->setContentsMargins(4, 4, 4, 4);
   }
   label->setObjectName(aName);
   layout->addWidget(label, 0, Qt::AlignCenter);
   aButton->setText("");
}

inline void ApplyButtonColor(QPushButton* aButton, const QColor& aColor)
{
   QPalette pal = aButton->palette();
   pal.setColor(QPalette::Button, aColor);
   aButton->setAutoFillBackground(true);
   aButton->setPalette(pal);
}

// All the information to create a user defined dialog
struct DialogDef
{
   QString           name;
   int               rows{2};
   int               cols{2};
   bool              dockable{true};
   ButtonSize        buttonSize{THIN};
   ScriptDefinitions scripts;
};

inline QDataStream& operator<<(QDataStream& stream, const DialogDef& dialog)
{
   // clang-format off
      stream << dialog.cols
             << dialog.dockable
             << dialog.name
             << dialog.rows
             << dialog.buttonSize
             << dialog.scripts;
   // clang-format on
   return stream;
}

inline QDataStream& operator>>(QDataStream& stream, DialogDef& dialog)
{
   int bSize;
   // clang-format off
      stream >> dialog.cols
             >> dialog.dockable
             >> dialog.name
             >> dialog.rows
             >> bSize
             >> dialog.scripts;
   // clang-format on
   dialog.buttonSize = (ButtonSize)bSize;
   return stream;
}

inline bool QStringToBool(const QString& aString)
{
   if (aString.compare("true", Qt::CaseInsensitive) == 0)
   {
      return true;
   }
   bool ok = false;
   int  i  = aString.toInt(&ok);
   if (ok && i != 0)
   {
      return true;
   }
   return false;
}
} // namespace WkDialogBuilder
#endif
