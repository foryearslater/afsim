.. ****************************************************************************
.. CUI
..
.. The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
..
.. The use, dissemination or disclosure of data in this file is subject to
.. limitation or restriction. See accompanying README and LICENSE for details.
.. ****************************************************************************

.. |WSF_DRAW| replace:: WSF Draw - Warlock
.. |MAP_DISPLAY_LINK| replace:: :doc:`wk_map_display`

.. include:: wkf_wsf_draw.txt

Overview
========

In the WsfDraw Browser draw layers may be shown/hidden from the Map Display by checking/unchecking the individual layer or by checking "Show All". Draws can also be received from other connected simulations by checking the top checkbox.

.. image:: ../images/wk_wsf_draw.png

The browser can be put in 'extended-mode' through the 'Extended Draw Browser' option in the :doc:`Developer Menu<../warlock_developer_menu>`.  When in extended-mode the contents of a given layer may be viewed.

.. note::
   Toggling the contents of a draw layer is for debugging purposes only.  The state of a draw command will change under normal WsfDraw operation.
