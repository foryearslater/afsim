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

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "usmtf_export.h"

#include <memory>
#include <utility>
#include <vector>

#include "Segment.hpp"
#include "Set.hpp"
#include "Validatable.hpp"

/*!+-----------------------------------------------------------------+
| Message Text Composition                            Message Text |
|                                                 Format Structure |
+------------------------------+----------------------------------+
| Introductory Text            | (Specified by Applicable         |
|                              | Communications Procedures)       |
+------------------------------+----------------------------------+
|                              |          EXER/OPER               |
|                              |                                  |
|                              |          MSGID                   |
|                              |                                  |
|                              |          REF                     |
|                              |                                  |
|                              |          --*                     |
|                              |                                  |
|                              |          --                      |
|        Main Text             |          --                      |
|                              |                                  |
|                              |          --                      |
|                              |          --                      |
|                              |         RMKS                     |
|                              |                                  |
|        Closing Text          |         DECL                     |
|                              |                                  |
+-------------------------------+----------------------------------+*/


/*! Structure of a sample hypothetical formatted message
Introductory Text -----> C O N F I D E N T I A L*
+-----------------> EXER/BRAVE SHIELD 85//
|                   MSGID/HELOALREQ/CTG89.05/1225001//
|Linear Set ----->  LIFTREQ/UNITDES:4FLDARTYBNCBTY/REQNO:SAM234/PRY:3A/HELMSN:AIRLF
|                   /NOTPAC:1-CH46//
|                   6LOAD
|                   /DE/ONLOC /ONTIME /QTY /LDTYP /OFFLOC/OFFTIME/QTY
|                   /01/KBSA /260900Z/ 3/PAX-CBT /KBSB /260915Z/ 1
|                   /02/- /- / -/- /FB A /260940Z/ 2
|                   /03/KBSB /260915Z/ 2/FUEL BLADDERS /FB C /261020Z
Main Text           /04/FB A /260940Z/ 1/PALLET /FB C /261020Z
|                   /05/FB C /261020Z/ 3/PAX-CBT /KBSA /261115Z//
|                   TIMEAMP/03/NET/260800Z//
|                   TIMEAMP/05/NLT/261200Z//
|                   7LOAD
|                   /DE/CARGOWT/CARGOSZ/LGTH /WDTH /HEIGHT /EX-IN/H
|Columnar Set ----> /03/10000LB/ 48CUF/ 4FT/ 4FT/ 3FT/EXT /N
|                   /04/ 2000LB/ 60CUF/ 5FT/ 3FT/ 4FT/IN /N//
|                   LANDPT/FB A/PTNM:FIREBASE APPLE/23ABC12001400//
|                   LANDPT/FB C/PTNM:FIREBASE CRABAPPLE/23ABC85009200//
|                   LANDSTS/KBSA/-/-/LSSTAT:COLD/CNTNAME:MAJ STEIN/-/-/-DE:01//
|                   LANDSTS/FB A/MRKING:PANEL/RED/LSSTAT:HOT/CNTCS:HOT POTATO/132.5//
|Free Text Set----> NARR/IT IS AUTHORIZED TO BYPASS HOT LANDING ZONES//
+-----------------> RMKS/THIS FREE TEXT SET IS ADDED FOR DEMONSTRATION PURPOSES ONLY//
Closing Text ----------> DECL/ORIG:USJFCOM/15A/INST:AT EXERCISE ENDEX//
*/
namespace usmtf
{

/*!Message are the composition of predetermined sequence of sets. Its separated into three subdivisions:
   - Intro Text
   - Main Text
   - Closing Text

Only the Main and closing text are governed by USMTF rules. 4.2.1.c (2), because of this, the parser
advanced itself to the main text.

Messages contain sets, that when presented in a certain order outline logical ideas and transfer those
ideas via the ordering and data in its enclosed Sets.

Messages have Sets which are sometimes grouped together or repeated to convey meaning.
For us, this class can either subscribe to that meaning or ignore it. This is done through registering Sets
we're interested in by providing the factory with a constructor for that set type, else the Set type is only parsed
and put into mSets as a generic set that has no guarantees of correctness. After Sets
are register a message may choose to extract segments its interested and turn those into other entities in the
system.

Its set up this way to enable class authors to use what they need and ignore what they don't very easily.
This might sounds complex, but its not after some examples, see the accompanying test suite for some
inspiration. Or, if you fancy, a concrete use is found in aco.<c,h>pp

Messages are the file/document/white paper of the USMTF if you will.
*/
class USMTF_EXPORT Message : public Validatable
{
public:
   Message(const std::string& aMTF, std::vector<std::unique_ptr<Set>> aSets) noexcept;
   Message(const Message&) = delete;
   Message& operator=(const Message&) = delete;
   virtual ~Message()                 = default;
   //! Get the Type of the Message
   const std::string& GetType() const noexcept;
   //! Retrieve a Set at a given index
   const Set& GetSet(Segment::SetPosition aSetIndex) const;
   //! Get all Sets that match the given set type
   Segment::SetLocations GetSets(const std::string& aSTF) const noexcept;
   //! Get all Sets that match any of the given set type
   Segment::SetLocations GetSets(const std::vector<std::string>& aSTF) const noexcept;
   //! Retrieve a grouping of sets by given start and end positions
   Segment GetSegment(const Segment::SetPosition& startPosition, const Segment::SetPosition& stopPosition) const;
   //! Returns vector of segments that start with set type and go until next position is found.
   //! If set type is not found at all, no segments are returned. If Set Type is found only once,
   //! Vector will contain a single segment of that set until the end of the message.
   std::vector<Segment> GetSegments(const std::string& aSTF) const;
   //! Number of sets in the Message
   size_t GetNumberOfSets() const noexcept;
   //! Tells you if the Message doesn't contain Sets
   bool IsEmpty() const noexcept;

protected:
   bool                              IsSetPositionOutOfBounds(const Segment::SetPosition& aPos) const noexcept;
   const std::string                 mMessageTextFormat;
   std::vector<std::unique_ptr<Set>> mSets;
}; // The message abstraction is necessary, as it prescribes ordered arrangement of sets. And sets, prescribe ordered
   // arrangement of fields.
} // namespace usmtf
#endif
