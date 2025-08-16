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

#include "Widget.hpp"

#include <QMessageBox>
#include <QPainter>
#include <QTextBlock>
#include <QTextCursor>

#include "CheckBox.hpp"
#include "Editor.hpp"
#include "EditorPrefObject.hpp"
#include "EnumComboBox.hpp"
#include "LineEdit.hpp"
#include "LinkUtil.hpp"
#include "MapComboBox.hpp"
#include "PartManagerPlugin.hpp"
#include "Position.hpp"
#include "Project.hpp"
#include "ProxyWatchNode.hpp"
#include "ProxyWatcher.hpp"
#include "UiResources.hpp"
#include "Undo.hpp"
#include "UnitValueLineEdit.hpp"
#include "UtQt.hpp"
#include "Util.hpp"
#include "WkfEnvironment.hpp"

PartManager::Widget::Widget(const WsfPProxyNode& aProxyNode, QWidget* aParent /*= nullptr*/)
   : QWidget(aParent)
   , mNode(aProxyNode)
{
   mUi.setupUi(this);
   UpdateLabel();

   // Icons

   mUi.deleteButton->setIcon(QIcon::fromTheme("delete"));
   mUi.addPartButton->setIcon(QIcon::fromTheme("add"));

   wizard::UiResources& resources         = wizard::UiResources::GetInstance();
   QIcon                arrowIcon         = QIcon::fromTheme("closed_arrow_inactive");
   QImage               openArrowInactive = resources.GetImageFromTheme("open_arrow_inactive");
   QImage               closedArrowActive = resources.GetImageFromTheme("closed_arrow_active");
   QImage               openArrowActive   = resources.GetImageFromTheme("open_arrow_active");

   arrowIcon.addPixmap(QPixmap::fromImage(openArrowInactive), QIcon::Normal, QIcon::On);
   arrowIcon.addPixmap(QPixmap::fromImage(closedArrowActive), QIcon::Active, QIcon::Off);
   arrowIcon.addPixmap(QPixmap::fromImage(openArrowActive), QIcon::Active, QIcon::On);
   arrowIcon.addPixmap(QPixmap::fromImage(closedArrowActive), QIcon::Selected, QIcon::Off);
   arrowIcon.addPixmap(QPixmap::fromImage(openArrowActive), QIcon::Selected, QIcon::On);
   mUi.showHideButton->setIcon(arrowIcon);

   // Toggle the visibility of widget content with the show/hide button.
   connect(mUi.showHideButton, &QPushButton::toggled, this, &Widget::SetContentVisible);

   // Prompt the user to add a new child part when the user clicks the add part
   // button.
   connect(mUi.addPartButton, &QPushButton::clicked, this, &Widget::HandleAddPartButtonPressed);

   // Prompt the user to delete this widget/part when the user clicks
   // the delete button.
   connect(mUi.deleteButton, &QPushButton::clicked, this, &Widget::HandleDeleteButtonPressed);

   // Handle URL links in widget labels.
   connect(mUi.nameLabel, &QLabel::linkActivated, this, &Widget::PropagateLinkActivated);

   mProxyCallbacks += wizard::WatchProxy(mNode.GetPath()).ThisUpdate.Connect(&Widget::HandleProxyChanged, this);
   mProxyCallbacks += wizard::ProxyWatcher::NodeRenamed.Connect(&Widget::HandleParentRenames, this);
}

//! Shows or hides various UI elements on the widget, depending on whether or
//! not the widget is at the top-level in the browser or if the widget
//! represents a basic type.
void PartManager::Widget::SetupUi(bool aIsTopLevel)
{
   SetContentVisible(aIsTopLevel);
   mUi.nameLabel->setVisible(!aIsTopLevel);
   mUi.showHideButton->setVisible(!aIsTopLevel);
   mUi.deleteButton->setVisible(!mNode.IsOfBasicType() && !aIsTopLevel);
}

//! Returns the proxy node that this widget represents.
const WsfPProxyNode& PartManager::Widget::GetNode() const
{
   return mNode;
}

//! Returns the HTML label of the widget.
QString PartManager::Widget::GetLabel() const
{
   return mUi.nameLabel->text();
}

//! Updates the HTML label of the widget.
void PartManager::Widget::UpdateLabel()
{
   mUi.nameLabel->setText(LinkUtil::MakeWidgetLabel(mNode));
}

//! On a parent widget, this method takes a path and expands its children
//! according to the path, highlighting the expanded widgets along the way.
//!
//! The destination widget will have its entire content highlighted, whereas
//! widgets "on the way" will only have their headers highlighted.
void PartManager::Widget::ExpandPath(const WsfPProxyPath& aPath)
{
   HighlightHeader();

   if (mNode.GetPath() == aPath)
   {
      HighlightContent();
   }
}

//! Re-emits a "link activated" signal. Parent widgets connect the "link
//! activated" signal of their children to this slot, and so this signal will
//! propagate all the way up to a top-level GUI widget (e.g. the browser), so
//! that it can open the link.
void PartManager::Widget::PropagateLinkActivated(const QString& aLink)
{
   emit LinkActivated(aLink);
}

//! Temporarily animate the widget's "header" elements. That is, the elements
//! that are visible even while the widget is collapsed.
void PartManager::Widget::HighlightHeader()
{
   UtQtHighlightWidget(mUi.showHideButton);
   UtQtHighlightWidget(mUi.nameLabel);
   UtQtHighlightWidget(mUi.deleteButton);
}

//! Temporarily animate the widget's "content" elements. That is, the elements
//! that are visible only when the widget is expanded.
void PartManager::Widget::HighlightContent()
{
   UtQtHighlightWidget(mUi.contentContainer);
   UtQtHighlightWidget(mUi.addPartButton);
}

//! Shows or hides the widget's content. The content should be hidden by default
//! for children/nested widgets, but shown by default for the top level. The
//! visibility of the content is toggled by the collapse/expand button.
void PartManager::Widget::SetContentVisible(bool aVisible)
{
   mUi.showHideButton->setChecked(aVisible);
   mUi.contentContainer->setVisible(aVisible);
   mUi.addPartButton->setVisible(IsAddPartSupported() && !mNode.IsOfBasicType() && aVisible);
}

//! Derived classes should override this when they support the "Add part" button.
void PartManager::Widget::HandleAddPartButtonPressed() {}

//! Handles the user pressing the delete button on a part.
void PartManager::Widget::HandleDeleteButtonPressed()
{
   QMessageBox prompt(this);

   prompt.setWindowTitle("Delete part");
   prompt.setText("Do you want to delete this part?");
   prompt.setInformativeText(wizard::Util::StripHtml(mUi.nameLabel->text()));
   prompt.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
   prompt.setDefaultButton(QMessageBox::No);

   const int userChoice = prompt.exec();

   switch (userChoice)
   {
   case QMessageBox::Yes:
   {
      WsfPProxyNode   parentMapNode = mNode.GetParent();
      WsfPM_ObjectMap parentMap(parentMapNode);
      const QString   nodeName   = QString::fromStdString(mNode.GetName());
      const QString   parentName = QString::fromStdString(parentMapNode.GetParent().GetName());

      QString typeStr = PartManager::Plugin::GetNodeTypeString(mNode);

      UtTextDocumentRange nodeTextRange = mNode.GetSourceDocumentRange();
      if (typeStr == "mover")
      {
         wizard::Editor* moverEditorPtr = GetEditor(mNode, nodeTextRange);

         QTextCursor tmpCursor = moverEditorPtr->textCursor();
         tmpCursor.setPosition(ut::cast_to_int(nodeTextRange.GetBegin()));
         moverEditorPtr->setTextCursor(tmpCursor);
         QString curBlockStr = moverEditorPtr->textCursor().block().text();
         curBlockStr         = curBlockStr.trimmed();

         if (!curBlockStr.startsWith("mover") && !curBlockStr.startsWith("edit mover"))
         {
            nodeTextRange.mSource = nullptr;
         }
      }

      if (!nodeTextRange.Valid() && mNode.IsInherited())
      {
         UtTextDocumentRange partTextRange;
         wizard::Editor*     editorPtr = GetEditor(mNode, partTextRange);
         if (editorPtr)
         {
            QTextCursor bottomCursor = editorPtr->textCursor();
            bottomCursor.setPosition(ut::cast_to_int(partTextRange.GetEnd()));
            bottomCursor.setPosition(bottomCursor.block().previous().position());
            bottomCursor.movePosition(QTextCursor::EndOfLine);

            QString tabSpace   = "";
            int     indentSize = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
            for (int i = 0; i < indentSize; ++i)
            {
               tabSpace.append(" ");
            }

            QString deleteStr = "\n" + tabSpace + "delete " + typeStr;
            if (typeStr != "fuel" && typeStr != "mover")
            {
               deleteStr.append(" ");
               deleteStr.append(nodeName);
            }
            bottomCursor.insertText(deleteStr);
            editorPtr->setTextCursor(bottomCursor);
            editorPtr->textCursor().movePosition(QTextCursor::StartOfLine);
            editorPtr->textCursor().clearSelection();
            int curPos = editorPtr->textCursor().blockNumber();
            editorPtr->GoToLine(curPos - 1);

            // need to check to see if this platform part is inherited by anyone else before deleting it...
            if (mNode.IsInherited())
            {
               DeleteInheritedParts(mNode);
            }
         }
      }
      else
      {
         UtTextDocumentRange partTextRange;
         wizard::Editor*     editorPtr = GetEditor(mNode, partTextRange);
         if (editorPtr)
         {
            QTextCursor partTextLine = editorPtr->textCursor();
            partTextLine.setPosition(ut::cast_to_int(partTextRange.GetBegin()));
            QString partStr = partTextLine.block().text();
            partStr         = partStr.trimmed();

            if (partStr.startsWith("add"))
            {
               editorPtr->Replace(partTextRange, "");
            }
            else if (mNode.IsInheritedFromParent())
            {
               QString tabSpace   = "";
               int     indentSize = wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->TabSpace();
               for (int i = 0; i < indentSize; ++i)
               {
                  tabSpace.append(" ");
               }
               QString replaceStr = "delete " + typeStr;
               if (typeStr != "fuel" && typeStr != "mover")
               {
                  replaceStr.append(" ");
                  replaceStr.append(nodeName);
               }
               editorPtr->Replace(partTextRange, replaceStr.toStdString());
            }
            else
            {
               // need to check to see if this platform part is inherited by anyone else before deleting it...
               if (mNode.IsInherited())
               {
                  DeleteInheritedParts(mNode);
               }

               editorPtr->Replace(partTextRange, "");
            }
         }
      }
      wizard::UndoContext undoCtx(QString("Remove %1 from %2").arg(nodeName).arg(parentName));
      break;
   }
   case QMessageBox::No:
   default:
      break;
   }
}

//! This function handles the case of a proxy node (being represented by a
//! top-level widget in the browser) being deleted and potentially re-added. Its
//! representative widget will need to display an error message unless and until
//! the proxy node is re-added.
//!
//! Note that since parent widgets *delete* children widgets whose respective
//! proxy nodes have been deleted, this function only pertains to top-level
//! widgets and not child widgets.
//!
//! Since top-level widgets unconditionally show their content and
//! cannot be collapsed, that assumption is used in this function when calling
//! setContentVisible.
//!
//! Renames are also handled. (Applies to any widget, not just the top-level one.)
void PartManager::Widget::HandleProxyChanged(const wizard::ProxyChange& aProxyChange)
{
   switch (aProxyChange.reason())
   {
   case wizard::ProxyChange::cREMOVED:
      // If the proxy node was deleted, then set an error label and hide the
      // widget content.
      mUi.nameLabel->setText(MakeErrorMsg());
      SetContentVisible(false);
      break;

   case wizard::ProxyChange::cADDED:
      // If the proxy node was re-added, then restore the widget label and
      // show the widget content.
      UpdateLabel();
      SetContentVisible(true);
      for (auto&& it : mAttributes)
      {
         it->UpdateAttribute();
      }
      break;

   case wizard::ProxyChange::cRENAMED:
      mNode = aProxyChange.changed();
      UpdateAttrsParentNode(aProxyChange.oldPath(), aProxyChange.changed().GetPath());
      UpdateLabel();
      break;

   default:
      break;
   }
}

//! Looks for and handles the case where any parent of this widget's node is
//! renamed; i.e., if this node is foo.bar and foo is renamed to foo2, then this
//! function will update this node to foo2.bar. (Its attributes will then be
//! called to update themselves similarly.)
void PartManager::Widget::HandleParentRenames(const WsfPProxyPath& aOldPath, const WsfPProxyPath& aNewPath)
{
   const WsfPProxyPath inheritedPath = mNode.GetInherited().GetPath();

   if (aOldPath.IsPrefixOf(mNode.GetPath()) && (aOldPath != mNode.GetPath()))
   {
      if (mNode.UpdateParentPath(aOldPath, aNewPath))
      {
         UpdateAttrsParentNode(aOldPath, aNewPath);
         UpdateLabel();
      }
   }
   else if (aNewPath.IsPrefixOf(inheritedPath))
   {
      UpdateLabel();
   }
}

//! Derived classes should override this when they support the "Add part" button.
bool PartManager::Widget::IsAddPartSupported() const
{
   return false;
}

//! Adds a check box attribute to the widget.
PartManager::CheckBox* PartManager::Widget::AddCheckBoxAttr(const std::string& aAttrName, const WsfPProxyNode& aAttrNode)
{
   CheckBox* attrWidget = nullptr;

   if (aAttrNode.IsValid())
   {
      attrWidget = new CheckBox(aAttrNode, aAttrName, this);
      AddAttr(aAttrName, attrWidget);
   }

   return attrWidget;
}

//! Adds a line edit (single line) attribute to the widget.
PartManager::LineEdit* PartManager::Widget::AddLineEditAttr(const std::string& aAttrName, const WsfPProxyNode& aAttrNode)
{
   LineEdit* attrWidget = nullptr;

   if (aAttrNode.IsValid())
   {
      attrWidget = new LineEdit(aAttrNode, aAttrName, this);
      AddAttr(aAttrName, attrWidget);
   }

   return attrWidget;
}

//! Adds a combo box representing a spatial domain attribute to the widget.
PartManager::EnumComboBox* PartManager::Widget::AddEnumComboBoxAttr(const std::string&   aAttrName,
                                                                    const WsfPProxyNode& aAttrNode)
{
   EnumComboBox* attrWidget = nullptr;

   if (aAttrNode.IsValid())
   {
      attrWidget = new EnumComboBox(aAttrNode, aAttrName, this);
      AddAttr(aAttrName, attrWidget);
   }

   return attrWidget;
}

//! Adds a combo box representing a signature attribute to the widget.
PartManager::MapComboBox* PartManager::Widget::AddMapComboBoxAttr(const std::string&     aAttrName,
                                                                  const WsfPProxyNode&   aAttrNode,
                                                                  const WsfPM_ObjectMap& aObjectMap)
{
   MapComboBox* attrWidget = nullptr;

   if (aAttrNode.IsValid() && aObjectMap.IsValid())
   {
      attrWidget = new MapComboBox(aAttrNode, aAttrName, aObjectMap, this);
      AddAttr(aAttrName, attrWidget);
   }

   return attrWidget;
}

//! Adds a position multi-attribute for the given platform to the widget.
PartManager::Position* PartManager::Widget::AddPositionAttr(const std::string& aAttrName, const WsfPProxyNode& aPositionNode)
{
   Position* attrWidget = nullptr;

   if (aPositionNode.IsValid())
   {
      attrWidget = new Position(aPositionNode, this);
      AddAttr(aAttrName, attrWidget);
   }

   return attrWidget;
}

//! Adds a line edit representing an angle attribute to the widget.
PartManager::UnitValueLineEdit* PartManager::Widget::AddAngleAttr(const std::string&   aAttrName,
                                                                  const WsfPProxyNode& aAttrNode)
{
   UnitValueLineEdit* attrWidget = nullptr;

   if (aAttrNode.IsValid())
   {
      attrWidget = new UnitValueLineEdit(aAttrNode, aAttrName, UtUnits::cANGLE, this);
      AddAttr(aAttrName, attrWidget);
   }

   return attrWidget;
}

//! Returns the content layout of the widget. This layout contains both
//! attributes and nested ViPpmWidgets.
QFormLayout* PartManager::Widget::ContentLayout() const
{
   return mUi.contentLayout;
}

//! Returns the number of attribute child widgets that have been added.
size_t PartManager::Widget::NumOfAttributes() const
{
   return mAttributes.size();
}

//! We override the paint event in order to color the background of
//! attributes which are inherited and have been overridden.
void PartManager::Widget::paintEvent(QPaintEvent* aEvent)
{
   QWidget::paintEvent(aEvent);

   if (!mNode.IsOfBasicType())
   {
      QPainter     painter(this);
      const QPoint contentOffset = mUi.contentContainer->geometry().topLeft();

      for (AttrList::iterator i = mAttributes.begin(); i != mAttributes.end(); ++i)
      {
         Attribute* attr = *i;
         if (attr->isVisible() && !attr->IsInherited())
         {
            const QWidget* attrLabel    = mUi.contentLayout->labelForField(attr);
            const QPoint   labelTopLeft = attrLabel->geometry().topLeft();
            const QPoint   attrBotRight = attr->geometry().bottomRight();

            const QPoint rowTopLeft(contentOffset.x() + labelTopLeft.x() - 4, contentOffset.y() + labelTopLeft.y() - 4);
            const QPoint rowBotRight(contentOffset.x() + attrBotRight.x() + 4, contentOffset.y() + attrBotRight.y() + 4);

            painter.fillRect(QRect(rowTopLeft, rowBotRight),
                             wkfEnv.GetPreferenceObject<wizard::EditorPrefObject>()->EditorColor("editor_line_color"));
            update();
         }
      }
   }
}

//! Creates an error string to be displayed to the user in the browser when
//! this widget's proxy node becomes null.
QString PartManager::Widget::MakeErrorMsg() const
{
   const QString plainTextLabel = wizard::Util::StripHtml(mUi.nameLabel->text());
   const QString errorMsg       = plainTextLabel + " no longer exists.";

   return errorMsg;
}

//! Updates attribute widget nodes when any of their parents' paths change, so
//! that they remain valid.
void PartManager::Widget::UpdateAttrsParentNode(const WsfPProxyPath& aOldPath, const WsfPProxyPath& aNewPath)
{
   for (auto&& it : mAttributes)
   {
      it->UpdateParentPath(aOldPath, aNewPath);
   }
}

//! Adds an attribute to the end of the content form layout.
void PartManager::Widget::AddAttr(const std::string& aAttrName, Attribute* aAttrWidget)
{
   QString formLabel = QString::fromStdString(aAttrName + ':');
   mUi.contentLayout->addRow(formLabel, aAttrWidget);
   mAttributes.push_back(aAttrWidget);
}


wizard::Editor* PartManager::Widget::GetEditor(const WsfPProxyNode& aNode, UtTextDocumentRange& aRange)
{
   aRange.SetRange(aNode.GetSourceDocumentRange().GetBegin(), aNode.GetSourceDocumentRange().GetEnd());
   aRange.mSource = aNode.GetSourceDocumentRange().mSource;
   if (aNode.IsOfType("Mover"))
   {
      WsfPProxyNode curNode = aNode;
      while (curNode.IsValid())
      {
         WsfPProxyIndexNode* proxyIndexNode = curNode.FindIndexNode();
         if (proxyIndexNode)
         {
            for (auto iter = proxyIndexNode->mEntries.rbegin(); iter != proxyIndexNode->mEntries.rend(); ++iter)
            {
               const WsfParseNode* parseNode = iter->mNodePtr;
               if (parseNode && parseNode->GetParent())
               {
                  WsfParseNode* parent = parseNode->GetParent();

                  aRange.SetRange(parent->SubtreeRange().GetBegin(), parent->SubtreeRange().GetEnd());
                  aRange.mSource = parent->SubtreeRange().mSource;

                  std::string     fileName  = aRange.mSource->GetFilePath().GetNormalizedPath();
                  wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(fileName);
                  QTextCursor     tmpCursor = editorPtr->textCursor();
                  tmpCursor.setPosition(ut::cast_to_int(aRange.GetBegin()));
                  editorPtr->setTextCursor(tmpCursor);
                  QString curBlockStr = editorPtr->textCursor().block().text();
                  curBlockStr         = curBlockStr.trimmed();

                  if (curBlockStr.startsWith("mover") || curBlockStr.startsWith("edit mover") ||
                      curBlockStr.startsWith("platform") || curBlockStr.startsWith("edit platform"))
                  {
                     return editorPtr;
                  }
               }
            }
         }
         curNode = curNode.GetParent();
      }
   }
   WsfPProxyIndexNode* proxyIndexNode = aNode.FindIndexNode();
   if (proxyIndexNode == nullptr)
   {
      WsfPProxyNode parentNode = aNode.GetParent();
      while (proxyIndexNode == nullptr && parentNode.IsValid())
      {
         proxyIndexNode = parentNode.FindIndexNode();

         parentNode = parentNode.GetParent();
      }
   }
   std::vector<WsfPProxyIndexEntry> entryVec;
   if (proxyIndexNode->mEntries.size() == 0)
   {
      entryVec = proxyIndexNode->mParentPtr->mEntries;
   }
   else
   {
      entryVec = proxyIndexNode->mEntries;
   }
   for (auto iter = entryVec.rbegin(); iter != entryVec.rend(); ++iter)
   {
      if (proxyIndexNode->mEntries.size() ==
          1) // there is only 1 and it's in the platform definition.  No need to look for an 'edit'
      {
         if (!aRange.Valid())
         {
            const WsfParseNode* parseNode = iter->mNodePtr;
            if (parseNode && parseNode->GetParent())
            {
               WsfParseNode* parent = parseNode->GetParent();
               aRange.SetRange(parent->SubtreeRange().GetBegin(), parent->SubtreeRange().GetEnd());
               aRange.mSource = parent->SubtreeRange().mSource;
            }
         }
         std::string fileName = aRange.mSource->GetFilePath().GetNormalizedPath();
         return wizard::Project::Instance()->GotoFile(fileName);
      }
      const WsfParseNode* parseNode = iter->mNodePtr;
      if (parseNode && parseNode->GetParent())
      {
         WsfParseNode* parent = parseNode->GetParent();

         aRange.SetRange(parent->SubtreeRange().GetBegin(), parent->SubtreeRange().GetEnd());
         aRange.mSource            = parent->SubtreeRange().mSource;
         std::string     fileName  = aRange.mSource->GetFilePath().GetNormalizedPath();
         wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(fileName);
         QTextCursor     tmpCursor = editorPtr->textCursor();
         tmpCursor.setPosition(ut::cast_to_int(aRange.GetBegin()));
         editorPtr->setTextCursor(tmpCursor);
         QString curBlockStr = editorPtr->textCursor().block().text();
         curBlockStr         = curBlockStr.trimmed();
         // loop through block().previous() until we get to platform definition or edit platform
         // if edit platform is found, mark this as the document range we want to deal with...
         while (!curBlockStr.startsWith("platform") && !curBlockStr.startsWith("edit platform"))
         {
            QTextBlock previousBlock   = editorPtr->textCursor().block().previous();
            curBlockStr                = previousBlock.text();
            QTextCursor curBlockCursor = editorPtr->textCursor();
            curBlockCursor.setPosition(previousBlock.position());
            editorPtr->setTextCursor(curBlockCursor);
         }
         if (curBlockStr.startsWith("edit platform") || curBlockStr.startsWith("platform"))
         {
            return editorPtr;
         }
      }
   }
   return nullptr;
}


void PartManager::Widget::DeleteInheritedParts(WsfPProxyNode& aNode)
{
   // many changes to our documents are about to happen.  The proxy informs these changes by finding the nodes of values
   // we are about to change and telling us where we can find them in file.  Unfortunately once we make a change in the
   // file, proxy nodes that point to the same file may be invalidated.  To avoid this, we sort the necessary changes
   // from the bottom of the file to the top, so that early changes never effect the location of later changes.

   // this class holds onto the deletion data.  This lets us sort everything in a single set.
   class DeleteWrapper
   {
   public:
      DeleteWrapper(UtTextDocumentRange aRange) { mRange = aRange; }
      bool operator<(const DeleteWrapper& aRHS) const
      {
         if (mRange.mSource->GetFilePath() == aRHS.mRange.mSource->GetFilePath())
         {
            if (mRange.GetBegin() == aRHS.mRange.GetBegin())
            {
               return mRange.GetEnd() >= aRHS.mRange.GetEnd();
            }
            else
            {
               return mRange.GetBegin() > aRHS.mRange.GetBegin();
            }
         }
         else
         {
            return mRange.mSource->GetFilePath() < aRHS.mRange.mSource->GetFilePath();
         }
      }
      UtTextDocumentRange GetRange() { return mRange; }

   private:
      UtTextDocumentRange mRange;
   };

   WsfPM_PlatformMap                                  platforms = WsfPM_Root(aNode.Root()).platforms();
   std::map<wizard::Editor*, std::set<DeleteWrapper>> platformEditorRangeMap;
   for (WsfPM_PlatformMap::iterator i = platforms.begin(); i != platforms.end(); ++i)
   {
      WsfPM_Platform curPlat = platforms.AtT(i);
      QString        typeStr = PartManager::Plugin::GetNodeTypeString(mNode);
      typeStr.append("s");
      WsfPProxyNode partNode = curPlat + typeStr.toStdString() + aNode.GetAttrName();
      if (partNode.IsValid())
      {
         UtTextDocumentRange aRange;
         aRange.SetRange(partNode.GetSourceDocumentRange().GetBegin(), partNode.GetSourceDocumentRange().GetEnd());
         aRange.mSource = partNode.GetSourceDocumentRange().mSource;
         if (partNode.IsOfType("Mover"))
         {
            WsfPProxyIndexNode* proxyIndexNode = partNode.FindIndexNode();
            if (proxyIndexNode)
            {
               for (auto iter = proxyIndexNode->mEntries.rbegin(); iter != proxyIndexNode->mEntries.rend(); ++iter)
               {
                  const WsfParseNode* parseNode = iter->mNodePtr;
                  if (parseNode && parseNode->GetParent())
                  {
                     WsfParseNode* parent = parseNode->GetParent();

                     aRange.SetRange(parent->SubtreeRange().GetBegin(), parent->SubtreeRange().GetEnd());
                     aRange.mSource = parent->SubtreeRange().mSource;

                     std::string     fileName  = aRange.mSource->GetFilePath().GetNormalizedPath();
                     wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(fileName);
                     QTextCursor     tmpCursor = editorPtr->textCursor();
                     tmpCursor.setPosition(ut::cast_to_int(aRange.GetBegin()));
                     editorPtr->setTextCursor(tmpCursor);
                     QString curBlockStr = editorPtr->textCursor().block().text();
                     curBlockStr         = curBlockStr.trimmed();

                     if (curBlockStr.startsWith("mover") || curBlockStr.startsWith("edit mover") ||
                         curBlockStr.startsWith("route") || curBlockStr.startsWith("edit route"))
                     {
                        platformEditorRangeMap[editorPtr].insert(DeleteWrapper(aRange));
                     }
                  }
               }
            }
         }
         else
         {
            WsfPProxyIndexNode* proxyIndexNode = partNode.FindIndexNode();
            if (proxyIndexNode && PlatformInheritsPart(partNode, aNode))
            {
               for (auto iter = proxyIndexNode->mEntries.rbegin(); iter != proxyIndexNode->mEntries.rend(); ++iter)
               {
                  const WsfParseNode* parseNode = iter->mNodePtr;
                  if (parseNode && parseNode->GetParent())
                  {
                     WsfParseNode* parent = parseNode->GetParent();

                     aRange.SetRange(parent->SubtreeRange().GetBegin(), parent->SubtreeRange().GetEnd());
                     aRange.mSource            = parent->SubtreeRange().mSource;
                     std::string     fileName  = aRange.mSource->GetFilePath().GetNormalizedPath();
                     wizard::Editor* editorPtr = wizard::Project::Instance()->GotoFile(fileName);
                     QTextCursor     tmpCursor = editorPtr->textCursor();
                     tmpCursor.setPosition(ut::cast_to_int(aRange.GetBegin()));
                     editorPtr->setTextCursor(tmpCursor);
                     QString curBlockStr = editorPtr->textCursor().block().text();
                     curBlockStr         = curBlockStr.simplified();
                     // loop through block().previous() until we get to platform definition or edit platform
                     // if edit platform is found, mark this as the document range we want to deal with...
                     while (!curBlockStr.startsWith("platform") && !curBlockStr.startsWith("edit platform"))
                     {
                        QTextBlock previousBlock   = editorPtr->textCursor().block().previous();
                        curBlockStr                = previousBlock.text().simplified();
                        QTextCursor curBlockCursor = editorPtr->textCursor();
                        curBlockCursor.setPosition(previousBlock.position());
                        editorPtr->setTextCursor(curBlockCursor);
                     }
                     if (curBlockStr.startsWith("edit platform") || curBlockStr.startsWith("platform"))
                     {
                        platformEditorRangeMap[editorPtr].insert(DeleteWrapper(aRange));
                     }
                  }
               }
            }
         }
      }
   }

   // now apply the deletions
   for (auto&& it : platformEditorRangeMap)
   {
      std::set<DeleteWrapper> editorSet = it.second;
      for (auto curSet : editorSet)
      {
         it.first->Replace(curSet.GetRange(), "");
      }
   }
}

bool PartManager::Widget::PlatformInheritsPart(const WsfPProxyNode& aPlatformNode, const WsfPProxyNode& aPartNode) const
{
   if (aPlatformNode.IsOfBasicType() || !aPlatformNode.IsValid())
   {
      return false;
   }

   const WsfPProxyPath inheritedPath = aPlatformNode.GetInheritedPath();
   if (inheritedPath.IsPrefixOf(aPartNode.GetPath()))
   {
      return true;
   }
   else
   {
      WsfPProxy*           proxy     = aPlatformNode.GetProxy();
      const WsfPProxyValue proxyRoot = proxy->mRoot;
      WsfPProxyPath        proxyPath = WsfPProxyPath::FromStringList(proxyRoot, inheritedPath.ToStringList(proxyRoot));
      WsfPProxyNode        inheritedNode(proxy, proxyPath);
      return PlatformInheritsPart(inheritedNode, aPartNode);
   }
   return false;
}
