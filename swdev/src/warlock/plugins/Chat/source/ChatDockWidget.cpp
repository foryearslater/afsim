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

#include "ChatDockWidget.hpp"

#include <QFocusEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QTabBar>
#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "ChatGroup.hpp"
#include "WkfEnvironment.hpp"
#include "WkfMainWindow.hpp"
#include "WkfVtkEnvironment.hpp"

Chat::DockWidget::DockWidget(QWidget* parent, int aId, Qt::WindowFlags f)
   : QDockWidget(parent, f)
   , mId(aId)
{
   if (mId == 0)
   {
      setObjectName("Chat");
      setWindowTitle("Chat");
   }
   else
   {
      setObjectName("Chat " + QString::number(aId + 1));
      setWindowTitle("Chat " + QString::number(aId + 1));
   }

   Build();
}

void Chat::DockWidget::SetGroupName(Group* aGroup, const QString& aName)
{
   const int index = mGroupings->indexOf(aGroup);
   if (index >= 0)
   {
      aGroup->setObjectName(aName);
      mGroupings->setTabText(index, aName);
   }
}

void Chat::DockWidget::InsertGroup(Group* aGroup)
{
   const int index = std::max(0, mGroupings->count() - 1);
   mGroupings->insertTab(index, aGroup, aGroup->objectName());
   mGroupings->setCurrentIndex(index);
   mChannelLabel->setText(aGroup->GetActiveChannelLabel());
   connect(aGroup, &Group::TextAdded, this, &DockWidget::NotifyGroup);
   connect(aGroup, &Group::ChannelChanged, this, &DockWidget::GroupChangedChannel);
}

bool Chat::DockWidget::TakeGroup(Group* aGroup)
{
   const int index = mGroupings->indexOf(aGroup);
   if (index >= 0)
   {
      mGroupings->removeTab(index);
      aGroup->disconnect();

      if (mGroupings->count() == 0)
      {
         deleteLater();
      }

      return true;
   }
   return false;
}

Chat::Group* Chat::DockWidget::GetActiveGroup() const
{
   return dynamic_cast<Group*>(mGroupings->currentWidget());
}

Chat::Group* Chat::DockWidget::GetGroup(int aIndex) const
{
   return dynamic_cast<Group*>(mGroupings->widget(aIndex));
}

void Chat::DockWidget::OnReturnPressed()
{
   QString input = mLineEdit->text().trimmed();
   mLineEdit->clear();
   auto* group = GetActiveGroup();
   if (group && !input.isEmpty())
   {
      emit UserInput(group, input);
   }
}

void Chat::DockWidget::Build()
{
   // build the ui
   auto baseWidget = new QWidget(this);
   baseWidget->setLayout(new QVBoxLayout(baseWidget));
   mGroupings = new TabWidget(baseWidget, mId);
   connect(mGroupings, &Chat::TabWidget::MoveTab, this, &Chat::DockWidget::MoveGrouping);
   connect(mGroupings, &QTabWidget::tabBarClicked, this, &Chat::DockWidget::TabClicked);
   connect(mGroupings, &QTabWidget::tabBarDoubleClicked, this, &Chat::DockWidget::TabDoubleClicked);
   mGroupings->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(mGroupings->tabBar(), &QTabBar::customContextMenuRequested, this, &Chat::DockWidget::TabBarContextMenu);
   mGroupings->tabBar()->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);
   mGroupings->setMovable(true);
   baseWidget->layout()->addWidget(mGroupings);
   auto hbox = new QHBoxLayout();
   baseWidget->layout()->addItem(hbox);
   hbox->setMargin(0);
   mChannelLabel = new QLabel("No active channel:", baseWidget);
   hbox->addWidget(mChannelLabel);
   mLineEdit = new CachingLineEdit(baseWidget);
   hbox->addWidget(mLineEdit);
   auto helpButton = new QPushButton(baseWidget);
   helpButton->setIcon(QIcon::fromTheme("help"));
   connect(helpButton, &QPushButton::clicked, this, &DockWidget::HelpClicked);
   hbox->addWidget(helpButton);
   connect(mLineEdit, &QLineEdit::returnPressed, this, &DockWidget::OnReturnPressed);
   setWidget(baseWidget);

   if (!mGroupLocked && mId == 0)
   {
      mGroupings->addTab(new QWidget(this), "+");
   }
}

void Chat::DockWidget::MoveGrouping(TabWidget* aSourcePtr, int aIndex)
{
   auto* group = dynamic_cast<Group*>(aSourcePtr->widget(aIndex));
   auto* dock  = dynamic_cast<DockWidget*>(aSourcePtr->parentWidget()->parentWidget());
   if (group && dock && dock->TakeGroup(group))
   {
      InsertGroup(group);
   }
}

void Chat::DockWidget::SetId(int aId)
{
   mId = aId;
   mGroupings->SetId(aId);

   if (mId == 0)
   {
      setObjectName("Chat");
      setWindowTitle("Chat");
   }
   else
   {
      setObjectName("Chat " + QString::number(aId + 1));
      setWindowTitle("Chat " + QString::number(aId + 1));
   }
}

void Chat::DockWidget::NotifyGroup(Group* aGroup)
{
   if (mGroupings != nullptr)
   {
      if (mGroupings->currentWidget() != aGroup)
      {
         for (int i = 0; i < mGroupings->count(); i++)
         {
            if (mGroupings->widget(i) == aGroup)
            {
               QString tabText = mGroupings->tabText(i);
               if (!tabText.endsWith("(*)"))
               {
                  mGroupings->setTabText(i, tabText + "(*)");
               }
            }
         }
      }
   }
}

void Chat::DockWidget::GroupChangedChannel(Group* aGroup)
{
   if (aGroup == mGroupings->currentWidget())
   {
      mChannelLabel->setText(aGroup->GetActiveChannelLabel());
   }
}

void Chat::DockWidget::TabClicked(int aIndex)
{
   if (mGroupings->tabText(aIndex) == "+")
   {
      emit AddGroupClicked(this);
   }
   else if (QApplication::mouseButtons() & Qt::LeftButton)
   {
      mGroupings->setCurrentIndex(aIndex);
      QString tabText = mGroupings->tabText(aIndex);
      if (tabText.endsWith("(*)"))
      {
         mGroupings->setTabText(aIndex, tabText.left(tabText.length() - 3));
      }
   }

   auto* group = GetActiveGroup();
   if (group)
   {
      mChannelLabel->setText(group->GetActiveChannelLabel());
   }
   else
   {
      mChannelLabel->setText("No active channel:");
   }
}

void Chat::DockWidget::TabDoubleClicked(int aIndex)
{
   QString name = mGroupings->tabText(aIndex).replace("(*)", "");
   if (name != "+")
   {
      Group* group = GetGroup(aIndex);
      if (group)
      {
         // pop up a line edit to change the name
         QDialog quickLine(this, Qt::Popup);
         quickLine.setLayout(new QVBoxLayout(&quickLine));
         quickLine.layout()->setMargin(0);
         auto lineEdit = new QLineEdit(&quickLine);
         lineEdit->setText(name);
         lineEdit->selectAll();
         quickLine.layout()->addWidget(lineEdit);
         connect(lineEdit, &QLineEdit::returnPressed, &quickLine, &QDialog::accept);
         lineEdit->setFocus();
         quickLine.move(mapToGlobal(mGroupings->tabBar()->tabRect(aIndex).bottomLeft()));
         quickLine.exec();
         if (!lineEdit->text().isEmpty() && lineEdit->text() != "+")
         {
            emit UserRenamedGroup(this, group, lineEdit->text());
         }
      }
   }
}

void Chat::DockWidget::TabBarContextMenu(QPoint aPos)
{
   int idx = mGroupings->tabBar()->tabAt(aPos);
   if (idx != -1)
   {
      QString name = mGroupings->tabText(idx).replace("(*)", "");
      if (name != "+")
      {
         QMenu    menu(this);
         QAction* deleteTabPtr = menu.addAction("Delete Group: " + name);
         QAction* undockTabPtr = nullptr;
         if (mGroupings->count() > 1)
         {
            undockTabPtr = menu.addAction("Undock Group: " + name);
         }
         QAction* actionPtr = menu.exec(mGroupings->tabBar()->mapToGlobal(aPos));
         if (actionPtr == deleteTabPtr)
         {
            emit DeleteGroupClicked(this, GetGroup(idx));
         }
         else if (actionPtr == undockTabPtr)
         {
            emit UndockGroupClicked(this, GetGroup(idx));
         }
      }
   }
}

void Chat::DockWidget::HelpClicked()
{
   Group* grp = GetActiveGroup();
   if (grp)
   {
      grp->WriteHelp();
   }
}

void Chat::CachingLineEdit::focusOutEvent(QFocusEvent* aEvent)
{
   if (aEvent->reason() == Qt::TabFocusReason)
   {
      setFocus();
   }
   else
   {
      QLineEdit::focusOutEvent(aEvent);
   }
}

void Chat::CachingLineEdit::keyPressEvent(QKeyEvent* aEvent)
{
   if (aEvent->key() == Qt::Key_Up)
   {
      if (!mCache.empty())
      {
         mCacheIdx++;
         if (mCacheIdx >= mCache.size())
         {
            mCacheIdx = mCache.size() - 1;
         }
         setText(mCache[mCacheIdx]);
      }
   }
   else if (aEvent->key() == Qt::Key_Down)
   {
      if (mCacheIdx > 0)
      {
         mCacheIdx--;
         if (mCacheIdx < 0)
         {
            mCacheIdx = 0;
         }
         setText(mCache[mCacheIdx]);
      }
   }
   else if ((aEvent->key() == Qt::Key_Return) || (aEvent->key() == Qt::Key_Enter))
   {
      mCacheIdx = -1;
      if (!text().isEmpty())
      {
         mCache.push_front(text());
         if (mCache.size() > 10)
         {
            mCache.pop_back();
         }
      }
   }
   else
   {
      mCacheIdx = -1;
   }
   QLineEdit::keyPressEvent(aEvent);
}
