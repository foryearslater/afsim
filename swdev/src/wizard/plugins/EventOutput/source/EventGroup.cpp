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

#include "EventGroup.hpp"

#include <QDesktopServices>
#include <QFileInfo>
#include <QMenu>
#include <QUrl>

#include "CommandValuePair.hpp"
#include "ParseResults.hpp"
#include "PluginUtil.hpp"
#include "Project.hpp"
#include "Tokenizer.hpp"
#include "UtQtMemory.hpp"
#include "WkfAction.hpp"
#include "WkfEnvironment.hpp"
#include "WsfExe.hpp"
#include "WsfParser.hpp"

namespace wizard
{
namespace EventOutput
{
namespace event
{

Item::Item(const QString& aLabel,
           const QString& aFalseValue /* = "false"   */,
           const QString& aTrueValue /* = "true"    */,
           const QString& aDefaultValue /* = "default" */,
           QWidget*       aParent /* = nullptr   */)
   : QWidget(aParent)
   , cDEFAULT_VALUE(aDefaultValue)
   , cFALSE_VALUE(aFalseValue)
   , cTRUE_VALUE(aTrueValue)
{
   // set up the UI
   mUi.setupUi(this);
   mUi.mCommand->setText(aLabel);
   mUi.mDefault->setText(cDEFAULT_VALUE);
   mUi.mFalse->setText(cFALSE_VALUE);
   mUi.mTrue->setText(cTRUE_VALUE);

   // set up the tooltips because they are important...
   mUi.mCommand->setToolTip("The name of the event command (" + mUi.mCommand->text() + ')');
   mUi.mDefault->setToolTip("Do not write the event command out to the block.");
   mUi.mFalse->setToolTip("Write 'disable " + mUi.mCommand->text() + "' out to the block.");
   mUi.mTrue->setToolTip("Write 'enable " + mUi.mCommand->text() + "' out to the block.");

   // do the necessary connections
   connect(mUi.mDefault, &QAbstractButton::clicked, this, &Item::OnDefault);
   connect(mUi.mFalse, &QAbstractButton::clicked, this, &Item::OnFalse);
   connect(mUi.mTrue, &QAbstractButton::clicked, this, &Item::OnTrue);
}

void Item::ReadEvent(const editor::tokenizer::Tokenizer* aRange)
{
   // retrieve the appropriate command-value pair
   const auto* eventToken = aRange->GetEvent(mUi.mCommand->text());
   // read the value if the command-value pair exists and is valid
   if (eventToken && eventToken->IsValid())
   {
      if (eventToken->GetValue() == cFALSE_VALUE)
      {
         OnFalse(true);
      }
      else if (eventToken->GetValue() == cTRUE_VALUE)
      {
         OnTrue(true);
      }
      else
      {
         OnDefault(true);
      }
   }
}

void Item::WriteEvent(editor::tokenizer::Tokenizer* aRange)
{
   // retrieve the appropriate command-value pair
   const auto* eventToken = aRange->GetEvent(mUi.mCommand->text());
   // check for existence and validity of the command-value pair
   bool validToken{eventToken && eventToken->IsValid()};
   // the event warrants writing to the block
   if (mState == Value::cFALSE || mState == Value::cTRUE)
   {
      // the command-value pair is already in the block, but needs updating
      if (validToken)
      {
         // copy the command-value pair
         auto oldEvent = *static_cast<const editor::token::EventCVP*>(eventToken);
         // and update its value
         if (mUi.mTrue->isChecked())
         {
            oldEvent.SetValue(cTRUE_VALUE);
         }
         else if (mUi.mFalse->isChecked())
         {
            oldEvent.SetValue(cFALSE_VALUE);
         }
         // then, update the range
         aRange->SetEvent(oldEvent.GetCommand(), oldEvent.GetValue());
      }
      // the event warrants insertion into the block
      else
      {
         // create a new command-value pair
         editor::token::EventCVP newEvent;
         newEvent.SetCommand(mUi.mCommand->text());
         if (mUi.mTrue->isChecked())
         {
            newEvent.SetValue(cTRUE_VALUE);
         }
         else if (mUi.mFalse->isChecked())
         {
            newEvent.SetValue(cFALSE_VALUE);
         }
         // insert the command-value pair into the block
         aRange->InsertEvent(newEvent.GetCommand(), newEvent.GetValue());
      }
   }
   // the event warrants removal from the block
   else if (mState == Value::cDEFAULT && validToken)
   {
      aRange->RemoveEvent(eventToken->GetCommand());
   }
}

void Item::RestoreDefault()
{
   mUi.mDefault->setChecked(true);
   mUi.mFalse->setChecked(false);
   mUi.mTrue->setChecked(false);
   mState = Value::cDEFAULT;
}

void Item::SetState(Value aState)
{
   QSignalBlocker block{this};
   switch (aState)
   {
   case Value::cFALSE:
      OnFalse(true);
      break;
   case Value::cTRUE:
      OnTrue(true);
      break;
   case Value::cDEFAULT:
      OnDefault(true);
      break;
   default:
      break;
   }
}

QString Item::GetStateString() const
{
   switch (mState)
   {
   case Value::cDEFAULT:
      return cDEFAULT_VALUE;
   case Value::cFALSE:
      return cFALSE_VALUE;
   case Value::cTRUE:
      return cTRUE_VALUE;
   default:
      return "";
   }
}

void Item::OnDefault(bool aChecked)
{
   QSignalBlocker defaultBlock{mUi.mDefault};
   mUi.mDefault->setChecked(true);
   if (aChecked)
   {
      // uncheck the false value and true value
      QSignalBlocker falseBlock{mUi.mFalse};
      mUi.mFalse->setChecked(false);
      QSignalBlocker trueBlock{mUi.mTrue};
      mUi.mTrue->setChecked(false);
      mState = Value::cDEFAULT;
      emit StateChanged(mState);
   }
}

void Item::OnFalse(bool aChecked)
{
   QSignalBlocker falseBlock{mUi.mFalse};
   mUi.mFalse->setChecked(true);
   if (aChecked)
   {
      // uncheck the default value and true value
      QSignalBlocker defaultBlock{mUi.mDefault};
      mUi.mDefault->setChecked(false);
      QSignalBlocker trueBlock{mUi.mTrue};
      mUi.mTrue->setChecked(false);
      mState = Value::cFALSE;
      emit StateChanged(mState);
   }
}

void Item::OnTrue(bool aChecked)
{
   QSignalBlocker trueBlock{mUi.mTrue};
   mUi.mTrue->setChecked(true);
   if (aChecked)
   {
      // uncheck the default value and false value
      QSignalBlocker defaultBlock{mUi.mDefault};
      mUi.mDefault->setChecked(false);
      QSignalBlocker falseBlock{mUi.mFalse};
      mUi.mFalse->setChecked(false);
      mState = Value::cTRUE;
      emit StateChanged(mState);
   }
}

void Item::Disable()
{
   mUi.mCommand->setDisabled(true);
   mUi.mDefault->setDisabled(true);
   mUi.mFalse->setDisabled(true);
   mUi.mTrue->setDisabled(true);
   mEnabled = false;
}

void Item::Enable()
{
   mUi.mCommand->setEnabled(true);
   mUi.mDefault->setEnabled(true);
   mUi.mFalse->setEnabled(true);
   mUi.mTrue->setEnabled(true);
   mEnabled = true;
}

Group* Group::DataToGroup(const GroupData& aData, QWidget* aParent /* = nullptr */)
{
   auto group = ut::qt::make_qt_ptr<Group>(aData.mTitle, aParent);
   group->AddCheckBoxes(aData.mItems, "disable", "enable");
   for (const auto& elem : aData.mGroups)
   {
      group->AddGroupBox(DataToGroup(elem, group));
   }
   return group;
}

Group::Group(QWidget* aParent /* = nullptr */)
   : QWidget(aParent)
   , mTitle(ut::qt::make_qt_ptr<Item>("<title>", "disable all", "enable all", "Default", this))
   , mSeparator(ut::qt::make_qt_ptr<QFrame>(this))
{
   // set up the horizontal rule
   mSeparator->setFrameShape(QFrame::HLine);
   mSeparator->setFrameShadow(QFrame::Sunken);

   // do the necessary connections
   connect(mTitle.data(),
           &Item::StateChanged,
           [this](Value aState)
           {
              if (mModifyStateOfChildren)
              {
                 CheckStateAll(aState);
              }
              emit ModifiedByUser();
           });
}

Group::Group(const QString& aTitle, QWidget* aParent /* = nullptr */)
   : QWidget(aParent)
   , mTitle(ut::qt::make_qt_ptr<Item>(aTitle, "disable all", "enable all", "Default", this))
   , mSeparator(ut::qt::make_qt_ptr<QFrame>(this))
{
   // set up the horizontal rule
   mSeparator->setFrameShape(QFrame::HLine);
   mSeparator->setFrameShadow(QFrame::Sunken);

   // do the necessary connections
   connect(mTitle.data(),
           &Item::StateChanged,
           [this](Value aState)
           {
              if (mModifyStateOfChildren)
              {
                 CheckStateAll(aState);
              }
              emit ModifiedByUser();
           });
}

Group::Group(const QString&     aLabel,
             const QStringList& aChildLabels,
             const QString&     aFalseValue /* = "false" */,
             const QString&     aTrueValue /* = "true"  */,
             QWidget*           aParent /* = nullptr */)
   : QWidget(aParent)
{
   if (!aLabel.isEmpty())
   {
      mTitle = ut::qt::make_qt_ptr<Item>(aLabel, "disable all", "enable all", "Default", this);
      // set up the horizontal rule
      mSeparator->setFrameShape(QFrame::HLine);
      mSeparator->setFrameShadow(QFrame::Sunken);
      // create the child check boxes
      AddCheckBoxes(aChildLabels, aFalseValue, aTrueValue);
   }
   // do the necessary connections
   connect(mTitle.data(),
           &Item::StateChanged,
           [this](Value aState)
           {
              if (mModifyStateOfChildren)
              {
                 CheckStateAll(aState);
              }
              emit ModifiedByUser();
           });
}

void Group::AddCheckBox(const QString& aLabel,
                        const QString& aFalseValue /* = "false" */,
                        const QString& aTrueValue /* = "true"  */)
{
   auto eventBox = ut::qt::make_qt_ptr<Item>(aLabel, aFalseValue, aTrueValue, "Default", this);
   // do the necessary connections
   eventBox->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(eventBox,
           &Item::customContextMenuRequested,
           [this, eventBox](const QPoint& aPos) { OnEventContextMenu(aPos, eventBox); });
   connect(eventBox,
           &Item::StateChanged,
           [this](Value aState)
           {
              if (mModifyStateOfParent)
              {
                 QSignalBlocker titleBlock{mTitle};
                 mTitle->SetState(Value::cDEFAULT);
              }
              emit ModifiedByUser();
           });
   mEventList.emplace_back(eventBox);
}

void Group::AddCheckBoxes(const QStringList& aChildLabels,
                          const QString&     aFalseValue /* = "false" */,
                          const QString&     aTrueValue /* = "true"  */)
{
   if (!aChildLabels.isEmpty())
   {
      for (const auto& elem : aChildLabels)
      {
         AddCheckBox(elem, aFalseValue, aTrueValue);
      }
   }
}

void Group::AddGroupBox(Group* aCBG)
{
   mGroupList.emplace_back(aCBG);
}

void Group::AddGroupBox(const QString&     aLabel,
                        const QStringList& aChildLabels,
                        const QString&     aFalseValue /* = "false" */,
                        const QString&     aTrueValue /* = "true"  */)
{
   if (!aLabel.isEmpty())
   {
      auto groupBox = ut::qt::make_qt_ptr<Group>(aLabel, this);
      groupBox->AddCheckBoxes(aChildLabels, aFalseValue, aTrueValue);
      mGroupList.emplace_back(groupBox);
   }
}

void Group::AddGroupBox(const QString& aLabel, const QList<Group*>& aChildList)
{
   auto groupBox = ut::qt::make_qt_ptr<Group>(aLabel, this);
   for (const auto& elem : aChildList)
   {
      groupBox->AddGroupBox(elem);
   }
   mGroupList.emplace_back(groupBox);
}

QStringList Group::GetEvents() const
{
   QStringList events;
   for (const auto& elem : mEventList)
   {
      events.append(elem->GetCommand());
   }
   return events;
}

void Group::SetState(Value aState)
{
   QSignalBlocker titleBlock{mTitle};
   mTitle->SetState(aState);
   if (mModifyStateOfChildren)
   {
      CheckStateAll(aState);
   }
}

void Group::CheckStateAll(Value aState)
{
   if (!mEventList.empty())
   {
      for (auto& elem : mEventList)
      {
         elem->SetState(aState);
      }
   }
   if (!mGroupList.empty())
   {
      for (auto& elem : mGroupList)
      {
         elem->CheckStateAll(aState);
      }
   }
}

void Group::ReadEvents(const editor::tokenizer::Tokenizer* aRange)
{
   if (!mEventList.empty())
   {
      for (auto& event : mEventList)
      {
         event->ReadEvent(aRange);
      }
   }
   if (!mGroupList.empty())
   {
      for (auto& group : mGroupList)
      {
         group->ReadEvents(aRange);
      }
   }
}

void Group::WriteEvents(editor::tokenizer::Tokenizer* aRange)
{
   if (!mEventList.empty())
   {
      for (auto& event : mEventList)
      {
         event->WriteEvent(aRange);
      }
   }
   if (!mGroupList.empty())
   {
      for (auto& group : mGroupList)
      {
         group->WriteEvents(aRange);
      }
   }
}

void Group::RestoreDefaults()
{
   mTitle->RestoreDefault();
   if (!mEventList.empty())
   {
      for (auto& event : mEventList)
      {
         event->RestoreDefault();
      }
   }
   if (!mGroupList.empty())
   {
      for (auto& group : mGroupList)
      {
         group->RestoreDefaults();
      }
   }
}

void Group::LayoutWidgets()
{
   mGroup = ut::qt::make_qt_ptr<QGroupBox>(this);
   // give the group box a list-like layout
   auto groupLayout = ut::qt::make_qt_ptr<QVBoxLayout>(this);
   mGroup->setLayout(groupLayout);
   // put the group box in a layout
   mLayout = ut::qt::make_qt_ptr<QVBoxLayout>(this);
   mLayout->addWidget(mGroup);
   mLayout->setContentsMargins(0, 0, 0, 0);
   mLayout->setSpacing(0);
   setLayout(mLayout);
   // add the parent Item to the group box
   if (mTitle)
   {
      groupLayout->addWidget(mTitle);
   }
   // separate the group name from the events
   if (mSeparator)
   {
      groupLayout->addWidget(mSeparator);
   }
   // add the child RadioButtonWidgets to the group box
   if (!mEventList.empty())
   {
      for (auto& elem : mEventList)
      {
         groupLayout->addWidget(elem);
      }
   }
   // lay out the CheckBoxGroups in the group box
   if (!mGroupList.empty())
   {
      for (auto& elem : mGroupList)
      {
         elem->LayoutWidgets();
         groupLayout->addWidget(elem);
      }
   }
}

void Group::Disable()
{
   if (mTitle)
   {
      mTitle->Disable();
   }
   if (!mEventList.empty())
   {
      for (auto& elem : mEventList)
      {
         elem->Disable();
      }
   }
   if (!mGroupList.empty())
   {
      for (auto& elem : mEventList)
      {
         elem->Disable();
      }
   }
   mEnabled = false;
}

void Group::Enable()
{
   if (mTitle)
   {
      mTitle->Enable();
   }
   if (!mEventList.empty())
   {
      for (auto& elem : mEventList)
      {
         elem->Enable();
      }
   }
   if (!mGroupList.empty())
   {
      for (auto& elem : mEventList)
      {
         elem->Enable();
      }
   }
   mEnabled = true;
}

void Group::OnEventContextMenu(const QPoint& aPos, Item* aEvent) const
{
   // Create the context menu
   QMenu docMenu;
   auto  docAction = ut::qt::make_qt_ptr<wkf::Action>(aEvent->GetCommand() + " Command documentation...", &docMenu);
   connect(docAction,
           &QAction::triggered,
           [this, aEvent]()
           {
              QFileInfo docDir{wkfEnv.GetDocumentationDir()};
              QString   docDirPath{docDir.absolutePath()};
              QString   eventDocDir{docDirPath + "/documentation/html/docs/event/"};
              QString   oldFormatName{aEvent->GetCommand().toLower() + ".html"};
              QFileInfo oldEventDocFile{eventDocDir + oldFormatName};
              // Check the old format
              if (!oldEventDocFile.exists())
              {
                 QString eventGroupDocName{GetEventGroup().toLower() + "_events.html"};
                 QString newFormatName{'#' + aEvent->GetCommand().toLower()};
                 // Check the new format
                 QFileInfo newEventDocFile{eventDocDir + eventGroupDocName};
                 if (!newEventDocFile.exists())
                 {
                    // The documentation does not exist, so display a message to the user
                    QMessageBox::information(nullptr,
                                             "Documentation",
                                             "Cannot open documentation for command: " + aEvent->GetCommand());
                 }
                 else
                 {
                    // Attempt to open the documentation using the new format
                    auto eventGroupFileUrl = QUrl::fromLocalFile(newEventDocFile.absoluteFilePath());
                    eventGroupFileUrl.setFragment(newFormatName);
                    QDesktopServices::openUrl(eventGroupFileUrl);
                 }
              }
              else
              {
                 // Attempt to open the documentation using the old format
                 QDesktopServices::openUrl(QUrl::fromLocalFile(oldEventDocFile.absoluteFilePath()));
              }
           });
   // Show the context menu
   docMenu.exec(aEvent->mapToGlobal(aPos));
}

} // namespace event
} // namespace EventOutput
} // namespace wizard
