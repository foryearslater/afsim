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

#include "OSMConverterCustomWidgets.hpp"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

void OSMConverterLineEdit::dragEnterEvent(QDragEnterEvent* event)
{
   if (event->mimeData()->hasUrls())
   {
      event->acceptProposedAction();
   }
}

void OSMConverterLineEdit::dropEvent(QDropEvent* event)
{
   if (event->mimeData()->hasUrls())
   {
      auto    urls = event->mimeData()->urls();
      QString text = urls.front().toLocalFile();
      setText(text);
   }
}
