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

#ifndef PREFWIDGET_HPP
#define PREFWIDGET_HPP

#include "PrefObject.hpp"
#include "WkfPrefWidget.hpp"

class QListWidgetItem;

namespace ProjectBrowser
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   PrefWidget(QWidget* parent = nullptr);

private:
   void showEvent(QShowEvent* aEvent) override;
   void ReadPreferenceData(const PrefData& aPrefData) override;
   void WritePreferenceData(PrefData& aPrefData) override;
};
} // namespace ProjectBrowser

#endif
