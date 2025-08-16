// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
#ifndef NETWORK_LOG_LIST_DIALOG_HPP
#define NETWORK_LOG_LIST_DIALOG_HPP

#include <QDialog>

#include "NetworkLogModel.hpp"
#include "WkNetwork.hpp"
#include "filter_widget/WkfFilterWidget.hpp"

namespace NetworkLog
{
class ListDialog : public QDialog
{
   Q_OBJECT
public:
   ListDialog(QWidget* aParent, Model& aModel);

signals:
   void PingClicked();

private:
   void     SetupUi();
   QLayout* SetupHeader();

   QWidget* MakePingButton();
   QWidget* MakeClearButton();
   QWidget* MakePreferencesButton();

   void OnRowsAppended();
   void OnColumnsAppended();
   void OnDoubleClicked(const QModelIndex& aIndex);
   void OnReconnect(const warlock::net::SenderInfo& aId);

   Model&                   mModel;
   WkfFilter::FilterWidget* mListWidget = nullptr;

   QLabel* mAddressLabel = nullptr;
   QLabel* mPortLabel    = nullptr;
   QLabel* mPIDLabel     = nullptr;
};
} // namespace NetworkLog

#endif
