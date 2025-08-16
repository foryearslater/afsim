// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

#include "ConstellationMakerOptions.hpp"

#include <ostream>
#include <sstream>
#include <string>

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>

#include "UtCast.hpp"
#include "UtUnitTypes.hpp"

namespace
{

template<typename Value>
Value InterpretInputValue(const QJsonObject& aInput, const QString& aField)
{
   Value             retval{};
   auto              iter = aInput.constFind(aField);
   std::stringstream ss{};
   ss << iter->toString().toStdString();
   ss >> retval;
   return retval;
}

bool HasInputString(const QJsonObject& aInput, const QString& aField)
{
   auto iter = aInput.find(aField);
   return (iter != aInput.end() && iter->isString());
}

std::string InterpretInputString(const QJsonObject& aInput, const QString& aField)
{
   return aInput.constFind(aField)->toString().toStdString();
}

double InterpretInputDouble(const QJsonObject& aInput, const QString& aField)
{
   return aInput.constFind(aField)->toString().toDouble();
}

int InterpretInputUnsignedInt(const QJsonObject& aInput, const QString& aField)
{
   return aInput.constFind(aField)->toString().toUInt();
}

WsfConstellationOptions InvalidOptions()
{
   return WsfConstellationOptions::CreateGeneral(0, 0, UtAngleValue{-1.0}, UtAngleValue{-1.0});
}

bool HasAllInputs(const QJsonObject& aInput)
{
   bool retval = HasInputString(aInput, WsfConstellationOptions::cBASE_NAME);
   retval      = retval && HasInputString(aInput, WsfConstellationOptions::cPLATFORM_TYPE);

   bool hasOrbitSize = HasInputString(aInput, WsfConstellationOptions::cALTITUDE) ||
                       HasInputString(aInput, WsfConstellationOptions::cSEMI_MAJOR_AXIS) ||
                       HasInputString(aInput, WsfConstellationOptions::cPERIOD) ||
                       HasInputString(aInput, WsfConstellationOptions::cREVS_PER_DAY);

   retval = retval && hasOrbitSize;
   retval = retval && HasInputString(aInput, WsfConstellationOptions::cINCLINATION);
   retval = retval && HasInputString(aInput, WsfConstellationOptions::cNUM_PLANES);
   retval = retval && HasInputString(aInput, WsfConstellationOptions::cSATS_PER_PLANE);
   retval = retval && HasInputString(aInput, WsfConstellationOptions::cINITIAL_RAAN);
   retval = retval && HasInputString(aInput, WsfConstellationOptions::cRAAN_RANGE);
   retval = retval && HasInputString(aInput, WsfConstellationOptions::cINITIAL_ANOMALY);
   retval = retval && HasInputString(aInput, WsfConstellationOptions::cANOMALY_ALIAS);
   return retval;
}

WsfConstellationOptions InterpretOptionString(const QString& aContents)
{
   WsfConstellationOptions retval = InvalidOptions();
   auto                    input  = QJsonDocument::fromJson(aContents.toLocal8Bit()).object();
   if (!input.isEmpty() && HasAllInputs(input))
   {
      unsigned int p         = InterpretInputUnsignedInt(input, WsfConstellationOptions::cNUM_PLANES);
      unsigned int s         = InterpretInputUnsignedInt(input, WsfConstellationOptions::cSATS_PER_PLANE);
      UtAngleValue anomAlias = InterpretInputValue<UtAngleValue>(input, WsfConstellationOptions::cANOMALY_ALIAS);
      UtAngleValue raanRange = InterpretInputValue<UtAngleValue>(input, WsfConstellationOptions::cRAAN_RANGE);
      retval                 = WsfConstellationOptions::CreateGeneral(p, s, anomAlias, raanRange);

      retval.SetConstellationName(InterpretInputString(input, WsfConstellationOptions::cBASE_NAME));
      retval.SetPlatformType(InterpretInputString(input, WsfConstellationOptions::cPLATFORM_TYPE));
      retval.SetBasePath(InterpretInputString(input, WsfConstellationOptions::cGENERATION_PATH));
      retval.SetInclination(InterpretInputValue<UtAngleValue>(input, WsfConstellationOptions::cINCLINATION));
      retval.SetInitialRAAN(InterpretInputValue<UtAngleValue>(input, WsfConstellationOptions::cINITIAL_RAAN));
      retval.SetInitialAnomaly(InterpretInputValue<UtAngleValue>(input, WsfConstellationOptions::cINITIAL_ANOMALY));

      if (HasInputString(input, WsfConstellationOptions::cSEMI_MAJOR_AXIS))
      {
         retval.SetSemiMajorAxis(InterpretInputValue<UtLengthValue>(input, WsfConstellationOptions::cSEMI_MAJOR_AXIS));
      }
      else if (HasInputString(input, WsfConstellationOptions::cALTITUDE))
      {
         retval.SetCircularAltitude(InterpretInputValue<UtLengthValue>(input, WsfConstellationOptions::cALTITUDE));
      }
      else if (HasInputString(input, WsfConstellationOptions::cPERIOD))
      {
         retval.SetOrbitalPeriod(InterpretInputValue<UtTimeValue>(input, WsfConstellationOptions::cPERIOD));
      }
      else if (HasInputString(input, WsfConstellationOptions::cREVS_PER_DAY))
      {
         retval.SetRevolutionsPerDay(InterpretInputDouble(input, WsfConstellationOptions::cREVS_PER_DAY));
      }
   }
   return retval;
}

} // namespace

//! Returns a set of default options for the Constellation Maker GUI.
WsfConstellationOptions SpaceTools::ConstellationMakerOptions::DefaultConstellationMakerOptions()
{
   WsfConstellationOptions retval = WsfConstellationOptions::CreateGeneral(5,
                                                                           4,
                                                                           UtAngleValue{0.0, UtUnitAngle::cDEGREES},
                                                                           UtAngleValue{360.0, UtUnitAngle::cDEGREES});
   retval.SetRevolutionsPerDay(2.0);
   retval.SetInclination(UtAngleValue{55.0, UtUnitAngle::cDEGREES});
   retval.SetInitialRAAN(UtAngleValue{0.0, UtUnitAngle::cDEGREES});
   retval.SetInitialAnomaly(UtAngleValue{0.0, UtUnitAngle::cDEGREES});
   return retval;
}

//! Interprets a set of constellation options from a document.
//!
//! This method returns a set of constellation options as interpreted from the comment
//! at the top of a file created by the constellation maker. If the document does not
//! contain a comment block with constellation options, or if there is some problem
//! interpreting those comments, this will return an invalid set of options.
//!
//! \param aDocument - The document from which to attempt to interpret constellation options.
//! \returns         - The interpreted options if successful; invalid options otherwise.
WsfConstellationOptions SpaceTools::ConstellationMakerOptions::InterpretOptions(QTextDocument* aDocument)
{
   WsfConstellationOptions retval         = InvalidOptions();
   QString                 contents       = aDocument->toPlainText();
   int                     markerPosition = contents.indexOf(WsfConstellationOptions::cHEADER_START_MARKER, 0);
   if (markerPosition >= 0)
   {
      int openPosition  = contents.indexOf("{", 0);
      int closePosition = contents.indexOf("}", 0);
      if (openPosition >= 0 && closePosition >= 0)
      {
         retval = InterpretOptionString(contents.mid(openPosition, closePosition - openPosition + 1));
      }
   }
   return retval;
}
