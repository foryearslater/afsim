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

#ifndef EDITORPREFOBJECT_HPP
#define EDITORPREFOBJECT_HPP

#include "UiResources.hpp"
#include "UtCast.hpp"
#include "ViExport.hpp"
#include "WkfPrefObject.hpp"

namespace wizard
{
enum class TextType : size_t
{
   // Normal styles

   // Text not parsed
   None = 0,
   // Text token matches keyword in grammar
   Keyword = 1,
   // Keyword marking the start or end of a block
   Block_Keyword = 2,
   // Keyword marking the start of a command
   Command = 3,
   // A user-defined input value (number, string, etc...)
   User_Input = 4,
   // A WSF type (WSF_PLATFORM ...)
   Type = 5,
   // A new user type (MY_PLATFORM)
   Name = 6,
   // include line
   Include = 7,
   // Comment  # or //
   Comment = 8,
   // Quoted string (not used right now)
   Quoted          = 9,
   Unmatched_Block = 10,
   Include_Error   = 11,
   Syntax_Tip      = 12,
   Syntax_Tip_Bold = 13,
   Preprocessor    = 14,

   // Script styles

   Script_Keyword = 15,
   Script_Operator,
   Script_Quoted,
   Script_Comment,
   Script_Type,
   Script_Number,
   Script_Static_Method,
   Script_Method,
   Script_Local_Method,
   Script_Local_Variable,
   Script_Parameter,
   Script_None,
   Script_End,

   Margin_None
};

struct TextStyle
{
   TextStyle() = default;

   TextStyle(const QColor& aColor, bool aBold, bool aItalic, bool aUnderline)
      : mColor(aColor)
      , mBold(aBold)
      , mItalic(aItalic)
      , mUnderline(aUnderline)
   {
   }

   QColor mColor{Qt::red};
   bool   mBold{false};
   bool   mItalic{false};
   bool   mUnderline{false};
};
struct EditorPrefData
{
   EditorPrefData();

   bool                          mShowLineNumbers{true};
   bool                          mShowSyntaxTips{true};
   bool                          mAutocompleteAfterPeriod{true};
   bool                          mEnableUndoPopups{false};
   unsigned int                  mTabSpace{3};
   bool                          mFoldText{true};
   std::map<QString, QColor>     mDarkEditorColor;
   std::map<QString, QColor>     mLightEditorColor;
   std::map<TextType, TextStyle> mDarkStyle;
   std::map<TextType, TextStyle> mLightStyle;
   QString                       mFontType{UiResources::GetInstance().GetFont().family()};
   unsigned int mFontSize{ut::safe_cast<unsigned int>(UiResources::GetInstance().GetFont().pointSize())};
   std::map<QString, std::pair<size_t, size_t>> mFilePositions;
};

class VI_EXPORT EditorPrefObject : public wkf::PrefObjectT<EditorPrefData>
{
   Q_OBJECT

public:
   static constexpr const char* cNAME = "EditorOptions";

   EditorPrefObject(QObject* parent = nullptr);
   ~EditorPrefObject() override = default;

   void Apply() override;

   EditorPrefData ReadSettings(QSettings& aSettings) const override;
   void           SaveSettingsP(QSettings& aSettings) const override;

   bool                                 ShowLineNumbers() const;
   bool                                 ShowSyntaxTips() const;
   const QColor                         EditorColor(const QString& aName) const;
   const unsigned int                   TabSpace() const;
   bool                                 AutocompleteAfterPeriod() const;
   bool                                 UndoPopupsEnabled() const;
   const std::map<TextType, TextStyle>& GetStyles();
   bool                                 FoldText() const;
   unsigned int                         FontSize() const;
   void                                 SetFontSize(unsigned int aSize);
   QString                              FontType() const;

   void                      RememberFilePosition(const QString& aPath, const size_t aLine, const size_t aColumn);
   std::pair<size_t, size_t> RecallFilePosition(const QString& aPath);
signals:
   void Changed();
};
} // namespace wizard

Q_DECLARE_METATYPE(wizard::EditorPrefData)
#endif
