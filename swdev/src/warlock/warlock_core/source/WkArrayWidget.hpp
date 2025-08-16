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

#ifndef WK_ARRAY_WIDGET_HPP
#define WK_ARRAY_WIDGET_HPP

#include "warlock_core_export.h"

#include <QListWidget>
#include <QWidget>

#include "WkScriptTypes.hpp"
#include "WkfSelectorWidgets.hpp"

namespace warlock
{
class WARLOCK_CORE_EXPORT ArrayWidget : public QWidget
{
   Q_OBJECT

public:
   ArrayWidget(QWidget* aParent = nullptr, warlock::script::DataType aType = warlock::script::cUNKNOWN);
   QVector<QVariant> GetArray() const;
   void              Populate(const QVector<QVariant>& aArray);

signals:
   void CancelSelection();

private:
   void     AddItem(const QString& aString = QString());
   QVariant DefaultValue(warlock::script::DataType) const;
   void     ListItemChangedHandler(QListWidgetItem* aItem);
   void     DeleteHandler();

   template<class SELECTOR_TYPE, typename... Args>
   QWidget* CreateSelectorWidget(const std::function<QString(Args...)>& aToString);

   warlock::script::DataType mType;
   QListWidget*              mListWidget; // array entries
};
} // namespace warlock

template<class SELECTOR_TYPE, typename... Args>
QWidget* warlock::ArrayWidget::CreateSelectorWidget(const std::function<QString(Args...)>& aToString)
{
   static_assert(std::is_base_of<wkf::SelectorWidgetBase, SELECTOR_TYPE>::value,
                 "Template argument to AddSelectorWidget must derive from SelectorWidgetBase");

   auto* selector = new SELECTOR_TYPE;
   selector->SetMultiSelectionEnabled(true);

   connect(this, &ArrayWidget::CancelSelection, selector, &wkf::SelectorWidgetBase::CancelSelection);

   // When an object is selected using the selector widget, append it to
   // the list widget using the provided function to convert to string.
   connect(selector,
           &SELECTOR_TYPE::Selected,
           this,
           [=](Args... args)
           {
              if (mListWidget->count() > 0) // should always be true
              {
                 mListWidget->item(mListWidget->count() - 1)->setText(aToString(args...));
              }
           });

   return selector;
}

#endif
