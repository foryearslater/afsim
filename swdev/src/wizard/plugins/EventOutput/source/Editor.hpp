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

#ifndef EVENT_OUTPUT_EDITOR_HPP
#define EVENT_OUTPUT_EDITOR_HPP

#include <QDockWidget>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTextCursor>

#include "UtOptional.hpp"

namespace wizard
{

class Editor;
class TextSource;

namespace EventOutput
{

class SettingsWidget;

//! Namespace for objects, functions, and variables related to the Editor
namespace editor
{
namespace tokenizer
{

struct BlockTags;
class Tokenizer;

} // namespace tokenizer

//! A leight-weight wrapper around a wizard::Editor for csv_event_output and event_output
//! @par details
//!   This class is a leight-weight wrapper around a wizard::Editor, allowing convenient editing of csv_event_output and
//!   event_output blocks. it has a list of tokenizer::Tokenizers that are the brains of the operation.
//! @note on construction, the Editor makes the wizard::Editor read-only.  on destruction, it undoes this.
//! @see tokenizer::Tokenizer for more information
class Editor
{
public:
   //! Constructs an Editor
   //! @param aFileName is the name of the file to which the wizard::Editor refers
   //! @note makes the wizard::Editor read-only
   Editor(std::string aFileName);
   Editor(const Editor& aSrc) = default;
   //! Constructs an Editor
   //! @param aSrc is the Editor from which to transfer ownership of its contents
   //! @note makes the wizard::Editor read-only
   Editor(Editor&& aSrc);

   //! Destructs an Editor
   //! @note restores the write capabilities to the wizard::Editor
   ~Editor();

   Editor& operator=(const Editor& aSrc) = default;
   //! Transfers ownership of the contents of an Editor to another
   //! @param aSrc is the Editor from which to transfer ownership of contents
   Editor& operator=(Editor&& aSrc);

   //! Get the file path
   //! @return the file path
   QString GetFilePath() const;
   //! Get the associated wizard::Editor
   wizard::Editor* GetWizEditor();
   //! Set the "need to save" property
   //! @param aTokenizerName is the name of the tokenizer::Tokenizer of which to set the property
   //! @param aNeedToSave denotes whether or not the tokenizer::Tokenizers need to stash the changes
   void SetNeedToSave(QString aTokenizerName, bool aNeedToSave);
   //! Get the "need to save" property
   //! @param aTokenizerName is the name of the tokenizer::Tokenizer of which to get the property
   //! @return whether or not the tokenizer::Tokenizers need to stash the changes
   bool NeedToSave(QString aTokenizerName) const;

   //! Creates a tokenizer::Tokenizer and adds it to the list
   //! @param aName is the name of the tokenizer::Tokenizer
   //! @param aTags are the begin and end tags of the block
   //! @return whether or not the tokenizer::Tokenizer was created
   bool AddTokenizer(const QString& aName, const tokenizer::BlockTags& aTags);
   //! Creates a tokenizer::Tokenizer and adds it to the list
   //! @param aName is the name of the tokenizer::Tokenizer to remove from the Editor
   //! @return whether or not the tokenizer::Tokenizer was removed from the Editor
   bool RemoveTokenizer(const QString& aName);

   //! Get a tokenizer::Tokenizer
   //! @param aName is the name of the tokenizer::Tokenizer
   //! @return a pointer to the tokenizer::Tokenizer corresponding to the given name (immutable)
   const tokenizer::Tokenizer* GetTokenizer(const QString& aName) const;
   //! Get a tokenizer::Tokenizer
   //! @param aName is the name of the tokenizer::Tokenizer
   //! @return a pointer to the tokenizer::Tokenizer corresponding to the given name (mutable)
   tokenizer::Tokenizer* GetTokenizer(const QString& aName);

   //! Attempts to bring the wizard::Editor to focus and load the tokenizer::Tokenizers with the appropriate text
   //! @param aFileName is the name of the file (with path) to which the wizard::Editor corresponds
   //! @return whether or not it was successful
   bool Open(const std::string& aFileName);
   //! Attempts to have the tokenizer::Tokenizers build "cache" strings
   //! @return whether or not it was successful
   //! @see tokenizer::Tokenizer for more information about the "cache" string
   bool Cache();
   //! Attempts to apply changes contained in the tokenizer::Tokenizers to the wizard::Editor
   //! @return whether or not it was successful
   bool Save();

   //! Method executed when a wizard::Editor is opened
   //! @param aWizEditor is the specified wizard::Editor
   void OnEditorOpen(wizard::Editor* aWizEditor);
   //! Method executed when a wizard::Editor is closed
   //! @param aWizEditor is the specified wizard::Editor
   void OnEditorClose(wizard::Editor* aWizEditor);
   //! Gives focus to the wizard::Editor associated with this Editor
   void GiveFocus();

private:
   //! Locks the wizard::Editor in read-only mode
   bool Lock();
   //! Undoes the read-only lock on the wizard::Editor
   bool Unlock();

   //! The file name (with path)
   std::string mFileName;
   //! The wrapped wizard::Editor
   QPointer<wizard::Editor> mWizEditor;
   //! The list of tokenizer::Tokenizers
   QMap<QString, tokenizer::Tokenizer> mTokenizers;
};

} // namespace editor
} // namespace EventOutput
} // namespace wizard

#endif // EVENT_OUTPUT_EDITOR_HPP
