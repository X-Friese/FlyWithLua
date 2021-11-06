#ifndef _XPLMInstance_h_
#define _XPLMInstance_h_

/*
 * Copyright 2005-2012 Sandy Barbour and Ben Supnik
 * 
 * All rights reserved.  See license.txt for usage.
 * 
 * X-Plane SDK Version: 2.1.1                                                  
 *
 */

/*
 * This API provides instanced drawing of X-Plane objects (.obj files). In 
 * contrast to old drawing APIs, which required you to draw your own objects 
 * per-frame, the instancing API allows you to simply register an OBJ for 
 * drawing, then move or manipulate it later (as needed). 
 * 
 * This provides one tremendous benefit: it keeps all dataref operations for 
 * your object in one place. Because datarefs are main thread only, allowing 
 * dataref access anywhere is a serious performance bottleneck for the 
 * simulator---the whole simulator has to pause and wait for each dataref 
 * access. This performance penalty will only grow worse as X-Plane moves 
 * toward an ever more heavily multithreaded engine. 
 * 
 * The instancing API allows X-Plane to isolate all dataref manipulations for 
 * all plugin object drawing to one place, potentially providing huge 
 * performance gains. 
 * 
 * Here's how it works: 
 * 
 * When an instance is created, it provides a list of all datarefs you want to 
 * manipulate in for the OBJ in the future. This list of datarefs replaces the 
 * ad-hoc collections of dataref objects previously used by art assets. Then, 
 * per-frame, you can manipulate the instance by passing in a "block" of 
 * packed floats representing the current values of the datarefs for your 
 * instance. (Note that the ordering of this set of packed floats must exactly 
 * match the ordering of the datarefs when you created your instance.)         
 *
 */

#include "XPLMDefs.h"
#include "XPLMScenery.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Instance Creation and Destruction
 ***************************************************************************/
/*
 * Registers and unregisters instances.                                        
 *
 */


/*
 * XPLMInstanceRef
 * 
 * An opaque handle to an instance.                                            
 *
 */
typedef void * XPLMInstanceRef;

/*
 * XPLMCreateInstance
 * 
 * Registers an instance of an X-Plane object.                                 
 *
 */
XPLM_API XPLMInstanceRef      XPLMCreateInstance(
                                   XPLMObjectRef        obj,    
                                   const char **        datarefs);    

/*
 * XPLMDestroyInstance
 * 
 * Unregisters an instance.                                                    
 *
 */
XPLM_API void                 XPLMDestroyInstance(
                                   XPLMInstanceRef      instance);    

/***************************************************************************
 * Instance Manipulation
 ***************************************************************************/
/*
 *                                                                             
 *
 */

/*
 * XPLMInstanceSetPosition
 * 
 * Updates both the position of the instance and all datarefs you registered 
 * for it.                                                                     
 *
 */
XPLM_API void                 XPLMInstanceSetPosition(
                                   XPLMInstanceRef      instance,    
                                   const XPLMDrawInfo_t * new_position,    
                                   const float *        data);    

#ifdef __cplusplus
}
#endif

#endif
