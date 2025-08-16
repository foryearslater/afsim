// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************
/*******************************************************************************
*
*   File: TimeUtilities.hpp
*
*   Classification: UNCLASSIFIED
*
*   Details:
*
*   Contract Number:
*   Contractor Name: Radiance Technologies
*   Address:         350 Wynn Drive
*                    Huntsville, AL 35805
*
*   Abstract:
*
*******************************************************************************/

#ifndef SCENARIO_ANALYZER_TIME_UTILITIES_HPP
#define SCENARIO_ANALYZER_TIME_UTILITIES_HPP

#include <QString>
#include <QValidator>
#include <QLabel>
#include <QLineEdit>

class QString;
class QWidget;

namespace ScenarioAnalyzer
{
class TimeValidator : public QValidator
{
   public:
      explicit TimeValidator(QWidget* parent = 0) : QValidator(parent) {}
      ~TimeValidator() {}

      QValidator::State validate(QString& input, int& pos) const;
};

class TimeLabel : public QLabel
{
      Q_OBJECT

   public:
      explicit TimeLabel(QWidget* parent = 0) : QLabel(parent) {}
      ~TimeLabel() {}

      static unsigned int convertTimeToSeconds(const QString& input);

   public slots:
      void setTextAsValidTime(const QString& input);
};

}

#endif