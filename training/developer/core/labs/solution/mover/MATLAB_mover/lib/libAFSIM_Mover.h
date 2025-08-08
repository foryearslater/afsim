// ****************************************************************************
// CUI
//
// The Advanced Framework for Simulation, Integration, and Modeling (AFSIM)
//
// The use, dissemination or disclosure of data in this file is subject to
// limitation or restriction. See accompanying README and LICENSE for details.
// ****************************************************************************

//
// MATLAB Compiler: 4.14 (R2010b)
// Date: Wed Oct 19 12:05:19 2011
// Arguments: "-B" "macro_default" "-N" "-W" "cpplib:libAFSIM_Mover" "-T"
// "link:lib" "-d" "./lib" "InitializeMover.m" "UpdateMover.m" 
//

#ifndef __libAFSIM_Mover_h
#define __libAFSIM_Mover_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_libAFSIM_Mover
#define PUBLIC_libAFSIM_Mover_C_API __global
#else
#define PUBLIC_libAFSIM_Mover_C_API /* No import statement needed. */
#endif

#define LIB_libAFSIM_Mover_C_API PUBLIC_libAFSIM_Mover_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_libAFSIM_Mover
#define PUBLIC_libAFSIM_Mover_C_API __declspec(dllexport)
#else
#define PUBLIC_libAFSIM_Mover_C_API __declspec(dllimport)
#endif

#define LIB_libAFSIM_Mover_C_API PUBLIC_libAFSIM_Mover_C_API


#else

#define LIB_libAFSIM_Mover_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_libAFSIM_Mover_C_API 
#define LIB_libAFSIM_Mover_C_API /* No special import/export declaration */
#endif

extern LIB_libAFSIM_Mover_C_API 
bool MW_CALL_CONV libAFSIM_MoverInitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_libAFSIM_Mover_C_API 
bool MW_CALL_CONV libAFSIM_MoverInitialize(void);

extern LIB_libAFSIM_Mover_C_API 
void MW_CALL_CONV libAFSIM_MoverTerminate(void);



extern LIB_libAFSIM_Mover_C_API 
void MW_CALL_CONV libAFSIM_MoverPrintStackTrace(void);

extern LIB_libAFSIM_Mover_C_API 
bool MW_CALL_CONV mlxInitializeMover(int nlhs, mxArray *plhs[], int nrhs, mxArray 
                                     *prhs[]);

extern LIB_libAFSIM_Mover_C_API 
bool MW_CALL_CONV mlxUpdateMover(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_libAFSIM_Mover_C_API
long MW_CALL_CONV libAFSIM_MoverGetMcrID();


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_libAFSIM_Mover
#define PUBLIC_libAFSIM_Mover_CPP_API __declspec(dllexport)
#else
#define PUBLIC_libAFSIM_Mover_CPP_API __declspec(dllimport)
#endif

#define LIB_libAFSIM_Mover_CPP_API PUBLIC_libAFSIM_Mover_CPP_API

#else

#if !defined(LIB_libAFSIM_Mover_CPP_API)
#if defined(LIB_libAFSIM_Mover_C_API)
#define LIB_libAFSIM_Mover_CPP_API LIB_libAFSIM_Mover_C_API
#else
#define LIB_libAFSIM_Mover_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_libAFSIM_Mover_CPP_API void MW_CALL_CONV InitializeMover(int nargout, mwArray& state, const mwArray& inLLA, const mwArray& inBoosterParams);

extern LIB_libAFSIM_Mover_CPP_API void MW_CALL_CONV UpdateMover(int nargout, mwArray& state, mwArray& hit_ground_time, const mwArray& t, const mwArray& x, const mwArray& inLLA, const mwArray& inOrientation, const mwArray& inBoosterParams);

#endif
#endif
