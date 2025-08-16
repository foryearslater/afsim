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

#ifndef PATTERNSTREEVIEW_HPP
#define PATTERNSTREEVIEW_HPP

#include <QTreeView>

namespace PatternVisualizer
{

class PatternsTreeView : public QTreeView
{
   Q_OBJECT
public:
   explicit PatternsTreeView(QWidget* aParentPtr)
      : QTreeView(aParentPtr)
   {
   }
   ~PatternsTreeView() override = default;

signals:
   void CurrentIndexChanged();

protected slots:
   void currentChanged(const QModelIndex& aCurrent, const QModelIndex& aPrevious) override;
};
} // namespace PatternVisualizer

#endif // PATTERNSTREEVIEW_HPP
