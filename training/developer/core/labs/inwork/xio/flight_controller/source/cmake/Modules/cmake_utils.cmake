# ****************************************************************************
# CUI
#
# The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
#
# The use, dissemination or disclosure of data in this file is subject to
# limitation or restriction. See accompanying README and LICENSE for details.
# ****************************************************************************

# Append a compile flag for target.  Surely there is an easier way to do this???
macro(append_target_properties TARGET PROPNAME VALUE)
   get_target_property(FLAGS ${TARGET} ${PROPNAME})
   if("${FLAGS}" STREQUAL "FLAGS-NOTFOUND")
      set(FLAGS "")
   endif("${FLAGS}" STREQUAL "FLAGS-NOTFOUND")
   set_target_properties(${TARGET} PROPERTIES ${PROPNAME} "${FLAGS} ${VALUE}")
endmacro(append_target_properties TARGET PROPNAME VALUE)
