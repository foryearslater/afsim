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

#include "WsfEditor.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDragMoveEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QToolTip>

#include "ActionManager.hpp"
#include "AutoCompletePopup.hpp"
#include "CommandDocumentation.hpp"
#include "ComponentPropertyManager.hpp"
#include "DragAndDrop.hpp"
#include "EditAssist.hpp"
#include "EditorManager.hpp"
#include "EditorPrefObject.hpp"
#include "Environment.hpp"
#include "FindReferencesResultDialog.hpp"
#include "ParseResults.hpp"
#include "ParseUtil.hpp"
#include "PasteContext.hpp"
#include "Project.hpp"
#include "ProjectWorkspace.hpp"
#include "Signals.hpp"
#include "SyntaxTips.hpp"
#include "TextSource.hpp"
#include "TextSourceCache.hpp"
#include "UiResources.hpp"
#include "UtCast.hpp"
#include "UtTextDocument.hpp"
#include "WkfEnvironment.hpp"
#include "WkfGeneralPrefObject.hpp"
#include "WsfExe.hpp"
#include "WsfExeManager.hpp"
#include "WsfPM_Root.hpp"
#include "WsfPProxyStructType.hpp"
#include "WsfParseBasicRules.hpp"
#include "WsfParseFormat.hpp"
#include "WsfParseSourceInclude.hpp"

namespace
{
struct OpenFileAction
{
   std::string mFileName;
   void        operator()() { wizard::Project::Instance()->OpenFile(mFileName); }
};
} // namespace

wizard::WsfEditor::WsfEditor(QWidget* aParent, Project* aProject, TextSource* aTextSource, EditorManager* aManagerPtr)
   : Editor(aParent, aProject, aTextSource, aManagerPtr)
   , mAutocompletePopupPtr(nullptr)
   , mSyntaxTipsPtr()
   , mContextMenuCharacterPosition(-1)
   , mDragIncludeLine(-1)
   , mFirstStyleComplete(false)
   , mUpdatingBreakpoints(false)
   , mHoveringFolds(false)
   , mFullStyleFirstLine(-1)
   , mFullStyleLastLine(-1)
   , mCurrentStylePos(ut::npos)
   , mCurrentStyleLength(0)
   , mStylingNow(false)
{
   setMouseTracking(true);
   mFocusCheckTimer.setInterval(500);
   QObject::connect(&mFocusCheckTimer, &QTimer::timeout, this, &WsfEditor::FocusCheck);
   connect(wizSignals, &Signals::BreakpointsChanged, this, &WsfEditor::UpdateBreakpointMargin);
   mUtCallbacks += wizSignals->CurrentDebugLineChanged.Connect(&WsfEditor::CurrentDebugLineChanged, this);
   connect(wizSignals, &Signals::ParseResultsInvalidate, this, &WsfEditor::ParseResultsInvalidate);
   connect(wkfEnv.GetPreferenceObject<wkf::GeneralPrefObject>(),
           &wkf::GeneralPrefObject::NumRecentConfigsChanged,
           this,
           &WsfEditor::GeneralPreferencesChanged);
   connect(wkfEnv.GetPreferenceObject<EditorPrefObject>(),
           &EditorPrefObject::Changed,
           this,
           &WsfEditor::EditorPreferencesChanged);
}

void wizard::WsfEditor::Initialize()
{
   Editor::Initialize();

   connect(GetProject()->GetWorkspace(), &wizard::ProjectWorkspace::ProxyUpdated, this, &WsfEditor::OnProxyUpdate);

   // Qt hasn't finished document related stuff -- wait a bit before trying to do anything.
   QTimer::singleShot(10, this, &WsfEditor::Initialize2);
}

void wizard::WsfEditor::Initialize2()
{
   OnProxyUpdate(nullptr);
}

void wizard::WsfEditor::TipsMoved(QRect aRect)
{
   if (mAutocompletePopupPtr && mAutocompletePopupPtr->isVisible())
   {
      mAutocompletePopupPtr->UpdateWindowPosition();
   }
}

int wizard::WsfEditor::GetContextMenuCharacterPosition()
{
   return mContextMenuCharacterPosition;
}

void wizard::WsfEditor::HideAutocomplete()
{
   if (mAutocompletePopupPtr != nullptr)
   {
      mAutocompletePopupPtr->setFocusProxy(nullptr);
      mAutocompletePopupPtr->setVisible(false);
      mAutocompletePopupPtr->deleteLater();
      mAutocompletePopupPtr = nullptr;
      setFocus();
   }
}

QRect wizard::WsfEditor::GetTipsGeometry()
{
   QRect geom;
   if (mSyntaxTipsPtr)
   {
      geom = mSyntaxTipsPtr->GetGeometry();
   }
   return geom;
}

void wizard::WsfEditor::UpdateStyle()
{
   mFullStyleFirstLine = mFullStyleLastLine = -1;

   ParseResults* results = GetProject()->GetParseResults();
   if (results)
   {
      WsfStyler* stylerPtr = wizExeMgr.GetStyler();
      if (FoldingEnabled())
      {
         // Grabs lines where folding is allowed
         std::vector<std::pair<int, int>> foldLevels      = stylerPtr->FoldDocument(results, GetSource());
         int                              locationInArray = 0;
         for (const auto& it : mFoldLevels)
         {
            if (!it.mIsExpanded)
            {
               // Can't use find on this function, because the vectors contain different objects (pairs and foldStates)
               bool found = false;
               for (const auto& pit : foldLevels)
               {
                  if (pit.first == it.first && pit.second == it.second)
                  {
                     found = true;
                  }
               }

               // two arrays are checked: if it is in foldLevels, then the fold lines have not been modified. mModifiedFolds
               // keeps track of what files have been modified from a keystroke The combination of these two allows for
               // a line to be modified as long at it legal and only unfolds specifically sections with illegal modifications
               auto val = std::find(mModifiedFolds.begin(), mModifiedFolds.end(), locationInArray);
               if (!found && val != mModifiedFolds.end())
               {
                  UnhideLines(it.first, it.second);
               }
            }
            locationInArray++;
         }
         mFoldLevels.clear();
         mModifiedFolds.clear();
         for (auto&& it : foldLevels)
         {
            mFoldLevels.emplace_back(it);
         }
         RedrawMargin();
      }

      if (!mStylingNow && !hasFocus()) // if we have focus, re-styling will already happen
      {
         StyleVisibleLines();
      }
   }

   DeferUpdateTimer().start();
}

// A big thing to note for this function (and comment folding in general) is lines in the document more or less align
// with text blocks in this document
void wizard::WsfEditor::UnhideLines(int aFirstLine, int aLastLine)
{
   QTextBlock foldingBlock = firstVisibleBlock();
   int        dirtyStart   = -1;
   while (foldingBlock.isValid())
   {
      // finds block that corresponds with the line after the first marked in a fold. This line should be the first hidden line
      if (GetSource()->PositionToLineNumber(foldingBlock.position()) == ut::cast_to_size_t(aFirstLine + 1))
      {
         dirtyStart = foldingBlock.position();
         break;
      }
      foldingBlock = foldingBlock.next();
   }
   // unhides lines till it goes past the end block
   while (foldingBlock.isValid() && foldingBlock.blockNumber() <= aLastLine)
   {
      foldingBlock.setVisible(true);
      foldingBlock = foldingBlock.next();
   }
   int docLength = document()->lastBlock().position() + document()->lastBlock().length();
   document()->markContentsDirty(dirtyStart, docLength - dirtyStart);
   RedrawMargin();
   // Bug: Qt isn't updating the text in some cases.  Needed to do a deferred update on the viewport
   QTimer::singleShot(1, viewport(), SLOT(update()));
}


// Is supposed to unfold code when inserted throght pasting or drag and drop
void wizard::WsfEditor::insertFromMimeData(const QMimeData* aSource)
{
   QTextCursor cur(textCursor());
   int         startLine = ut::cast_to_int(GetSource()->PositionToLineNumber(cur.selectionStart()));
   int         endLine   = ut::cast_to_int(GetSource()->PositionToLineNumber(cur.selectionEnd()));
   QString     insertTxt = aSource->text();
   auto        newLine   = std::find(insertTxt.begin(), insertTxt.end(), '\n');
   // no reason to take an action if the inserted code is only one line in length
   if (newLine != insertTxt.end())
   {
      // integer iterator
      int locationInArray = 0;
      // the used of inSelection was ment to mark if multiple blocks were selected
      // In this case, a block would have neither the first line or last line in the block, so it would not be taken
      // into account for this function
      bool inSelection = false;
      // assumes mFoldLevels is sorted (should be to prevent potential UI errors)
      for (auto&& it : mFoldLevels)
      {
         if (!it.mIsExpanded &&
             ((it.first <= startLine && it.second >= startLine) || (it.first <= endLine && it.second >= endLine)))
         {
            inSelection = true;
         }
         if (inSelection && it.first >= endLine)
         {
            auto val = std::find(mModifiedFolds.begin(), mModifiedFolds.end(), locationInArray);
            if (val == mModifiedFolds.end())
            {
               mModifiedFolds.push_back(locationInArray);
            }
         }
         locationInArray++;
      }
   }
   Editor::insertFromMimeData(aSource);
}

void wizard::WsfEditor::GeneralPreferencesChanged(int aValue)
{
   LoadSettings();
}

void wizard::WsfEditor::EditorPreferencesChanged()
{
   LoadSettings();
   UpdateStyle();
   repaint();
}

void wizard::WsfEditor::mousePressEvent(QMouseEvent* e)
{
   if (e->modifiers() & Qt::ControlModifier)
   {
      if (e->button() == Qt::LeftButton)
      {
         // Strange: e->pos() != mapFromGlobal(e->globalPos()).  e->pos() appears to be in viewport() coordinates already?
         size_t pos = GetPosition(mapFromGlobal(e->globalPos()));

         QTextCursor cur(document());
         cur.setPosition(ut::cast_to_int(pos));
         cur.movePosition(QTextCursor::PreviousWord);
         cur.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
         if (!cur.selectedText().isEmpty())
         {
            setTextCursor(cur);
         }
      }
   }
   else
   {
      Editor::mousePressEvent(e);
   }
}

void wizard::WsfEditor::focusInEvent(QFocusEvent* e)
{
   Editor::focusInEvent(e);
   mFocusCheckTimer.start();
   if (mSyntaxTipsPtr == nullptr)
   {
      if (wkfEnv.GetPreferenceObject<EditorPrefObject>()->ShowSyntaxTips())
      {
         mSyntaxTipsPtr = new SyntaxTips(this);
      }
   }
}

void wizard::WsfEditor::focusOutEvent(QFocusEvent* e)
{
   Editor::focusOutEvent(e);
}

void wizard::WsfEditor::keyPressEvent(QKeyEvent* e)
{
   bool        passOn    = true;
   QTextCursor cur       = textCursor();
   int         startLine = ut::cast_to_int(GetSource()->PositionToLineNumber(ut::cast_to_size_t(cur.selectionStart())));
   int         endLine   = ut::cast_to_int(GetSource()->PositionToLineNumber(ut::cast_to_size_t(cur.selectionEnd())));
   bool        inSelection = false;
   // marks position of cursor when button is pressed to catch modifications
   for (std::size_t i = 0; i < mFoldLevels.size(); i++)
   {
      // the used of inSelection was ment to mark if multiple blocks were selected
      // In this case, a block would have neither the first line or last line in the block, so it would not be taken
      // into account for this function
      foldStates fold = mFoldLevels[i];
      if ((startLine >= fold.first && startLine <= fold.second) || (endLine >= fold.first && endLine <= fold.second))
      {
         inSelection = true;
      }
      else if (endLine < fold.first)
      {
         inSelection = false;
      }
      if (inSelection)
      {
         auto val = std::find(mModifiedFolds.begin(), mModifiedFolds.end(), i);
         if (val == mModifiedFolds.end())
         {
            mModifiedFolds.push_back(i);
         }
      }
   }

   if (mAutocompletePopupPtr != nullptr && !mAutocompletePopupPtr->isHidden())
   {
      passOn = !mAutocompletePopupPtr->InterceptKeyDown(e);
   }

   if (passOn)
   {
      passOn = false;
      // Perform auto-indent
      if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Down || e->key() == Qt::Key_Right || e->key() == Qt::Key_Left)
      {
         Editor::keyPressEvent(e);
         QTextCursor cur  = textCursor();
         int         line = GetSource()->PositionToLineNumber(cur.position());
         for (const auto& it : mFoldLevels)
         {
            if (it.first < line && it.second >= line && !it.mIsExpanded)
            {
               if (e->key() == Qt::Key_Left)
               {
                  cur.setPosition(GetSource()->GetLinePosition(it.first));
                  cur.movePosition(QTextCursor::EndOfLine);
               }
               else if (e->key() == Qt::Key_Up)
               {
                  for (int i = 0; i < it.second - it.first; i++)
                  {
                     cur.movePosition(QTextCursor::Up);
                  }
               }
               else if (e->key() == Qt::Key_Right)
               {
                  cur.setPosition(GetSource()->GetLinePosition(it.second + 1));
               }
               else if (e->key() == Qt::Key_Down)
               {
                  for (int i = 0; i < it.second - it.first; i++)
                  {
                     cur.movePosition(QTextCursor::Down);
                  }
               }
            }
         }
         setTextCursor(cur);
      }
      else if (e->key() == Qt::Key_Return)
      {
         QTextCursor cur    = textCursor();
         int         curPos = GetSource()->PositionToLineNumber(cur.position());
         for (auto&& it : mFoldLevels)
         {
            if (it.first == curPos && !it.mIsExpanded)
            {
               int newLineInsert = ut::cast_to_size_t(it.second);
               cur               = textCursor();
               cur.setPosition(GetSource()->GetLinePosition(newLineInsert), QTextCursor::MoveAnchor);
               cur.movePosition(QTextCursor::EndOfLine);
               setTextCursor(cur);
            }
         }


         // Disable special handling of shift-return
         if (e->modifiers() == Qt::ShiftModifier)
         {
            e->setModifiers(nullptr);
         }
         Editor::keyPressEvent(e);
         // if pressing return moved to the beginning of the line,
         // get the indent of the previous line and apply to the current line
         if (cur.block().position() == cur.position())
         {
            cur.movePosition(QTextCursor::Up);
            cur.movePosition(QTextCursor::StartOfLine);
            cur.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
            QString txt    = cur.selectedText();
            int     indent = 0;
            for (auto&& c : txt)
            {
               if (c == ' ')
               {
                  ++indent;
               }
               else
               {
                  break;
               }
            }
            if (indent > 0)
            {
               // insert indent and move cursor
               std::string indentStr(indent, ' ');
               cur = textCursor();
               cur.insertText(indentStr.c_str());
               setTextCursor(cur);
            }
         }
      }
      else if (e->key() == Qt::Key_Backtab && e->modifiers() == Qt::ShiftModifier) // un-indent
      {
         Unindent();
      }
      else if (e->key() == Qt::Key_Tab)
      {
         Indent();
      }
      else if (e->key() == Qt::Key_Space && e->modifiers() == (Qt::ControlModifier))
      {
         if (mAutocompletePopupPtr == nullptr)
         {
            BeginAutoComplete();
         }
         e->ignore();
      }
      else if (e->key() == Qt::Key_Period && e->modifiers() == 0)
      {
         Editor::keyPressEvent(e);
         if (mAutocompletePopupPtr == nullptr)
         {
            if (wkfEnv.GetPreferenceObject<EditorPrefObject>()->AutocompleteAfterPeriod())
            {
               WsfScriptLookup lookup(GetProject());
               size_t          offset, line, col;
               GetCurrentPosition(offset, line, col);
               UtTextDocumentLocation currentPos(GetSource(), offset);
               lookup.SetLocation(currentPos);
               if ((lookup.Update() != WsfScriptLookup::cNO_SCRIPT) && lookup.HasMethodSymbols())
               {
                  BeginAutoComplete();
               }
            }
         }
      }
      else if (e->key() == Qt::Key_F1)
      {
         OpenHelp();
      }
      //      else if ((e->key() == Qt::Key_3) && (e->modifiers() == Qt::ShiftModifier))
      else if (e->key() == Qt::Key_NumberSign)
      {
         ToggleSelectionComment();
      }
      else if (e->key() == Qt::Key_Escape)
      {
         if (mSyntaxTipsPtr)
         {
            mSyntaxTipsPtr->Hide();
         }
      }
      else
      {
         passOn = true;
      }
   }

   if (passOn)
   {
      Editor::keyPressEvent(e);
   }
}

void wizard::WsfEditor::contextMenuEvent(QContextMenuEvent* e)
{
   QMenu popupMenu;

   // Temporarily change the effect of some commands to reference the location of the right-click
   mContextMenuCharacterPosition = cursorForPosition(e->pos()).position();
   UtTextDocumentRange range(GetSource(), mContextMenuCharacterPosition, mContextMenuCharacterPosition);
   mClickedIncludePtr = nullptr;

   ParseResults* parseResultsPtr = GetProject()->WaitForParseResults();
   if (parseResultsPtr)
   {
      // Currently this only grabs the relevant data if you right click on the command name you want (if you right click
      // a subcommand, etc. it won't get the parent). If you pass in a WsfParseNode* as the argument to the plugin, you
      // could probably use the ParseResults FindBlock function and the WsfParseNode's GetParent function to get the parent data
      UtTextDocumentRange selectionRange;
      GetSelectionRange(selectionRange);
      std::vector<WsfParseNode*> nodes;
      // in this case will always grab 2 nodes, the command itself and the command plus all of its contents
      parseResultsPtr->GetRangeNodes(selectionRange, nodes);
      std::set<WsfParseNode*> nodeSet(nodes.begin(), nodes.end());
      std::string             selection = "";
      for (auto it : nodeSet)
      {
         std::string tmp = it->GetTextValue();
         if (tmp.size() > selection.size())
         {
            selection = tmp;
         }
      }
      // get the directory path
      std::string            filename = GetSource()->GetFileName();
      std::string            path     = GetSource()->GetSystemPath();
      std::string::size_type i        = path.find(filename);
      if (i != std::string::npos)
      {
         path.erase(i, filename.length());
      }
      emit wizSignals->ActionsForCommandContentsRequested(selection, path, &popupMenu, this);

      // proxy-related context events
      int           entryType;
      WsfPProxyPath proxyPath = parseResultsPtr->FindProxyPathFromLocation(range, entryType);
      if ((entryType == WsfPProxyIndex::cADDED || entryType == WsfPProxyIndex::cEDITED) && !proxyPath.Empty())
      {
         WsfPM_Root    root(parseResultsPtr->Proxy());
         WsfPProxyNode node(root + proxyPath);
      }
      else
      {
         ParseResults::TypeInformation typeAtPosition;
         if (parseResultsPtr->ProxyRoot() &&
             parseResultsPtr->TypeInformationFromPosition(range.GetPosition(), typeAtPosition))
         {
            WsfPM_Root    root(parseResultsPtr->Proxy());
            WsfPProxyNode node(root + typeAtPosition.mTypeKey);
         }
      }

      ParseResults::FileNotification note = parseResultsPtr->GetNotification(range.GetPosition());
      if (note.IsValid() && (note.mNotificationType & ParseResults::cINFORMATION_BIT))
      {
         QString what("View ");
         int     noteKind = note.GetNotificationKind();
         if (noteKind == ParseResults::cTYPE_KIND)
         {
            what += "Type";
         }
         else if (noteKind == ParseResults::cINCLUDE_KIND)
         {
            what += "Include";
         }
         else if (noteKind == ParseResults::cREFERENCE_KIND)
         {
            what += "Reference";
         }

         QAction* notificationAction = nullptr;
         if (note.mNotificationType & ParseResults::cERROR_BIT)
         {
            notificationAction = popupMenu.addAction(QIcon::fromTheme("exclamation"), what + " Error Info...");
         }
         else if (note.mNotificationType & ParseResults::cWARNING_BIT)
         {
            notificationAction = popupMenu.addAction(QIcon::fromTheme("question"), what + " Warning Info...");
         }
         else
         {
            notificationAction = popupMenu.addAction(QIcon::fromTheme("star"), what + " Info...");
         }

         if (notificationAction)
         {
            QObject::connect(notificationAction, &QAction::triggered, this, &WsfEditor::NotificationAction);
         }

         if ((noteKind == ParseResults::cTYPE_KIND) || (noteKind == ParseResults::cREFERENCE_KIND))
         {
            WsfPProxyNode node;

            if (noteKind == ParseResults::cTYPE_KIND)
            {
               ParseResults::TypeInformation typeInfo;
               UtTextDocumentLocation        pos(GetSource(), mContextMenuCharacterPosition);
               if (parseResultsPtr->TypeInformationFromPosition(pos, typeInfo))
               {
                  WsfPProxyValue proxyRoot = parseResultsPtr->ProxyRoot();
                  // WIZARD_TODO: typeInfo.mParent is set, but we aren't using it. This
                  // will cause us to be unable to handle when a user right-clicks
                  // e.g. the weapon name identifier in a weapon definition
                  // nested inside a platform definition.
                  WsfPProxyPath path = WsfPProxyPath::FromTypePath(proxyRoot, typeInfo.mTypeKey);
                  if (proxyRoot.Lookup(path))
                  {
                     node = WsfPProxyNode(parseResultsPtr->Proxy(), path);
                  }
               }
            }
            else if (noteKind == ParseResults::cREFERENCE_KIND)
            {
               int           entryType;
               WsfPProxyPath path = parseResultsPtr->FindProxyPathFromLocation(range, entryType);
               if ((entryType == WsfPProxyIndex::cASSIGNED) && !path.Empty())
               {
                  const WsfPProxyNode attrNode(parseResultsPtr->Proxy(), path);
                  const std::string&  attrName  = attrNode.GetAttrName();
                  WsfPProxyValue      attrValue = attrNode.GetValue();
                  assert(!attrName.empty());
                  assert(attrValue.IsValid());
                  if (!attrName.empty() && attrValue.IsValid())
                  {
                     WsfPM_ObjectMap signatures(WsfPM_Root(parseResultsPtr->Proxy()) + attrName);
                     node = signatures.Find(attrName);
                  }
               }
            }
            if (node.IsValid())
            {
               emit wizSignals->ActionsForNodeRequested(node, &popupMenu, this);
            }
         }
      }

      // Find any files being referenced here
      bool                       foundFileReference = false;
      std::vector<WsfParseNode*> fileReferences;
      parseResultsPtr->FindReferencedFiles(fileReferences);
      for (auto&& referencePtr : fileReferences)
      {
         if (referencePtr->mValue.mSource == GetSource())
         {
            if (referencePtr->mValue.Intersects(range))
            {
               WsfParseFileReferenceData* fileRefDataPtr = (WsfParseFileReferenceData*)referencePtr->GetAuxiliaryValue();
               UtPath                     pathToFile(fileRefDataPtr->mFilePath);
               std::string                fileName       = pathToFile.GetFileName();
               bool                       useSearchPaths = true;
               if (referencePtr->mRulePtr->Type() == WsfParseRule::cQUOTED_STRING)
               {
                  useSearchPaths = ((WsfParseQuotedStringRule*)referencePtr->mRulePtr)->mFileSearchPaths;
               }
               foundFileReference = true;

               if (useSearchPaths)
               {
                  // This is an input file
                  TextSource* txtFilePtr = GetProject()->GetSourceCache().FindSource(fileRefDataPtr->mFilePath);
                  if (txtFilePtr)
                  {
                     QAction* action = popupMenu.addAction(("Go To File: " + fileName).c_str());
                     connect(action, &QAction::triggered, this, &WsfEditor::GoToInclude);
                     mClickedIncludePtr = txtFilePtr;
                  }
               }
               else
               {
                  // this is an output file
                  UtPath filePath = GetProject()->WorkingDirectory() + fileRefDataPtr->mFilePath.Get();
                  if (filePath.IsFile())
                  {
                     OpenFileAction fn;
                     fn.mFileName    = filePath.GetSystemPath();
                     QAction* action = popupMenu.addAction(("Open: " + fileName).c_str());
                     connect(action, &QAction::triggered, fn);
                  }
               }
            }
         }
      }

      // Find any includes at this location
      WsfParseSourceInclude* sourceIncludePtr = nullptr;
      int                    includeEntry     = -1;
      TextSource*            includeSource    = nullptr;
      if (!foundFileReference &&
          parseResultsPtr->FindIncludeAtPoint(GetSource()->GetSource(), range, sourceIncludePtr, includeEntry, includeSource))
      {
         if (includeSource)
         {
            std::string fileName = includeSource->GetFilePath().GetFileName();
            QAction*    action   = popupMenu.addAction(("Go To Include: " + fileName).c_str());
            QObject::connect(action, &QAction::triggered, this, &WsfEditor::GoToInclude);
            mClickedIncludePtr = includeSource;
            foundFileReference = true;
         }
         else if (includeEntry != -1)
         {
            UtTextRange& includeRange  = sourceIncludePtr->mEntries[includeEntry].mLocation;
            mCreateFileFromIncludeName = UtTextDocumentRange(sourceIncludePtr->mSourcePtr, includeRange).Text();
            size_t spaceLoc            = mCreateFileFromIncludeName.find_first_of(" \t");
            if (spaceLoc != std::string::npos)
            {
               mCreateFileFromIncludeName = mCreateFileFromIncludeName.substr(spaceLoc + 1);
               UtStringUtil::TrimWhiteSpace(mCreateFileFromIncludeName);
               QAction* action = popupMenu.addAction(("Create File: " + mCreateFileFromIncludeName).c_str());
               QObject::connect(action, &QAction::triggered, this, &WsfEditor::CreateFileFromInclude);
            }
         }
      }

      if (GetProject()->GetExe())
      {
         std::shared_ptr<GrammarDocumentation> doc = GetProject()->GetExe()->GetGrammarDocumentation();
         if (doc)
         {
            UtTextDocument& source = *GetSource()->GetSource();
            QString         docURL = CommandDocumentation::FindDocumentationUrl(*doc,
                                                                        parseResultsPtr->FindLastParseNode(
                                                                           UtTextDocumentRange(&source, range)));
            if (!docURL.isEmpty())
            {
               QAction* action = popupMenu.addAction("Command Documentation...");
               QObject::connect(action, &QAction::triggered, this, &WsfEditor::CommandDocumentationAction);
            }
         }
      }

      // If the location is in a script, add action to open the assistant
      WsfParseNode* nodePtr = parseResultsPtr->FindLastParseNode(range);
      if (nodePtr && (nodePtr->GetFlags() & WsfParseNode::cSCRIPT_MASK))
      {
         QAction* notificationAction = popupMenu.addAction(QIcon::fromTheme("star"), "Script Info...");
         QObject::connect(notificationAction, &QAction::triggered, this, &WsfEditor::NotificationAction);
      }
   }

   emit wizSignals->EditorContextMenu(this, popupMenu);

   if (!popupMenu.isEmpty())
   {
      popupMenu.addSeparator();
   }
   ProjectWorkspace::Instance()->GetActionManager()->AppendEditActions(&popupMenu);

   popupMenu.exec(mapToGlobal(e->pos()));

   mContextMenuCharacterPosition = -1;
}

void wizard::WsfEditor::GoToInclude()
{
   if (mClickedIncludePtr)
   {
      GetProject()->GotoFile(mClickedIncludePtr, true);
   }
}

void wizard::WsfEditor::BeginAutoComplete()
{
   if (mSyntaxTipsPtr)
   {
      mSyntaxTipsPtr->UserModification();
   }

   if (mAutocompletePopupPtr == nullptr)
   {
      mAutocompletePopupPtr = new AutoCompletePopup(this, this);
   }

   UpdateAutoComplete();
   if (mAutocompletePopupPtr)
   {
      mAutocompletePopupPtr->show();
   }
}

void wizard::WsfEditor::UpdateAutoComplete()
{
   if (mAutocompletePopupPtr != nullptr)
   {
      QRect  r  = cursorRect();
      QPoint pt = viewport()->mapToParent(r.topLeft());
      mAutocompletePopupPtr->CursorMove(pt.x(), pt.y(), r.height());
   }
}

void wizard::WsfEditor::GoToDefinition(bool aAllRefs)
{
   UtTextDocumentRange definitionRange;

   UtTextDocumentRange range;
   GetSelectionRange(range);
   if (mContextMenuCharacterPosition >= 0)
   {
      auto contextPos = ut::cast_to_size_t(mContextMenuCharacterPosition);
      range.SetRange(contextPos, contextPos);
   }
   ParseResults* resultsPtr = nullptr;
   if (GetProject())
   {
      resultsPtr = GetProject()->WaitForParseResults();
   }

   if (resultsPtr)
   {
      bool                             processed = false;
      std::vector<WsfParseNode*>       definitions;
      std::vector<UtTextDocumentRange> definitionRanges;
      WsfParseNode*                    nodePtr = resultsPtr->FindLastParseNode(range);
      std::string                      word;
      if (nodePtr)
      {
         const int cTYPED_NODE_FLAGS = WsfParseNode::cTYPE_REFERENCE_NODE | WsfParseNode::cTYPE_NAME_NODE |
                                       WsfParseNode::cLAZY_TYPE_REFERENCE_NODE | WsfParseNode::cLAZY_TYPENAME_NODE;
         word = nodePtr->mValue.Text();
         if (nodePtr->GetFlags() & WsfParseNode::cSCRIPT_MASK)
         {
            size_t      line    = GetSource()->PositionToLineNumber(range.GetBegin());
            std::string lineStr = GetSource()->GetLineString(line);
            size_t      linePos = GetSource()->GetLinePosition(line);
            size_t      tokBeg, tokEnd;
            if (WsfScriptLookup::IdentifierToken(lineStr.c_str(),
                                                 lineStr.c_str() + lineStr.length(),
                                                 range.GetBegin() - linePos,
                                                 tokBeg,
                                                 tokEnd))
            {
               std::string     ident = lineStr.substr(tokBeg, tokEnd - tokBeg);
               WsfScriptLookup lookup(GetProject());
               lookup.SetLocation(range.GetPosition());
               if (WsfScriptLookup::cSUCCESS == lookup.Update())
               {
                  UsSymbol* symPtr = lookup.Search(ident);
                  if (symPtr)
                  {
                     definitionRange = lookup.GetDefinitionLocation(symPtr);
                  }
                  else
                  {
                     // Not a symbol; look for type (script_stuct)
                     UsType* typePtr = lookup.FindType(ident);
                     if (typePtr)
                     {
                        resultsPtr->FindDefinitions(ident, "script_struct", definitions);
                     }
                  }
               }
            }
         }
         else
         {
            if (nodePtr->GetFlags() & WsfParseNode::cNAMED_NODE)
            {
               resultsPtr->FindNamedNodes(nodePtr->mType, word, definitions);
            }
            else
            {
               WsfParseTypePath typeKey;
               if (nodePtr->GetFlags() & (cTYPED_NODE_FLAGS))
               {
                  bool isNested;
                  wizard::ParseUtil::FindReferenceType(nodePtr, typeKey, isNested);
               }
               if (!typeKey.empty())
               {
                  if (aAllRefs)
                  {
                     resultsPtr->FindReferences(word, typeKey[0], definitions);
                  }
                  else
                  {
                     resultsPtr->FindDefinitions(word, typeKey[0], definitions);
                  }

                  if (definitions.empty() && resultsPtr->ProxyRoot().IsValid()) // check the proxy
                  {
                     WsfPProxyNode pos = WsfPProxyNode::FromProxy(resultsPtr->Proxy());
                     for (auto&& it : typeKey)
                     {
                        pos += it.Get();
                     }
                     WsfPProxyValue definitionValue = pos.GetValue();
                     if (definitionValue.IsValid())
                     {
                        definitionRange = resultsPtr->FindDefinitionLocation(pos.GetPath());
                     }
                     else
                     {
                        if (typeKey.size() >= 2)
                        {
                           int           entryType;
                           WsfPProxyPath path = resultsPtr->FindProxyPathFromLocation(range, entryType);
                           if ((entryType == WsfPProxyIndex::cADDED || entryType == WsfPProxyIndex::cEDITED) &&
                               !path.Empty())
                           {
                              WsfPProxyNode n(resultsPtr->Proxy(), path);
                              WsfPProxyNode ref = ParseResults::FindProxyReference(n, typeKey);
                              if (ref)
                              {
                                 UtTextDocumentRange defLoc = resultsPtr->FindDefinitionLocation(ref.GetPath());

                                 if (!defLoc.mSource)
                                 {
                                    resultsPtr->FindModificationLocationsExtended(ref.GetPath(), definitionRanges);
                                 }
                                 else
                                 {
                                    definitionRanges.push_back(defLoc);
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
      else
      {
         WsfParseSourceInclude* sourceIncludePtr = nullptr;
         TextSource*            includeSource    = nullptr;
         int                    includeEntry     = -1;
         if (resultsPtr->FindIncludeAtPoint(GetSource()->GetSource(), range, sourceIncludePtr, includeEntry, includeSource))
         {
            if (includeSource)
            {
               GetProject()->GotoFile(includeSource, true);
               processed = true;
            }
         }
         else
         {
            // we are in an unparsed section
            range.Text(word);
            if (!word.empty())
            {
               resultsPtr->FindDefinitions(word, "", definitions);
            }
            if (definitions.empty())
            {
               QTextCursor cur(document());
               cur.setPosition(ut::cast_to_int(range.GetBegin()));
               cur.movePosition(QTextCursor::StartOfWord);
               cur.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
               range.SetRange(cur.selectionStart(), cur.selectionEnd() - 1);
               range.Text(word);
               if (!word.empty())
               {
                  resultsPtr->FindDefinitions(word, "", definitions);
               }
            }
         }
      }
      if (!processed)
      {
         std::vector<UtTextDocumentRange> locations(definitionRanges);
         if (!definitions.empty())
         {
            for (auto&& it : definitions)
            {
               locations.push_back(it->mValue);
            }
         }

         if (locations.size() > 1)
         {
            FindReferencesResultDialog* dlg = new FindReferencesResultDialog(this);
            QString                     label("Find References: \"");
            label += word.c_str();
            label += '"';
            dlg->Show(GetProject(), locations, label);
         }
         else if (locations.size() == 1)
         {
            GetProject()->GotoRange(locations[0]);
            RecordCurrentLocation();
         }
         else if (definitionRange.Valid())
         {
            GetProject()->GotoRange(definitionRange);
            RecordCurrentLocation();
         }
         else
         {
            QMessageBox::information(this, tr("Find References"), tr("No reference found."));
         }
      }
   }
}

void wizard::WsfEditor::marginMouseMoveEvent(QMouseEvent* e)
{
   if (FoldingEnabled())
   {
      int  marginX            = Margin()->width() - e->x();
      bool hoveringFoldMargin = (marginX > 0 && marginX < 10);
      if (hoveringFoldMargin != mHoveringFolds)
      {
         mHoveringFolds = hoveringFoldMargin;
         RedrawMargin();
      }
      else if (hoveringFoldMargin)
      {
         RedrawMargin();
      }
   }
}

void wizard::WsfEditor::marginLeaveEvent(QEvent* e)
{
   if (FoldingEnabled() && mHoveringFolds)
   {
      mHoveringFolds = false;
      RedrawMargin();
   }
}

void wizard::WsfEditor::marginMousePressEvent(QMouseEvent* e)
{
   if (FoldingEnabled())
   {
      int  marginX      = Margin()->width() - e->x();
      bool inFoldMargin = (marginX > 0 && marginX < 10);
      if (inFoldMargin)
      {
         QTextBlock foldingBlock;
         // Scan blocks looking for y coordinate
         QTextBlock blk = firstVisibleBlock();
         while (blk.isValid())
         {
            if (blk.isVisible())
            {
               QRectF blkRect = blockBoundingGeometry(blk);
               blkRect.translate(contentOffset());
               if (e->y() >= blkRect.top() && e->y() <= blkRect.bottom())
               {
                  foldingBlock = blk;
               }
            }
            blk = blk.next();
         }
         if (foldingBlock.isValid())
         {
            // Find fold that matches the line #
            int line = foldingBlock.blockNumber();
            for (auto&& it : mFoldLevels)
            {
               if (it.first == line)
               {
                  // Apply folding
                  foldingBlock = foldingBlock.next();
                  if (foldingBlock.isValid())
                  {
                     int  dirtyStart  = foldingBlock.position();
                     bool makeVisible = !foldingBlock.isVisible();
                     int  lastLine    = it.second;

                     // Moves cursor out of folded regions when folded
                     QTextCursor cur     = textCursor();
                     int         curLine = GetSource()->PositionToLineNumber(cur.position());
                     if (curLine >= line && curLine <= lastLine)
                     {
                        cur.setPosition(GetSource()->GetLinePosition(line), QTextCursor::MoveAnchor);
                        cur.movePosition(QTextCursor::EndOfLine);
                        setTextCursor(cur);
                     }

                     while (foldingBlock.isValid() && foldingBlock.blockNumber() <= lastLine)
                     {
                        foldingBlock.setVisible(makeVisible);
                        foldingBlock = foldingBlock.next();
                     }
                     int docLength = document()->lastBlock().position() + document()->lastBlock().length();
                     document()->markContentsDirty(dirtyStart, docLength - dirtyStart);
                     RedrawMargin();
                     // QTimer::singleShot(1, this, SLOT(update()));
                     //  Bug: Qt isn't updating the text in some cases.  Needed to do a deferred update on the viewport
                     QTimer::singleShot(1, viewport(), SLOT(update()));
                     break;
                  }
               }
            }
         }
      }
   }

   // symbol margin
   if (e->button() == Qt::LeftButton && (e->x() - LineNumberWidth() < cSYMBOL_MARGIN_WIDTH))
   {
      QTextBlock blk = firstVisibleBlock();
      while (blk.isValid())
      {
         QRectF r = blockBoundingGeometry(blk).translated(contentOffset());
         if (r.bottom() > e->y())
         {
            wizEnv.MarginClickAction(Qt::LeftButton, GetSource(), ut::cast_to_size_t(blk.blockNumber()));
            break;
         }
         blk = blk.next();
      }
   }
}

void wizard::WsfEditor::marginContextMenuEvent(QContextMenuEvent* e)
{
   // symbol margin
   if (e->x() - LineNumberWidth() < cSYMBOL_MARGIN_WIDTH)
   {
      QTextBlock blk = firstVisibleBlock();
      while (blk.isValid())
      {
         QRectF r = blockBoundingGeometry(blk).translated(contentOffset());
         if (r.bottom() > e->y())
         {
            wizEnv.MarginClickAction(Qt::RightButton, GetSource(), ut::cast_to_size_t(blk.blockNumber()));
            break;
         }
         blk = blk.next();
      }
   }
}

bool wizard::WsfEditor::HoverTimeout()
{
   QToolTip::hideText();

   bool   showingTip = false;
   size_t charPos    = GetPosition(mapFromGlobal(QCursor::pos()));
   if (GetProject()->GetParseResults())
   {
      UtTextDocumentRange pos(GetSource(), charPos, 1);
      WsfScriptLookup     lookup(GetProject());
      lookup.SetLocation(pos.GetPosition(), WsfScriptLookup::cSTOP_AT_END_OF_FUNCTION);
      lookup.SetStoreErrors(true);
      if (lookup.Update() == WsfScriptLookup::cSUCCESS)
      {
         WsfScriptInfo*                             infoPtr      = lookup.GetScriptInfo();
         size_t                                     offset       = infoPtr->mLocation.mPosition;
         int                                        cursorPosRel = ut::cast_to_int(charPos) - ut::cast_to_int(offset);
         std::vector<WsfScriptParser::ErrorRecord>* errorsPtr    = lookup.GetErrors();
         if (errorsPtr)
         {
            for (auto&& e : *errorsPtr)
            {
               if (cursorPosRel >= e.mPosition && cursorPosRel <= e.mPosition + e.mLength)
               {
                  QPoint   rectCorner     = mapFromGlobal(QCursor::pos());
                  QWidget* childWidgetPtr = childAt(rectCorner);
                  rectCorner.rx() -= 20;
                  rectCorner.ry() -= 10;
                  QRect region(rectCorner, QSize(40, 20));
                  // NOTE: We must pass the child widget in here for the region to work
                  QToolTip::showText(QCursor::pos(), QString::fromStdString(e.mText), childWidgetPtr, region);
                  showingTip = true;
               }
            }
         }
      }
   }
   if (mSyntaxTipsPtr)
   {
      mSyntaxTipsPtr->HoverShow(charPos);
      showingTip = true;
   }
   return showingTip;
}

void wizard::WsfEditor::CommandDocumentationAction()
{
   UtTextDocumentRange range;
   GetSelectionRange(range);
   if (mContextMenuCharacterPosition >= 0)
   {
      auto contextPos = ut::cast_to_size_t(mContextMenuCharacterPosition);
      range.SetRange(contextPos, contextPos);
   }
   wizSignals->OpenCommandDocumentation(GetSource(), range);
}

void wizard::WsfEditor::CreateFileFromInclude()
{
   QMenu         popup;
   ParseResults* resultsPtr = GetProject()->WaitForParseResults();
   if (!resultsPtr)
   {
      return;
   }

   // Get the current working directory
   UtPath cwd = GetProject()->WorkingDirectory();

   // Get any include paths
   const std::vector<UtStringRef>& includePaths = resultsPtr->mIncludePaths;

   // Create the context menu
   std::set<QString> optionSet;
   if (includePaths.empty())
   {
      UtPath path = cwd + mCreateFileFromIncludeName;
      UtPath fileDir(path);
      fileDir.Up();
      QString newDirMessage;
      if (fileDir.Stat() != UtPath::cDIRECTORY)
      {
         newDirMessage = " (new directory)";
      }
      QString fileName = QString::fromStdString(path.GetSystemPath());
      // Don't add the same path twice
      if (optionSet.find(fileName) == optionSet.end())
      {
         popup.addAction("Create file: " + fileName + newDirMessage)->setData(QVariant::fromValue(fileName));
         optionSet.insert(fileName);
      }
   }
   else
   {
      // Loop through the file path options
      for (auto&& it : includePaths)
      {
         UtPath path = cwd + it;
         path += mCreateFileFromIncludeName;
         UtPath fileDir(path);
         fileDir.Up();
         QString newDirMessage;
         if (fileDir.Stat() != UtPath::cDIRECTORY)
         {
            newDirMessage = " (new directory)";
         }
         QString fileName = QString::fromStdString(path.GetSystemPath());
         // Don't add the same path twice
         if (optionSet.find(fileName) == optionSet.end())
         {
            popup.addAction("Create file: " + fileName + newDirMessage)->setData(QVariant::fromValue(fileName));
            optionSet.insert(fileName);
         }
      }
   }

   // Pop up the menu
   popup.setTitle(tr("Choose Location"));
   QAction* chosenPtr = popup.exec(QCursor::pos());
   if (chosenPtr != nullptr)
   {
      std::string pathStr(chosenPtr->data().toString().toStdString());
      UtPath      path(pathStr);
      path.Up();
      bool ok = true;
      if (path.Stat() != UtPath::cDIRECTORY)
      {
         ok = path.Mkdir();
      }
      if (ok)
      {
         wizard::Util::CreateNewFile(pathStr);
         GetProject()->TriggerReparse();
      }
   }
}

void wizard::WsfEditor::LoadSettings()
{
   Editor::LoadSettings();

   if (!wkfEnv.GetPreferenceObject<EditorPrefObject>()->ShowSyntaxTips())
   {
      if (mSyntaxTipsPtr)
      {
         mSyntaxTipsPtr->deleteLater();
         // delete mSyntaxTipsPtr;
         mSyntaxTipsPtr = nullptr;
      }
   }
   else
   {
      if (!mSyntaxTipsPtr && hasFocus())
      {
         mSyntaxTipsPtr = new SyntaxTips(this);
      }
   }
}

void wizard::WsfEditor::FocusCheck()
{
   // I haven't found a good way to detect when focus has left the edit window or the auto-complete window
   // For now, we do this sequence of checks on a timer

   QObject* focus = qApp->focusObject();
   // If the QObject with focus, is this, a child of this, or AutoCompletePopup return and do nothing.
   while (focus != nullptr)
   {
      if ((focus == this) || (focus == mAutocompletePopupPtr))
      {
         // Return and do nothing
         return;
      }
      focus = focus->parent();
   }

   // If the QObject with focus is not part of the Editor, we need to close the Syntax and AutoComplete pop-ups.
   mFocusCheckTimer.stop();
   Manager()->TabFocusOut(this);
   if (mSyntaxTipsPtr)
   {
      mSyntaxTipsPtr->deleteLater();
      mSyntaxTipsPtr = nullptr;
   }
   HideAutocomplete();
}

bool wizard::WsfEditor::CommentBlock()
{
   UtTextDocumentRange range;
   GetSelectionRange(range);
   QTextCursor cur(document());
   cur.setPosition(ut::cast_to_int(range.GetBegin()));
   int firstLine = cur.block().blockNumber();
   cur.setPosition(ut::cast_to_int(range.GetEnd()));
   int finalLine = cur.block().blockNumber();
   if (firstLine <= finalLine)
   {
      cur.beginEditBlock();
      cur.setPosition(ut::cast_to_int(range.GetBegin()));
      QTextBlock blk      = cur.block();
      int        startPos = blk.position();
      size_t     end{range.GetEnd()};
      while (blk.isValid() && ut::cast_to_size_t(blk.position()) <= end)
      {
         cur.setPosition(blk.position());
         cur.insertText("#");
         ++end;
         blk = blk.next();
      }
      range.SetEnd(end);
      cur.setPosition(startPos);
      cur.endEditBlock();
      setTextCursor(cur);
      return true;
   }
   return false;
}

bool wizard::WsfEditor::UncommentBlock()
{
   int         begin = textCursor().selectionStart();
   size_t      end   = textCursor().selectionEnd();
   QTextCursor cur(textCursor());
   cur.setPosition(begin);
   QTextBlock blk          = cur.block();
   bool       canUncomment = true;
   while (blk.isValid() && (ut::cast_to_size_t(blk.position()) < end))
   {
      QString txt     = blk.text();
      QString txtTrim = txt.trimmed();
      if (!txtTrim.isEmpty())
      {
         if (txtTrim[0] != '#')
         {
            canUncomment = false;
            break;
         }
         // else
         //{
         // cur.setPosition(blk.position() + txt.indexOf('#'));
         // cur.deleteChar();
         // }
      }
      blk = blk.next();
   }

   if (canUncomment)
   {
      cur.beginEditBlock();
      cur.setPosition(textCursor().selectionStart());
      blk = cur.block();
      while (blk.isValid() && (ut::cast_to_size_t(blk.position()) <= end))
      {
         QString txt = blk.text();
         int     idx = txt.indexOf('#');
         if (idx != -1)
         {
            cur.setPosition(blk.position() + idx);
            cur.deleteChar();
            --end; // char deleted
         }
         blk = blk.next();
      }
      cur.endEditBlock();
   }
   return canUncomment;
}

void wizard::WsfEditor::ToggleSelectionComment()
{
   bool                passOn = true;
   UtTextDocumentRange range;
   bool                isSelected = GetSelectionRange(range);
   if (isSelected)
   {
      if (!UncommentBlock())
      {
         passOn = !CommentBlock();
      }
      else
      {
         passOn = false;
      }
   }
   if (passOn)
   {
      textCursor().insertText("#");
   }
}

void wizard::WsfEditor::OnProxyUpdate(ProxyMerge* aMerge)
{
   ParseResults* resultsPtr = GetProject()->GetParseResults();
   if (!resultsPtr || !resultsPtr->ProxyIndex())
   {
      return;
   }

   mMarkers.clear();
   WsfPProxyIndex*      indexPtr = resultsPtr->ProxyIndex();
   WsfPProxyStructValue rootInst(resultsPtr->ProxyRoot());
   WsfPProxyStructType* platformTypePtr = (WsfPProxyStructType*)resultsPtr->ProxyRegistry()->GetType("Platform");

   WsfPProxyPath platformListPath     = WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "platform");
   WsfPProxyPath platformTypeListPath = WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "platformType");

   // Locate all proxy changes made in this file:
   UiResources& resources = UiResources::GetInstance();

   std::map<size_t, QImage> partMapIndexToImage;

   if (platformTypePtr && platformTypePtr->mInitialized)
   {
      partMapIndexToImage[platformTypePtr->GetMemberIndex("sensors")] = resources.GetImageFromTheme(QString("radar"));
      partMapIndexToImage[platformTypePtr->GetMemberIndex("processors")] =
         resources.GetImageFromTheme(QString("gears"));
      partMapIndexToImage[platformTypePtr->GetMemberIndex("weapons")] = resources.GetImageFromTheme(QString("bomb"));
      partMapIndexToImage[platformTypePtr->GetMemberIndex("comms")]   = resources.GetImageFromTheme(QString("chat"));
   }

   WsfPProxyIndexNode* platformMapNode = indexPtr->Find(WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "platform"));
   WsfPProxyIndexNode* platformTypeMapNode =
      indexPtr->Find(WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "platformType"));
   WsfPProxyIndexNode* commTypeMapNode = indexPtr->Find(WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "commType"));
   WsfPProxyIndexNode* processorMapNode =
      indexPtr->Find(WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "processorType"));
   WsfPProxyIndexNode* sensorMapNode = indexPtr->Find(WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "sensorType"));
   WsfPProxyIndexNode* weaponMapNode = indexPtr->Find(WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "weaponType"));
   WsfPProxyIndexNode* moverMapNode  = indexPtr->Find(WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "moverType"));
   WsfPProxyIndexNode* fuelMapNode   = indexPtr->Find(WsfPProxyPath::FromString(resultsPtr->ProxyRoot(), "fuelType"));
   WsfPProxyIndex::ReverseIndexEntry fileStart;
   fileStart.mLocation.mSource   = GetSource();
   fileStart.mLocation.mPosition = 0;

   const std::vector<WsfPProxyIndex::ReverseIndexEntry>&          reverseIndex = indexPtr->GetReverseIndex();
   std::vector<WsfPProxyIndex::ReverseIndexEntry>::const_iterator iter =
      std::lower_bound(reverseIndex.begin(), reverseIndex.end(), fileStart);
   while (iter != reverseIndex.end() && iter->mLocation.mSource == GetSource())
   {
      WsfPProxyIndexNode* idxPtr = iter->mNodePtr;
      WsfPProxyPath       nodePath;
      idxPtr->GetPath(nodePath);

      QImage marker;
      if (idxPtr->mParentPtr == platformMapNode)
      {
         marker = resources.GetImageFromTheme(QString("f-18"));
      }
      if (idxPtr->mParentPtr == platformTypeMapNode)
      {
         marker = resources.GetImageFromTheme(QString("f-18"));
      }
      if (idxPtr->mParentPtr == commTypeMapNode)
      {
         marker = resources.GetImageFromTheme(QString("chat"));
      }
      if (idxPtr->mParentPtr == processorMapNode)
      {
         marker = resources.GetImageFromTheme(QString("gears"));
      }
      if (idxPtr->mParentPtr == sensorMapNode)
      {
         marker = resources.GetImageFromTheme(QString("radar"));
      }
      if (idxPtr->mParentPtr == weaponMapNode)
      {
         marker = resources.GetImageFromTheme(QString("bomb"));
      }
      if (idxPtr->mParentPtr == moverMapNode)
      {
         marker = resources.GetImageFromTheme(QString("horse"));
      }
      if (idxPtr->mParentPtr == fuelMapNode)
      {
         marker = resources.GetImageFromTheme(QString("gas"));
      }
      if (marker.isNull())
      {
         if (nodePath.size() == 4 && (platformListPath.IsPrefixOf(nodePath) || platformTypeListPath.IsPrefixOf(nodePath)))
         {
            auto it = partMapIndexToImage.find(nodePath[2].GetIndex());
            if (it != partMapIndexToImage.end())
            {
               marker = it->second;
            }
         }
      }

      if (!marker.isNull())
      {
         // Attempt to override the part icon with a specific part icon
         WsfPProxyPath path;
         idxPtr->GetPath(path);
         if (!path.Empty())
         {
            WsfPProxyValue val = rootInst.Lookup(path);
            if (val.IsValid())
            {
               QString partStr = ComponentPropertyManager::Instance()->FindPartIcon(val);
               if (!partStr.isEmpty())
               {
                  marker = resources.GetImageFromTheme(partStr);
               }
            }
         }
         QTextBlock blk = document()->findBlock(ut::cast_to_int(iter->mLocation.mPosition));
         if (blk.isValid())
         {
            AddMarker(blk.blockNumber(), marker);
         }
      }
      ++iter;
   }
   RedrawMargin();
}

void wizard::WsfEditor::NotificationAction()
{
   UtTextDocumentLocation pos(GetSource(), mContextMenuCharacterPosition);
   ParseResults*          parseResults = GetProject()->GetParseResults();
   if (parseResults)
   {
      EditAssist* editAssistantPtr = EditAssist::MakeAssistant(this, pos);
      if (editAssistantPtr)
      {
         editAssistantPtr->show();
      }
   }
}

void wizard::WsfEditor::Escape()
{
   bool actionTaken = false;
   if (mAutocompletePopupPtr)
   {
      mAutocompletePopupPtr->deleteLater();
      mAutocompletePopupPtr = nullptr;
      actionTaken           = true;
   }
   if (mSyntaxTipsPtr && mSyntaxTipsPtr->isVisible())
   {
      mSyntaxTipsPtr->Hide();
      actionTaken = true;
   }
   if (!actionTaken)
   {
      // Remove selection
      QTextCursor cur = textCursor();
      if (cur.selectionEnd() > cur.selectionStart())
      {
         cur.setPosition(cur.position());
         setTextCursor(cur);
      }
   }
}

bool wizard::WsfEditor::isValidSymbolChar(char c) const
{
   return isalnum(c) || c == '_';
}

std::string wizard::WsfEditor::GetSymbolToken(const char* buf, int len, int pos) const
{
   int start = pos;
   int stop  = pos;
   if (!isValidSymbolChar(buf[pos]))
   {
      return "";
   }
   while (start - 1 >= 0 && isValidSymbolChar(buf[start - 1]))
   {
      --start;
   }
   while (stop + 1 < len && isValidSymbolChar(buf[stop + 1]))
   {
      ++stop;
   }
   return std::string(buf + start, buf + stop + 1);
}

void wizard::WsfEditor::FormatSelection()
{
   GetProject()->WaitForParsing();
   UtTextDocumentRange range;
   GetSelectionRange(range);
   ParseResults*              results = GetProject()->GetParseResults();
   std::vector<WsfParseNode*> nodes;
   results->GetRangeNodes(range, nodes);
   std::set<WsfParseNode*> nodeSet(nodes.begin(), nodes.end());

   WsfParseFormat formatter;
   formatter.Format(nodeSet);
   std::set<UtTextDocument*> changedFiles = formatter.GetModifiedDocuments();

   for (auto&& it : changedFiles)
   {
      TextSource* src = (TextSource*)(it);
      src->ReloadViews();
   }

   wizard::ProjectWorkspace::Instance()->ReparseAll();
}

int wizard::WsfEditor::IsBrace(char c) const
{
   switch (c)
   {
   case '(':
   case '[':
   case '{':
      return 1;
   case ']':
   case '}':
   case ')':
      return -1;
   default:
      return 0;
   }
}

char wizard::WsfEditor::OtherBrace(char c) const
{
   switch (c)
   {
   case '(':
      return ')';
   case '[':
      return ']';
   case '{':
      return '}';
   case ')':
      return '(';
   case ']':
      return '[';
   case '}':
      return '{';
   default:
      return c;
   }
}

int wizard::WsfEditor::findMatch(int startIndex, int aDirection, const QString& aText, char aBeginChar, char aEndChar) const
{
   int nesting      = 1;
   int quoteNesting = 0;
   int i            = startIndex;
   while (i >= 0 && i < aText.size())
   {
      const QChar& c = aText[i];
      if (quoteNesting)
      {
         if (c == '"')
         {
            quoteNesting = 0;
         }
      }
      else
      {
         if (c == aBeginChar)
         {
            ++nesting;
         }
         else if (c == aEndChar)
         {
            --nesting;
         }
         else if (c == '"')
         {
            quoteNesting = 1;
         }
         if (nesting == 0)
         {
            return i;
         }
      }
      i += aDirection;
   }
   return -1;
}

void wizard::WsfEditor::UpdateBraceMatch()
{
   int  offset      = textCursor().position();
   char character   = document()->characterAt(offset).toLatin1();
   char endChar     = OtherBrace(character);
   int  braceBegins = IsBrace(character);
   if (!braceBegins)
   {
      --offset;
      character   = document()->characterAt(offset).toLatin1();
      braceBegins = IsBrace(character);
      endChar     = OtherBrace(character);
   }

   if (!braceBegins)
   {
      if (!mBraceHighlightSelections.empty())
      {
         mBraceHighlightSelections.clear();
      }
      return;
   }
   // const int cLOOKAHEAD_LENGTH = 150;
   const int cLOOKAHEAD_LINES = 50;
   int       direction        = braceBegins;
   int       idx              = -1;
   if (direction < 0)
   {
      QTextCursor cur = textCursor();
      // cur.setPosition(std::max(0, offset - cLOOKAHEAD_LENGTH));
      cur.setPosition(offset);
      cur.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, cLOOKAHEAD_LINES);
      cur.movePosition(QTextCursor::StartOfLine);
      cur.setPosition(offset, QTextCursor::KeepAnchor);
      const QString& text       = cur.selectedText();
      int            matchIndex = findMatch(text.size() - 1, direction, text, character, endChar);
      if (matchIndex >= 0)
      {
         idx = offset - (text.size() - matchIndex); // -1;
      }
   }
   else
   {
      QTextCursor cur = textCursor();
      cur.setPosition(offset + 1);
      cur.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, cLOOKAHEAD_LINES);
      // cur.setPosition(offset + cLOOKAHEAD_LENGTH, QTextCursor::KeepAnchor);
      const QString& text       = cur.selectedText();
      int            matchIndex = findMatch(0, direction, text, character, endChar);
      if (matchIndex >= 0)
      {
         idx = offset + matchIndex + 1;
      }
   }

   mBraceHighlightSelections.clear();
   if (idx != -1)
   {
      QTextEdit::ExtraSelection sel;
      sel.cursor = textCursor();
      sel.cursor.setPosition(offset);
      sel.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
      sel.format.setBackground(wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("editor_highlight"));
      mBraceHighlightSelections += sel;
      sel.cursor.setPosition(idx);
      sel.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
      mBraceHighlightSelections += sel;
   }
   else
   {
      QTextEdit::ExtraSelection sel;
      sel.cursor = textCursor();
      sel.cursor.setPosition(offset);
      sel.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
      sel.format.setBackground(Qt::red);
      mBraceHighlightSelections += sel;
   }
}

void wizard::WsfEditor::UpdateBreakpointMargin()
{
   // Set this flag to ignore document modifications relating to breakpoint lines
   mUpdatingBreakpoints = true;
   RedrawMargin();
   mUpdatingBreakpoints = false;
}

void wizard::WsfEditor::CurrentDebugLineChanged()
{
   DeferUpdateTimer().start();
}

void wizard::WsfEditor::ReloadComplete()
{
   UpdateBreakpointMargin();
}

void wizard::WsfEditor::UpdateExtraSelections()
{
   QList<QTextEdit::ExtraSelection> extraSelections;

   // Add script backgrounds
   extraSelections += mScriptBackgroundSelections;

   // Add current line
   AddCurrentLineSelection(extraSelections);

   // Brace matching
   UpdateBraceMatch();
   extraSelections += mBraceHighlightSelections;

   QList<size_t> lines;
   wizEnv.GetHighlightLines(GetSource()->GetSystemPath().c_str(), lines);
   for (auto&& it : lines)
   {
      QTextEdit::ExtraSelection activeLine;
      activeLine.cursor = QTextCursor(document()->findBlockByLineNumber(ut::cast_to_int(it)));
      activeLine.cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
      activeLine.format.setProperty(QTextFormat::FullWidthSelection, true);
      activeLine.format.setBackground(QIcon::themeName() == "Dark" ? QColor(0xFF, 0xDF, 0xDF, 0x2F) :
                                                                     QColor(0, 0, 0, 0x2F));
      extraSelections.push_back(activeLine);
   }

   setExtraSelections(extraSelections);
}

int wizard::WsfEditor::MarginWidth()
{
   return Editor::MarginWidth() + cSYMBOL_MARGIN_WIDTH + (FoldingEnabled() ? cFOLD_MARGIN_WIDTH : 0);
}

void wizard::WsfEditor::PaintMargin(QPaintEvent* aEvent)
{
   // Line numbers handled by the base class
   Editor::PaintMargin(aEvent);

   // Text folding
   // int lineHeight = fontMetrics().height();
   QPainter   painter(Margin());
   QTextBlock topBlock = firstVisibleBlock();

   int                                   foldLeft    = LineNumberWidth() + cSYMBOL_MARGIN_WIDTH;
   int                                   foldCenterX = foldLeft + cFOLD_MARGIN_WIDTH / 2;
   std::pair<unsigned int, unsigned int> imageValues = wizard::UiResources::GetInstance().ResizeImageWithFont();
   int                                   symbolLeft  = (LineNumberWidth() + foldLeft - imageValues.first) / 2;

   int hoverFoldY = -100;
   if (mHoveringFolds)
   {
      QPoint cursorPos = this->mapFromGlobal(QCursor::pos());
      hoverFoldY       = cursorPos.y();
      // hoverFoldLine = (cursorPos.y() - top) / lineHeight;
   }

   if (FoldingEnabled())
   {
      painter.setPen(wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("margin_fold_pen"));
      painter.drawLine(foldCenterX, aEvent->rect().top(), foldCenterX, aEvent->rect().bottom());
   }

   int foldEntry = 0;
   // Iterate visible blocks:
   QTextBlock blk    = topBlock;
   int        top    = (int)blockBoundingGeometry(blk).translated(contentOffset()).top();
   int        bottom = top + (int)blockBoundingRect(blk).height();
   while (blk.isValid() && top < aEvent->rect().bottom())
   {
      if (blk.isVisible() && bottom >= aEvent->rect().top())
      {
         // Icon in the margin
         if (mMarkers.find(blk.blockNumber()) != mMarkers.end())
         {
            QImage& img = mMarkers[blk.blockNumber()];
            QPoint  point(symbolLeft, (top + bottom - img.height()) / 2);
            painter.drawImage(point.x(),
                              point.y() + imageValues.second,
                              img.scaled(imageValues.first, imageValues.first, Qt::KeepAspectRatio));
         }

         // Folding
         if (FoldingEnabled())
         {
            while ((foldEntry + 1) < (int)mFoldLevels.size() && mFoldLevels[foldEntry + 1].first <= blk.blockNumber())
            {
               ++foldEntry;
            }
            if (foldEntry < (int)mFoldLevels.size() && mFoldLevels[foldEntry].first == blk.blockNumber())
            {
               // found fold
               bool       isExpanded = true;
               QTextBlock nextBlk    = blk.next();
               if (nextBlk.isValid() && !nextBlk.isVisible())
               {
                  isExpanded = false;
               }
               mFoldLevels[foldEntry].mIsExpanded = isExpanded;
               QPoint pt(foldLeft, top + ((bottom - top) - 9) / 2);

               if (hoverFoldY >= top && hoverFoldY <= bottom)
               {
                  QTextBlock last     = blk;
                  int        lastLine = mFoldLevels[foldEntry].second;
                  while (last.isValid() && last.blockNumber() < lastLine &&
                         blockBoundingGeometry(last).translated(contentOffset()).top() < aEvent->rect().bottom())
                  {
                     last = last.next();
                  }

                  // Find the bottom of the code fold; may be off the page
                  int bottomY = aEvent->rect().bottom();
                  if (last.isValid())
                  {
                     bottomY = blockBoundingGeometry(last).translated(contentOffset()).bottom();
                  }

                  // Set the pen; this will color the edge of the rect below
                  painter.setPen(wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("margin_fold_pen"));

                  // Highlight the lines to fold
                  QRect selectedFold(foldLeft, top + ((bottom - top) - 9) / 2, cSYMBOL_MARGIN_WIDTH / 2, bottomY - top);
                  painter.drawRect(selectedFold);
                  painter.fillRect(selectedFold,
                                   wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("margin_fill"));

                  // Fill the fold box
                  painter.fillRect(QRect(pt, QSize(9, 9)),
                                   wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("margin_fold_fill"));
               }
               else
               {
                  painter.fillRect(QRect(pt, QSize(9, 9)),
                                   wkfEnv.GetPreferenceObject<EditorPrefObject>()->EditorColor("margin_fill"));
               }

               QImage foldImage;
               if (isExpanded)
               {
                  foldImage = UiResources::GetInstance().GetImageFromTheme(QString("fold_collapse"), false);
               }
               else
               {
                  foldImage = UiResources::GetInstance().GetImageFromTheme(QString("fold_expand"), false);
               }
               painter.drawImage(pt, foldImage);
            }
         }
      }

      blk    = blk.next();
      top    = bottom;
      bottom = top + (int)blockBoundingRect(blk).height();
   }

   QMap<size_t, QPoint> visibleLineMap;

   blk    = topBlock;
   top    = (int)blockBoundingGeometry(blk).translated(contentOffset()).top();
   bottom = top + (int)blockBoundingRect(blk).height();
   while (blk.isValid() && top < aEvent->rect().bottom())
   {
      if (blk.isVisible() && bottom >= aEvent->rect().top())
      {
         visibleLineMap[ut::cast_to_size_t(blk.blockNumber())] = QPoint(symbolLeft, (top + bottom - 16) / 2);
      }
      blk    = blk.next();
      top    = bottom;
      bottom = top + (int)blockBoundingRect(blk).height();
   }

   wizEnv.MarginDraw(GetSource()->GetSystemPath().c_str(), painter, visibleLineMap);
}

void wizard::WsfEditor::OnCursorPositionChanged()
{
   Editor::OnCursorPositionChanged();
   if (mSyntaxTipsPtr)
   {
      mSyntaxTipsPtr->CursorMove();
   }
   if (mAutocompletePopupPtr)
   {
      int x, y, lh;
      GetCursorPoint(x, y, lh);
      mAutocompletePopupPtr->CursorMove(x, y, lh);
   }
}

void wizard::WsfEditor::AddMarker(int aLineNumber, const QImage& aImage)
{
   mMarkers[aLineNumber] = aImage;
}

void wizard::WsfEditor::OnUpdateRequest(const QRect& rect, int w)
{
   Editor::OnUpdateRequest(rect, w);
   if (!mStylingNow)
   {
      StyleVisibleLines();
   }
}

void wizard::WsfEditor::StyleLines(int aFirstLine, int aLastLine)
{
   mStylingNow         = true;
   QTextBlock firstBlk = document()->findBlockByNumber(aFirstLine);
   QTextBlock lastBlk  = document()->findBlockByNumber(aLastLine);
   int        startPos = firstBlk.position();
   int        endPos   = lastBlk.position() + lastBlk.length();
   if (endPos - startPos > 0)
   {
      mCurrentStylePos    = ut::cast_to_size_t(startPos);
      mCurrentStyleLength = endPos - startPos;
      mFormatChanges.fill(QTextCharFormat(), endPos - startPos);
      ParseResults* results = GetProject()->GetParseResults();
      if (results && results->parseTree())
      {
         WsfStyler*              stylerPtr = wizExeMgr.GetStyler();
         WsfStyler::StyleRequest request;
         request.mControl           = this;
         request.mStylerInterface   = this;
         request.mResultsPtr        = GetProject()->GetParseResults();
         request.mSourcePtr         = GetSource();
         request.mShowNotifications = !request.mResultsPtr->isProxyDataOld();
         request.mFirstPosition     = ut::cast_to_size_t(startPos);
         request.mLastPosition      = ut::cast_to_size_t(endPos);
         stylerPtr->UpdateStyle(request);
         Editor::UpdateStyleFromArray(mFormatChanges, firstBlk, lastBlk.blockNumber(), startPos, endPos);
         document()->markContentsDirty(startPos, endPos - startPos);
         mFormatChanges.clear();
      }
   }
   mStylingNow = false;
}


void wizard::WsfEditor::SetTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt)
{
   size_t end = std::min(aStart + aCount, mCurrentStylePos + mCurrentStyleLength);
   if (end >= mCurrentStylePos)
   {
      for (size_t i = std::max(mCurrentStylePos, aStart) - mCurrentStylePos; i < end - mCurrentStylePos; ++i)
      {
         mFormatChanges[ut::cast_to_int(i)] = aFmt;
      }
   }
}

void wizard::WsfEditor::MergeTextFormat(size_t aStart, size_t aCount, const QTextCharFormat& aFmt)
{
   size_t end = std::min(aStart + aCount, mCurrentStylePos + mCurrentStyleLength);
   if (end >= mCurrentStylePos)
   {
      for (size_t i = std::max(mCurrentStylePos, aStart) - mCurrentStylePos; i < end - mCurrentStylePos; ++i)
      {
         mFormatChanges[ut::cast_to_int(i)].merge(aFmt);
      }
   }
}

void wizard::WsfEditor::StyleVisibleLines()
{
   ParseResults* results = GetProject()->GetParseResults();
   if (results && results->parseTree())
   {
      // Find the first and last visible lines
      QTextBlock blk            = firstVisibleBlock();
      int        firstLine      = blk.blockNumber();
      int        lastLine       = firstLine;
      int        viewportHeight = viewport()->rect().height();
      while (blk.isValid())
      {
         if (blk.isVisible())
         {
            if (blockBoundingGeometry(blk).top() < viewportHeight)
            {
               lastLine = blk.blockNumber();
            }
            else
            {
               break;
            }
         }
         blk = blk.next();
      }

      if (firstLine - 1 > mFullStyleLastLine || lastLine + 1 < mFullStyleFirstLine)
      {
         // No overlap, forget about the already styled range.
         StyleLines(firstLine, lastLine);
         mFullStyleFirstLine = firstLine;
         mFullStyleLastLine  = lastLine;
      }
      else
      {
         // overlap, expand styled range
         if (firstLine < mFullStyleFirstLine)
         {
            StyleLines(firstLine, mFullStyleLastLine - 1);
            mFullStyleLastLine = firstLine;
         }
         if (lastLine > mFullStyleLastLine)
         {
            StyleLines(mFullStyleLastLine + 1, lastLine);
            mFullStyleLastLine = lastLine;
         }
      }
   }
   else
   {
      mFullStyleLastLine = mFullStyleFirstLine = -1;
   }
}

void wizard::WsfEditor::DocContentsChange(int from, int charsRemoves, int charsAdded)
{
   Editor::DocContentsChange(from, charsRemoves, charsAdded);

   // our styler can't run until we re-parse everything.
   // This code handles small edits by just updating ranges based on added or removed characters
   // The result is less flickering while typing

   // arbitrary cap of 10 chars.  we want to capture typing, but not pasting of large blocks of code
   if (charsRemoves < 10 && charsAdded < 10)
   {
      // note: we are only handling a single block and assuming no carriage returns are being added
      QTextBlock blk = document()->findBlock(from);
      if (blk.isValid() && blk.layout())
      {
         from -= blk.position();
         QList<QTextLayout::FormatRange> ranges = blk.layout()->additionalFormats();
         for (auto&& range : ranges)
         {
            if (charsRemoves > 0)
            {
               if (range.start > from)
               {
                  range.start -= charsRemoves;
                  if (range.start < from)
                  {
                     int newLen   = std::max(0, range.start + range.length - from);
                     range.length = newLen;
                     range.start  = from;
                  }
               }
            }
            if (charsAdded > 0)
            {
               if (range.start >= from)
               {
                  range.start += charsAdded;
               }
               else if (range.start + range.length >= from)
               {
                  range.length += charsAdded;
               }
            }
         }
         blk.layout()->setAdditionalFormats(ranges);
      }
   }
   if (mSyntaxTipsPtr != nullptr)
   {
      mSyntaxTipsPtr->UserModification();
   }
}

void wizard::WsfEditor::Paste()
{
   if (!PasteMime(QApplication::clipboard()->mimeData()))
   {
      Editor::Paste();
   }
}

void wizard::WsfEditor::dragEnterEvent(QDragEnterEvent* e)
{
   if (e->mimeData()->hasUrls())
   {
      QWidget::dragEnterEvent(e);
      QList<QUrl> files = e->mimeData()->urls();
      if (!files.empty())
      {
         QString localFile = files[0].toLocalFile();
         if (!localFile.isEmpty())
         {
            e->accept();
         }
      }
   }
   else
   {
      return QPlainTextEdit::dragEnterEvent(e);
   }
}

void wizard::WsfEditor::dragLeaveEvent(QDragLeaveEvent* e)
{
   return QPlainTextEdit::dragLeaveEvent(e);
}

void wizard::WsfEditor::dragMoveEvent(QDragMoveEvent* e)
{
   if (e->mimeData()->hasFormat(DragAndDrop::cPROPERTY_SET_MIME_TYPE))
   {
      e->accept();
   }
   else
   {
      if (e->mimeData()->hasUrls())
      {
         QWidget::dragMoveEvent(e);
      }
      else
      {
         return QPlainTextEdit::dragMoveEvent(e);
      }
   }
}

void wizard::WsfEditor::dropEvent(QDropEvent* e)
{
   if (e->mimeData()->hasUrls())
   {
      QWidget::dropEvent(e);
      QList<QUrl> files = e->mimeData()->urls();
      if (!Project::Instance())
      {
         QStringList fileList;
         for (const QUrl& url : files)
         {
            fileList << url.toLocalFile();
         }
         Environment::GetInstance().LoadScenario(fileList);
      }
      // Load scenario only seems to open one file, so open the others if necessary
      if (Project::Instance())
      {
         for (const auto& fileIt : files)
         {
            QString localFile = fileIt.toLocalFile();
            if (!localFile.isEmpty())
            {
               Project::Instance()->OpenFile(localFile.toStdString());
            }
         }
      }
   }
   else
   {
      return QPlainTextEdit::dropEvent(e);
   }
}

bool wizard::WsfEditor::PasteMime(const QMimeData* aMimeData)
{
   if (!aMimeData)
   {
      return false;
   }
   PasteContext ctx("text-editor");
   ctx.SetWidget(this);
   ctx.ApplyToTextImmediately(true);
   ctx.SetFile(GetSource()->GetSystemPath());
   return ctx.Exec(aMimeData);
}

void wizard::WsfEditor::hideEvent(QHideEvent* e)
{
   Editor::hideEvent(e);
   HideAutocomplete();
}

bool wizard::WsfEditor::event(QEvent* e)
{
   if (e->type() == QEvent::ToolTip)
   {
      QHelpEvent* helpEvent = static_cast<QHelpEvent*>(e);
      helpEvent->setAccepted(false);
      emit wizSignals->EditorToolTipEvent(this, helpEvent);
      if (!helpEvent->isAccepted())
      {
         if (HoverTimeout())
         {
            helpEvent->accept();
            // helpEvent->ignore();
         }
      }
   }
   return Editor::event(e);
}
