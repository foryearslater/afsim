// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "Editor.hpp"

#include <cctype>

#include <QString>
#include <QStringList>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextFragment>
#include <QTextFrame>
#include <QTextList>
#include <QTextStream>

#include "CommandValuePair.hpp"
#include "Editor.hpp"
#include "EventWidget.hpp"
#include "ParseResults.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "SettingWidget.hpp"
#include "Signals.hpp"
#include "TextSource.hpp"
#include "Tokenizer.hpp"
#include "WsfEditor.hpp"
#include "WsfParseRule.hpp"
#include "WsfParser.hpp"

namespace wizard
{
namespace EventOutput
{
namespace editor
{

Editor::Editor(std::string aFileName)
   : mFileName(std::move(aFileName))
   , mWizEditor(ProjectWorkspace::Instance()->GotoFile(mFileName))
{
   Lock();
}

Editor::Editor(Editor&& aSrc)
   : mFileName(std::move(aSrc.mFileName))
   , mWizEditor(std::move(aSrc.mWizEditor))
{
   // prevent a reset of the read-only lock
   aSrc.mWizEditor = nullptr;
}

Editor::~Editor()
{
   Unlock();
}

Editor& Editor::operator=(Editor&& aSrc)
{
   if (this != &aSrc)
   {
      mFileName  = std::move(aSrc.mFileName);
      mWizEditor = std::move(aSrc.mWizEditor);
      // prevent a reset of the read-only lock
      aSrc.mWizEditor = nullptr;
   }

   return *this;
}

QString Editor::GetFilePath() const
{
   if (mWizEditor)
   {
      return QString::fromStdString(mWizEditor->GetSource()->GetFilePath().GetNormalizedPath());
   }
   return {};
}

wizard::Editor* Editor::GetWizEditor()
{
   return mWizEditor;
}

void Editor::SetNeedToSave(QString aTokenizerName, bool aNeedToSave)
{
   auto iter = mTokenizers.find(std::move(aTokenizerName));
   if (iter != mTokenizers.end())
   {
      iter->SetNeedToSave(aNeedToSave);
   }
}

bool Editor::NeedToSave(QString aTokenizerName) const
{
   auto iter = mTokenizers.find(std::move(aTokenizerName));
   if (iter != mTokenizers.end())
   {
      return iter->NeedToSave();
   }
   return false;
}

bool Editor::AddTokenizer(const QString& aName, const tokenizer::BlockTags& aTags)
{
   auto iter = mTokenizers.constFind(aName);
   if (iter != mTokenizers.constEnd())
   {
      return false;
   }
   mTokenizers.insert(aName, tokenizer::Tokenizer{aTags});
   return true;
}

bool Editor::RemoveTokenizer(const QString& aName)
{
   auto iter = mTokenizers.find(aName);
   if (iter != mTokenizers.end())
   {
      return false;
   }
   mTokenizers.remove(aName);
   return true;
}

const tokenizer::Tokenizer* Editor::GetTokenizer(const QString& aName) const
{
   auto iter = mTokenizers.constFind(aName);
   if (iter != mTokenizers.constEnd())
   {
      return &iter.value();
   }
   return nullptr;
}

tokenizer::Tokenizer* Editor::GetTokenizer(const QString& aName)
{
   auto iter = mTokenizers.find(aName);
   if (iter != mTokenizers.end())
   {
      return &iter.value();
   }
   return nullptr;
}

bool Editor::Open(const std::string& aFileName)
{
   if (mWizEditor)
   {
      for (auto& elem : mTokenizers)
      {
         // initialize the tokenizer::Tokenizer
         if (elem.ConnectToEditor(this))
         {
            elem.SetSource(mWizEditor->GetSource());
            elem.CreateCVPsFromTokens();
         }
      }
      return true;
   }
   else
   {
      return false;
   }
}

bool Editor::Cache()
{
   for (auto& elem : mTokenizers)
   {
      elem.CreateStringFromCVPs();
   }
   return true;
}

bool Editor::Save()
{
   std::multimap<QTextCursor, const tokenizer::Tokenizer*, std::greater<QTextCursor>> sortedTokenizers;
   for (auto& elem : mTokenizers)
   {
      if (elem.NeedToSave())
      {
         if (!elem.IsCached())
         {
            elem.CreateStringFromCVPs();
         }
         // get the block range and store it into a set
         sortedTokenizers.insert({elem.GetRangeFromQtDocument(), &elem});
      }
   }
   for (auto elem : sortedTokenizers)
   {
      // apply the changes to the wizard::Editor in reverse order
      if (!elem.first.selection().isEmpty())
      {
         mWizEditor->Replace(UtTextRange{ut::cast_to_size_t(elem.first.selectionStart()),
                                         ut::cast_to_size_t(elem.first.selectionEnd())},
                             QString{elem.second->GetCache() + '\n'}.toStdString());
      }
      else
      {
         mWizEditor->Replace(UtTextRange{mWizEditor->GetSource()->Size() - 1, mWizEditor->GetSource()->Size()},
                             QString{elem.second->GetCache() + '\n'}.toStdString());
      }
   }
   return true;
}

void Editor::OnEditorOpen(wizard::Editor* aWizEditor)
{
   if (!aWizEditor /* this is the case to force lock */ || !mWizEditor ||
       (aWizEditor && mFileName == aWizEditor->GetSource()->GetFilePath().GetNormalizedPath()))
   {
      mWizEditor = aWizEditor;
      Lock();
      for (auto& elem : mTokenizers)
      {
         elem.SetSource(mWizEditor->GetSource());
      }
   }
}

void Editor::OnEditorClose(wizard::Editor* aWizEditor)
{
   if (!aWizEditor /* this is the case to force unlock */ || mWizEditor ||
       (aWizEditor && mFileName == aWizEditor->GetSource()->GetFilePath().GetNormalizedPath()))
   {
      Unlock();
      mWizEditor = nullptr;
   }
}

void Editor::GiveFocus()
{
   ProjectWorkspace::Instance()->GotoFile(mFileName);
}

bool Editor::Lock()
{
   if (mWizEditor && !mWizEditor->isReadOnly())
   {
      mWizEditor->setReadOnly(true);
      return true;
   }
   return false;
}

bool Editor::Unlock()
{
   if (mWizEditor && mWizEditor->isReadOnly())
   {
      mWizEditor->setReadOnly(false);
      return true;
   }
   return false;
}

} // namespace editor
} // namespace EventOutput
} // namespace wizard
