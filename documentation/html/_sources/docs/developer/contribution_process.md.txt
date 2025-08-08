<!-- **************************************************************************** -->
<!-- CUI                                                                          -->
<!--                                                                              -->
<!-- The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)     -->
<!--                                                                              -->
<!--  Copyright (C) 2021 Stellar Science; U.S. Government has Unlimited Rights.   -->
<!--                                                                              -->
<!--  The use, dissemination or disclosure of data in this file is subject to     -->
<!--  limitation or restriction. See accompanying README and LICENSE for details. -->
<!-- **************************************************************************** -->

# Contents

1. [Introduction](#introduction)
    1. [Role Definition](#role-definition)
    2. [Motivation](#motivation)
    3. [Guiding Principles](#guiding-principles)
2. [Contribution Process](#contribution-process)
    1. [Pull Request Flow](#pull-request-flow)
    2. [Release Branches](#release-branches)
3. [Git Procedures](#git-procedures)
    1. [History Rewriting](#history-rewriting)
    2. [Commit Message](#commit-messages)
4. [Tool Configuration](#tool-configuration)
    1. [Developer Git Configuration](#developer-git-configuration)
    2. [Infrastructure Configuration](#infrastructure-configuration)

# Introduction

This document describes the AFSIM processes for change contribution, whether submitted by core maintainers or external contributors.
The document describes the process flow, but neither depends on nor presupposes a particular toolset or infrastructure availability other than Git itself.
Likewise, this document does not provide detailed implementation procedures since AFSIM development occurs in multiple environments and is facilitated by multiple hosting infrastructures.
To the extent possible the tools in each environment should be configured to support and encourage following this process, but the ultimate responsibility is with the individuals doing development and maintenance.
The process still applies even in highly constrained environments with no supporting infrastructure (e.g. file access only, or no access permission restrictions).

Similarly, this process does not specify particular acceptance criteria for contributions managed by this process.
For the current criteria see the `CONTRIBUTING.md` file at the top-level in the `afsim` source repository.

The eventual entry point for contributing changes is through a Pull Request (PR).
Maintainers should coordinate development for PRs within the issue tracking system.
For external contributors, maintainers will create a tracking issue once the contribution has been identified and coordinated.

As of this writing, the reference Unclassified development infrastructure is [DI2E](https://www.di2e.net).
For specific mechanics in that environment see AFSIM's [Creating Pull Requests](https://confluence.di2e.net/x/lAgxP) page and other associated documentation in that space.
External contributors on DI2E should initiate contribution discussions through the AFSIM project discussion on [Discourse](https://discourse.di2e.net/c/private-project-discussion/afsim/30) (private project discussion limited to AFSIM distribution channels).
After discussion, external contributors should then use the "Propose Enhancement" functionality on the [https://confluence.di2e.net/display/AFSIM/Enhancements](Enhancements page on DI2E).

---
**NOTE**:
This process is normative for artifacts and repositories in the distributed codebase.
The maintainers of managed, shared, and external AFSIM-related repositories are encouraged to follow a similar process, but may choose to adapt as best suits their needs.

---

## Role Definition

The three primary roles associated with the contribution process are Contributor, Maintainer, and Release Manager.
Some individuals may operate in multiple roles at different times, but should only take any given action when acting in a particular single role.

* Contributor

  A contributor is any individual from the AFSIM community who submits changes to AFSIM to be available to the community at large.
  Contributed changes may include new features, bug fixes, improvements, compatibility updates, or other valuable changes.
  The only requirement to be a contributor is that the individual is covered by an active AFSIM distribution agreement (e.g. MOU or ITA).
  It may sometimes be the case that a contributor is not the original author.
  However, by submitting a contribution they participate in the process as if they were the author, taking responsibility for review actions, either directly or as a liaison for the original author.

* Maintainer

  A maintainer is an individual designated by the AFSIM PMT with obligation for ongoing maintenance of some or all of the AFSIM codebase.
  At the discretion of the PMT, the pool of maintainers may include representatives from Military, Government, and Contractors.
  Maintainers have responsibilities beyond contributors, including overseeing product stability, advancing product capabilities, maintaining supported releases, and coordinating development and support activities.
  As it relates to the contribution process, maintainers' primary interaction is as reviewers on individual contributions.
  When an individual maintainer submits a contribution they are acting as a contributor and not a maintainer.

* Release Manager

  The release manager is a maintainer with decision authority on what changes are applied to previous releases.
  The primary release manager should delegate a small number of individuals to assist in their duties.
  The primary release manager and their delegates may all act in the role of release manager.
  While release managers have decision authority, all participants in the contribution process should be constantly cognizant of potential impacts to supported releases.

In addition to the three named roles above, the term "reviewer" is used.
Reviewer is not a dedicated role, but describes a maintainer performing review within the scope of a particular contribution.
Not all maintainers are required or even expected to review all contributions.

Since AFSIM development is conducted in an open source-like fashion, contributors have the opportunity to view and comment on contributions of others.
Such a contributor does not constitute a "reviewer", but a "commenter" whose constructive feedback, though non-binding, is welcomed and encouraged.

## Motivation

In order to coordinate and prioritize development efforts across the AFSIM community, there is a need for a coordinated contribution process.
The fastest acceptance path for any individual change would be to directly commit the change to the mainline development branch.
While such a process would invariably be faster, it brings with it tendencies for cross-organizational barriers which are undesirable for AFSIM development.
This contribution process was established to invite, encourage, and manage concurrent contributions from many groups.
As a process it invariably adds some overhead and accompanying processing delay.
However, in doing so, this process is designed to be as lightweight as possible, while still reasonably accomplishing the following objectives.

* Ensure overall correctness of changes
* Encourage net product improvement and consistency for the overall community
* Facilitate cohesive product design and evolution
* Enforce adherence to contribution guidelines
* Enable management of software API stability between releases
* Coordinate interdependent ingress of multiple changes
* Engage early to assure branches/commits are suitably structured for backporting to release branch(es)
* Coordinate multiple ongoing developments, manage merge conflicts vs. order dependent merges
* Provide intermediate stages for automated testing (CI, unit tests, documentation)
* Provide the community visible insight and tracking of work planned and completed

## Guiding Principles

The following are the primary guiding principles motivating this contribution process.

* Contributions are welcome to be submitted from all, regardless of organization or funding source.
  The submitting party is responsible to participate in reviews, possibly resulting in multiple iterations and reworking of a contribution.

* No contribution is too small, though some may be too large for effective review and acceptance when contributed as a whole.

* All contributions must go through a review process, regardless of contribution source.

* Contribution cadence and velocity in the review process should be as unburdened and efficient as possible, while still facilitating a trend of product improvement.

* The time of all participants is valuable; no undue work should be required of either.
  An efficient process helps all maximize community benefit.

* To help with process efficiency, some guidelines for good contributions include:
   * Make smaller PRs
   * Write useful descriptions and titles
   * Have on-point commit messages
   * Add comments to help guide the reviewer

* Review should ascertain if things are incrementally improved without regression.
  Contributions should meet applicable contribution guidelines, but as perfect is the enemy of good, discussions that turn towards "wouldn't it be nice if?" should usually be split off separately from the PR review.
  Said another way, reviewers should keep the [YAGNI](https://en.wikipedia.org/wiki/You_aren%27t_gonna_need_it) principle in mind.

* The main development branch (e.g. “master”, “main”, “develop”) is not sacrosanct.
  All should strive to keep it buildable and stable on all supported configurations, but if CI identifies issues then CI is doing its job.
  This is especially the case for cross-platform or differing compiler versions than a developer may have used.
  Issues missed during review but subsequently identified should be highly prioritized for resolution by the original PR contributor through a subsequent PR.
  CI is merely a tool. CI jobs failing for causes clearly unrelated to the PR itself may be reasonably excluded as a criteria as long as required testing on supported platforms has been conducted.

* As AFSIM is a GOTS product, there are no exclusive code owners.
  To manage the ongoing product evolution, maintainers of the codebase are designated by the AFSIM PMT.
  The set of maintainers may morph and evolve over time.
  As maintainership may evolve over time, having current and clearly documented processes serves an important coordinating function between maintainer groups.
  Maintainership implies neither rigid nor impermeable boundaries, and is not exclusive authority or ownership.

* No single team is expected to have full awareness of all aspects of the product.
  Frequent collaboration and consultation between maintainer groups is paramount.

* It should be clearly identifiable to contributors where to start for any given contribution.

* While any maintainer can review any given contribution, a contribution has an associated primary reviewer.
  The primary reviewer serves as the coordinator and point of contact for the contribution, as well as a determiner of when "good enough" has been achieved.
  Responsibility for the final merge of a contribution is usually handled by the primary reviewer for that contribution.

* In cases of differences of opinion among maintainers, the Principal Architect serves as a decisional arbiter.

* A variety of maintainer reviews within and across teams should be encouraged to foster consistent review practices and facilitate breadth of exposure to the codebase.

# Contribution Process

Just because a PR is structured does not mean it needs to be slow or burdensome.

Just because a PR should be fast does not mean it should be loose.

All contributions must be reviewed and approved by at least two suitable maintainers (other than the contributor) prior to acceptance.
The requirement for two review approvals also applies to "fixup" contributions such as typo corrections; these changes should be trivial and quick to review.

It is suggested that three initial reviewers be selected for a PR.
While having three reviewers is not required, it enables identification of suitable reviewers and provides flexibility for which of an initial reviewers pool will perform the review.
Adding too many reviewers can be counterproductive, in that it frequently leads to a situation where each reviewer sees many others, assumes their input is not key.
Such conditions often give rise to a situation where many are able to handle a task, but all assume someone else will handle it, resulting in no one addressing it.

In cases where a given contribution impacts many teams a PR should have more reviewers assigned to provide opportunity for comment across teams.
In such cases, it is not expected that all reviewers provide comprehensive review.
High-level concurrence (or at least lack of non-concur) on the general approach can itself be useful in such situations.
When given, concurrence is not sufficient to count as a review.
Detailed review is still required from at least two reviewers, and they should be clearly identified.

To assist contributors in identifying initial candidates to serve as reviewers, `REVIEWERS` files are included throughout the codebase.
These are purely a convenience mechanism, and do not rigidly prescribe who may be a fitting reviewer (either via inclusion or exclusion).
Maintainers should add and remove themselves as appropriate from `REVIEWERS` files throughout the codebase.
This helps the `REVIEWERS` files to serve as an accurate reflection of point in time maintainership.

When a reviewer is designated for a given PR, the reviewer may defer to another reviewer as needed.
This may be due to expertise in a particular aspect of the product, or simply an individual having more time available when needed.
Maintainers should also have alternates and backups in place in cases of leave, meetings, or other obligations.

The best set of reviewers for any given PR is case dependent.
It may be software tool heavy, operations interaction heavy, physics heavy, or any mixture including various matters of subject expertise.
The primary maintainer for a PR should strive to ensure that an appropriate representation of reviewers is used given the nature of a PR.

Maintainers who also serve as contributors (as is very often the case) must not count toward the required reviewer threshold for their own contributions.
This includes individual maintainers who also serve as external contributors in other capacities.

## Pull Request Flow

A structured request to merge a branch containing contributions into the main development branch is interchangeably termed a pull request (PR) or merge request (MR).
An alternative (often more generalized) term is a change request (CR), or changelist (CL) within Google.
AFSIM documentation uses the term PR for the following reasons:

* It is the term used by Bitbucket, the DI2E code hosting service
* It is [more commonly used](https://trends.google.com/trends/explore?geo=US&q=pull%20request,merge%20request).

### PR Preparation

Before creation of a PR a contributor may use branches on the git repository as a staging area.
By so doing, a developer may enable early access CI opportunities, exchange with other developers, and provide a backup of work in progress.
Branches may exist without an associated PR.
However, they may be subject to occasional pruning of idle, abandoned, or otherwise stale branches.

Should one so choose, a contributor may create a work-in-progress (WIP) or draft PR by clearly labeling it as such.
The conventional way to mark a PR as work-in-progress is by prefixing the title with `WIP:` or `(WIP)`.
The WIP status can be useful to indicate openness for informal feedback on a general direction while allowing reviewers to distinguish it as not expected to be fully ready.
There are no process requirements for WIP PRs.
Accordingly, the same timeliness and responsiveness from reviewers should not be expected.
In fact, reviewers may or may not consider changes ready for review while in WIP state, unless specifically requested by the contributor.
WIP PRs are often useful for intra-team collaboration and performance of internal team reviews, while subsequently having team review comments available to final reviewers rather than having to duplicate discussions.
WIP PRs should be actively worked and not allowed to languish idly for long durations or go stale.

PR branches should be structured to be considered logically both separate and complete.
Interdependent changes should be made in a single commit, while a logical sequential application of changes should be a series of multiple commits in a single branch.
Short-lived branches and more frequent [complete] PRs should be preferred in order to reduce associated churn such as merge conflicts.

For automated or tool driven changes (i.e., sed, clang-tidy, etc.) a dedicated commit should be used just for execution of the command without other pre- or post-editing involved.
For such automated changes, include the command and arguments used in the commit message, both to be preserved in history but also to assist reviewers.

To facilitate efficient contribution and review, single purpose contributions are strongly encouraged.
A large bulk contribution is often better structured as a sequence of individual PRs building on top of each other.

A new branch will preferably be based on the main development branch at the time of initial development.
It is fully expected that the PR branch and the mainline branch will diverge.
There is no general need to rebase a branch onto the main development branch prior to submitting a PR, and doing so preemptively or out of habit is discouraged.
When merge conflicts arise it is the responsibility of the contributor to merge the main development branch into the PR branch when needed in order to resolve conflicts.

The commit history of a repository is important and should serve as a living engineering notebook for the product, including rationale, constraints, and considerations at the time changes were made.
Likewise, the branch for a PR should contain a logical progression of commits.
In some cases a single commit is sufficient, but often multiple well structured and well ordered commits add considerable context, both for review as well as historical information.
The final branch submitted for review should not contain haphazard commits along the lines of “oh, forgot two other places” or “whoops, forgot yet another three places”.
An interactive rebase may be used to adjust commit messages and/or improve structure or ordering of commits.
Only the contributor should perform an interactive rebase, but it may be requested by a reviewer.
Changes made in response to review feedback should not be interactively rebased into original commits, although it may be desirable to interactively rebase a section of multiple review addressing commits.
In fact, retaining the separation of commits and history is often valuable.
When used, automated tool commits generated on behalf of the contributor need not be interactively rebased, but may be if desired.

A branch should be compile tested with both MSVC and GCC (the supported compilers) before being turned into a PR.
This testing may be on a local development environment and/or using an available CI system.
It is not required to use the minimum compiler versions.
CI builds against the PR branch should help identify such issues and each developer maintaining multiple local compiler versions tends to be cumbersome.

A contributor should strive to collect all associated changes of a particular theme into the same PR to avoid multiple back-to-back submissions of the same content and context.
This helps reduce reviewers' need for mental switching of “Didn’t I just see this change?”, as well as enables easier collection of all changes on a single branch for branch-based CI execution.

### PR Creation

The creation of a PR is a specific action taken by a contributor to indicate the end of development on a branch and begin of review.
It is the last stage of contribution, preceded by creation of an issue and subsequent development against the issue.

When a PR is initiated (or updated to remove the WIP designation) by a contributor it should reflect a belief that a particular branch is ready to be merged into the main development branch.
It is the responsibility of a contributor to perform due diligence in testing and ensuring the submitted branch is complete according to contribution guidelines as specified in `CONTRIBUTING.md`.

The contributor should themselves review the series of commits and differences in each to ensure all changes are expected.
A quick follow-up of additional commits often indicates a hasty submission.
Once created, it is the responsibility of the contributor to notify appropriate reviewers that the PR is ready for review.

A PR should have a description covering the scope of all constituent commits.
Substantial time should not be used to craft idealized PR descriptions as they are not included in the permanent change history.
Oftentimes the description may be autogenerated from the first commit message and/or a summary of commit subjects may be clear enough to describe the purpose.
If extra time is needed to craft a PR message, the contributor might consider opportunities to improve the underlying commit messages.

In most cases, a PR should reference an outstanding issue addressed or resolved by the PR.
PRs that provide trivial fixes may be submitted as "fixup" PRs, without the requirement to be tied to an issue.
A fixup PR should include the "fixup: " prefix in the PR title.
Trivial in this context means typographical errors, cleanup, or other readily and clearly reviewable changes.
They must be changes that improve understandability or maintainability without changing the original design, implementation, or runtime behavior.

Examples of acceptable scope of "fixup" PRs include, but are not limited to:

* Editorial changes to user-facing documentation in the form of reStructuredText or Markdown:

   * Fixing grammatical or spelling errors
   * Revising a sentence or paragraph to improve readability without changing the original written intent
   * Improving style consistency in accordance with the latest AFSIM Documentation Guide
   * Reconciling documentation with features, as implemented

      * Updating (outdated) documentation (e.g., adding a deprecation note)
      * Updating (erroneous) documentation (e.g., an incorrect parameter name or type)
      * Adding (missing) documentation (e.g., limited documentation coverage for a feature)

   * General cleanup of C++ source artifacts:

      * Improving source code formatting and consistency in accordance with the latest AFSIM Coding Standards
      * Fixing spelling or grammatical errors in source comments
      * Fixing a swapped or incorrectly named variable or argument (e.g., a find-replace or refactoring mistake)


Once a PR is created the associated branch should remain stable.
Stability of a PR branch means that no new commits should be added which do not address review commentary.
Keeping a PR stable assists reviewers in having a stable target for review.
Similarly, the contributor should refrain from rewriting branch history except as coordinated with reviewers.
This facilitates easier tracking by reviewers of what has been reviewed so far, and during review follow-up enables only showing changes since last review.

The contributor should identify at least one initial reviewer, and more if known.
If no reviewer is assigned the PR has a high likelihood of going unnoticed for a while until identified in periodic checks of lingering PRs.

### PR Review

In order to be ultimately merged, a PR must have two review approvals.

The review of a PR commences with the initially selected reviewers.
After creation and selection of initial reviewers, the initial reviewers self-organize, identifying the primary reviewer for the PR.
First, the selected reviewers assess their suitability as reviewer for the particular change, identify a primary reviewer, and add (or replace themselves with) alternative reviewers as needed for the particular change.

Not all PRs are equal in size, even when only considering single-purpose PRs.
Some are trivial to fix a single typo which can be very quickly visually reviewed, while others span many files across several areas.

There is no absolute threshold mandated for timeliness in PR review.
An arbitrary mandate would disincentivize thoroughness in review.
That being said, the expected time for review should be small, relative to the size of the changes.
Large and/or complex changes all at once often take exponentially compounded time for adequate review.
Once received, contributions from external sources may be subject to maintainer resource prioritization.

As a rule of thumb however, a contributor should reasonably be able to expect initial review feedback within 24-48 hours of submission, with smaller PRs taking less time and larger ones prone to taking longer.
Initial review feedback may not be comprehensive, but should provide the contributor an initial acknowledgement as well as expected timeframe based on size and complexity of the change, as well as reviewer schedule availability.
Specifying such a mean-time-to-review helps gives the contributor a basis to know when to followup or consider adding and alternate reviewer.

While reviews should be comprehensive, there is no requirement or expectation that a reviewer find something on which to comment.
If a PR is deemed ready, then a simple approval is a perfectly acceptable fulfillment of a reviewers duty.
Searching solely to find minutia for comment is counterproductive and strongly discouraged.
A review is of no lesser value if the only action from a review is approval.

In addition to end state content of the change itself, reviewers should review commit messages for context and correctness.
Commit messages will live on in history, while PR specific writeups and descriptions often will not.
Reviewers will vary in their approach to review, either the large resulting differences and commits separately, or incrementally review the commits as a sequence.
Either approach is equally acceptable, as long as both content and commit messages are reviewed.

It is helpful for contributors when reviewers make a clear distinction between comments for discussion and asking for clarity versus tasks to be done blocking successful review.
Both should be taken into consideration by the contributor, but knowing the remaining asked items helps with task tracking.
A task or action on a PR does not always mean that the suggestion must be taken.
If the contributor believes it would not be correct or appropriate, then explanation in the PR (and/or code comment) may be a suitable resolution.
An item specified as a contribution guideline in `CONTRIBUTING.md` should not be lightly disregarded.
By the same token the applicability of contribution guidelines should be subjective given the current state of effected code.
For example, if a change is made to code with no existing unit tests, then unit tests for the changed code are likely still warranted, but writing unit tests for the entire impacted module may be an excessive task compared to the relative scope of the PR.

PR review happens visible to the community.
As part of being in the open, early exposure of review and community comment should be welcomed.
Comments from non-maintainers should always be considered on their merit, but only reviewers may designate actions required for acceptance of the PR.

Tasks or actions generated as part of a PR will often include code changes, but may also include creation of spin-off issues or other side effects.
A reviewer may conditionally approve a PR pending resolution of identified actions with clear resolution outcomes.
Reviewers may choose to not use this option until path of resolution is clear and agreeable to contributor and reviewers.

The focus and energy of review should be on semantics, interactions, and interfaces.
Reviewers should rely on CI, code formatting, and code linting tools and their reports where available.
Subject to availability, all PRs should pass basic CI executions.
Issues identified by CI during review which are caused by changes in the PR must be resolved before final PR approval.

Reviewers may additionally include execution of auxiliary CI pipelines in cases where a PR is reasonably expected to have higher likelihood for impact.
For example, overhauling of a documentation generation process would likely warrant additional execution of the documentation generation pipeline.
Reviewers should strive to balance usage of CI in review, considering CI resource capacity, expected impacts of a given PR, and not unduly impeding the PR process.
Prudently running auxiliary CI pipelines can be greatly beneficial in catching issues during review, however it should not become a requirement or expectation to run the full battery of available pipelines against every PR since it can delay review.

Reviewers should by mindful of protracted PR discussions.
In particular distinction should be made as to whether it is addressing a core aspect of the motivating issue, or venturing into postulation or identification of opportunity for future improvement.
Such discussion should usually be captured but noted separately so as to not encourage long review periods with unclear resolution.

A contributor may begin working and adding commits to address review feedback while review is ongoing.
Only fixes for review items with a clear resolution path should be pushed, items with active discussion or unclear path should be withheld until a clear resolution path is found.
As with the original development commits, review commits should be well organized and complete; "shotgunning" of multiple review fix commits can be distracting to the review process.
This is even more important for review commits since it enables reviewers to only review additional delta commits, thereby being efficient with review time.

If review of a branch identifies significant rework involved, it should be transitioned to a WIP state.
This helps reviewers keep it on their radar, but not have to frequently check if review action is required.
Once rework is complete, it should be transitioned back to an active PR state.

No one but the contributor should rewrite the branch history.
As an exception, with prior concurrence of the submitter, a reviewer may perform history rewriting within a branch under review.

In coordinating between reviewers, reasonableness should prevail and straight-forward changes which, as part of a larger change incidentally touch multiple other areas may not require review by each potential reviewer.
Many open source projects have guidelines on handling such cross-cutting or large scale changes (e.g. [Google Chrome](https://chromium.googlesource.com/chromium/src/+/refs/heads/main/docs/process/lsc/large_scale_changes.md) and [LibreOffice](https://wiki.documentfoundation.org/Development/LargeScaleChanges).
For AFSIM, distinct PRs should be used for such large scale changes, and should have a focused purpose such as formatting, standards conformance, consistent API updates, etc.
Using a distinct PR for each purpose helps with review efficiency as well as easing cleaner backporting of future changes onto release branches.

### PR Disposition

All actions from PR review should be resolved before merging.
Appropriate resolution of a PR task may include opening a new issue for future action, as long as it does not diminish the accuracy of the PR content.

If review determines that the requirement or approach in the PR is no longer applicable, or if the contributor withdraws the PR, then a PR should be closed without merging.
Occasional PR closure (i.e. decline to accept) is an indicator of healthy software development in the open and should in no way be construed as failure.

Once all actions are completed and sufficient reviewers have approved, the PR should be merged.
The primary reviewer for the PR should perform the actual merge after confirming that review thresholds are met and all relevant contribution criteria are met.
In some cases a maintainer may need to coordinate merging of several separate PRs in a preferred order.

When merging PRs, it should always be done as a non-fast-forward merge commit.
The original PR branch should not have history rewritten as part of the merge process, either via branch squashing or rebasing.
It is permissible for the primary reviewer to append additional cleanup commits onto the existing branch prior to merging, but history of the contribution must be preserved.
Rewriting history of a branch during PR merge is discouraged because it loses information and is detrimental, including the following notable ways:
* Loses context of timeline and basis of commits (development breakpoints)
* Elides credit from the original author and comitter
* Reduces effectiveness of git bisect and other tools for isolating/finding commits at natural phases of development
* Prevents contributor (and all others who have pulled the branch from the remote) from tracking and automatically identifying and removing merged local branches based on git history

If a PR grows excessively stale it should be rejected.
The opportunity always remains for a contributor to reengage and reopen a new PR at a future point in time.

## Release Branches

The Release Manager retains discretion on which commits are selected to be applied to one or more release branches.
Nevertheless, all participants in the contribution process should consider impacts to actively supported release branches and strive to identify release applicable change during initial issue creation.
Expectation of a change eventually being applied to one or more release branches may inform and influence the approach or structure taken for a given change, making it easier to backport.
Starting with the issue linked to a given PR, a contributor may recommend the PR as a candidate for one or more release branches.
Individual commits from within a larger branch may also be identified as candidates for backporting, but this is generally discouraged in preference to separate PRs.

Except in unusual circumstances as determined by the Release Manager, no commits should be made to a release branch that aren't already on the latest development branch (i.e. PR already approved and merged).
The preferred pattern for applying commits from the development branch to a release branch is usage of `git cherry-pick -x` in order to include traceability to the original commit.
This makes locating ancestry of an arbitrary commit easier at a later point.

The criteria for "stable" and "latest" release branches may each differ according to lifecycle timing and needs.
For example, criteria may be looser on a new release branch preparing for its initial release than an update to a previously released version from the branch.
The [Linux kernel stable release rules](https://www.kernel.org/doc/html/latest/process/stable-kernel-rules.html) are one of several public examples of other criteria to consider for release inclusion.
Some typical criteria for inclusion and exclusion are:

* Include
   * security updates
   * fixes for bugs in functionality contained in the release
   * fixes for compatibility, e.g. unintentional API changes
* Exclude
   * new features
   * new compatibility with newer toolchain than supported in the release

When nominating a PR for backporting to one or more release branches, it is preferable for this to be done using a "trailer" in the commit message itself such as `AFSIM-Backport: release_2p7, release_2p8`.
Branches identified in a commit message for backport should be tested by the contributor to identify if they will cleanly apply when cherry-picked.
If conflicts exist, a summary of recommended conflict resolution steps and rationale often greatly assists the Release Manager.
PRs for branches expected to be applied to release branches carry a heightened importance of only fixing one thing.

# Git Procedures

This section provides an overview of git commands frequently used in the PR process.
It only contains commandline references, but most graphical tools (e.g. UI wrappers, IDE integration) provide means to accomplish the same end result.

## History Rewriting

Rewriting history on a PR branch is frequently useful.
It allows cleanup of commit messages like "Whoops, missed three other occurrences".
Knowing that such cleanup can be done later helps encourage one to commit and push frequently during development.

History rewriting may be used frequently during development and preparation of a branch for submission.
However, once a branch has been submitted as a [non-WIP] PR, all should refrain from rewriting history unless requested by a reviewer.
Coordination during review helps minimize disruption for reviewers in tracking what has and has not yet been reviewed (e.g. when reviewing adjustments after initial review).

Rewriting history simply to stay current with the mainline development branch should be avoided.
It can however, be a reasonable and useful approach to resolve merge conflicts in the event they arise, but should only be used when needed and not preemptively.

Some git terminology can at times be confusing due to usage of overlapping terms.
Notably, "squashing" of individual commits within an interactive rebase should not be confused with an overall "squashing" of an entire branch on merge.
The squashing or fixup of individual commits within an interactive rebase provides control and granularity that branching squashing does not.
It also remains a separate branch, resulting in an eventual merge commit into the mainline development branch.
In contrast, squashing an entire branch on merge usually yields a single new commit on the mainline development branch, removing the metadata history that a merge had even taken place.

Each contributor will have their individual preferred development workflow.
A tutorial about how and when to use various history rewriting techniques in git is beyond the scope of this document.
Some commonly used approaches include `git rebase --interactive --rebase-merges`, `git rebase --autosquash`, and `git reset --soft`.

## Commit Messages

Usage of git as a version control system implies [customary formats](https://git-scm.com/docs/git-commit#_discussion) for commit messages.
AFSIM development does not mandate adherence to formal specifications like [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/), however contributors should be aware of the customary commit format.
Commit messages should adhere to the following guidelines:

* Required
   * Use a concise subject as the first line (soft limit of approximately 50 characters, hard limit of 70).
     Linux documentation: "It is challenging to be both succinct and descriptive, but that is what a well-written summary should do."
   * Start the body on the third line (leaving the second line blank).
   * Wrap the body at 72 chars (except contiguous string like URL references, and external quoting such as compiler error messages).

* Recommended

   * Begin the subject line with a relevant capitalized verb.
   * Use imperative mood and present tense, for example "Fix", "Improve", "Update" instead of "Fixed", "Improves", "Updating".
   * Don't include a trailing period on the subject.
   * Use commit ["trailers"](https://git-scm.com/docs/git-interpret-trailers) such as "Fix: AFSIM-1234" at the end of the message to associate commits to issues.

* Consider

   * Describe the "why" and the "how", not the "what".
     Exactly what change is captured and easily viewable in the commit diff itself.
     The background context and rationale for making a change is not always obvious (and when it is, a commit body may be superfluous).
     In particular, names of changed files are frequently redundant.
   * Strive to structure commits into logically distinct groupings.
     Some open source projects mandate that each individual commit in series successfully build; that is not a requirement but is a laudable goal.

# Tool Configuration

This section provides suggestions on tool configurations across environments.
It does not specify specific procedures for any tool or product, but declaratively states resultant behaviors for consideration.

As mentioned in the [introduction](#Introduction), tooling support is helpful, but the ultimate responsibility is with the people doing development and maintenance.

Various environments may offer different levels of service offerings and/or be at different stages of technical readiness; the following are considerations only and do not in any way represent a minimum capability.
Some limited environments may use nothing more than file/SSH hosted git repos and review PRs through `git-diff`, but the process should remain the same.

## Developer Git Configuration

In order to remain compliant with coding stadards, a git pre-commit hook will be executed to handle code formatting when a commit is made.
This hook will call clang-format on the changed files that are part of the commit. The git environment will be configured during the cmake configuration step to utilize
the afsim directory containing this hook (.githooks).

## Developer IDE Configuration 

Some IDEs like Visual Studio have clang-format support built-in or enabled via add-ons.
A contributor that desires to run clang-format locally via thier IDE before making a commit should
ensure that their IDE is properly configured to use a clang-format 9.0+ executable. Some IDEs such as 
VS2017 are integrated with a earlier version by default. 
 
## Infrastructure Configuration

Subject to availability within any given environment, the following are recommendations for consideration related to particular centralized infrastructure configuration.

* limit commit and merge access to maintainers for protected development branches (e.g. `master`, `main`, `develop`)
* limit commit and merge access to release managers for release branches (e.g. `release/*`)
* require all commits to release branches to either be annotated cherry-picks or merges of branch already merged to main development branch
* require tags to be annotated tags, not lightweight tags
* limit pushing of version tags to release manager (e.g. `v*`, `*-release`)
* limit pushing of other tags to be user prefixed (e.g. `${USERNAME}-*`)
* automatically assign initial reviewers using `REVIEWERS` files, considering WIP vs. final status
* require neccessary count of review approvals on a PR before it is mergeable
* require resolving all tasks/actions on a PR before it is mergeable
* require successful CI execution against the branch before a PR is mergeable
* forbid squash merge and fast-forward merges to protected development branches
* permit forch push on user branches by the matching user (i.e. those matching `${USERNAME}/*`)
* enforce server-side precommit git hook to validate clang-format for changed sections
* enforce server-side precommit git hook to validate commit message structure (with accomodation for some slop)
* provide notification (e.g. via email) on changes, including merge conflicts arising on PR branches
