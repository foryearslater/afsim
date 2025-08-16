// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2019 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef COLLAPSIBLEGROUP_HPP
#define COLLAPSIBLEGROUP_HPP

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QSpacerItem>

namespace DemoBrowser
{
class CollapsibleGroup : public QGroupBox
{
public:
   CollapsibleGroup(QWidget* aParent, const QString& aTitle);
   ~CollapsibleGroup() override = default;

   QString Title() const { return mTitlePtr->text(); }
   void    SetTitle(const QString& aTitle) { mTitlePtr->setText(aTitle); }
   void    SetExpanded(bool aState) { ShowContents(aState); }
   void    AddContentWidget(QWidget* aWidgetPtr) { mContentPtr->layout()->addWidget(aWidgetPtr); }
   void    AddHeaderWidget(QWidget* aWidgetPtr) { mHeaderPtr->layout()->addWidget(aWidgetPtr); }
   void    AddContentSpacer()
   {
      mContentPtr->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));
   }

private:
   void ShowContents(bool aState) { mContentPtr->setVisible(aState); }

   QLabel*  mTitlePtr;
   QWidget* mHeaderPtr;
   QWidget* mContentPtr;
};
} // namespace DemoBrowser

#endif
