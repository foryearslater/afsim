// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

/*
 * sidebar.js
 * ~~~~~~~~~~
 *
 * This script makes the Sphinx sidebar collapsible.
 *
 * .sphinxsidebar contains .sphinxsidebarwrapper.  This script adds
 * in .sphixsidebar, after .sphinxsidebarwrapper, the #sidebarbutton
 * used to collapse and expand the sidebar.
 *
 * When the sidebar is collapsed the .sphinxsidebarwrapper is hidden
 * and the width of the sidebar and the margin-left of the document
 * are decreased. When the sidebar is expanded the opposite happens.
 * This script saves a per-browser/per-session cookie used to
 * remember the position of the sidebar among the pages.
 * Once the browser is closed the cookie is deleted and the position
 * reset to the default (expanded).
 *
 * :copyright: Copyright 2007-2018 by the Sphinx team, see AUTHORS.
 * :license: BSD, see LICENSE for details.
 *
 */

$(function() {
   
   
   
   
   
   
   

   // global elements used by the functions.
   // the 'sidebarbutton' element is defined as global after its
   // creation, in the add_sidebar_button function
   var document = $('.document');
   var bodywrapper = $('.bodywrapper');
   var sidebar = $('.sphinxsidebar');
   var sidebarwrapper = $('.sphinxsidebarwrapper');

   // for some reason, the document has no sidebar; do not run into errors
   if (!sidebar.length) return;

   // original margin-left of the bodywrapper and width of the sidebar
   // with the sidebar expanded
   var bw_margin_expanded = bodywrapper.css('margin-left');
   var ssb_width_expanded = sidebar.width();

   // margin-left of the bodywrapper and width of the sidebar
   // with the sidebar collapsed
   var bw_margin_collapsed = '.8em';
   var ssb_width_collapsed = '.8em';

   // colors used by the current theme
   var dark_color = $('.related').css('background-color');
   var light_color = $('.document').css('background-color');

   // saves the state of sidebar data (collapsed states)
   // Try catch to handle Edge not allowing localStorage with local files
   try{
      var saveData = window.sessionStorage;
   }catch(e){
      // catch to stop Edge from crashing javascript
   }

   function sidebar_is_collapsed() {
     return sidebarwrapper.is(':not(:visible)');
   }

   window.addEventListener('resize', function(event){
      resize_document();
      resize_collapsable_sidebar();
      resize_collapsable_sidebar();
   });

   // Used to fix width and height of the sidebar.
   // Helps use and display of sidebar
   function resize_collapsable_sidebar(){
      sidebarwrapper.css({
         'width': $('.sphinxsidebar').width() - 28
      });
      var sidebarbutton = $('#sidebarbutton');
      // minus 5 to avoid possible scroll bar appearing
      var minHeight = $('.sphinxsidebar').height() - 5;
     
      // Fixes resize of screen and avoid sidebar disappearing
      var marginFix = $('.sphinxsidebar').width()-12; 
      if(sidebar_is_collapsed()){
         marginFix = 0;
      }
      sidebarbutton.css({
         'margin-left': marginFix,
         'height': minHeight
      });
      
      // Redo the arrows to be center
      sidebarbutton.find('span').css({
          'margin-top': minHeight / 2
      });
   }

   function toggle_sidebar() {
      if (sidebar_is_collapsed())
      {
         expand_sidebar();
         saveSidebar(false);
      }
      else {
         collapse_sidebar();
         saveSidebar(true);
      }
      // Run twice to avoid sidebar disappearing behind scroll bar
      resize_collapsable_sidebar();
      resize_collapsable_sidebar();      
   }

   // Store state in localStorage
   function saveSidebar(collapsed) {
      if(!saveData){
         return;
      }
      saveData.setItem("collapsed", collapsed);
   }

   // Get the saved state of the collapsed sidebar
   function loadSidebarState() {
      if(!saveData){
         return;
      }
      var collapsed = saveData.getItem("collapsed");
      if(collapsed === "true")
      {
         if(collapsed !== sidebar_is_collapsed())
         {
            toggle_sidebar();
         }
      }
   }

   function collapse_sidebar() {
      sidebarwrapper.hide();
      sidebar.css('width', ssb_width_collapsed);
      bodywrapper.css('margin-left', bw_margin_collapsed);
      sidebarbutton.css({
         'margin-left': '0',
         'height': bodywrapper.height()
      });
      sidebarbutton.find('span').text('»');
      sidebarbutton.attr('title', _('Expand sidebar'));
      document.cookie = 'sidebar=collapsed';
   }

   function expand_sidebar() {
      bodywrapper.css('margin-left', bw_margin_expanded);
      sidebar.css('width', '');
      sidebarwrapper.show();
      sidebarbutton.css({
         'margin-left': ssb_width_expanded-12,
         'height': bodywrapper.height()
      });
      sidebarbutton.find('span').text('«');
      sidebarbutton.attr('title', _('Collapse sidebar'));
      document.cookie = 'sidebar=expanded';
   }

   function add_sidebar_button() {
      // create the button
      sidebar.append(
         '<div id="sidebarbutton"><span>&laquo;</span></div>'
      );
      var sidebarbutton = $('#sidebarbutton');
      light_color = sidebarbutton.css('background-color');
      // find the height of the viewport to center the '<<' in the page
      var viewport_height;
      if (window.innerHeight)
         viewport_height = window.innerHeight;
      else
         viewport_height = $(window).height();
      sidebarbutton.find('span').css({
         'display': 'block',
         'margin-top': (viewport_height - sidebar.position().top - 20) / 2
      });

      sidebarbutton.click(toggle_sidebar);
      sidebarbutton.attr('title', _('Collapse sidebar'));
      sidebarbutton.css({
         'color': '#FFFFFF',
         'border-left': '1px solid ' + dark_color,
         'font-size': '1.2em',
         'cursor': 'pointer',
         'height': $('.sphinxsidebarwrapper').height(),
         'padding-top': '1px',
         'margin-left': ssb_width_expanded - 12
      });

      sidebarbutton.hover(
         function () {
            $(this).css('background-color', dark_color);
         },
         function () {
            $(this).css('background-color', light_color);
         }
      );
   }

   function set_position_from_cookie() {
      if (!document.cookie)
         return;
      var items = document.cookie.split(';');
      for(var k=0; k<items.length; k++) {
         var key_val = items[k].split('=');
         var key = key_val[0].replace(/ /, "");  // strip leading spaces
         if (key == 'sidebar') {
            var value = key_val[1];
            if ((value == 'collapsed') && (!sidebar_is_collapsed()))
               collapse_sidebar();
            else if ((value == 'expanded') && (sidebar_is_collapsed()))
               expand_sidebar();
         }
      }
   }

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

   // On document load, resize the sidebar
   $(document).ready(function() {
      resize_collapsable_sidebar();
   });

   add_sidebar_button();
   var sidebarbutton = $('#sidebarbutton');
   loadSidebarState();
   set_position_from_cookie();
});