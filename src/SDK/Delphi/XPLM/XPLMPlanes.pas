{
   Copyright 2005-2025 Laminar Research, Sandy Barbour and Ben Supnik All
   rights reserved.  See license.txt for usage. X-Plane SDK Version: 4.0.0
}

UNIT XPLMPlanes;
INTERFACE
{
   The XPLMPlanes APIs allow you to control the various aircraft in X-Plane,
   both the user's and the sim's.
   
   You cannot initialize a flight from any XPLM callback. Only initialize a
   flight in response to:
    * Command handlers
    * Menu handlers
    * UI handlers (keyboard, mouse) from XPLMDisplay/widgets
    * The pre-flightmodel processing callback
   
   In particular, do not initialize a flight from:
    * The post-flightmodel processing callback
    * Dataref get/set handlers
    * Any drawing callbacks
   
   *Note*: Some older APIs for accessing aircraft require full paths and not
    paths relative to the X-Plane folder for historical reasons. You will need
    to prefix all relative paths with the  X-Plane path as accessed via
    XPLMGetSystemPath.
}

USES
    XPLMDefs;
   {$A4}
{___________________________________________________________________________
 * USER AIRCRAFT ACCESS
 ___________________________________________________________________________}
{
   These routines are used to initialize and manipulate the user's aircraft.
}


{$IFDEF XPLM430}
   {
    XPLMInitResult
    
    Result codes from initializing or updating the user's aircraft.
    Initialization can fail due to unparsable/invalid data, or due to the
    contents of the initialization containing parameters the sim cannot fulfill
    (e.g. an aircraft not on disk, a ramp start not present in an airport due
    to custom scenery).
    
    If an initialization fails, a human-readable string is sent to your
    plugin's error function. This is meant for debugging purposes only and
    should not be parsed. Your plugin's logic should only use the result code
    for flow control.
   }
TYPE
   XPLMInitResult = (
     { The initialization succeeded.                                              }
      xplm_Init_Success                        = 0
 
     { The provided argument was invalid. This can be returned if the provided    }
     { string is not a valid json string. This error can also be returned if one  }
     { or more of the provided arguments is invalid, such as a missing  required  }
     { field or an unrecognized parameter such as an unknown runway name. Invalid }
     { errors imply that your calling code is generating incorrect JSON and should}
     { be fixed; use your plugin's error callback to find more detailed           }
     { information about the problem with your input.                             }
     ,xplm_Init_Invalid                        = 1
 
     { The new flight could not be initialized because one of the aircraft        }
     { requested could not be found on disk or loaded.                            }
     ,xplm_Init_MissingAircraft                = 2
 
     { The new flight could not be initialized because one of the aircraft's'     }
     { requested liveries could not be found on disk or loaded.                   }
     ,xplm_Init_MissingLivery                  = 3
 
     { The new flight could not be initialized because the requested airport was  }
     { not found in X-Plane's airport database.                                   }
     ,xplm_Init_MissingAirport                 = 4
 
     { The new flight could not be initialized because the requested ramp start   }
     { was not found at the specified airport in X-Plane's airport database.      }
     ,xplm_Init_MissingRamp                    = 5
 
     { The new flight could not be initialized because the requested runway was   }
     { not found at the specified airport in X-Plane's airport database.          }
     ,xplm_Init_MissingRunway                  = 6
 
   );
   PXPLMInitResult = ^XPLMInitResult;
{$ENDIF XPLM430}

{$IFDEF XPLM430}
   {
    XPLMInitFlight
    
    Initialize a new flight, ending th user's current flight. The flight config
    is provided as json string. See
    https://developer.x-plane.com/article/flight-initialization-api/ for the
    JSON format specification. 
    
    Returns a XPLMInitResult enum value specifying whether the initalization
    succeeeded (and if not, what  caused it to fail).
   }
   FUNCTION XPLMInitFlight(
                                        inJsonData          : P) : XPLMInitResult;
    cdecl; external XPLM_DLL;
{$ENDIF XPLM430}

{$IFDEF XPLM430}
   {
    XPLMUpdateFlight
    
    Updates the user's 'current flight, modifying some flight parameters. The
    flight config is provided as a JSON string, see
    https://developer.x-plane.com/article/flight-initialization-api/ for the
    JSON format  specification.
    
    Returns an XPLMInitResult enum value specifying whether hte update
    suceeeded (and if not, what caused  it to fail).
   }
   FUNCTION XPLMUpdateFlight(
                                        inJsonData          : P) : XPLMInitResult;
    cdecl; external XPLM_DLL;
{$ENDIF XPLM430}

   {
    XPLMSetUsersAircraft
    
    This routine changes the user's aircraft.  Note that this will reinitialize
    the user to be on the nearest airport's first runway.  Pass in a full path
    (hard drive and everything including the .acf extension) to the .acf file.
    
    Use XPLMInitFlight for complete control over initialization.
    
    **WARNING**: this API takes a full, not relative aicraft path.
   }
   PROCEDURE XPLMSetUsersAircraft(
                                        inAircraftPath      : XPLMString);
    cdecl; external XPLM_DLL;

   {
    XPLMPlaceUserAtAirport
    
    This routine places the user at a given airport.  Specify the airport by
    its X-Plane airport ID (e.g. 'KBOS').
    
    Use XPLMInitFlight for complete control over initialization.
   }
   PROCEDURE XPLMPlaceUserAtAirport(
                                        inAirportCode       : XPLMString);
    cdecl; external XPLM_DLL;

{$IFDEF XPLM300}
   {
    XPLMPlaceUserAtLocation
    
    Places the user at a specific location after performing any necessary
    scenery loads.
    
    As with in-air starts initiated from the X-Plane user interface, the
    aircraft will always start with its engines running, regardless of the
    user's preferences (i.e., regardless of what the dataref
    `sim/operation/prefs/startup_running` says).
    
    Use XPLMInitFlight for complete control over initialization.
   }
   PROCEDURE XPLMPlaceUserAtLocation(
                                        latitudeDegrees     : Real;
                                        longitudeDegrees    : Real;
                                        elevationMetersMSL  : Single;
                                        headingDegreesTrue  : Single;
                                        speedMetersPerSecond: Single);
    cdecl; external XPLM_DLL;
{$ENDIF XPLM300}

{___________________________________________________________________________
 * GLOBAL AIRCRAFT ACCESS
 ___________________________________________________________________________}
{
   These APIs let you control the AI aircraft and take over multiplayer/aI
   aircraft control.
}


CONST
    { The user's aircraft is always index 0.                                     }
   XPLM_USER_AIRCRAFT   = 0;

{$IFDEF XPLM_DEPRECATED}
   {
    XPLMPlaneDrawState_t
    
    This structure contains additional plane parameter info to be passed to
    draw plane.  Make sure to fill in the size of the structure field with
    sizeof(XPLMDrawPlaneState_t) so that the XPLM can tell how many fields you
    knew about when compiling your plugin (since more fields may be added
    later).
    
    Most of these fields are ratios from 0 to 1 for control input.  X-Plane
    calculates what the actual controls look like based on the .acf file for
    that airplane.  Note for the yoke inputs, this is what the pilot of the
    plane has commanded (post artificial stability system if there were one)
    and affects ailerons, rudder, etc.  It is not necessarily related to the
    actual position of the plane's surfaces!
   }
TYPE
   XPLMPlaneDrawState_t = RECORD
     { The size of the draw state struct.                                         }
     structSize               : Integer;
     { A ratio from [0..1] describing how far the landing gear is extended.       }
     gearPosition             : Single;
     { Ratio of flap deployment, 0 = up, 1 = full deploy.                         }
     flapRatio                : Single;
     { Ratio of spoiler deployment, 0 = none, 1 = full deploy.                    }
     spoilerRatio             : Single;
     { Ratio of speed brake deployment, 0 = none, 1 = full deploy.                }
     speedBrakeRatio          : Single;
     { Ratio of slat deployment, 0 = none, 1 = full deploy.                       }
     slatRatio                : Single;
     { Wing sweep ratio, 0 = forward, 1 = swept.                                  }
     wingSweep                : Single;
     { Thrust power, 0 = none, 1 = full fwd, -1 = full reverse.                   }
     thrust                   : Single;
     { Total pitch input for this plane.                                          }
     yokePitch                : Single;
     { Total Heading input for this plane.                                        }
     yokeHeading              : Single;
     { Total Roll input for this plane.                                           }
     yokeRoll                 : Single;
   END;
   PXPLMPlaneDrawState_t = ^XPLMPlaneDrawState_t;
{$ENDIF XPLM_DEPRECATED}

   {
    XPLMCountAircraft
    
    This function returns the number of aircraft X-Plane is capable of having,
    as well as the number of aircraft that are currently active.  These numbers
    count the user's aircraft.  It can also return the plugin that is currently
    controlling aircraft.  In X-Plane 7, this routine reflects the number of
    aircraft the user has enabled in the rendering options window.
   }
   PROCEDURE XPLMCountAircraft(
                                        outTotalAircraft    : PInteger;
                                        outActiveAircraft   : PInteger;
                                        outController       : PXPLMPluginID);
    cdecl; external XPLM_DLL;

   {
    XPLMGetNthAircraftModel
    
    This function returns the aircraft model for the Nth aircraft.  Indices are
    zero based, with zero being the user's aircraft.  The file name should be
    at least 256 chars in length; the path should be at least 512 chars in
    length.
   }
   PROCEDURE XPLMGetNthAircraftModel(
                                        inIndex             : Integer;
                                        outFileName         : XPLMString;
                                        outPath             : XPLMString);
    cdecl; external XPLM_DLL;

{___________________________________________________________________________
 * EXCLUSIVE AIRCRAFT ACCESS
 ___________________________________________________________________________}
{
   The following routines require exclusive access to the airplane APIs. Only
   one plugin may have this access at a time.
}


   {
    XPLMPlanesAvailable_f
    
    Your airplanes available callback is called when another plugin gives up
    access to the multiplayer planes.  Use this to wait for access to
    multiplayer.
   }
TYPE
     XPLMPlanesAvailable_f = PROCEDURE(
                                    inRefcon            : pointer); cdecl;

   {
    XPLMAcquirePlanes
    
    XPLMAcquirePlanes grants your plugin exclusive access to the aircraft.  It
    returns 1 if you gain access, 0 if you do not.
    
    inAircraft - pass in an array of pointers to strings specifying the planes
    you want loaded.  For any plane index you do not want loaded, pass a
    0-length string.  Other strings should be full paths with the .acf
    extension.  NULL terminates this array, or pass NULL if there are no planes
    you want loaded.
    
    Aircraft paths for this API are full, not relative aircraft paths.
    
    If you pass in a callback and do not receive access to the planes your
    callback will be called when the airplanes are available. If you do receive
    airplane access, your callback will not be called.
   }
   FUNCTION XPLMAcquirePlanes(
                                        inAircraft          : PXPLMString;    { Can be nil }
                                        inCallback          : XPLMPlanesAvailable_f;
                                        inRefcon            : pointer) : Integer;
    cdecl; external XPLM_DLL;

   {
    XPLMReleasePlanes
    
    Call this function to release access to the planes.  Note that if you are
    disabled, access to planes is released for you and you must reacquire it.
   }
   PROCEDURE XPLMReleasePlanes;
    cdecl; external XPLM_DLL;

   {
    XPLMSetActiveAircraftCount
    
    This routine sets the number of active planes.  If you pass in a number
    higher than the total number of planes availables, only the total number of
    planes available is actually used.
   }
   PROCEDURE XPLMSetActiveAircraftCount(
                                        inCount             : Integer);
    cdecl; external XPLM_DLL;

   {
    XPLMSetAircraftModel
    
    This routine loads an aircraft model.  It may only be called if you have
    exclusive access to the airplane APIs.  Pass in the path of the model with
    the .acf extension.  The index is zero based, but you may not pass in 0
    (use XPLMSetUsersAircraft to load the user's aircracft).
    
    This API takes a full aircraft path.
   }
   PROCEDURE XPLMSetAircraftModel(
                                        inIndex             : Integer;
                                        inAircraftPath      : XPLMString);
    cdecl; external XPLM_DLL;

   {
    XPLMDisableAIForPlane
    
    This routine turns off X-Plane's AI for a given plane.  The plane will
    continue to draw and be a real plane in X-Plane, but will not move itself.
   }
   PROCEDURE XPLMDisableAIForPlane(
                                        inPlaneIndex        : Integer);
    cdecl; external XPLM_DLL;

{$IFDEF XPLM_DEPRECATED}
   {
    XPLMDrawAircraft
    
    WARNING: Aircraft drawing via this API is deprecated and WILL NOT WORK in
    future versions of X-Plane.  Use XPLMInstance for 3-d drawing of custom
    aircraft models.
    
    This routine draws an aircraft.  It can only be called from a 3-d drawing
    callback.  Pass in the position of the plane in OpenGL local coordinates
    and the orientation of the plane.  A 1 for full drawing indicates that the
    whole plane must be drawn; a 0 indicates you only need the nav lights
    drawn. (This saves rendering time when planes are far away.)
   }
   PROCEDURE XPLMDrawAircraft(
                                        inPlaneIndex        : Integer;
                                        inX                 : Single;
                                        inY                 : Single;
                                        inZ                 : Single;
                                        inPitch             : Single;
                                        inRoll              : Single;
                                        inYaw               : Single;
                                        inFullDraw          : Integer;
                                        inDrawStateInfo     : PXPLMPlaneDrawState_t);
    cdecl; external XPLM_DLL;
{$ENDIF XPLM_DEPRECATED}

{$IFDEF XPLM_DEPRECATED}
   {
    XPLMReinitUsersPlane
    
    WARNING: DO NOT USE.  Use XPLMPlaceUserAtAirport or
    XPLMPlaceUserAtLocation.
    
    This function recomputes the derived flight model data from the aircraft
    structure in memory.  If you have used the data access layer to modify the
    aircraft structure, use this routine to resynchronize X-Plane; since
    X-Plane works at least partly from derived values, the sim will not behave
    properly until this is called.
    
    WARNING: this routine does not necessarily place the airplane at the
    airport; use XPLMSetUsersAircraft to be compatible.  This routine is
    provided to do special experimentation with flight models without resetting
    flight.
   }
   PROCEDURE XPLMReinitUsersPlane;
    cdecl; external XPLM_DLL;
{$ENDIF XPLM_DEPRECATED}


IMPLEMENTATION

END.
