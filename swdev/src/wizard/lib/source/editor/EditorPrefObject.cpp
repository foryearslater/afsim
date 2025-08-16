// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2018 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "EditorPrefObject.hpp"

#include <QIcon>

#include "WkfEnvironment.hpp"
#include "WkfGeneralPrefObject.hpp"
#include "WkfMainWindow.hpp"

wizard::EditorPrefData::EditorPrefData()
{
   // Normal Text
   mLightStyle.emplace(TextType::Keyword, TextStyle(QColor(63, 72, 204), false, false, false));
   mLightStyle.emplace(TextType::Block_Keyword, TextStyle(QColor(64, 72, 204), true, false, false));
   mLightStyle.emplace(TextType::Command, TextStyle(QColor(46, 129, 177), false, false, false));
   mLightStyle.emplace(TextType::User_Input, TextStyle(QColor(159, 99, 66), false, false, false));
   mLightStyle.emplace(TextType::Type, TextStyle(QColor(43, 97, 164), false, false, false));
   mLightStyle.emplace(TextType::Name, TextStyle(QColor(99, 180, 99), false, false, false));
   mLightStyle.emplace(TextType::Include, TextStyle(QColor(191, 0, 211), false, false, false));
   mLightStyle.emplace(TextType::Comment, TextStyle(QColor(0, 130, 0), false, false, false));
   mLightStyle.emplace(TextType::Quoted, TextStyle(QColor(200, 100, 100), false, false, false));
   mLightStyle.emplace(TextType::Unmatched_Block, TextStyle(QColor(175, 32, 32), false, false, false));
   mLightStyle.emplace(TextType::None, TextStyle(QColor(0, 0, 0), false, false, false));
   mLightStyle.emplace(TextType::Preprocessor, TextStyle(QColor(220, 10, 220), false, false, false));

   // Script
   mLightStyle.emplace(TextType::Script_Keyword, TextStyle(QColor(63, 72, 204), false, false, false));
   mLightStyle.emplace(TextType::Script_Operator, TextStyle(QColor(0, 0, 0), false, false, false));
   mLightStyle.emplace(TextType::Script_Quoted, TextStyle(QColor(200, 100, 100), false, false, false));
   mLightStyle.emplace(TextType::Script_Comment, TextStyle(QColor(0, 130, 0), false, false, false));
   mLightStyle.emplace(TextType::Script_Type, TextStyle(QColor(43, 97, 164), false, false, false));
   mLightStyle.emplace(TextType::Script_Number, TextStyle(QColor(159, 99, 66), false, false, false));
   mLightStyle.emplace(TextType::Script_Static_Method, TextStyle(QColor(74, 132, 151), true, false, false));
   mLightStyle.emplace(TextType::Script_Method, TextStyle(QColor(74, 132, 151), false, true, false));
   mLightStyle.emplace(TextType::Script_Local_Method, TextStyle(QColor(99, 113, 72), false, false, false));
   mLightStyle.emplace(TextType::Script_Local_Variable, TextStyle(QColor(99, 113, 72), false, false, false));
   mLightStyle.emplace(TextType::Script_Parameter, TextStyle(QColor(99, 113, 72), false, true, false));
   mLightStyle.emplace(TextType::Script_None, TextStyle(QColor(180, 0, 150), false, false, false));

   // Editor Margin
   mLightEditorColor.emplace("margin_fill", QColor(192, 192, 192)); // #c0c0c0
   mLightEditorColor.emplace("margin_pen", QColor(0, 0, 0));
   mLightEditorColor.emplace("margin_fold_fill", QColor(0, 0, 255));
   mLightEditorColor.emplace("margin_fold_pen", QColor(0, 0, 0));

   // Editor
   mLightEditorColor.emplace("editor_line_color", QColor(238, 238, 255));
   mLightEditorColor.emplace("editor_syntax_tips", QColor(255, 255, 225));
   mLightEditorColor.emplace("editor_auto_complete_1", QColor(150, 150, 150));
   mLightEditorColor.emplace("editor_auto_complete_2", QColor(75, 75, 75));
   mLightEditorColor.emplace("editor_auto_complete_3", QColor(0, 0, 0));
   mLightEditorColor.emplace("editor_highlight", QColor(255, 255, 0));

   // Normal Text
   mDarkStyle.emplace(TextType::Keyword, TextStyle(QColor(86, 156, 214), false, false, false));
   mDarkStyle.emplace(TextType::Block_Keyword, TextStyle(QColor(86, 156, 214), false, false, false));
   mDarkStyle.emplace(TextType::Command, TextStyle(QColor(200, 200, 200), false, false, false));
   mDarkStyle.emplace(TextType::User_Input, TextStyle(QColor(184, 215, 163), false, false, false));
   mDarkStyle.emplace(TextType::Type, TextStyle(QColor(78, 201, 176), false, false, false));
   mDarkStyle.emplace(TextType::Name, TextStyle(QColor(146, 202, 244), false, false, false));
   mDarkStyle.emplace(TextType::Include, TextStyle(QColor(155, 155, 155), false, false, false));
   mDarkStyle.emplace(TextType::Comment, TextStyle(QColor(87, 166, 74), false, false, false));
   mDarkStyle.emplace(TextType::Quoted, TextStyle(QColor(214, 157, 133), false, false, false));
   mDarkStyle.emplace(TextType::Unmatched_Block, TextStyle(QColor(255, 0, 0), false, false, false));
   mDarkStyle.emplace(TextType::None, TextStyle(QColor(240, 240, 240), false, false, false));
   mDarkStyle.emplace(TextType::Preprocessor, TextStyle(QColor(189, 99, 197), false, false, false));

   // Script
   mDarkStyle.emplace(TextType::Script_Keyword, TextStyle(QColor(86, 156, 214), false, false, false));
   mDarkStyle.emplace(TextType::Script_Operator, TextStyle(QColor(240, 240, 240), false, false, false));
   mDarkStyle.emplace(TextType::Script_Quoted, TextStyle(QColor(214, 157, 133), false, false, false));
   mDarkStyle.emplace(TextType::Script_Comment, TextStyle(QColor(87, 166, 74), false, false, false));
   mDarkStyle.emplace(TextType::Script_Type, TextStyle(QColor(255, 215, 0), false, false, false));
   mDarkStyle.emplace(TextType::Script_Number, TextStyle(QColor(181, 206, 168), false, false, false));
   mDarkStyle.emplace(TextType::Script_Static_Method, TextStyle(QColor(255, 128, 0), false, false, false));
   mDarkStyle.emplace(TextType::Script_Method, TextStyle(QColor(255, 128, 0), false, false, false));
   mDarkStyle.emplace(TextType::Script_Local_Method, TextStyle(QColor(189, 140, 70), false, false, false));
   mDarkStyle.emplace(TextType::Script_Local_Variable, TextStyle(QColor(189, 140, 70), false, false, false));
   mDarkStyle.emplace(TextType::Script_Parameter, TextStyle(QColor(127, 127, 127), false, false, false));
   mDarkStyle.emplace(TextType::Script_None, TextStyle(QColor(180, 0, 150), false, false, false));

   // Editor Margin
   mDarkEditorColor.emplace("margin_fill", QColor(45, 45, 47));
   mDarkEditorColor.emplace("margin_pen", QColor(43, 145, 175));
   mDarkEditorColor.emplace("margin_fold_fill", QColor(51, 153, 255));
   mDarkEditorColor.emplace("margin_fold_pen", QColor(220, 220, 220));

   // Editor
   mDarkEditorColor.emplace("editor_line_color", QColor(75, 75, 75));
   mDarkEditorColor.emplace("editor_syntax_tips", QColor(45, 45, 47));
   mDarkEditorColor.emplace("editor_auto_complete_1", QColor(75, 75, 75));
   mDarkEditorColor.emplace("editor_auto_complete_2", QColor(150, 150, 150));
   mDarkEditorColor.emplace("editor_auto_complete_3", QColor(200, 200, 200));
   mDarkEditorColor.emplace("editor_highlight", QColor(14, 69, 131));
}

wizard::EditorPrefObject::EditorPrefObject(QObject* parent /*= nullptr*/)
   : wkf::PrefObjectT<EditorPrefData>(parent, cNAME)
{
}

void wizard::EditorPrefObject::Apply()
{
   emit Changed();
}

wizard::EditorPrefData wizard::EditorPrefObject::ReadSettings(QSettings& aSettings) const
{
   EditorPrefData pData;
   pData.mShowLineNumbers         = aSettings.value("showLineNumber", mDefaultPrefs.mShowLineNumbers).toBool();
   pData.mShowSyntaxTips          = aSettings.value("showSyntaxTips", mDefaultPrefs.mShowSyntaxTips).toBool();
   pData.mAutocompleteAfterPeriod = aSettings.value("autocomplete", mDefaultPrefs.mAutocompleteAfterPeriod).toBool();
   pData.mEnableUndoPopups        = aSettings.value("undoPopups", mDefaultPrefs.mEnableUndoPopups).toBool();
   pData.mFoldText                = aSettings.value("foldText", mDefaultPrefs.mFoldText).toBool();
   pData.mTabSpace                = aSettings.value("tabSize", mDefaultPrefs.mTabSpace).toUInt();
   pData.mFontSize                = aSettings.value("fontSize", mDefaultPrefs.mFontSize).toUInt();
   pData.mFontType                = aSettings.value("fontType", mDefaultPrefs.mFontType).toString();
   pData.mDarkEditorColor         = mDefaultPrefs.mDarkEditorColor;
   int arraySize                  = aSettings.beginReadArray("darkEditorColor");
   for (int i = 0; i < arraySize; ++i)
   {
      aSettings.setArrayIndex(i);
      QString name;
      QColor  color                = Qt::red;
      name                         = aSettings.value("name", "dummy").toString();
      color                        = aSettings.value("color").value<QColor>();
      pData.mDarkEditorColor[name] = color;
   }
   aSettings.endArray();
   pData.mLightEditorColor = mDefaultPrefs.mLightEditorColor;
   arraySize               = aSettings.beginReadArray("lightEditorColor");
   for (int i = 0; i < arraySize; ++i)
   {
      aSettings.setArrayIndex(i);
      QString name;
      QColor  color                 = Qt::red;
      name                          = aSettings.value("name", "dummy").toString();
      color                         = aSettings.value("color").value<QColor>();
      pData.mLightEditorColor[name] = color;
   }
   aSettings.endArray();
   pData.mDarkStyle = mDefaultPrefs.mDarkStyle;
   arraySize        = aSettings.beginReadArray("darkStyle");
   for (int i = 0; i < arraySize; ++i)
   {
      aSettings.setArrayIndex(i);
      int    idx                                   = aSettings.value("name").toInt();
      QColor color                                 = aSettings.value("color").value<QColor>();
      bool   bold                                  = aSettings.value("bold").toBool();
      bool   italic                                = aSettings.value("italic").toBool();
      bool   underline                             = aSettings.value("underline").toBool();
      pData.mDarkStyle[static_cast<TextType>(idx)] = TextStyle(color, bold, italic, underline);
   }
   aSettings.endArray();
   pData.mLightStyle = mDefaultPrefs.mLightStyle;
   arraySize         = aSettings.beginReadArray("lightStyle");
   for (int i = 0; i < arraySize; ++i)
   {
      aSettings.setArrayIndex(i);
      int    idx                                    = aSettings.value("name").toInt();
      QColor color                                  = aSettings.value("color").value<QColor>();
      bool   bold                                   = aSettings.value("bold").toBool();
      bool   italic                                 = aSettings.value("italic").toBool();
      bool   underline                              = aSettings.value("underline").toBool();
      pData.mLightStyle[static_cast<TextType>(idx)] = TextStyle(color, bold, italic, underline);
   }
   aSettings.endArray();

   arraySize = aSettings.beginReadArray("filePos");
   for (int i = 0; i < arraySize; ++i)
   {
      aSettings.setArrayIndex(i);
      QString name               = aSettings.value("file").toString();
      size_t  line               = aSettings.value("line").toUInt();
      size_t  col                = aSettings.value("column").toUInt();
      pData.mFilePositions[name] = std::make_pair(line, col);
   }
   aSettings.endArray();
   return pData;
}

void wizard::EditorPrefObject::SaveSettingsP(QSettings& aSettings) const
{
   aSettings.setValue("showLineNumber", mCurrentPrefs.mShowLineNumbers);
   aSettings.setValue("showSyntaxTips", mCurrentPrefs.mShowSyntaxTips);
   aSettings.setValue("autocomplete", mCurrentPrefs.mAutocompleteAfterPeriod);
   aSettings.setValue("undoPopups", mCurrentPrefs.mEnableUndoPopups);
   aSettings.setValue("foldText", mCurrentPrefs.mFoldText);
   aSettings.setValue("tabSize", mCurrentPrefs.mTabSpace);
   aSettings.setValue("fontSize", mCurrentPrefs.mFontSize);
   aSettings.setValue("fontType", mCurrentPrefs.mFontType);
   aSettings.beginWriteArray("darkEditorColor");
   int idx = 0;
   for (auto&& it : mCurrentPrefs.mDarkEditorColor)
   {
      aSettings.setArrayIndex(idx++);
      aSettings.setValue("name", it.first);
      aSettings.setValue("color", it.second);
   }
   aSettings.endArray();
   aSettings.beginWriteArray("lightEditorColor");
   idx = 0;
   for (auto&& it : mCurrentPrefs.mLightEditorColor)
   {
      aSettings.setArrayIndex(idx++);
      aSettings.setValue("name", it.first);
      aSettings.setValue("color", it.second);
   }
   aSettings.endArray();
   aSettings.beginWriteArray("darkStyle");
   idx = 0;
   for (auto&& it : mCurrentPrefs.mDarkStyle)
   {
      aSettings.setArrayIndex(idx++);
      aSettings.setValue("name", static_cast<int>(it.first));
      aSettings.setValue("color", it.second.mColor);
      aSettings.setValue("bold", it.second.mBold);
      aSettings.setValue("italic", it.second.mItalic);
      aSettings.setValue("underline", it.second.mUnderline);
   }
   aSettings.endArray();
   aSettings.beginWriteArray("lightStyle");
   idx = 0;
   for (auto&& it : mCurrentPrefs.mLightStyle)
   {
      aSettings.setArrayIndex(idx++);
      aSettings.setValue("name", static_cast<int>(it.first));
      aSettings.setValue("color", it.second.mColor);
      aSettings.setValue("bold", it.second.mBold);
      aSettings.setValue("italic", it.second.mItalic);
      aSettings.setValue("underline", it.second.mUnderline);
   }
   aSettings.endArray();
   aSettings.beginWriteArray("filePos");
   idx = 0;
   for (auto&& it : mCurrentPrefs.mFilePositions)
   {
      aSettings.setArrayIndex(idx++);
      aSettings.setValue("file", it.first);
      aSettings.setValue("line", (unsigned int)(it.second.first));
      aSettings.setValue("column", (unsigned int)(it.second.second));
   }
   aSettings.endArray();
}

bool wizard::EditorPrefObject::ShowLineNumbers() const
{
   return mCurrentPrefs.mShowLineNumbers;
}

const QColor wizard::EditorPrefObject::EditorColor(const QString& aName) const
{
   if (QIcon::themeName() == "Dark")
   {
      std::map<QString, QColor>::const_iterator it = mCurrentPrefs.mDarkEditorColor.find(aName);
      if (it != mCurrentPrefs.mDarkEditorColor.end())
      {
         return it->second;
      }
   }
   else
   {
      std::map<QString, QColor>::const_iterator it = mCurrentPrefs.mLightEditorColor.find(aName);
      if (it != mCurrentPrefs.mLightEditorColor.end())
      {
         return it->second;
      }
   }
   return Qt::red;
}

const unsigned int wizard::EditorPrefObject::TabSpace() const
{
   return mCurrentPrefs.mTabSpace;
}

bool wizard::EditorPrefObject::AutocompleteAfterPeriod() const
{
   return mCurrentPrefs.mAutocompleteAfterPeriod;
}

bool wizard::EditorPrefObject::UndoPopupsEnabled() const
{
   return mCurrentPrefs.mEnableUndoPopups;
}

const std::map<wizard::TextType, wizard::TextStyle>& wizard::EditorPrefObject::GetStyles()
{
   if (QIcon::themeName() == "Dark")
   {
      return mCurrentPrefs.mDarkStyle;
   }
   else
   {
      return mCurrentPrefs.mLightStyle;
   }
}

bool wizard::EditorPrefObject::FoldText() const
{
   return mCurrentPrefs.mFoldText;
}

unsigned int wizard::EditorPrefObject::FontSize() const
{
   return mCurrentPrefs.mFontSize;
}

void wizard::EditorPrefObject::SetFontSize(unsigned int aSize)
{
   if (aSize != mCurrentPrefs.mFontSize)
   {
      mCurrentPrefs.mFontSize = aSize;
      emit Changed();
   }
}

QString wizard::EditorPrefObject::FontType() const
{
   return mCurrentPrefs.mFontType;
}

void wizard::EditorPrefObject::RememberFilePosition(const QString& aPath, const size_t aLine, const size_t aColumn)
{
   mCurrentPrefs.mFilePositions[aPath] = std::make_pair(aLine, aColumn);
}

std::pair<size_t, size_t> wizard::EditorPrefObject::RecallFilePosition(const QString& aPath)
{
   std::map<QString, std::pair<size_t, size_t>>::const_iterator it = mCurrentPrefs.mFilePositions.find(aPath);
   if (it != mCurrentPrefs.mFilePositions.end())
   {
      return it->second;
   }
   return std::make_pair(0, 0);
}

bool wizard::EditorPrefObject::ShowSyntaxTips() const
{
   return mCurrentPrefs.mShowSyntaxTips;
}
