// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

// global elements used by the functions.
// the 'sidebarbutton' element is defined as global after its
// creation, in the add_sidebar_button function
var document = $('.document');
var bodywrapper = $('.bodywrapper');
var sidebar = $('.sphinxsidebar');
var sidebarwrapper = $('.sphinxsidebarwrapper');

// add padding to bottom so search bar does not get hidden
sidebarwrapper.css({'padding-bottom': '60px'});

// saves the state of sidebar data (collapsed states)
// Try catch to handle Edge not allowing localStorage with local files
try{
   var saveData = window.sessionStorage;
}catch(e){
   // catch to stop Edge from crashing javascript
}

// On loads
$(document).ready(function () {
   // Set the sidebar and document according to window size on load
   resize_document(); // Sets up the proper size of small pages

   sidebarCollapsibleElements();
});

// Resizes the DOM whenever elements are added to the page
// Added b/c sidebar does not determine bottom document height only main body
var observer = new MutationObserver(function(mutations, observer) {
   mutations.forEach(function(mutation) {
      resize_document();
   });
});

// Listen for html elements to be added to the page to resize the document when to small
$(document).ready(function() {
   var target = $('.body')[0];

   observer.observe(target, {
      characterData: true,
      childList: true,
      subtree: true,
      characterDataOldValue: true
   });
});

// Check to see which is longer and set the document height to this.
// Because the sidebar can collapse, the document doesn't take it's size into consideration
// and for small pages this causes overlap of content
function resize_document() {
   $('.documentwrapper').css({'height': ''});
   var sidebarHeight = parseInt(sidebar.height(), 10);
   var bodyHeight = parseInt(bodywrapper.height(), 10);

   // Resize only if need to
   if (bodyHeight <= sidebarHeight) {
      $('.documentwrapper').css({'height': sidebarHeight});
   }else{
      $('.documentwrapper').css({'height': ''});
   }
}

// When the document is loaded it sets the state of the lists below headers
function sidebarCollapsibleElements() {
   $(".sphinxsidebarwrapper h3").each(function () {
      //Implement collapsibility, for sphinxsidebar menu items.

      // If the element is followed by a list...
      var nextElement = $(this).next();
      if (nextElement.prop("tagName") === "UL") {

         // First, retrieve collapsed/expanded state from saveData if available
         var displayValue = "";
         if (saveData) {
            displayValue = saveData.getItem($(this).text());
         }

         // Default to collapsed
         if(!displayValue)
         {
            displayValue = "none";
         }

         // Apply collapsed/expanded symbol, depending on displayValue (See collapse.css)
         if (displayValue !== "none") {
            this.classList.add("expanded");
         }
         else {
            this.classList.add("collapsed");
         }

         // Apply saved or default display state to current list
         nextElement.css({'display': displayValue});

         // Connect a collapse/expand handler function for when this menu is clicked
         this.addEventListener("click", toggleCollapseSidebarMenu);
      }
      else // Only apply collapsibility to lists
      {
         return;
      }
   });
}

// Listener that gets the h3 menu element when clicked and hides or shows the list below
function toggleCollapseSidebarMenu(){
      this.classList.toggle("expanded");
      this.classList.toggle("collapsed");
      var ulElement = $(this).next();
      ulElement.toggle();

      if(saveData) {
         saveData.setItem($(this).text(), ulElement.css('display'));
      }

      resize_document();
}
