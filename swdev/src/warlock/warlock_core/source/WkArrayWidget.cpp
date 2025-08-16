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

#include "WkArrayWidget.hpp"

#include <sstream>

#include <QBoxLayout>
#include <QMessageBox>

#include "UtLLAPos.hpp"
#include "WkfSelectorWidgets.hpp"
#include "WkfTrack.hpp"

warlock::ArrayWidget::ArrayWidget(QWidget* aParent, warlock::script::DataType aType)
   : QWidget(aParent)
   , mType(aType)
   , mListWidget(new QListWidget)
{
   auto* layout = new QHBoxLayout(this);

   AddItem();

   layout->addWidget(mListWidget);

   mListWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

   // There is no edit trigger for "single click". We also need to add the double click trigger,
   // because if the item we are trying to edit IS the current item,
   // the current item will not have changed. The user will need to double click in this case.
   mListWidget->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::DoubleClicked);

   auto* buttonLayout = new QVBoxLayout;

   QPushButton* deleteButton = new QPushButton(QIcon::fromTheme("delete"), {});
   deleteButton->setToolTip("Remove the selected entry");
   connect(deleteButton, &QPushButton::clicked, this, &warlock::ArrayWidget::DeleteHandler);

   buttonLayout->addWidget(deleteButton);
   deleteButton->setEnabled(false);

   switch (mType)
   {
   case warlock::script::cWSF_GEOPOINT:
      mListWidget->setMinimumWidth(200);
      {
         std::function<QString(double, double, double)> f = [](double aLat, double aLon, double aAlt)
         {
            std::stringstream lla;
            lla << UtLLAPos(aLat, aLon, aAlt);
            return QString::fromStdString(lla.str());
         };
         buttonLayout->addWidget(CreateSelectorWidget<wkf::LocationSelector, double, double, double>(f));
      }
      break;
   case warlock::script::cWSF_PLATFORM:
   {
      std::function<QString(wkf::Platform*)> f = [](wkf::Platform* p) { return QString::fromStdString(p->GetName()); };
      buttonLayout->addWidget(CreateSelectorWidget<wkf::PlatformSelector, wkf::Platform*>(f));
   }
   break;
   case warlock::script::cWSF_TRACK:
   {
      std::function<QString(wkf::Track*)> f = [](wkf::Track* t) { return t->GetTrackId().toString(); };
      buttonLayout->addWidget(CreateSelectorWidget<wkf::TrackSelector, wkf::Track*>(f));
   }
   break;
   default:
      break;
   }

   buttonLayout->addStretch();
   layout->addLayout(buttonLayout);

   connect(mListWidget, &QListWidget::itemChanged, this, &warlock::ArrayWidget::ListItemChangedHandler);
   connect(mListWidget,
           &QListWidget::itemSelectionChanged,
           [this, deleteButton]
           {
              // This currently allows the user to delete the last entry which may be have no text, as it is a
              // placeholder.
              //  We could prevent deletion of the placeholder however an empty text string may be a valid input to an
              //  array element. ie:  { "foo", "", "bar" } should be interpretted as an array of size 3 (not 2). Since
              //  an empty string may be valid input, then we must allow users to delete an empty string too.
              deleteButton->setEnabled(!mListWidget->selectedItems().empty());
           });
}

QVector<QVariant> warlock::ArrayWidget::GetArray() const
{
   // construct and return an array from the contents of the list widget
   QVector<QVariant> arr;
   for (int i = 0; i < mListWidget->count(); ++i)
   {
      // For empty list items, set the array element to default if it's not
      // part of a trailing group of empty elements.
      QString text = mListWidget->item(i)->text();
      if (text.isEmpty())
      {
         bool setDefaultValue = false;
         for (int j = i; j < mListWidget->count(); ++j)
         {
            if (!mListWidget->item(j)->text().isEmpty())
            {
               setDefaultValue = true;
               break;
            }
         }
         if (setDefaultValue)
         {
            arr.push_back(DefaultValue(mType));
         }
      }
      else
      {
         if (mType == warlock::script::cWSF_TRACK)
         {
            wkf::TrackId trackId(text);
            if (!trackId.IsNull())
            {
               arr.push_back(QVariant::fromValue(trackId));
            }
            else if (!text.isEmpty())
            {
               QMessageBox::warning(
                  nullptr,
                  "Invalid Track Id",
                  QString("Track Id \"%1\" is not in the expected format <platform name>:<track number>").arg(text));
            }
         }
         else
         {
            arr.push_back(text);
         }
      }
   }
   return arr;
}

void warlock::ArrayWidget::Populate(const QVector<QVariant>& aArray)
{
   mListWidget->clear();

   for (const QVariant& v : aArray)
   {
      AddItem(v.toString());
   }
   AddItem();
}

void warlock::ArrayWidget::AddItem(const QString& aString)
{
   auto* item = new QListWidgetItem(aString);
   item->setFlags(item->flags() | Qt::ItemIsEditable);
   mListWidget->addItem(item);
}

QVariant warlock::ArrayWidget::DefaultValue(warlock::script::DataType aType) const
{
   switch (aType)
   {
   case warlock::script::cINTEGER:
   {
      return QVariant::fromValue(0);
   }
   case warlock::script::cDOUBLE:
   {
      return QVariant::fromValue(0.0);
   }
   case warlock::script::cSTRING:
   {
      return QVariant::fromValue(QString());
   }
   case warlock::script::cBOOL:
   {
      return QVariant::fromValue(false);
   }
   case warlock::script::cWSF_PLATFORM:
   {
      return QVariant::fromValue(QString());
   }
   case warlock::script::cWSF_TRACK:
   {
      return QVariant::fromValue(wkf::TrackId());
   }
   case warlock::script::cWSF_GEOPOINT:
   {
      UtLLAPos          llaPos(0, 0, 0);
      std::stringstream lla;
      lla << llaPos;
      return QVariant::fromValue(QString::fromStdString(lla.str()));
   }
   default:
      return QVariant();
   }
}

void warlock::ArrayWidget::ListItemChangedHandler(QListWidgetItem* aItem)
{
   // if the user finished editing an array element that was the last one in the array,
   // add an empty editable item to the end of the list.
   if (mListWidget->row(aItem) == mListWidget->count() - 1 && !aItem->text().isEmpty())
   {
      AddItem();
   }
}

void warlock::ArrayWidget::DeleteHandler()
{
   int  r    = mListWidget->currentRow();
   bool last = (r == mListWidget->count() - 1);

   delete mListWidget->takeItem(r);

   // Don't allow user to delete all the elements, otherwise there won't be an
   // editor to add points from. There should be at least one item in the list at all times.
   // Don't allow deleting of the last element. We need it there for adding new items.
   if (last)
   {
      AddItem(); // construct a new, empty item
   }
}
