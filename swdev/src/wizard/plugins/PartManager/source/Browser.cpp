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

#include "Browser.hpp"

#include <QScrollArea>
#include <QUndoStack>

#include "LinkUtil.hpp"
#include "ProjectWorkspace.hpp"
#include "ProxyWatcher.hpp"
#include "Signals.hpp"
#include "Undo.hpp"
#include "UtQt.hpp"
#include "Widget.hpp"
#include "WidgetFactory.hpp"
#include "WsfExe.hpp"

PartManager::Browser::Browser(const WsfPProxyNode& aProxyNode, QWidget* aParent /*= nullptr*/)
   : QDialog(aParent)
{
   mUi.setupUi(this);

   // Delete the browser widget when the user closes its window, closes the
   // current project, or changes the executable.
   setAttribute(Qt::WA_DeleteOnClose);
   connect(wizard::ProjectWorkspace::Instance(), &wizard::ProjectWorkspace::ProjectClosed, this, &Browser::deleteLater);
   connect(wizSignals, &wizard::Signals::WsfExeChanged, this, &Browser::deleteLater);

   // Hide the "?" title bar button on Windows.
   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   // Handle the user clicking the Back/Forward navigation buttons.
   connect(mUi.backButton, &QPushButton::clicked, this, &Browser::NavigateBack);
   connect(mUi.fwdButton, &QPushButton::clicked, this, &Browser::NavigateFoward);
   mUi.backButton->setIcon(QIcon::fromTheme("left"));
   mUi.fwdButton->setIcon(QIcon::fromTheme("right"));

   // Update the navigation buttons every time the current widget changes. We
   // want this to trigger when adding our first widget below, so connect this
   // before adding it.
   connect(mUi.ppmWidgetStack, &QStackedWidget::currentChanged, this, &Browser::UpdateHeader);

   // The top-level widget's label content is displayed in a label widget next
   // to the navigation buttons in the browser.
   connect(mUi.headerLabel, &QLabel::linkActivated, this, &Browser::OpenLink);

   // Add our first widget _after_ signals and slots are connected. (See above.)
   AddWidget(CreateWidget(aProxyNode));
   adjustSize();
}

//! Handles the user clicking a link in a widget label by navigating to the link
//! destination. This creates a new PPM widget and adds it to the top of the
//! navigation stack.
//! @param link The link to navigate to.
void PartManager::Browser::OpenLink(const QString& aLink)
{
   Widget*       curWidget = GetWidget(mUi.ppmWidgetStack->currentIndex());
   WsfPProxy*    proxy     = curWidget->GetNode().GetProxy();
   WsfPProxyPath proxyPath = LinkUtil::MakePathFromUrl(proxy->mRoot, aLink);
   WsfPProxyPath checkPath;

   for (size_t i = 0; i < proxyPath.size(); i++)
   {
      checkPath += proxyPath[i];

      WsfPProxyNode destNode(proxy, checkPath);

      if (!destNode.IsNull())
      {
         Widget* widget = CreateWidget(destNode);

         if (widget != nullptr)
         {
            AddWidget(widget);
            mUi.ppmWidgetStack->setCurrentIndex(mUi.ppmWidgetStack->currentIndex() + 1);
            widget->ExpandPath(proxyPath);
            UtQtHighlightWidget(mUi.headerLabel);
            break;
         }
      }
   }
}

//! This function is called automatically whenever the user changes which widget
//! is currently being viewed.
//! Handles enabling/disabling navigation buttons based on the state of the
//! browser. E.g. if there's no "previous" widget, disable the Back button, just
//! like in a web browser.
//! Also updates the browser label next to the navigation buttons.
void PartManager::Browser::UpdateHeader(int aIndex)
{
   const bool canNavBack = (aIndex > 0);
   const bool canNavFwd  = IsInherited(aIndex) ? true : false;

   mUi.backButton->setEnabled(canNavBack);
   mUi.fwdButton->setEnabled(canNavFwd);

   UpdateLabel(aIndex);
}

void PartManager::Browser::NavigateBack()
{
   mUi.ppmWidgetStack->setCurrentIndex(mUi.ppmWidgetStack->currentIndex() - 1);
}

void PartManager::Browser::NavigateFoward()
{
   // create the next widget if it hasn't already been created
   if (mUi.ppmWidgetStack->currentIndex() + 1 >= mUi.ppmWidgetStack->count())
   {
      Widget* curWidget = GetWidget(mUi.ppmWidgetStack->currentIndex());

      QString label       = curWidget->GetLabel();
      int     firstQuotes = label.indexOf("\"");
      int     lastQuotes  = label.lastIndexOf("\"");
      QString subStr      = label.mid(firstQuotes + 1, lastQuotes - firstQuotes - 1);
      OpenLink(subStr);
   }
   else
   {
      mUi.ppmWidgetStack->setCurrentIndex(mUi.ppmWidgetStack->currentIndex() + 1);
   }
}

PartManager::Widget* PartManager::Browser::CreateWidget(const WsfPProxyNode& aProxyNode)
{
   return WidgetFactory().CreateUi(aProxyNode, true, this);
}

void PartManager::Browser::AddWidget(Widget* aWidgetPtr)
{
   const int newIndex = mUi.ppmWidgetStack->currentIndex() + 1;
   const int maxIndex = mUi.ppmWidgetStack->count() - 1;

   // Pop widgets off the top of the stack.
   for (int i = maxIndex; i >= newIndex; i--)
   {
      DeleteCallbacksForIndex(i);
      DeleteWidgetAtIndex(i);
   }

   AddWidgetAtIndex(newIndex, aWidgetPtr);
   WatchChild(aWidgetPtr);
}

void PartManager::Browser::AddWidgetAtIndex(int aIndex, Widget* aWidgetPtr)
{
   QScrollArea* scrollArea = new QScrollArea(this);
   scrollArea->setWidgetResizable(true);
   scrollArea->setWidget(aWidgetPtr);
   mUi.ppmWidgetStack->insertWidget(aIndex, scrollArea);
   int areaHeight = aWidgetPtr->height() + (mUi.headerLabel->height() * 2);
   setMinimumHeight(areaHeight);
   connect(aWidgetPtr, &Widget::LinkActivated, this, &Browser::OpenLink);
}

void PartManager::Browser::UpdateLabel(int aIndex)
{
   Widget* widget = GetWidget(aIndex);
   QString newLabel;

   if (widget)
   {
      widget->UpdateLabel();
      newLabel = widget->GetLabel();
   }

   if (!IsInherited(aIndex))
   {
      newLabel.append(" <span style = \" color:#ff0000;\">(not modifiable)< / span > ");
   }

   mUi.headerLabel->setText(newLabel);
}

PartManager::Widget* PartManager::Browser::GetWidget(int aStackIndex)
{
   const QScrollArea* scrollArea = dynamic_cast<const QScrollArea*>(mUi.ppmWidgetStack->widget(aStackIndex));
   Widget*            widget     = (scrollArea ? dynamic_cast<Widget*>(scrollArea->widget()) : nullptr);

   return widget;
}

void PartManager::Browser::DeleteCallbacksForIndex(int aIndex)
{
   mProxyCallbacks.erase(GetWidget(aIndex));
}

void PartManager::Browser::DeleteWidgetAtIndex(int aIndex)
{
   QWidget* widgetToDelete = mUi.ppmWidgetStack->widget(aIndex);
   mUi.ppmWidgetStack->removeWidget(widgetToDelete);
   widgetToDelete->deleteLater();
}

void PartManager::Browser::WatchChild(const Widget* aWidgetPtr)
{
   const WsfPProxyPath& childPath     = aWidgetPtr->GetNode().GetPath();
   const WsfPProxyPath  childTypePath = aWidgetPtr->GetNode().GetInherited().GetPath();
   mProxyCallbacks[aWidgetPtr]        = std::make_pair(WatchPath(childPath), WatchPath(childTypePath));
}

std::unique_ptr<UtCallback> PartManager::Browser::WatchPath(const WsfPProxyPath& aPath)
{
   return wizard::WatchProxy(aPath).ThisUpdate.Connect(&Browser::HandleRenamed, this);
}

void PartManager::Browser::HandleRenamed(const wizard::ProxyChange& aProxyChange)
{
   switch (aProxyChange.reason())
   {
   case wizard::ProxyChange::cRENAMED:
      UpdateLabel(mUi.ppmWidgetStack->currentIndex());
      break;

   case wizard::ProxyChange::cUPDATED:
      for (int i = 0; i < mUi.ppmWidgetStack->count(); i++)
      {
         Widget*              widget    = GetWidget(i);
         const WsfPProxyPath& childPath = widget->GetNode().GetPath();

         if (childPath == aProxyChange.oldPath())
         {
            Widget* renamedWidget = CreateWidget(aProxyChange.changed());
            MoveCallbacks(widget, renamedWidget);
            DeleteWidgetAtIndex(i);
            AddWidgetAtIndex(i, renamedWidget);
            break;
         }
      }
      break;

   default:
      break;
   }
}

void PartManager::Browser::MoveCallbacks(const Widget* aFromWidgetPtr, const Widget* aToWidgetPtr)
{
   mProxyCallbacks[aToWidgetPtr] = std::move(mProxyCallbacks[aFromWidgetPtr]);
   mProxyCallbacks.erase(aFromWidgetPtr);
}

bool PartManager::Browser::IsInherited(int aIndex)
{
   Widget*             widget        = GetWidget(aIndex);
   const WsfPProxyPath inheritedPath = widget->GetNode().GetInheritedPath();
   return !inheritedPath.Empty();
}
