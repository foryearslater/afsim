// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

// **************************************************************
//
// Copyright 2020 Infoscitex, a DCS Company. All rights reserved.
//
// **************************************************************


// Global Resize Timer
var gResizeTimer;

// Trigger collapsible resize handler AFTER
// built-in page resizing is complete
$(window).on('resize', function(e) {
   clearTimeout(gResizeTimer);
   gResizeTimer = setTimeout(resizeCollapsibleSections, 200);
});


// JQuery Event Handler: Ready
// When document is ready (All text is printed onscreen)
$(document).ready(function () {
   // Retrieve level 2 and 3 section headers on this page
   var h2SectionHeaders = checkSectionHeader(document.body.getElementsByTagName("H2"));
   var h3SectionHeaders = checkSectionHeader(document.body.getElementsByTagName("H3"));

   // Make level 2 and 3 section headers collapsible
   makeCollapsible(h3SectionHeaders);
   makeCollapsible(h2SectionHeaders);

   // Pseudo-Classes are reserved for use in the AFSIM Documentation Standard.
   // Exclusively in afsim_documentation_standard enable
   // collapsibility for Header-Type pseudo-class elements.
   var pageName = location.pathname.split("/").pop();
   if (pageName.startsWith("afsim_documentation_standard")) {
      makeCollapsible(document.body.getElementsByClassName("pseudo-h3"));
      // To make pseudo-element collapsibility behave as the actual - matching HTML DOM structure
      // parent layers twice wrap child pseudo-elements.
      makeCollapsible(document.body.getElementsByClassName("pseudo-h2"));
      makeCollapsible(document.body.getElementsByClassName("pseudo-h2"));
   }
});

// Function checkSectionHeader
// aHeaderList: An unfiltered list of headers in the current document
// return: A list containing only section headers
function checkSectionHeader(aHeaderList) {
   var sectionHeaderList = [];
   for (let i = 0; i < aHeaderList.length; i++) {
      var parent = aHeaderList[i].parentNode;
      if (parent.classList.contains("section")) {
         sectionHeaderList.push(aHeaderList[i]);
      }
   }
   return sectionHeaderList;
}

// Function makeCollapsible
// Designed to work specifically with Sphinx generated ReStructuredText to HTML
// aSectionHeaderList: Section headers to be made collapsible
function makeCollapsible(aSectionHeaderList) {
   // For each section header
   for (let i = 0; i < aSectionHeaderList.length; i++) {
      var header = aSectionHeaderList[i];
      // Add the expanded class to introduce collapsed/expanded symbology - default expanded (See collapse.css)
      header.classList.add("expanded");
      // Connect a collapse/expand handler function for when this section header is clicked
      header.addEventListener("click", toggleCollapse);

      // Section contents are generated, by Sphinx, as HTML siblings to the section header
      // The section header, and any number of siblings (visually children),
      // are children of a <div> ... </div>, where the header is the first child
      // Use this information to determine all content belonging to this section
      var contents = [];
      // Start at header's first sibling
      node = header.nextElementSibling || header.nextSibling;
      while (node) {
         // As long as node isn't the section header itself, and it is an element
         if (node !== header && node.nodeType === Node.ELEMENT_NODE) {
            contents.push(node); // It is content belonging to this section
         }
         node = node.nextElementSibling || node.nextSibling; // Go to the next content
      }

      // Once section content is known, wrap it to preserve appearance and simplify collapse/expand operation
      var sectionContentWrapper = document.createElement('div');
      header.parentNode.appendChild(sectionContentWrapper);
      for (let i = 0; i < contents.length; i++) {
         sectionContentWrapper.appendChild(contents[i]);
      }
      // Apply CSS rules for collapsible content
      sectionContentWrapper.classList.add("collapsible-content");
      sectionContentWrapper.style.maxHeight = sectionContentWrapper.scrollHeight + "px";
   }
}

// Function toggleCollapse
// Toggle visibility of section content
function toggleCollapse() {
   // Toggle current symbol (See collapse.css)
   this.classList.toggle("expanded");
   this.classList.toggle("collapsed");

   // For content belonging to this header
   var contents = this.nextElementSibling;
   var mHeight = window.getComputedStyle(contents).maxHeight;
   if (mHeight !== "0px") {
      contents.style.maxHeight = "0px"; // If contents are visible, hide them (i.e. Collapse)
   }
   else {
      contents.style.maxHeight = contents.scrollHeight + "px"; // Otherwise, show them (i.e. Expand)

      // If this collapsible section is nested within another collapsible section, resize the parent collapsible as well
      // A recursive version of this may be required if deeper section collapsibility is introduced in the future (h4 - h6)
      parentIsCollapsible = this.parentNode.parentNode.classList.contains("collapsible-content");
      if (parentIsCollapsible) {
         parentCollapsible = this.parentNode.parentNode;
         // Only resize if the parent is expanded
         if (parentCollapsible.previousElementSibling.classList.contains("expanded")) {
            var maxHeightValue = parseInt(parentCollapsible.style.maxHeight, 10);
            var contentHeightValue = parseInt(contents.scrollHeight, 10);
            var newHeightValue = maxHeightValue + contentHeightValue;
            var newMaxHeight = newHeightValue + "px";
            parentCollapsible.style.maxHeight = newMaxHeight;
         }
      }
   }
}

// Function resizeCollapsibleSections
// Correct any collapsible section height issues due to page resizing
// Must occur after built-in page resizing is finished
function resizeCollapsibleSections() {
   // Because correction needs to happen after page resizing,
   // the simplest way to correct is to cycle collapse state for each section,
   // allowing toggleCollapse to calculate the necessary adjustments
   sectionContents = document.body.getElementsByClassName("collapsible-content");
   for (let i = sectionContents.length - 1; i >= 0; i--) {
      content = sectionContents[i];
      content.classList.add('notransition'); // Disable transitions to make this fast
      content.previousElementSibling.click();
      content.previousElementSibling.click();
      sectionContents[i].offsetHeight; // Trigger a reflow, flushing CSS Changes - force notransition
      sectionContents[i].classList.remove('notransition'); // Re-enable transitions
   }
}
