// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2016 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#ifndef TEXTSOURCESIGNALS_HPP
#define TEXTSOURCESIGNALS_HPP

#include <QObject>

namespace wizard
{

class TextSourceSignals : public QObject
{
   Q_OBJECT

public:
   static TextSourceSignals& GetInstance();
   TextSourceSignals(const TextSourceSignals&) = delete;
   TextSourceSignals& operator=(const TextSourceSignals&) = delete;

signals:
   void requestBackup();

private:
   TextSourceSignals() = default;
};

} // namespace wizard

#endif
