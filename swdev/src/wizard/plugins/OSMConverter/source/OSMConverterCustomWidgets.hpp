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

#ifndef OSMCONVERTERCUSTOMWIDGET_HPP
#define OSMCONVERTERCUSTOMWIDGET_HPP

#include <QDialog>
#include <QLineEdit>

class OSMConverterLineEdit : public QLineEdit
{
   Q_OBJECT
public:
   explicit OSMConverterLineEdit(QWidget* parent = nullptr) {}
signals:
   void DroppedFile();

protected:
   void dragEnterEvent(QDragEnterEvent* event) override;
   void dropEvent(QDropEvent* event) override;
};

#endif
