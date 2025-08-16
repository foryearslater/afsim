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

#include "PatternsTreeView.hpp"

namespace PatternVisualizer
{
void PatternsTreeView::currentChanged(const QModelIndex& aCurrent, const QModelIndex& aPrevious)
{
   emit CurrentIndexChanged();
   QTreeView::currentChanged(aCurrent, aPrevious);
}
} // namespace PatternVisualizer
