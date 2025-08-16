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

#include "EditorPrefWidget.hpp"

#include <QFile>
#include <QTextStream>

wizard::EditorPrefWidget::EditorPrefWidget(QWidget* parent /*= nullptr*/)
   : PrefWidgetT<EditorPrefObject>(parent)
{
   mUi.setupUi(this);

   mStyleNames[static_cast<int>(TextType::Keyword)]         = "keyword";
   mStyleNames[static_cast<int>(TextType::Block_Keyword)]   = "block_keyword";
   mStyleNames[static_cast<int>(TextType::Command)]         = "command";
   mStyleNames[static_cast<int>(TextType::User_Input)]      = "user_input";
   mStyleNames[static_cast<int>(TextType::Type)]            = "type";
   mStyleNames[static_cast<int>(TextType::Name)]            = "name";
   mStyleNames[static_cast<int>(TextType::Include)]         = "include";
   mStyleNames[static_cast<int>(TextType::Comment)]         = "comment";
   mStyleNames[static_cast<int>(TextType::Quoted)]          = "quoted";
   mStyleNames[static_cast<int>(TextType::Unmatched_Block)] = "unmatched_block";
   mStyleNames[static_cast<int>(TextType::None)]            = "regular";
   mStyleNames[static_cast<int>(TextType::Preprocessor)]    = "preprocessor";

   // Script
   mStyleNames[static_cast<int>(TextType::Script_Keyword)]        = "script_keyword";
   mStyleNames[static_cast<int>(TextType::Script_Operator)]       = "script_operator";
   mStyleNames[static_cast<int>(TextType::Script_Quoted)]         = "script_quoted";
   mStyleNames[static_cast<int>(TextType::Script_Comment)]        = "script_comment";
   mStyleNames[static_cast<int>(TextType::Script_Type)]           = "script_type";
   mStyleNames[static_cast<int>(TextType::Script_Number)]         = "script_number";
   mStyleNames[static_cast<int>(TextType::Script_Static_Method)]  = "script_static_method";
   mStyleNames[static_cast<int>(TextType::Script_Method)]         = "script_method";
   mStyleNames[static_cast<int>(TextType::Script_Local_Method)]   = "script_local_method";
   mStyleNames[static_cast<int>(TextType::Script_Local_Variable)] = "script_local_variable";
   mStyleNames[static_cast<int>(TextType::Script_Parameter)]      = "script_parameter";
   mStyleNames[static_cast<int>(TextType::Script_None)]           = "script_regular";

   connect(mUi.fontComboBox, &QFontComboBox::currentFontChanged, this, &wizard::EditorPrefWidget::UpdatePreview);
   connect(mUi.fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &wizard::EditorPrefWidget::UpdatePreview);
   connect(mUi.styleColor, &UtQtColorButton::ColorChanged, this, &wizard::EditorPrefWidget::UpdateStyle);
   connect(mUi.styleBold, &QCheckBox::toggled, this, &wizard::EditorPrefWidget::UpdateStyle);
   connect(mUi.styleItalic, &QCheckBox::toggled, this, &wizard::EditorPrefWidget::UpdateStyle);
   connect(mUi.styleUnderline, &QCheckBox::toggled, this, &wizard::EditorPrefWidget::UpdateStyle);
   connect(mUi.styleList, &QListWidget::itemClicked, this, &wizard::EditorPrefWidget::StyleClicked);
}

void wizard::EditorPrefWidget::showEvent(QShowEvent* aEvent)
{
   // This is necessary because the pref widget isn't constructed until after settings are loaded into pref object
   ReadPreferenceData(mPrefObjectPtr->GetPreferences());
}

void wizard::EditorPrefWidget::ReadPreferenceData(const EditorPrefData& aPrefData)
{
   mUi.mLineNumbers->setChecked(aPrefData.mShowLineNumbers);
   mUi.mTabSize->setValue(aPrefData.mTabSpace);
   mUi.mSyntaxTips->setChecked(aPrefData.mShowSyntaxTips);
   mUi.mAutocomplete->setChecked(aPrefData.mAutocompleteAfterPeriod);
   mUi.mEnableUndoPopups->setChecked(aPrefData.mEnableUndoPopups);
   mUi.fontComboBox->setCurrentFont(QFont(aPrefData.mFontType));
   mUi.fontSizeSpinBox->setValue(aPrefData.mFontSize);

   mUi.styleList->clear();
   mDarkThemed = (QIcon::themeName() == "Dark");
   if (mDarkThemed)
   {
      for (auto&& it : aPrefData.mDarkStyle)
      {
         QListWidgetItem* item = new QListWidgetItem(mStyleNames[static_cast<int>(it.first)]);
         item->setData(Qt::UserRole, static_cast<int>(it.first));
         item->setData(Qt::UserRole + 1, it.second.mColor);
         item->setData(Qt::UserRole + 2, it.second.mBold);
         item->setData(Qt::UserRole + 3, it.second.mItalic);
         item->setData(Qt::UserRole + 4, it.second.mUnderline);
         mUi.styleList->addItem(item);
      }
   }
   else
   {
      for (auto&& it : aPrefData.mLightStyle)
      {
         QListWidgetItem* item = new QListWidgetItem(mStyleNames[static_cast<int>(it.first)]);
         item->setData(Qt::UserRole, static_cast<int>(it.first));
         item->setData(Qt::UserRole + 1, it.second.mColor);
         item->setData(Qt::UserRole + 2, it.second.mBold);
         item->setData(Qt::UserRole + 3, it.second.mItalic);
         item->setData(Qt::UserRole + 4, it.second.mUnderline);
         mUi.styleList->addItem(item);
      }
   }

   UpdatePreview();
}

void wizard::EditorPrefWidget::WritePreferenceData(EditorPrefData& aPrefData)
{
   aPrefData.mShowLineNumbers         = mUi.mLineNumbers->isChecked();
   aPrefData.mTabSpace                = mUi.mTabSize->value();
   aPrefData.mShowSyntaxTips          = mUi.mSyntaxTips->isChecked();
   aPrefData.mAutocompleteAfterPeriod = mUi.mAutocomplete->isChecked();
   aPrefData.mEnableUndoPopups        = mUi.mEnableUndoPopups->isChecked();
   aPrefData.mFontType                = mUi.fontComboBox->currentFont().family();
   aPrefData.mFontSize                = mUi.fontSizeSpinBox->value();

   // NOTE, the user may have changed themes along with the style changes
   // but we want to write to the theme we read from
   if (mDarkThemed)
   {
      int styleCount = mUi.styleList->count();
      for (int i = 0; i < styleCount; ++i)
      {
         QListWidgetItem* item = mUi.styleList->item(i);
         aPrefData.mDarkStyle[static_cast<TextType>(item->data(Qt::UserRole).toInt())] =
            TextStyle(item->data(Qt::UserRole + 1).value<QColor>(),
                      item->data(Qt::UserRole + 2).toBool(),
                      item->data(Qt::UserRole + 3).toBool(),
                      item->data(Qt::UserRole + 4).toBool());
      }
   }
   else
   {
      int styleCount = mUi.styleList->count();
      for (int i = 0; i < styleCount; ++i)
      {
         QListWidgetItem* item = mUi.styleList->item(i);
         aPrefData.mLightStyle[static_cast<TextType>(item->data(Qt::UserRole).toInt())] =
            TextStyle(item->data(Qt::UserRole + 1).value<QColor>(),
                      item->data(Qt::UserRole + 2).toBool(),
                      item->data(Qt::UserRole + 3).toBool(),
                      item->data(Qt::UserRole + 4).toBool());
      }
   }
}

void wizard::EditorPrefWidget::UpdatePreview()
{
   // Clear the text preview area and reformat
   mUi.textPreview->clear();

   QFile file(":/wsf_example_formatting.txt");
   file.open(QFile::ReadOnly | QFile::Text);
   QTextStream stream(&file);
   QString     textToStyle = stream.readAll();
   file.close();
   // Quick return if no text is loaded
   if (textToStyle.isEmpty())
   {
      return;
   }

   QString                                      text = textToStyle.replace("\r", "");
   QRegExp                                      re("[{][{]([^}]*)[}][}]");
   std::vector<std::pair<int, QTextCharFormat>> formatStarts;
   while (true)
   {
      int idx = text.indexOf(re);
      if (idx == -1)
         break;
      QString                 tag   = re.cap(1);
      QList<QListWidgetItem*> items = mUi.styleList->findItems(tag, Qt::MatchExactly);
      if (!items.isEmpty())
      {
         QColor    color     = (*items.begin())->data(Qt::UserRole + 1).value<QColor>();
         bool      bold      = (*items.begin())->data(Qt::UserRole + 2).toBool();
         bool      italic    = (*items.begin())->data(Qt::UserRole + 3).toBool();
         bool      underline = (*items.begin())->data(Qt::UserRole + 4).toBool();
         TextStyle stylePtr(color, bold, italic, underline);
         formatStarts.emplace_back(std::make_pair(idx, MakeFormat(stylePtr)));
         text = text.mid(0, idx) + text.mid(idx + 4 + re.cap(1).length());
      }
      else
      {
         TextStyle stylePtr(QColor(Qt::red), false, false, false);
         formatStarts.emplace_back(std::make_pair(idx, MakeFormat(stylePtr)));
         text = text.mid(0, idx) + text.mid(idx + 4 + re.cap(1).length());
      }
   }
   formatStarts.push_back(formatStarts.back());
   formatStarts.back().first = text.size();

   // Walk through text and insert styled text into editor
   int pos = 0;

   QTextCursor     c           = mUi.textPreview->textCursor();
   QTextCharFormat spaceFormat = formatStarts[0].second;
   for (size_t i = 1; i < formatStarts.size(); ++i)
   {
      QString segment = text.mid(pos, formatStarts[i].first - pos);
      for (int j = 0; j < segment.size(); ++j)
      {
         if (j == 0 || segment[j].isSpace() != segment[j - 1].isSpace())
         {
            if (segment[j].isSpace())
            {
               c.setCharFormat(spaceFormat);
            }
            else
            {
               c.setCharFormat(formatStarts[i - 1].second);
            }
         }
         c.insertText(segment.mid(j, 1));
      }
      pos = formatStarts[i].first;
   }
}

QTextCharFormat wizard::EditorPrefWidget::MakeFormat(const TextStyle& aStyle) const
{
   QTextCharFormat fmt;
   fmt.setForeground(aStyle.mColor);
   if (aStyle.mBold)
   {
      fmt.setFontWeight(QFont::Bold);
   }
   else
   {
      fmt.setFontWeight(QFont::Normal);
   }
   fmt.setFontUnderline(aStyle.mUnderline);
   fmt.setFontItalic(aStyle.mItalic);
   fmt.setFontPointSize(mUi.fontSizeSpinBox->value());
   fmt.setFontFamily(mUi.fontComboBox->currentFont().family());
   return fmt;
}

void wizard::EditorPrefWidget::UpdateStyle()
{
   QListWidgetItem* item = mUi.styleList->currentItem();
   if (item)
   {
      item->setData(Qt::UserRole + 1, mUi.styleColor->GetColor());
      item->setData(Qt::UserRole + 2, mUi.styleBold->isChecked());
      item->setData(Qt::UserRole + 3, mUi.styleItalic->isChecked());
      item->setData(Qt::UserRole + 4, mUi.styleUnderline->isChecked());
   }
   UpdatePreview();
}

void wizard::EditorPrefWidget::StyleClicked(QListWidgetItem* aItem)
{
   mUi.styleColor->SetColor(aItem->data(Qt::UserRole + 1).value<QColor>());
   mUi.styleBold->setChecked(aItem->data(Qt::UserRole + 2).toBool());
   mUi.styleItalic->setChecked(aItem->data(Qt::UserRole + 3).toBool());
   mUi.styleUnderline->setChecked(aItem->data(Qt::UserRole + 4).toBool());
}
