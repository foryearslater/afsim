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

#ifndef PREFWIDGET_HPP
#define PREFWIDGET_HPP

#include "PrefObject.hpp"
#include "WkfPrefWidget.hpp"

namespace PatternVisualizer
{
class PrefWidget : public wkf::PrefWidgetT<PrefObject>
{
public:
   explicit PrefWidget(QWidget* aParent = nullptr);

private:
   void ReadPreferenceData(const PrefData& aPrefData) override{};
   void WritePreferenceData(PrefData& aPrefData) override{};
};
} // namespace PatternVisualizer

#endif // !PREFWIDGET_HPP
