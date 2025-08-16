# AFSIM Contributing Guide

Code contributions are a central component of AFSIM DevOps.
This guide is available to assist contributors in understanding the requirements and expectations for code contributions to ensure that
submissions are reviewed and accepted quickly.
This guide serves as a general set of quality checks used by AFSIM repository maintainers and should be used by contributors to ensure that their
change request meets the criteria for acceptance.
If you or your team are interested in contributing, please continue reading this guide and complete each of the requirements before submitting
your change request.

While this guide focuses on requirements and expectations for source contributions, there are
many other ways to contribute to AFSIM. For guidance on alternative ways to become a contributor, see
[AFSIM Portal: Contributors](https://confluence.di2e.net/x/Rx9LH).

Any change to this repository should be made with the goal of preserving and/or enhancing the value and stability of AFSIM.
Out of necessity, the acceptance criteria for contributions to this repository are many - making the bar for acceptance high.

This guide provides all general requirements to ensure that your contribution meets the bar for acceptance.

## Preparing Your Contribution

The AFSIM codebase is divided into the following zones:

 - AFSIM Shared Codebase: The Shared Codebase consists of those modules that are simply shared with the community.
   Contributors have no obligation to the community other than making their module accessible to others in a
   manner that supports its use and/or continued development by others.
 - AFSIM Managed Codebase: The managed codebase consists of those modules with a responsible steward or configuration manager.
   These stewards, typically the contributor, have a basic obligation to the community to ensure that their modules remain compatible 
   with others in the managed codebase. This generally means that these modules readily support snapshot builds from the Development stream.
   Ideally, a Continuous Integration (CI) pipeline enforces this guarantee.
 - AFSIM Distributed Codebase: The distributed codebase consists of those modules that "ship with" the mainline AFSIM distribution.
   Each module has a responsible maintainer, typically members of core development teams with a plan to cover the servicing period over each release.
   Maintainers have significant obligations to the user community.
   These modules must support snapshot builds from the Development stream, in addition to release builds from the Latest and Stable streams.
   Furthermore, maintainers must address community problems, bug reports, and security-related issues within AFSIM's patch cycle.

The bar for acceptance depends largely on which zone your contribution is targeting.

This repository is part of the AFSIM Distributed Codebase. Specifically, this repository is the central/main AFSIM repository.
All other AFSIM modules use some part of this codebase to derive function and value.
Given that, the bar for contribution quality here is the highest possible.
Expect that all change requests to this repository will be scrutinized in detail before they are accepted.

For more detail on specific requirements for contributions under each category, visit [AFSIM Portal: Configuration Management](https://confluence.di2e.net/x/PsK0H).

Once your contribution is in the right place, the first question any reviewer will ask when considering a contribution is, "What does it do?".
The answer should be clear and concise to both the contributor and the reviewer.

In order to describe a change concisely, the change must itself be well-scoped. This is a central focus of reviewers.
In most cases, a single change may be written as a single issue or story, which will define the scope of the change and thus define what it does.

### Organization and Directory Placement

Organization is critical for new features/enhancements. These usually include new plugins for AFSIM applications,
which will require the creation and organization of new source files and directories. 
Organizing your contribution into a single package (no more than a few directories) which encapsulates all source additions
and configuration files under the appropriate project(s) (core, tools, wizard, etc.) will help reviewers to understand the scope of your
changes as they become familiar with your contribution. Existing plugins may be referenced as examples of well-organized and properly
configured contributions. Consider the following when organizing, configuring, and scoping your change:

 - Can your contribution be toggled in the CMake build system via inclusion/exclusion of a single CMake target?
 - Does your plugin structure and naming convention align with that of comparable plugins?

For smaller changes, namely those that exclusively affect existing source files, organization is still important.
For bug fixes, focus on solving one problem at a time rather than attempting to fix several problems in one pass.
The more functionality that's changed, the more effort it takes to verify the correctness of each change.
If you are refactoring existing source, ensure your changes are systematic and predictable.
It is often faster to review ten syntactically similar changes than two unrelated changes.

### AFSIM Coding Standards

The AFSIM Coding Standard is a component of the AFSIM Documentation. It is accessible within the documentation under the name "Coding Standard".
Alternatively, it may be accessed in source from `./doc/developer/coding_standards.rst`.

Adherence to this standard ensures that other developers are able to quickly parse your code.
Conversely, diverging from this standard slows the code review process considerably,
and will often result in suggestions focused on bringing your contribution in line with the standard,
rather than the actual functionality the change provides. This standard also provides requirements for source-level documentation/comments.
This may include a requirement for Doxygen depending on the module. See Doxygen Requirements below for more information.
Significant divergence from this standard may result in a contribution being altogether rejected.

For changes to existing source, a reliable rule is to follow the conventions of adjacent code.
If your code blends in with the existing code, it is less likely that a reviewer will take issue with it stylistically.

### Clang-Format 

Clang-format is a tool that can be used to auto-format C/C++ code according to pre-defined style rules. 
A clang-format, style configuration file named `.clang-format` is included in the top-level directory. It contains various style options
determining how .cpp and .hpp documents can be formatted to adhere to some of the AFSIM coding standards, mostly in regards to whitespace and `#include` ordering.
Many IDE's such as Visual Studio 15+ will either have built-in support or optional plugins to take advantage of clang-format. Additionally,
clang-format can be installed and executed via the command line, utilizing the `.clang-format` file in the source directory.
The included style configuration requires a minimum of clang-format 9.0.0 to use. 

Clang-format is currently configured to auto-execute upon changed files when making a commit. If potential format 
changes are detected, these will be diplayed in an output terminal and the changes will be saved in 
a temporary patch file. Instructions on how to apply the changes will be given at the end of the 
output terminal. You will need to re-commit after applying these.  

#### Clang-Format Visual Studio Configuration

The need to apply a git patch whenever clang-format changes are made can be avoided by manually running clang-format locally either via the command line or through IDE integration before making a commit.

Specifically, if you are using VS2017(15.6+), it comes with clang-format version 5.0. VS2019 comes with version 11.0.


#### IDE Manual Formatting

   Clang formatting is suggested to be done in either of two ways. 

   **Default VS Integration**
    
   VS has clang-format enabled by default and will autodetect the style file in the AFSIM source directory. 
   
   Running a manual format on a file can be done with VS integration via:
   (Edit>Advanced>Format Document) or (Ctrl+K, Ctrl+D).

   It will also format when specific formatting actions are triggered like using braces "{}". These options can be changed in:

   Tools>Options>Text Editor>C/C++>Code Style> Formatting.

   *Note for VS2017 Users

   If you are using VS2017, you will need to manually set the clang-format binary to one that is version 9.0+ since it uses version 5.0 by default.

   Go into (Tools>Options>Text Editor>C/C++>Code Style> Formatting) and set the "use custom clang-format.exe file" to the clang-format.exe from LLVM 9+ you installed.


   **Clang Power Tools**
   (Free VS Extension)(For either VS2017 or VS2019 users)

   Disable clang-format support in (Tools>Options>Text Editor>C/C++>Code Style> Formatting)
   
   Download the free VS extension "Clang Power Tools". It comes with LLVM 12.0 by default. 
   
   After downloading, enable the toolbar via:
   (View>Toolbars>Clang Power Tools)
   
   You can change settings with the ClangPowerTools settings button to download and use other versions of LLVM if you want. 
   
   Additionally, it will add a button to the VS toolbar (Alt+M) to run clang format on the current file.


#### Formatting on Save

   To further support making file formatting seamless and not dealing with applying patch files, formatting a file when you save it has been recommended.

   VS doesn't support formatting when you save a file by default. However, it is possible if you choose one of the aforementioned clang-format options (default integration or ClangPowerTools)


   Default Integration:
   
   Download a free VS extension 

   (Format Document on Save recommended)
   https://marketplace.visualstudio.com/items?itemName=mynkow.FormatDocumentOnSaveLegacy


   ClangPowerTools:

   Click the ClangPowerTools "Settings" button on the toolbar
   
   Click the "Format" tab
   
   Turn on the "Format on save" option

### AFSIM Documentation Standards

It is expected that all new functionality will be documented completely when it is first introduced into the framework.
This includes documentation for all user interfaces, such as commands, models, script classes, displays, etc.
Similarly, when existing interfaces are modified, the accompanying documentation must be updated to reflect the changes.

The AFSIM Documentation Standard is a component of the AFSIM Documentation.
It is accessible within the documentation under the name "AFSIM Documentation Standard".
Alternatively, it may be accessed in source from `./doc/developer/afsim_documentation_standard.rst`.

This standard serves as a guide to developers for how documentation should be structured, and what reStructuredText utilities are available to
enhance the presentation/clarity/accessibility of your documentation in a generated form (usually HTML).

If you add or change AFSIM documentation, you are responsible for verifying that it builds successfully, error and warning free.
You can do this by using the DOCUMENTATION* CMake Target for supported documentation types.
Usually an error/warning free build of DOCUMENTATION is sufficient.

### Creating a Changelog Entry

A changelog is a record containing changelog entries - each detailing a discrete, usually user-facing/front-end/API, change. 
Usually, a changelog entry is no more than one to two sentences describing the change at a high level.
If a change cannot be entirely described in one to two sentences, this is an indicator that the change may not be well scoped.

Changelog entries should be added to applicable changelog files for each affected project/application.
Changelog files are located under `<project>/doc/changelog/` and always include a version number.
For changes tracked via AFSIM Portal issues, the issue ID should be included following the changelog entry.

### Doxygen Requirements

Doxygen documentation is present in many existing AFSIM modules.
Though it is not always enforced as a requirement for new features, consistent usage of Doxygen is required.
This means that if you use Doxygen in new source, it must be used fully throughout the change.
Similarly, if changes are made to source that use Doxygen, the Doxygen comments for the affected source must be updated to reflect the change.

If you add or change Doxygen you are responsible for verifying that Doxygen builds successfully.
You can do this by using the DOXYGEN CMake Target.

## Configuration Management

### Toolchain Requirements

 - AFSIM Distributed: Distributed modules must be compatible with the minimum software toolchain requirements as specified in Build Instructions for
   both Linux and Windows operating systems.

Patch versions of AFSIM (e.g. AFSIM vX.Y.2) will not introduce changes to the minimum software toolchain requirements indicated in Build
Instructions unless a high risk vulnerability is discovered which necessitates a change.
In the event that a vulnerability is discovered, exceptions may be made for vulnerability corrections.
These corrections will be documented in the applicable security changelog.

Minor versions of AFSIM may introduce changes to minimum toolchain requirements indicated in Build Instructions.
To the extent possible, backwards compatibility should be maintained for existing AFSIM modules.
For new features across minor versions, additional requirements may be added for those modules, and
subsequent minor versions of AFSIM must maintain those minimum requirements.

Major versions of AFSIM are likely to introduce significant changes to minimum software toolchain requirements 
and may additionally introduce new requirements to the core codebase.
To the fullest extent possible, forward compatibility should be preserved with existing modules
and all changes will be clearly documented in Build Instructions and related documentation.

### Git Hook Configuration 

The AFSIM source directory contains the `.githooks/` directory which contains custom git hooks,
and is set when AFSIM is configured in CMake. Git version 2.9+ is required. 

### Version Compatibility

 - AFSIM Distributed: Distributed modules must meet the requirements for AFSIM Managed modules and additionally maintain compatibility with
   all actively supported AFSIM Release Streams (stable, latest, development).

In addition to those general rules, because this repository affects all other AFSIM modules and repositories,
many of which are not publicly managed, it is critical that changes to public interfaces are carefully planned and announced far in advance.
Interface changes involve removal or redefinition ([See Deprecation](#Deprecation)) of AFSIM commands, models, and/or script classes.
Interface changes will not be applied to currently available patch releases of AFSIM.
In other words, interface changes may be announced (via deprecation in documentation) in AFSIM vX.Y.0 and applied to vX.(Y+2).0 or later,
but should not be applied to AFSIM vX.Y.1 or vX.Y.2, where X is the major version number, and Y is the minor version number.

Major version changes are infrequent. It is acceptable to introduce significant interface changes between major versions.
These changes may not be documented in the previous major version documentation,
but should be documented in the applicable major version documentation, providing conversion instructions or analogous examples where applicable.

Version support is enforced through auto tests in multi-platform (Windows, Linux) Continuous Integration (CI).

### Deprecation

When deprecating features in user-space… (input commands, script classes/methods)

 - Generate a warning in the code if the user attempts to use the feature. The warning should indicate the feature is deprecated,
   and should also guide the user to an alternative or replacement, if applicable.

   ```cpp
      auto out = ut::log::warning() << "The <EVENT_NAME> event has been deprecated and will be removed "
                                    << "in a future release. Use <ALTERNATIVE_EVENT_NAME> instead.";
      out.AddNote() << aInput.GetLocation();
   ```

 - Add deprecation warning to the user-facing documentation, using the deprecated directive, which indicates the version in which the feature was deprecated.
   As above, suggest an alternative.

   ```
      .. deprecated:: 2.9 <EVENT_NAME> event is deprecated and will be removed in a future release. Use <ALTERNATIVE_EVENT_NAME> instead.
   ```

 - Add an entry to the General section of the appropriate changelog documenting the deprecation.
 - Update everything managed in user-space which previously used the deprecated feature.
   This may include, but is not limited to, demos, training, and automated tests.
 - If you are deprecating an input command/parameter, remove it from the grammar (script grammar is generated automatically from source code).
   This serves two purposes:

   1. In Wizard, the command will no longer appear in the auto-complete popup as a suggestion to users.
   2. It will allow you to catch all usages of the command in the automated tests when the grammar check fails.

When deprecating developer-facing functionality… (c++ classes, methods, aliases, namespaces, etc.)

 - Add the appropriate deprecated macro, generated by CMake in the “exports” header, or the CPP14_DEPRECATED macro defined in UtCompilerAttributes.hpp

   ```cpp
      WSF_DEPRECATED int AddEvent(const std::string&          aEventName,
                                  std::unique_ptr<UtCallback> aCallbackPtr,
                                  const std::string&          aDefaultScriptName,
                                  const std::string&          aExpectedArgumentTypes)
   ```

 - Update all usages in the managed codebase, including source modules in afsim_shared if necessary.
 - Add an entry to the Development section of the changelog documenting the deprecation.

In each case, create an issue in the backlog to track the removal of the deprecated feature/functionality, under the Manage Technical Debt Epic.

A deprecated feature may be removed two minor releases (approx. 1 year) after the release in which it is deprecated.
Features should not be deprecated in patch releases.

## Testing Your Contribution

The following tools and processes should be considered for use prior to submitting a contribution for review.
In some cases one or more of the tools described below may not be necessary or applicable with respect to your change.
If you're not certain whether a test is needed for your change, it is better to err on the side of caution.
Likely, if you suspect that you are skipping a step, a reviewer will request that you complete it as part of the review anyway.

### Demos and Tests

When introducing new functionality or correcting issues with existing functionality,
it is critical to incorporate/update tests and demos with your contribution.
Unit tests and integration tests (i.e. auto tests) are available for reference throughout the distributed codebase in test and test_* directories.
Tests should use built-in testing utilities such that tests are run during CI.
Cases should be in place to identify and return the conditions for success and failure.
Compiler checks may be created in source code to verify program correctness at compile time where appropriate.
If asserts are used, debug information must clearly specify the conditions for success.

See "Tests" under `./doc/developer/integration_guide.rst` for more information on configuring and writing tests.

Similarly, new functionality should be accompanied by a demonstration file, or file set,
(demo) included in the AFSIM Demos repository or other appropriate location.
A demonstration file should highlight one or more functions of the feature, but a demonstration of all functionality is not strictly necessary.
This differs from a test in that tests should, to the extent possible, verify feature functionality exhaustively.

### Code Analysis

In many instances, the following tools provide insights and value when tested against contributions.
These tools are used in CI against the distributed codebase periodically to catch hard-to-detect defects/vulnerabilities in source code.
Because each of these tools is run against this repository in CI, it is appropriate and necessary to run each against your contribution
prior to submission. In many cases, your contribution will result in no new errors or warning from these tools, but not always. Errors and warnings
generated by these tools relating specifically to your contribution must be addressed. If these tools are omitted from your testing and an issue/
vulnerability is later discovered by routine CI, the responsibility falls to the contributor/steward to address these with priority. 

#### Cppcheck

[Cppcheck](http://cppcheck.sourceforge.net/) is a static analysis tool for C/C++ code.
As described in the Cppcheck documentation, it provides unique C++ code analysis to detect bugs and focuses on detecting undefined behavior
and dangerous coding constructs with the goal of having very few false positives.

Cppcheck is executed against this repository in multi-platform (Windows, Linux) Continuous Integration (CI).

To manually execute Cppcheck against the working branch with the proper configuration, run the CPPCHECK CMakeTarget.

By default, output is piped to standard out and may be directed to an alternate output via configuration.

For more information on Cppcheck and its usage, see [Cppcheck on SourceForge](http://cppcheck.sourceforge.net/).

#### Valgrind

[Valgrind](https://valgrind.org/) is an instrumentation framework for building dynamic analysis tools.
There are Valgrind tools that can automatically detect many memory management and threading bugs, and profile your programs in detail.

Valgrind is executed against this repository in Linux Continuous Integration (CI).
Valgrind reports memory-leaks and related defects which should be corrected before changes are accepted.

On Unix compatible systems, Valgrind is executed against AFSIM core applications through the `<application>_AUTO_TEST_VALGRIND` CMake target,
Valgrind is invoked through the --run-valgrind argument to`auto_test.py` for each application, which then calls `tools/misc/auto_test.py`

By default, auto tests are configured to generate Valgrind output to XML.

#### Fortify

As described in the
[Fortify SCA User Guide](https://www.microfocus.com/documentation/fortify-static-code-analyzer-and-tools/1920/SCA_Guide_19.2.0.pdf),
Fortify Static Code Analyzer is a set of software security analyzers that search for violations of security specific coding rules and guidelines
in a variety of languages.
The Fortify Static Code Analyzer language technology provides rich data that enables the analyzers to pinpoint and prioritize violations so that
fixes are fast and accurate.
Fortify Static Code Analyzer produces analysis information to help you deliver more secure software,
as well as make security code reviews more efficient, consistent, and complete.
Its design enables you to quickly incorporate new third-party and customer-specific security rules.

A Fortify scan is the minimal test to be conducted on any AFSIM module that concerns networking capabilities such as ports, IP addresses, etc. and
is additionally necessary to ensure defensive coding practices are used throughout the distributed codebase.
Fortify runs against AFSIM via CI, and reports potential vulnerabilities. Known vulnerabilities are not to be introduced to this repository.
When existing vulnerabilities are discovered they are patched with priority depending on severity.
Currently, Fortify scans are conducted by AFSIM core development teams and not strictly necessary for contributors for submission.

### Regression Testing

Regression tests are valuable when refactoring code where no change in output or small changes in output are expected.
Regressions on your branch may be compared against the versions created by master to verify that no differences,
or only expected differences, are present between the test sets.

Regressions are valuable when making changes to core code, particularly when refactoring core code,
where it is otherwise difficult to verify the impact or validity of changes.

Regression test sets may be generated via the applicable *_REGRESSION_TEST CMake target from your working branch.

These are to be compared against the same test set as generated by the same *_REGRESSION_TEST CMake target from the master branch.

Regression tests should be identical except for expected cases where the intent is to change the output of specific regressions.

See "Running the Regression Tests" under `./doc/developer/build_instructions.rst` for more information on regression testing.

### Performance Analysis

For contributions which are likely to affect run time performance of AFSIM applications,
care should be taken to ensure that implementation is computationally optimized.

Performance metrics may be created by testing application performance under conditions where quantitative dependent variables are
measured against a chosen independent variable. In this context, the independent variable is the presence of the contribution itself.
Performance profiles for a contribution or change must be measured against a baseline (usually the performance of the master branch for a given scenario).
Contribution performance must be maintained well in excess of the magnitude of expected usage, and new contributions should not introduce
run-time bottlenecks nor should they exacerbate existing bottlenecks.

Changes designed to alleviate existing bottlenecks should similarly provide a performance comparison against a baseline and be tested in well in
excess of the expected use case.

The results of performance profiling should be included with the contribution when the change request is submitted for review.

## Submitting Your Contribution

When you are ready to submit your changes for review, you may submit a change request through the appropriate AFSIM portal.
This will vary by organization and purpose, but is likely to be the same place where you originally cloned the AFSIM repository.

### How & Where to Make a Contribution

See [Creating Pull Requests](https://confluence.di2e.net/x/lAgxP)

### Assigning Reviewers
In each directory of the afsim core repository there are REVIEWERS files with a list of reviewers to be assigned to a Pull Request with changes in that directory.
Use the following guidance to select reviewers for your Pull Request.

 - Assign the Reviewers under that directory to your Pull Request.
 - If there are no Reviewers listed for your changed directory traverse up the path for determining appropriate reviewers to assign.
 - Continue assigning reviewers until a minimum of 3 reviewers have been assigned or you have included all reviewers in your changed directory tree up to and including the root afsim directory.

Further information including identification of Release Managers can be found at [Creating Pull Requests](https://confluence.di2e.net/x/lAgxP)

### Submitter Review Expectations

It will depend on the focus of the reviewer, in general:

 - Each reviewer will do one or two things:

   - Suggest improvements to your changes
   - Identify problems created by your changes

If the reviewer has no further concerns or suggestions relating to a given change, he/she is expected to approve
in a timely fashion contingent upon resolution of outstanding comments/tasks of other reviewers or the submitter.

All reviewers and submitters have common expectations:

 - Professionalism, Respectful Discourse
 - Focused Dialogue
 - Promptness/Responsiveness on Discussions
 - It is expected that the change request author will address open discussions and tasks
 - Once addressed, it is usually the responsibility of the task originator to resolve tasks they have created.
 - Tasks indicate action is requested by the reviewer.
 - Task resolution is potentially a lengthy operation - multiple asynchronous communications

   - Minimum 2 actions required after task origination (1 submitter response + 1 reviewer response/resolution)
   - Reviewers will plan to open focused, concise tasks.

 - If a reviewer knows exactly what the expected change looks like to close a discussion/task,
   the reviewer is encouraged to make exact/explicit suggestions for changes to optimize review and revision time.

### The Review Process

See [Contributions](https://confluence.di2e.net/x/m6_uH)

## Contribution Acceptance

To reiterate: For the longevity of AFSIM, it is critical that existing functionality and operational stability are maintained and/or enhanced
for all changes introduced to this repository.
In this effort, contributions will undergo scrutiny and testing with all applicable tools before acceptance.
This means that contributions must meet all applicable requirements defined in this contributing guide,
in addition to other case-specific requirements.

Contributions should meet all applicable requirements defined in this guide before submission for review.
Changes will be accepted upon verification of functionality, stability, and completeness in accordance with this guide.
