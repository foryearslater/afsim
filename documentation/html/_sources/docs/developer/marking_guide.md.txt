<!--
.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. This is a US Government Work not subject to copyright protection in the US.
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************
-->

# Marking Guide

1. [Purpose](#purpose)
2. [Background](#background)
3. [Policies](#policies)
4. [FAQs](#frequently-asked-questions)
5. [Templates](#templates)
    + [File Header Templates](#file-header-templates)
    + [README Template](#readme-template)
    + [LICENSE Template](#license-template)
    + [Variations](#variations)
6. [Worked Examples](#worked-examples)
    + [An Unrestricted, Source Module](#an-unrestricted-source-module)
    + [A Restricted, Source Module](#a-restricted-source-module)
    + [An Unrestricted, Community Source Module](#an-unrestricted-community-source-module)
    + [An Unrestricted, Data Package](#an-unrestricted-data-package)
    + [A Restricted, Mixed Data Package](#a-restricted-mixed-data-package)

## **Purpose**
This guide describes how to properly mark Controlled Unclassified Information
(CUI) as Covered Information pursuant to terms and conditions in the AFSIM
Information Transfer Agreement (ITA) or Memorandum of Understanding (MOU).

As with other controlled or classified information, access to AFSIM requires an
authorized and lawful government purpose. Furthermore, every AFSIM user is a
steward of information that the United States has deemed valuable or sensitive.

The continued accessibility and security of AFSIM depends upon every member of
the community understanding how to receive, handle, create, and disseminate
CUI. All AFSIM users should become familiar with this marking guide and the
official guidance and training published by the [DoD CUI Program][1].

## **Background**
The AFSIM ITA or MOU, as applicable, define the terms and conditions for proper
control and use of AFSIM and related information (i.e., Covered Information).
Such information (source code, documentation, or data products) distributed,
generated, or used by the AFSIM community under these agreements require
appropriate markings.

Appropriate markings help all members of the community safeguard CUI on their
own systems while actively participating in AFSIM's continued development and
evolution on shared platforms. Appropriate markings also help prevent confusion
regarding suitable and non-suitable uses of AFSIM.

Suitable uses of AFSIM include, but are not limited to, the following:

* Use by a DoD organization in support of DoD efforts
* Use by a DoD organization in support of international partnerships with
   appropriate agreements
* Use by a DoD contractor on DoD contracted efforts
* Use by a DoD contractor for Internal Research and Development (IR&D)
* Use by a DoD contractor in support of efforts with export license covering
   output generated from AFSIM

Non-suitable uses of AFSIM include, but are not limited to, the following:

* Use by a DoD organization or DoD contractor with the intent to release
   significant portions of generated data to the public
* Use by a DoD organization or DoD contractor to partner with a non-DoD
   entity (such as a University unable to handle ITAR material)

### Controlled Unclassified Information
Controlled Unclassified Information (CUI) requires safeguarding until
authorized for public release in accordance with applicable law, federal
regulations, and departmental policies. CUI policies (EO 13556, 32 CFR Part
2002, DoDI 5200.48 4 , DFARS 252.204-7008 and DFARS 252.204-7012) establish a
uniform marking system across the DoD that replaces a variety of
agency-specific or legacy markings such as FOUO. Refer to the official guidance
and training published by the [DoD CUI Program][1] for more information.

### CUI Categories
The [DoD CUI Program][1] has defined multiple categories of CUI. Two of these
categories are applicable to AFSIM.

#### Controlled Technical Information
[Controlled Technical Information (CTI)][2] means technical information with
military or space application that is subject to controls on the access, use,
reproduction, modification, performance, display, release, disclosure, or
dissemination. Controlled technical information is to be marked with one of the
distribution statements B through F, in accordance with Department of Defense
Instruction 5230.24, "Distribution Statements of Technical Documents."

The term does not include information that is lawfully publicly available
without restrictions. "Technical Information" means technical data or computer
software, as those terms are defined in Defense Federal Acquisition Regulation
Supplement clause 252.227-7013, "Rights in Technical Data - Noncommercial
Items" (48 CFR 252.227-7013).

Examples of technical information include research and engineering data,
engineering drawings, and associated lists, specifications, standards, process
sheets, manuals, technical reports, technical orders, catalog-item
identifications, data sets, studies and analyses and related information, and
computer software executable code and source code.

#### Export Controlled Information
[Export control][3] of DoD articles, services, and technical data is governed
by the International Traffic in Arms Regulations (ITAR) (22 CFR Parts 120-130).
In particular Part 121 of the ITAR, called the US Munitions List (USML),
enumerates the particular categories and sub-categories of DoD articles,
services, and technical data which are to be export controlled.

Official rulings on whether or not a DoD article, service, or technical data
generated by the DoD falls under the USML are provided by the Defense
Technology Security Administration (DTSA).

DoD component organizations are already eligible to receive and process export
controlled material. However, DoD contractors must have a form DD Form 2345 in
place for each facility (per CAGE Code) at which the export controlled material
will be received and used. The [Defense Logistics Agency (DLA) Joint
Certification Program (JCP)][4] manages DD Form 2345s.

### Limited Dissemination Controls
Limited Dissemination Controls (LDCs) establish restrictions on the *secondary
distribution (i.e., redistribution)* of CUI. For instance, [DoDI 5230.24][5]
establishes distribution statements for managing, sharing, safeguarding, and
disseminating CTI. All DoD components generating or responsible for CTI
establish these controls before primary distribution. Likewise, DoDI 5230.24,
DoDD 5230.25, and 32 CFR Part 250 establish guidelines for export controlled
information. Applicable LDCs follow directly from the terms and conditions in
the AFSIM ITA and MOU or help promote community-wide collaboration and exchange
of information.

## **Policies**
The following policies apply to all CUI distributed, generated, or used under
the terms of the AFSIM ITA or MOU.

### AFSIM is Controlled Technical Information
Unless otherwise determined by the DoD controlling office (AFRL/RQ), AFSIM and
any output from AFSIM is Controlled Technical Information. 

### AFSIM is Export Controlled
Per DTSA review (March 2018), AFSIM is an export controlled product under USML
Category IX(b)(4) & IX(b)(5). Unless otherwise determined by DTSA, all AFSIM
modules and components are also export controlled products.

> **_NOTE:_** Organizations cannot export AFSIM (or portions of it) without
> written authorization from the DoD controlling office (AFRL/RQ) even if
> covered by an existing export license or agreement. Please contract
> [afrl.rq.afsim@us.af.mil][6] for assistance.

### AFSIM Output is also Export Controlled
Per DTSA guidance (November 2018), _all output from AFSIM is export controlled
regardless of input_. Through the AFSIM ITA, industry partners acknowledge
their responsibility under U.S. export control laws and regulations (including
the obligation, under certain circumstances, to obtain an export license prior
to release of data generated using AFSIM) and agree to not disseminate any
export controlled data in a manner that would violate applicable export control
laws and regulations. Violation of these export laws are subject to severe
criminal penalties. Through the AFSIM MOU, DoD organizations acknowledge a
similar responsibility to export data through established country-to-country
agreements (e.g., a Project Agreement (PA).

### Public Release Coordination
Neither the DoD controlling office (AFRL/RQ) nor the AFSIM Product Management
Team (PMT) can authorize community-generated content for public release. Only
an authorized DoD OPSEC coordination process and official can approve
AFSIM-related CUI for public release.

DoD organizations should initiate this activity through their local OPSEC
coordinator and adhere to all organization-specific policies and guidelines.
DoD contractors should initiate this activity through a cognizant government
program manager or contract officer.

DoD organizations and industry partners may choose to submit material to the
AFSIM PMT for an informal review (from an AFSIM content perspective) prior to
submitting through an authorized release process. Many DoD organizations
require this informal review as evidence of joint/program-level coordination
prior to public release.

> **_NOTE:_** Unless or until authorized for public release, CUI requires
> proper safeguarding. This requires the use of approved methods of
> transmitting CUI material such as encrypted mail or secure file exchange.
> * [afrl.rq.afsim@us.af.mil][6] supports encryption. Exchange certificates
> prior to sending CUI.
> * Use [DoD SAFE][7] to transmit large files. First contact the recipient to
> arrange a drop-off.

### Mandatory LDCs
The terms and conditions in the AFSIM ITA or MOU prohibit secondary
distribution (i.e., redistribution) of AFSIM binaries and source code. These
artifacts should consistently use LDC/Distribution Statement F.

**AFSIM Binaries**
: The compiled executables, libraries, or plugins distributed to the community
through packages and installers

**AFSIM Source Code**
: The distributed codebase that includes software source artifacts (e.g., C++
and Python), documentation source artifacts (e.g., reStructuredText and
Markdown), and ancillary build system artifacts (e.g., CMake scripts)

| Artifact          | Mandatory LDCs   | 
| ----------------- | ---------------- |
| AFSIM Binaries    | DIST-F           |
| AFSIM Source Code | DIST-F           |

### Recommended LDCs
Whereas the AFSIM ITA or MOU prohibit secondary distribution of AFSIM binaries
and source code, these agreements should support collaboration and exchange of
artifacts generated or consumed by AFSIM. Such artifacts should consistently
use LDC/Distribution Statement(s) C or D, as applicable. DoDI 3200.12 (STINFO),
governs this data to:
* Facilitate enterprise-wide collaboration on MS&A problems
* Accompany official reports maintained in DTIC
* Accompany analytical results and findings
* Accelerate other government-funded efforts as GFI

**Community Extensions**
: User-developed extensions (e.g., plugins) or applications that use AFSIM's
framework API(s).

**Input Data**
: AFSIM-compatible models, scenarios, or scripts

**Output Data**
: Generated data or results to include application screenshots and videos

**Generated User Documentation**
: Generated user documentation such as guides or manuals (e.g., in PDF or HTML)

**Generated Source Documentation**
: Generated UML class diagrams and source code annotations (e.g., Doxygen HTML)

| Artifact              | Recommended LDCs | Reason(s)                                 |
| --------------------- | ---------------- | ----------------------------------------- |
| Community Extensions  | DIST-C or DIST-D | Critical Technology, Export Controlled    |
| Input Data            | DIST-C or DIST-D | Critical Technology, Export Controlled    |
| Output Data           | DIST-C or DIST-D | Critical Technology, Export Controlled    |
| Generated User Docs   | DIST-C or DIST-D | Software Documentation, Export Controlled |
| Generated Source Docs | DIST-C or DIST-D | Software Documentation, Export Controlled |

> **_NOTE:_** Additional dissemination controls may apply; in all cases, users
> should consult applicable organizational or programmatic guidance.

### Special LDCs
In cases where the DoD controlling office (AFRL/RQ) authorizes a variant of
AFSIM for export through an appropriate country-to-country agreement,
additional dissemination controls may apply to individual source or data
module.

*Unrestricted Modules*
: Dissemination of these modules may be authorized to those individuals,
organizations, or entities in The Technical Cooperation Program (TTCP). The use
of "REL TO USA, FVEY" is not appropriate in this case because the authorized
dissemination list is more restrictive.

*Restricted Modules* 
: Dissemination of these modules will not be authorized to individuals,
organizations, or entities in other countries. The Foreign Disclosure Office
(FDO) has made a decision to not include these modules in any export variant.

| Source/Data Module    | Special LDCs     | Reason(s)                        |
| --------------------- | ---------------- | -------------------------------- |
| Unrestricted Modules  | DL Only          | Dissemination List Controlled    |
| Restricted Modules    | REL TO USA ONLY  | FDO Exclusion                    |

>**_Note:_** The DoD controlling office (AFRL/RQ) establishes export variants
> in coordination with the AFRL Foreign Disclosure Office (FDO). Restricted
> Modules are excluded from future FDO consideration.

## **Frequently Asked Questions**

### Including AFSIM Output in Reports
> What distribution statement should I use if my briefing (or report) contains
> AFSIM output or results?

Choose the least restrictive control that will promote the free flow of
information within the DoD while using discretion based on programmatic
guidance. See [Recommended LDCs](#recommended-ldcs) for suggestions. 

Keep in mind that all output from AFSIM is export controlled until downgraded
through an appropriate DoD review process. Remember to also identify all
applicable CUI categories and warnings. See [CUI Categories](#cui-categories)
for more information.

### Including Videos or Screenshots in Reports
> What distribution statement should I use if my report (or briefing) contains
> videos or screenshots from AFSIM applications?

Videos and screenshots are also considered AFSIM output. Refer to [Including
AFSIM Output in Reports](#including-afsim-output-in-reports). 

### Including Source Code Snippets
> What distribution statement should I use if my report (or briefing) contains
> AFSIM source code snippets?

Content that includes AFSIM code snippets (broadly interpreted to include both
application source code and input/ script files) should retain the most
restrictive dissemination control from code sources. For a point of reference,
core framework source code typically uses the most restrictive control (i.e.,
DIST-F or DL Only) whereas model and scenario input/script/data files use less
restrictive controls (i.e., DIST-C or DIST-D). 

Keep in mind that AFSIM is export controlled. Remember to also retain all
applicable CUI categories and warnings from these code sources. See [CUI
Categories](#cui-categories) for more information.

### Including Information from User Documentation
> What distribution statement should I use if my report (or briefing) includes
> information from AFSIM user documentation?

Content that includes (through quotation or copied in the form of an appendix)
information from the AFSIM user documentation should retain the most
restrictive dissemination control from documentation sources. For a point of
reference, the generated (e.g., HTML or PDF) AFSIM user documentation uses an
LDC/Distribution Statement of DIST-C. This permits AFSIM documentation to
accompany official reports maintained in DTIC.

Keep in mind that AFSIM is export controlled. Remember to also retain all
applicable CUI categories and warnings from these code sources. See [CUI
Categories](#cui-categories) for more information.

### Including Information from Source Documentation
> What distribution statement should I use if my report (or briefing) includes
> information from AFSIM source documentation?

Handle information included from generated source documentation (e.g., Doxygen
output with UML diagrams) in the same manner as generated user documentation.
Keep in mind that distribution statements differ between source code and
documentation generated from source code. For a point of reference, the
generated (e.g., HTML) source documentation uses an LDC/Distribution Statement
of DIST-C. This permits architectural and design documentation to accompany
official reports maintained in DTIC.

Keep in mind that AFSIM is export controlled. Remember to also retain all
applicable CUI categories and warnings from these code sources. See [CUI
Categories](#cui-categories) for more information.

## **Templates**
The following templates adhere to the [policies](#policies) described in this
document. Use these templates to mark applicable artifacts.

### File Header Templates

#### Unclassified File Header
Each unclassified source or data file should include a CUI banner that
references accompanying README and LICENSE files for details. Authors:
* Should choose a format-specific comment control character (e.g., #, //, """)
* Should use the extended REL TO banner for Restricted Modules
* Should provide a copyright statement
* May add additional file-specific metadata after the first block

> **_NOTE:_** Not all data formats support comments, so authors should use the
> README file to resolve any ambiguity.

```cpp
//*****************************************************************************
// CUI[//REL TO USA ONLY]
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// <COPYRIGHT>
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************
// [METADATA]
//*****************************************************************************
```

```bash
#******************************************************************************
# CUI[//REL TO USA ONLY]
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# <COPYRIGHT>
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
#******************************************************************************
# [METADATA]
#******************************************************************************
```

#### Classified File Header
For the sake of comparison, classified source or data files should include a
classification banner and a classification authority block. Otherwise, the
headers are purposefully consistent.

```cpp
//*****************************************************************************
// <CLASSIFICATION//REL//TRIGRAPHS>
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// <COPYRIGHT>
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************
// Classified by: <FIRST LAST (CTR), ORGANIZATION>
// Derived from:  <SOURCE | Multiple Sources>
// Declassify on: <YYYYMMDD>
//*****************************************************************************
// [METADATA]
//*****************************************************************************
```

```bash
#******************************************************************************
# <CLASSIFICATION//REL//TRIGRAPHS>
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# <COPYRIGHT>
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
#******************************************************************************
# Classified by: <FIRST LAST (CTR), ORGANIZATION>
# Derived from:  <SOURCE | Multiple Sources>
# Declassify on: <YYYYMMDD>
#******************************************************************************
# [METADATA]
#******************************************************************************
```

### README Template
Authors should organize source and data files into distinct modules or
packages. Each module or package should include a README file at its root
directory. A README performs a similar function to a CUI coversheet and conveys
the CUI designation indicator with notices or warnings applicable to all files
in that module. The following template includes many possible notices and
warnings. Authors should tailor their selections accordingly.

| Condition                | Applicable Notice or Warning                          |
| ------------------------ | ----------------------------------------------------- |
| CUI                      | Retain HANDLING AND DESTRUCTION NOTICE                |
| CTI Category             | Choose DISTRIBUTION STATEMENT F, D, or C              |
| EXPT Category            | Retain WARNING - EXPORT CONTROLLED                    |
| Mixed Classified         | Retain  WARNING - CONTROLLED UNCLASSIFIED INFORMATION |
| Restricted Module        | Remove NOTICE TO ACCOMPANY FOREIGN DISCLOSURE         |
| Restricted Module        | Retain extended REL TO banner                         |

```text
**CUI[//REL TO USA ONLY]**

# <MODULE_OR_PACKAGE_NAME>

[ABOUT]

## CUI Designation Indicator
* Controlled by: <DOD_COMPONENT>
* Controlled by: <CONTROLLING_OFFICE>
* CUI Categories: <CATEGORIES>
* LDC/Distribution Statement: <LDC>
* POC: <CONTACT>

## Notices and Warnings

### DISTRIBUTION STATEMENT F
Further dissemination only as directed by <CONTROLLING_OFFICE> (<YYYYMMDD>) or
higher DoD authority.

### DISTRIBUTION STATEMENT D
Distribution authorized to Department of Defense and U.S. DoD contractors only
<REASON>; <YYYYMMDD>. Other requests for this information shall be referred to
<CONTROLLING_OFFICE>.

### DISTRIBUTION STATEMENT C
Distribution authorized to U.S. Government agencies and their contractors
<REASON>; <YYYYMMDD>. Other requests for this information shall be referred to
<CONTROLLING_OFFICE>.

### NOTICE TO ACCOMPANY FOREIGN DISCLOSURE
This content is furnished on the condition that it will not be released to
another nation without specific authority of the Department of the Air Force of
the United States, that it will be used for military purposes only, that
individual or corporate rights originating in the information, whether patented
or not, will be respected, that the recipient will report promptly to the
United States any known or suspected compromise, and that the information will
be provided substantially the same degree of security afforded it by the
Department of Defense of the United States. Also, regardless of any other
markings on the document, it will not be downgraded or declassified without
written approval from the originating U.S. agency.

### WARNING - EXPORT CONTROLLED
This content contains technical data whose export is restricted by the Arms
Export Control Act (Title 22, U.S.C. Sec 2751 et seq.) or the Export
Administration Act of 1979, as amended, Title 50 U.S.C., App. 2401 et seq.
Violations of these export laws are subject to severe criminal penalties.
Disseminate in accordance with provisions of DoD Directive 5230.25.

### WARNING - CONTROLLED UNCLASSIFIED INFORMATION
This content is classified at the <CLASSIFICATION> level and may contain
elements of controlled unclassified information (CUI), unclassified, or
information classified at a lower level than the overall classification
displayed. This content shall not be used as a source of derivative
classification; refer instead to <SOURCE>. It must be reviewed for both
Classified National Security Information (CNSI) and CUI in accordance with DoDI
5230.09 prior to public release.

### HANDLING AND DESTRUCTION NOTICE
Handle this information in accordance with DoDI 5200.48. Destroy by any
approved method that will prevent unauthorized disclosure or reconstruction of
this information in accordance with NIST SP 800-88 and 32 C.F.R 2002.14
(Safeguarding Controlled Unclassified Information).

**CUI[//REL TO USA ONLY]**
```

### LICENSE Template
Each module or package should include a LICENSE file at its root directory.
Note that the LICENSE cites the terms and conditions enumerated in the AFSIM
MOU or ITA. Authors should identify the module, by name, in the LICENSE file.

```text
**CUI**

## License to Accompany <MODULE_OR_PACKAGE_NAME>
You may not use this product except in compliance with the terms and conditions
of 48 C.F.R. 252.204-7000 (Disclosure of Information), 48 C.F.R. 252.227-7025
(Limitations on the Use or Disclosure of Government-Furnished Information
Marked with Restrictive Legends), and the AFSIM Memorandum of Understanding or
Information Transfer Agreement as applicable.

This product is provided "as is" without warranties of any kind.

**CUI**
```

### NO_EXPORT Template
Authors should include a NO_EXPORT guard file for Restricted Modules excluded
from export variants. There are a variety of reasons why an author, after
consulting with their local Foreign Disclosure Office, may decide that a module
or package is not suitable for export. The NO_EXPORT guard does NOT document
that reason; it simply excludes the module.

```text
**CUI//REL TO USA ONLY**

Exclude this module or package from all export variants.

**CUI//REL TO USA ONLY**
```

### Variations

#### On Copyright Statements
For contracted activities or industry contributions through the AFSIM ITA,
authors should use a standard copyright statement.

```cpp
// Copyright YYYY-[YYYY] [COMPANY NAME]. All rights reserved.
```

For organic government activities or government (government civilian or
military member) contributions through the AFSIM MOU, authors should use a
copyright disclaimer.

```cpp
// This is a US Government Work not subject to copyright protection in the US.
```

#### On Metadata Fields

##### Modified by Field
In cases where one company or organization produces an original work and
another company or organization modifies that work (for maintenance or
enhancement), authors should use an optional "Modified by" metadata field to
track this history. A version control system maintains the technical change log
whereas a "Modified by" field provides attribution to secondary contributors
and tracks the history of the original copyrighted work.

```bash
#******************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright YYYY The First Company. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
#******************************************************************************
# Modified by:
#     YYMM A Second Company:  Fix bugs in track processor
#     YYMM Some Organization: Add output reporting features
#******************************************************************************
```

## **Worked Examples**
The following worked examples illustrate how to apply CUI marking guidance to
specific information types.

### An Unrestricted, Source Module
The following example is typical of most framework-level source modules. Both
the Controlled Technical Information (CTI) and Export Controlled (EXPT)
categories apply. The module is not excluded from export variants, but still
governed by the AFSIM MOU or ITA, so strict dissemination controls apply.

#### Module Organization
```text
wsf_core_module/
├── doc/
│   └── core_module.rst
├── grammar/
├── source/
│   ├── core_module.hpp
│   └── core_module.cpp
├── test/
├── wsf_module
├── README.md
└── LICENSE.md
```

#### core_module.cpp
```cpp
//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Eregion Forge. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************
```

#### core_module.rst
```text
.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. Copyright 2021 Eregion Forge. All rights reserved.
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************
```

#### README
```text
**CUI**

# wsf_core_module

* This module is part of the World Simulation Framework (WSF).
* Export variants of AFSIM may include it.

## CUI Designation Indicator
* Controlled by: Air Force Research Laboratory
* Controlled by: Aerospace Systems Directorate
* CUI Categories: CTI, EXPT
* LDC/Distribution Statement: DIST-F
* POC: afrl.rq.afsim@us.af.mil

## Notices and Warnings

### DISTRIBUTION STATEMENT F
Further dissemination only as directed by AFRL Aerospace Systems Directorate
(20211209) or higher DoD authority.

### NOTICE TO ACCOMPANY FOREIGN DISCLOSURE
This content is furnished on the condition that it will not be released to
another nation without specific authority of the Department of the Air Force of
the United States, that it will be used for military purposes only, that
individual or corporate rights originating in the information, whether patented
or not, will be respected, that the recipient will report promptly to the
United States any known or suspected compromise, and that the information will
be provided substantially the same degree of security afforded it by the
Department of Defense of the United States. Also, regardless of any other
markings on the document, it will not be downgraded or declassified without
written approval from the originating U.S. agency.

### WARNING - EXPORT CONTROLLED
This content contains technical data whose export is restricted by the Arms
Export Control Act (Title 22, U.S.C. Sec 2751 et seq.) or the Export
Administration Act of 1979, as amended, Title 50 U.S.C., App. 2401 et seq.
Violations of these export laws are subject to severe criminal penalties.
Disseminate in accordance with provisions of DoD Directive 5230.25.

### HANDLING AND DESTRUCTION NOTICE
Handle this information in accordance with DoDI 5200.48. Destroy by any
approved method that will prevent unauthorized disclosure or reconstruction of
this information in accordance with NIST SP 800-88 and 32 C.F.R 2002.14
(Safeguarding Controlled Unclassified Information).

**CUI**
```

#### LICENSE
```text
**CUI**

## License to Accompany wsf_core_module
You may not use this product except in compliance with the terms and conditions
of 48 C.F.R. 252.204-7000 (Disclosure of Information), 48 C.F.R. 252.227-7025
(Limitations on the Use or Disclosure of Government-Furnished Information
Marked with Restrictive Legends), and the AFSIM Memorandum of Understanding or
Information Transfer Agreement as applicable.

This product is provided "as is" without warranties of any kind.

**CUI**
```

### A Restricted, Source Module
The following example applies to several framework-level source modules. Both
the Controlled Technical Information (CTI) and Export Controlled (EXPT)
categories apply. Unlike the previous example, this module is excluded from
export variants. Accordingly, the REL TO banner is retained, the NOTICE TO
ACCOMPANY FOREIGN DISCLOSURE is dropped, and a NO_EXPORT guard file is added to
the root folder.

#### Module Organization
```text
wsf_mil_module/
├── doc/
│   └── mil_module.rst
├── grammar/
├── source/
│   ├── mil_module.hpp
│   └── mil_module.cpp
├── test/
├── wsf_module
├── NO_EXPORT
├── README.md
└── LICENSE.md
```

#### mil_module.cpp
```cpp
//*****************************************************************************
// CUI//REL TO USA ONLY
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Eregion Forge. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************
```

#### mil_module.rst
```text
.. ****************************************************************************
.. CUI//REL TO USA ONLY
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. Copyright 2021 Eregion Forge. All rights reserved.
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************
```

#### README
```text
**CUI//REL TO USA ONLY**

# wsf_mil_module

* This module is part of the World Simulation Framework (WSF).
* Export variants of AFSIM must exclude it.

## CUI Designation Indicator
* Controlled by: Air Force Research Laboratory
* Controlled by: Aerospace Systems Directorate
* CUI Categories: CTI, EXPT
* LDC/Distribution Statement: DIST-F, REL TO USA ONLY
* POC: afrl.rq.afsim@us.af.mil

## Notices and Warnings

### DISTRIBUTION STATEMENT F
Further dissemination only as directed by AFRL Aerospace Systems Directorate
(20211209) or higher DoD authority.

### WARNING - EXPORT CONTROLLED
This content contains technical data whose export is restricted by the Arms
Export Control Act (Title 22, U.S.C. Sec 2751 et seq.) or the Export
Administration Act of 1979, as amended, Title 50 U.S.C., App. 2401 et seq.
Violations of these export laws are subject to severe criminal penalties.
Disseminate in accordance with provisions of DoD Directive 5230.25.

### HANDLING AND DESTRUCTION NOTICE
Handle this information in accordance with DoDI 5200.48. Destroy by any
approved method that will prevent unauthorized disclosure or reconstruction of
this information in accordance with NIST SP 800-88 and 32 C.F.R 2002.14
(Safeguarding Controlled Unclassified Information).

**CUI//REL TO USA ONLY**
```

#### LICENSE
```text
**CUI**

## License to Accompany wsf_mil_module
You may not use this product except in compliance with the terms and conditions
of 48 C.F.R. 252.204-7000 (Disclosure of Information), 48 C.F.R. 252.227-7025
(Limitations on the Use or Disclosure of Government-Furnished Information
Marked with Restrictive Legends), and the AFSIM Memorandum of Understanding or
Information Transfer Agreement as applicable.

This product is provided "as is" without warranties of any kind.

**CUI**
```
### An Unrestricted, Community Source Module
The following example is typical of community-developed source modules (e.g.,
shared plugins or extensions). Both the Controlled Technical Information (CTI)
and Export Controlled (EXPT) categories apply. The module is not excluded from
export variants and distribution is not limited by the terms and conditions in
the AFSIM MOU or ITA.

#### Module Organization
```text
wsf_shared_module/
├── doc/
│   └── shared_module.rst
├── grammar/
├── source/
│   ├── shared_module.hpp
│   └── shared_module.cpp
├── test/
├── wsf_module
├── README.md
└── LICENSE.md
```

#### shared_module.cpp
```cpp
//*****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// Copyright 2021 Shire Works. All rights reserved.
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
//*****************************************************************************
```

#### shared_module.rst
```text
.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. Copyright 2021 Shire Works. All rights reserved.
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************
```

#### README
```text
**CUI**

# wsf_shared_module

* This module is shared with others in the AFSIM community.
* Some Other Government Agency manages or maintains it.
* Export variants of AFSIM may include it.

## CUI Designation Indicator
* Controlled by: Some Other Government Agency
* Controlled by: Some Other Directorate
* CUI Categories: CTI, EXPT
* LDC/Distribution Statement: DIST-C
* POC: some.organization@mail.mil

## Notices and Warnings

### DISTRIBUTION STATEMENT C
Distribution authorized to US Government agencies and their contractors;
Critical Technology, Export Controlled; 20211209. Other requests for this
information shall be referred to Some Other Government Agency, 123 A Street,
Somewhere, USA 12345.

### NOTICE TO ACCOMPANY FOREIGN DISCLOSURE
This content is furnished on the condition that it will not be released to
another nation without specific authority of the Department of the Air Force of
the United States, that it will be used for military purposes only, that
individual or corporate rights originating in the information, whether patented
or not, will be respected, that the recipient will report promptly to the
United States any known or suspected compromise, and that the information will
be provided substantially the same degree of security afforded it by the
Department of Defense of the United States. Also, regardless of any other
markings on the document, it will not be downgraded or declassified without
written approval from the originating U.S. agency.

### WARNING - EXPORT CONTROLLED
This content contains technical data whose export is restricted by the Arms
Export Control Act (Title 22, U.S.C. Sec 2751 et seq.) or the Export
Administration Act of 1979, as amended, Title 50 U.S.C., App. 2401 et seq.
Violations of these export laws are subject to severe criminal penalties.
Disseminate in accordance with provisions of DoD Directive 5230.25.

### HANDLING AND DESTRUCTION NOTICE
Handle this information in accordance with DoDI 5200.48. Destroy by any
approved method that will prevent unauthorized disclosure or reconstruction of
this information in accordance with NIST SP 800-88 and 32 C.F.R 2002.14
(Safeguarding Controlled Unclassified Information).

**CUI**
```

#### LICENSE
```text
**CUI**

## License to Accompany wsf_shared_module
You may not use this product except in compliance with the terms and conditions
of 48 C.F.R. 252.204-7000 (Disclosure of Information), 48 C.F.R. 252.227-7025
(Limitations on the Use or Disclosure of Government-Furnished Information
Marked with Restrictive Legends), and the AFSIM Memorandum of Understanding or
Information Transfer Agreement as applicable.

This product is provided "as is" without warranties of any kind.

**CUI**
```

### An Unrestricted, Data Package
The following example is typical of data packages (e.g., boxed set scenarios,
models, and demos) included with the AFSIM distribution. Both the Controlled
Technical Information (CTI) and Export Controlled (EXPT) categories apply. The
package is not excluded from export variants and distribution is not limited by
the terms and conditions in the AFSIM MOU or ITA. This package uses the
optional metadata block to convey model-specific pedigree and V&V information. 
 
#### Package Organization
```text
afsim_scenario/
├── doc/
│   └── scenario.rst
├── platforms/
├── processors/
│   ├── weapon_processor.txt
│   └── track_processor.txt
├── sensors/
├── output/
├── scenario.txt
├── README.md
└── LICENSE.md
```

#### weapon_processor.txt
```bash
#******************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2021 Eregion Forge. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
#******************************************************************************
# Description:
#     [DESCRIPTION]
# Sources:
#     [SOURCES]
# Limitations:
#     [LIMITATIONS]
# Keywords:
#     [COMMA_SEPARATED_LIST_OF_KEYWORDS]
# Pedigree:
#     AFSIM Versions: [AFSIM_VERSIONS]
#     File Version:   [FILE_VERSION]
#     Change Log:     
#         [[FILE_VERSION] [CHANGE_DESCRIPTION]]
#     Dependencies:
#         [DEPENDENCIES]
#     V&V:
#         [VERIFICATION_AND_VALIDATION]
#******************************************************************************
```

#### scenario.rst
```text
.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. Copyright 2021 Shire Works. All rights reserved.
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************
```

#### README
```text
**CUI**

# afsim_scenario

* This scenario demonstrates the use of AFSIM processors.
* Export variants of AFSIM may include it.

## CUI Designation Indicator
* Controlled by: Air Force Research Laboratory
* Controlled by: Aerospace Systems Directorate
* CUI Categories: CTI, EXPT
* LDC/Distribution Statement: DIST-C
* POC: afrl.rq.afsim@us.af.mil

## Notices and Warnings

### DISTRIBUTION STATEMENT C
Distribution authorized to US Government agencies and their contractors;
Critical Technology, Export Controlled; 20211209. Other requests for this
information shall be referred to AFRL Aerospace Systems Directorate,
afrl.rq.afsim@us.af.mil.

### NOTICE TO ACCOMPANY FOREIGN DISCLOSURE
This content is furnished on the condition that it will not be released to
another nation without specific authority of the Department of the Air Force of
the United States, that it will be used for military purposes only, that
individual or corporate rights originating in the information, whether patented
or not, will be respected, that the recipient will report promptly to the
United States any known or suspected compromise, and that the information will
be provided substantially the same degree of security afforded it by the
Department of Defense of the United States. Also, regardless of any other
markings on the document, it will not be downgraded or declassified without
written approval from the originating U.S. agency.

### WARNING - EXPORT CONTROLLED
This content contains technical data whose export is restricted by the Arms
Export Control Act (Title 22, U.S.C. Sec 2751 et seq.) or the Export
Administration Act of 1979, as amended, Title 50 U.S.C., App. 2401 et seq.
Violations of these export laws are subject to severe criminal penalties.
Disseminate in accordance with provisions of DoD Directive 5230.25.

### HANDLING AND DESTRUCTION NOTICE
Handle this information in accordance with DoDI 5200.48. Destroy by any
approved method that will prevent unauthorized disclosure or reconstruction of
this information in accordance with NIST SP 800-88 and 32 C.F.R 2002.14
(Safeguarding Controlled Unclassified Information).

**CUI**
```

#### LICENSE
```text
**CUI**

## License to Accompany afsim_scenario
You may not use this product except in compliance with the terms and conditions
of 48 C.F.R. 252.204-7000 (Disclosure of Information), 48 C.F.R. 252.227-7025
(Limitations on the Use or Disclosure of Government-Furnished Information
Marked with Restrictive Legends), and the AFSIM Memorandum of Understanding or
Information Transfer Agreement as applicable.

This product is provided "as is" without warranties of any kind.

**CUI**
```

### A Restricted, Mixed Data Package
Where a data package mixes both classified and CUI, it is necessary to retain
the CUI markings. The following example applies to a few data packages (e.g.,
boxed set scenarios, models, and demos) included with the classified AFSIM
distribution. Both the Controlled Technical Information (CTI) and Export
Controlled (EXPT) categories apply. Unlike the previous example, this module is
excluded from export variants. Accordingly, the REL TO banner is retained, the
NOTICE TO ACCOMPANY FOREIGN DISCLOSURE is dropped, and a NO_EXPORT guard file
is added to the root folder.

#### Package Organization
```text
afsim_scenario_snf/
├── doc/
│   └── scenario_u.rst
├── platforms/
├── processors/
│   ├── weapon_processor_u.txt
│   ├── track_processor_u.txt
│   └── track_processor_snf.txt
├── sensors/
├── output/
├── scenario_snf.txt
├── NO_EXPORT
├── README.md
└── LICENSE.md
```

#### track_processor_u.txt
```bash
#******************************************************************************
# CUI//REL TO USA ONLY
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2021 Eregion Forge. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
#******************************************************************************
# Description:
#     [DESCRIPTION]
# Sources:
#     [SOURCES]
# Limitations:
#     [LIMITATIONS]
# Keywords:
#     [COMMA_SEPARATED_LIST_OF_KEYWORDS]
# Pedigree:
#     AFSIM Versions: [AFSIM_VERSIONS]
#     File Version:   [FILE_VERSION]
#     Change Log:     
#         [[FILE_VERSION] [CHANGE_DESCRIPTION]]
#     Dependencies:
#         [DEPENDENCIES]
#     V&V:
#         [VERIFICATION_AND_VALIDATION]
#******************************************************************************
```

#### track_processor_snf.txt
> **_NOTE:_** This example is contrived and the following markings are for
> instructional purposes only.

```bash
#******************************************************************************
# SECRET//NOFORN
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# Copyright 2021 Eregion Forge. All rights reserved.
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
#******************************************************************************
# Classified by: An. Analyst (Ctr)
# Derived from:  Multiple Sources
# Declassify on: 20461209
#******************************************************************************
# Description:
#     [DESCRIPTION]
# Sources:
#     [SOURCES]
# Limitations:
#     [LIMITATIONS]
# Keywords:
#     [COMMA_SEPARATED_LIST_OF_KEYWORDS]
# Pedigree:
#     AFSIM Versions: [AFSIM_VERSIONS]
#     File Version:   [FILE_VERSION]
#     Change Log:     
#         [[FILE_VERSION] [CHANGE_DESCRIPTION]]
#     Dependencies:
#         [DEPENDENCIES]
#     V&V:
#         [VERIFICATION_AND_VALIDATION]
#******************************************************************************
```

#### README
> **_NOTE:_** This example is contrived and the following markings are for
> instructional purposes only.

```text
**SECRET//NOFORN**

# (U) afsim_scenario_snf

* (U) This scenario demonstrates the use of AFSIM processors.
* (U) This scenario includes both classified and unclassified input files.
* (SNF) The README file itself may include classified content.

## CUI Designation Indicator
* Controlled by: Air Force Research Laboratory
* Controlled by: Aerospace Systems Directorate
* CUI Categories: CTI, EXPT
* LDC/Distribution Statement: DIST-D, REL TO USA ONLY
* POC: afrl.rq.afsim@us.af.mil

## Notices and Warnings

## DISTRIBUTION STATEMENT D
Distribution authorized to Department of Defense and U.S. DoD contractors only
Critical Technology, Export Controlled; 20211209. Other requests for this
document shall be referred to AFRL Aerospace Systems Directorate,
afrl.rq.afsim@us.af.mil.

### WARNING - EXPORT CONTROLLED
This content contains technical data whose export is restricted by the Arms
Export Control Act (Title 22, U.S.C. Sec 2751 et seq.) or the Export
Administration Act of 1979, as amended, Title 50 U.S.C., App. 2401 et seq.
Violations of these export laws are subject to severe criminal penalties.
Disseminate in accordance with provisions of DoD Directive 5230.25.

### WARNING - CONTROLLED UNCLASSIFIED INFORMATION
This content is classified at the SECRET//NOFORN level and may contain elements
of controlled unclassified information (CUI), unclassified, or information
classified at a lower level than the overall classification displayed. This
content shall not be used as a source of derivative classification; refer
instead to <SOURCE>. It must be reviewed for both Classified National Security
Information (CNSI) and CUI in accordance with DoDI 5230.09 prior to public
release.

### HANDLING AND DESTRUCTION NOTICE
Handle this information in accordance with DoDI 5200.48. Destroy by any
approved method that will prevent unauthorized disclosure or reconstruction of
this information in accordance with NIST SP 800-88 and 32 C.F.R 2002.14
(Safeguarding Controlled Unclassified Information).

**SECRET//NOFORN**
```

#### LICENSE
```text
**CUI**

## License to Accompany afsim_scenario_snf
You may not use this product except in compliance with the terms and conditions
of 48 C.F.R. 252.204-7000 (Disclosure of Information), 48 C.F.R. 252.227-7025
(Limitations on the Use or Disclosure of Government-Furnished Information
Marked with Restrictive Legends), and the AFSIM Memorandum of Understanding or
Information Transfer Agreement as applicable.

This product is provided "as is" without warranties of any kind.

**CUI**
```


[1]: <https://www.dodcui.mil/>
[2]: <https://www.archives.gov/cui/registry/category-detail/controlled-technical-info.html>
[3]: <https://www.archives.gov/cui/registry/category-detail/export-control.html>
[4]: <https://www.dla.mil/HQ/LogisticsOperations/Services/JCP/>
[5]: <https://www.esd.whs.mil/Portals/54/Documents/DD/issuances/dodi/523024p.pdf>
[6]: <mailto:afrl.rq.afsim@us.af.mil>
[7]: <https://safe.apps.mil>
