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

#ifndef EDITOR_HELPERS_HPP
#define EDITOR_HELPERS_HPP

#include <sstream>

#include <QString>
#include <QStringList>
#include <QTextCursor>

// Util Includes
#include "UtCompilerAttributes.hpp"
class UtTextDocumentRange;

// WKF Includes
#include "WkfEnvironment.hpp"
#include "WkfUnitsObject.hpp"

// WSF Includes
#include "WsfPProxyNode.hpp"

// Wizard Includes
#include "Editor.hpp"
#include "ViExport.hpp"

namespace vespa
{

class VaPosition;

}

namespace wizard
{

class Editor;

namespace EditorHelpers
{

//! Replaces the value of the given attribute with a new value or removes the attribute
//! @param aEntryRange the range that is to be modified
//! @param aAttributeName the name of the attribute
//! @param aInsert the text to insert
//! @returns a string representing the given range after the modification
QString VI_EXPORT GetInsertWithComments(const UtTextDocumentRange& aEntryRange,
                                        const QString&             aAttributeName,
                                        const QString&             aInsert);

//! Modifies the given range by replacing the given attribute's value or removing the attribute
//! @param aEntryRange the range that is to be modified
//! @param aAttributeName the name of the attribute
//! @param aInsert the text to insert
//! @pre The callee made a call to wizEnv.BeginUndoCapture() BEFORE calling this function.
//! @return whether the text replacement was a success
CPP17_NODISCARD bool VI_EXPORT ReplaceTextInEditor(const UtTextDocumentRange& aEntryRange,
                                                   const QString&             aAttributeName,
                                                   const QString&             aInsert);

//! Get the cursor pointing to the start of the definition
//! @param aNode is the proxy node containing the definition
//! @return the cursor pointing to the start of the definition
QTextCursor VI_EXPORT FindStartCursor(const WsfPProxyNode& aNode) noexcept;

//! Get the cursor pointing to the end of the definition
//! @param aNode is the proxy node containing the definition
//! @return the cursor pointing to the end of the definition
QTextCursor VI_EXPORT FindEndCursor(const WsfPProxyNode& aNode) noexcept;

//! Gets the level of indent for the given line
//! @param aLine is the line of text
//! @return the level of indent for the given line, or -1 if the line is entirely whitespace
int VI_EXPORT GetIndentLevel(const QString& aLine) noexcept;

//! Constructs the new text to append to the AFSIM input block designated by the given QTextCursors
//! @param aStartCursor is the cursor pointing to the start of the AFSIM input block
//! @param aEndCursor is the cursor pointing to the end of the AFSIM input block
//! @param aAttributeName is the name of the attribute to append
//! @param aDataValue is the value of the attribute to append
QString VI_EXPORT ConstructTextToAppendToBlock(const QTextCursor& aStartCursor,
                                               const QTextCursor& aEndCursor,
                                               const QString&     aAttributeName,
                                               const QString&     aDataValue) noexcept;

//! Add a new attribute in the text
//! @param aProxyNode contains the context for the new attribute
//! @param aAttributeName is the name of the new attribute
//! @param aDataValue is the value of the new attribute
void VI_EXPORT AddNewAttributeInText(const WsfPProxyNode& aProxyNode,
                                     const QString&       aAttributeName,
                                     const QString&       aDataValue) noexcept;

//! Moves the QTextCursor to the beginning of the previous non-whitespace token
//! @param aCursor is the QTextCursor (a return value parameter)
//! @param aMoveMode is whether to move or keep the anchor
//! @param aOperationCount is the number of times to move the cursor
//! @return whether the operations were a success (to mimic the behavior of QTextCursor::movePosition)
bool VI_EXPORT MoveToPreviousNonWhitespaceToken(QTextCursor&          aCursor,
                                                QTextCursor::MoveMode aMoveMode,
                                                int                   aOperationCount = 1) noexcept;

//! Determines whether the given string is composed of only whitespace characters
//! @param aText is the input text
//! @return whether the given string is composed of only whitespace characters
bool VI_EXPORT IsWhitespaceString(const QString& aText) noexcept;

//! Replaces the position's old latitude and longitude text with new latitude and longitude text
//! @param aPositionTokens are the tokens from the "position" (or similar) UtTextDocumentRange
//! @param aCommandIndex is the index in aPositionTokens of the "position" (or similar) command
//! @param aNewPosition is the new position whose latitude and longitude text will replace the old latitude and longitude text
bool VI_EXPORT ReplacePositionText(QStringList&             aPositionTokens,
                                   int                      aCommandIndex,
                                   const vespa::VaPosition& aNewPosition) noexcept;

//! Replaces the old MGRS coordinate text with the new MGRS coordinate text
//! @param aMGRS_Tokens are the tokens from the "mgrs_coordinate" (or similar) UtTextDocumentRange
//! @param aCommandIndex is the index in aMGRS_Tokens of the "mgrs_coordinate" (or similar) command
//! @param aNewPosition is the new position whose MGRS coordinate text will replace the old MGRS coordinate text
bool VI_EXPORT ReplaceMGRS_Text(QStringList& aMGRS_Tokens, int aCommandIndex, const vespa::VaPosition& aNewPosition) noexcept;

//! Replaces the old unitary property with the new unitary property
//! @param aUnitaryTokens are the tokens from the unitary property's UtTextDocumentRange
//! @param aCommandIndex is the index in aUnitaryTokens of the unitary property's command
//! @param aNewUnitary is the new unitary property whose text will replace the old unitary property's text
template<typename UNITARY_PROPERTY>
bool ReplaceUnitaryText(QStringList& aUnitaryTokens, int aCommandIndex, UNITARY_PROPERTY& aNewUnitary) noexcept
{
   static_assert(std::is_base_of<UtUnitaryValue<typename UNITARY_PROPERTY::UnitType>, UNITARY_PROPERTY>::value,
                 "UNITARY_PROPERTY must derive from UtUnitaryValue");

   // Read the unitary property's value
   int valueIndex{wizard::Editor::FindNextValidToken(aUnitaryTokens, aCommandIndex + 1)};
   if (valueIndex >= 0)
   {
      // Read the unitary property's unit
      int unitIndex{wizard::Editor::FindNextValidToken(aUnitaryTokens, valueIndex + 1)};
      if (unitIndex >= 0)
      {
         // Edit the unitary property
         QString     unitaryValueStr;
         std::string unitaryUnitStr{aUnitaryTokens[unitIndex].toStdString()};
         if (UtUnitAngle::IsUnitValid(unitaryUnitStr))
         {
            // Convert the unitary property to the preferred unit
            if (aNewUnitary.ConvertToUnit(unitaryUnitStr))
            {
               // Convert the unitary property to a string
               std::ostringstream oss;
               oss << aNewUnitary;
               unitaryValueStr = QString::fromStdString(oss.str());
               // Split the unitary property and unit
               QStringList unitaryValueStrSplit{unitaryValueStr.split(' ')};
               if (unitaryValueStrSplit.size() == 2)
               {
                  // Store the unitary property value
                  unitaryValueStr = unitaryValueStrSplit[0];
               }
            }
         }

         // Replace the old unitary property with the new unitary property
         aUnitaryTokens[valueIndex] = unitaryValueStr;
         // The replacement succeeded
         return true;
      }
   }
   // The replacement failed
   return false;
}

//! Inserts the given QString into the given wizard::Editor at the given QTextCursor and resets the QTextCursor's
//! position to the start of the block
//! @param aEditorPtr is the wizard::Editor in which to add the text
//! @param aCursor is the QTextCursor pointing to the location to insert the text
//! @param aTextToInsert is the text to insert
void VI_EXPORT InsertTextAndResetCursor(wizard::Editor* aEditorPtr, QTextCursor& aCursor, const QString& aTextToInsert) noexcept;

//! Converts a vespa::VaPosition to a QString
//! @param aPosition is the vespa::VaPosition to convert to a QString
//! @return the vespa::VaPosition as a QString
//! @note the user preferences determine the format of the position's latitude and longitude
QString VI_EXPORT GetPositionString(const vespa::VaPosition& aPosition) noexcept;

//! Converts a UtUnitaryValue to a QString
//! @param aUnitary is the UtUnitaryValue to convert to a QString
//! @return the UtUnitaryValue as a QString
//! @note the user preferences determine the unit of the unitary property's value
template<wkf::ValueType UNITARY_TYPE, typename UNITARY_PROPERTY>
QString GetUnitaryString(UNITARY_PROPERTY& aUnitary) noexcept
{
   static_assert(std::is_base_of<UtUnitaryValue<typename UNITARY_PROPERTY::UnitType>, UNITARY_PROPERTY>::value,
                 "UNITARY_PROPERTY must derive from UtUnitaryValue");

   // Get the unitary property's unit preference
   int unitaryUnit{wkfEnv.GetPreferenceObject<wkf::UnitsObject>()->GetUnit(UNITARY_TYPE)};
   // Convert the unitary property to the preferred unit
   aUnitary.ConvertToUnit(unitaryUnit);
   // Convert the unitary property to a string
   std::ostringstream oss;
   oss << aUnitary;
   // The unitary property string contains the value AND unit strings delimited by a single ' ' character
   return QString::fromStdString(oss.str());
}

} // namespace EditorHelpers
} // namespace wizard

#endif // EDITOR_HELPERS_HPP
