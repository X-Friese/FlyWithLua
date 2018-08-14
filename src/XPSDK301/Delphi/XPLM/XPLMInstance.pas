{
   Copyright 2005-2012 Sandy Barbour and Ben Supnik
   
   All rights reserved.  See license.txt for usage.
   
   X-Plane SDK Version: 2.1.1                                                  
}

UNIT XPLMInstance;
INTERFACE
{
   This API provides instanced drawing of X-Plane objects (.obj files). In 
   contrast to old drawing APIs, which required you to draw your own objects 
   per-frame, the instancing API allows you to simply register an OBJ for 
   drawing, then move or manipulate it later (as needed). 
   
   This provides one tremendous benefit: it keeps all dataref operations for 
   your object in one place. Because datarefs are main thread only, allowing 
   dataref access anywhere is a serious performance bottleneck for the 
   simulator---the whole simulator has to pause and wait for each dataref 
   access. This performance penalty will only grow worse as X-Plane moves 
   toward an ever more heavily multithreaded engine. 
   
   The instancing API allows X-Plane to isolate all dataref manipulations for 
   all plugin object drawing to one place, potentially providing huge 
   performance gains. 
   
   Here's how it works: 
   
   When an instance is created, it provides a list of all datarefs you want to 
   manipulate in for the OBJ in the future. This list of datarefs replaces the 
   ad-hoc collections of dataref objects previously used by art assets. Then, 
   per-frame, you can manipulate the instance by passing in a "block" of 
   packed floats representing the current values of the datarefs for your 
   instance. (Note that the ordering of this set of packed floats must exactly 
   match the ordering of the datarefs when you created your instance.)         
}

USES   XPLMDefs;
USES   XPLMScenery;
   {$A4}
{$IFDEF MSWINDOWS}
   {$DEFINE DELPHI}
{$ENDIF}
{___________________________________________________________________________
 * Instance Creation and Destruction
 ___________________________________________________________________________}
{
   Registers and unregisters instances.                                        
}


TYPE
   {
    XPLMInstanceRef
    
    An opaque handle to an instance.                                            
   }
   XPLMInstanceRef = pointer;
   PXPLMInstanceRef = ^XPLMInstanceRef;

   {
    XPLMCreateInstance
    
    Registers an instance of an X-Plane object.                                 
   }
   FUNCTION XPLMCreateInstance(
                                        obj                 : XPLMObjectRef;    
                                        datarefs            : PPchar) : XPLMInstanceRef;    
{$IFDEF DELPHI}
                                       cdecl; external 'XPLM.DLL';
{$ELSE}
                                       cdecl; external '';
{$ENDIF}

   {
    XPLMDestroyInstance
    
    Unregisters an instance.                                                    
   }
   PROCEDURE XPLMDestroyInstance(
                                        instance            : XPLMInstanceRef);    
{$IFDEF DELPHI}
                                       cdecl; external 'XPLM.DLL';
{$ELSE}
                                       cdecl; external '';
{$ENDIF}

{___________________________________________________________________________
 * Instance Manipulation
 ___________________________________________________________________________}
{
                                                                               
}

   {
    XPLMInstanceSetPosition
    
    Updates both the position of the instance and all datarefs you registered 
    for it.                                                                     
   }
   PROCEDURE XPLMInstanceSetPosition(
                                        instance            : XPLMInstanceRef;    
                                        new_position        : PXPLMDrawInfo_t;    
                                        data                : Psingle);    
{$IFDEF DELPHI}
                                       cdecl; external 'XPLM.DLL';
{$ELSE}
                                       cdecl; external '';
{$ENDIF}

IMPLEMENTATION
END.
