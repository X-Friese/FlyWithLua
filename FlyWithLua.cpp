// --------------------------------------------------
//  FlyWithLua Plugin for X-Plane 10 (and X-Plane 9)
// --------------------------------------------------

// #define PLUGIN_VERSION "2.2.5 nightly build " __DATE__ " " __TIME__
#define PLUGIN_VERSION "2.2.4"
#define PLUGIN_NAME "FlyWithLua"
#define PLUGIN_DESCRIPTION "Use Lua to manipulate DataRefs and control HID devices."

// Copyright (c) 2012 Carsten Lynker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.



/***** (this is old stuff reused from FlyVFR)
 *  Snagar Modifications
 *  v2.0.7 [added] namespace to not conflict with missionx plugin.
 *         [replace] explicit extern call, with lua.hpp, does the same.
 *         [compatibility code] Since my gcc is 4.6 new x0 abilities are not available.
 *                              Added some code with #ifdef directives to workaround these issues.
 *         [fyi] static link against lua 5.1.4 library, since many airplanes plugins uses  this lua library. It resolve the crashing of XPLANE.
 *
 *  v2.1.14 [compatibility] some (int) cast were changed to std::size_t to compile under gcc x86_64
 *          [changed] " string " was changed to "char x[size] ={...}" to fix gcc deprecate conversion warning.
 *          [changed] Linux: dynamic linked against LuaJIT2.0.0 final and XPSDK211
 *  v2.1.15 [added] OSX x64 + XP10x64 test code for Ben Supnik, to test LuaJit.
 *  v2.1.18 [added] lin.xpl and mac.xpl 32-bit build by Snagar, no code change compared to 2.1.17
 *  v2.1.29 [linux/osx build] removed disabled HID codefor OSX (again)
 */

/** Commands made during FlyWithLua development
 *  Carsten (X-Friese):
 *  v2.1.4 [changed] dynamic link against LuaJIT 1.1.8 library on Windows system.
 *  v2.1.5 [changed] dynamic link against a nightly build of LuaJIT 2.0 to take advantage of FFI library
 *         [fyi] must observe LuaJIT development, to grap a stable version than the beta10 we use at the moment
 *               a stable 2.0 of LuaJIT is announced for 2013 by Mike Pall, see http://www.freelists.org/post/luajit/LuaJIT-Roadmap-20122013
 *  v2.1.6 [fyi] first (Windows only) stable release of FlyWithLua 2.1, same code as 2.1.5, some fine tuning to the manual (logo added)
 *  v2.1.8 [solved] will no longer load files like "this.is.no.lua.script" or "backup.lua~"
 *         [solved] disable and enable won't crash Lua engine any more
 *         [solved] strings containing '\n' now result in multiple lines when send to XSquawkBox
 *         [added] two new libraries: LuaXML and proteaAudio from http://viremo.eludi.net/index.html
 *         [added] custom init and exit file user.ini and user.exit, to be edited by the user
 *                 (no need to touch Internals folder when adding complex libraries)
 *  v2.1.12 [changed] dynamic linked against LuaJIT2.0.0 final and XPSDK211
 *          [changed] new fat format for 32/64 bit plugins
 *  v2.1.13 [added] the 32-bit plugin will ignore script file endian ".lua64" and the 64-bit plugin ignores ".lua32"
 *  v2.1.16 [changed] redesign of LuaDrawString(), added a helper function to fill the RGB array
 *  v2.1.17 [changed] enabled alpha testing (and blending) by default in function FWLDrawWindowCallback(), lines 507ff
 *  v2.1.19 [changed] bug fixed in LuaSetAxisAssignments(), it now sets non-reverse correct
 *  v2.1.21 [changed] function ReadScriptFile() was made Mountain Lion safe
 *  v2.1.24 [added] new Lua functions to draw a string: draw_string_Helvetica_10(), draw_string_Helvetica_12(), draw_string_Helvetica_18(),
 *                  draw_string_Times_Roman_10() and draw_string_Times_Roman_24()
 *  v2.1.25 [changed] LuaJIT.DLL now with Ben's modification on Windows 64-bit
 *  v2.1.28 [solved] some misspelling corrected
 *          [solved] Key commands won't be blocked if Lua crashes (disabled key sniffer if Lua is not running)
 *          [solved] error messages containing multiple lines are now well formatted
 *  v2.2.0  [added] mouse click and wheel event callbacks do_on_mouse_click() and do_on_mouse_wheel()
 *  v2.2.1  [changed] no more classic or modern script mixing errors (hopefully)
 *          [added] new functions set_pilots_head() and get_pilots_head()
 *
 *  Markus (Teddii):
 *  v2.1.20 [changed] bug fixed in Luahid_open() and Luahid_open_path(), setting last HID device index back if no device was found
 *          [changed] extended logMsg() with logType=logToAll|logToDevCon|logToSqkBox. If XSquawkBox is not connected logMsg() will fall back to DevCon
 *          [changed] overworked all logMsg() and XSBSpeakString() calls - so there are no more doubled strings in the code
 *          [fixed]   fixed some copy/pasted logMessages in LuaAddMacro(), LuaLastButton(), LuaSetArray()
 *          [fixed]   fixed a bug in function LuaSpeakString()
 */

/* Configure Code:Blocks to compile FlyWithLua on Windows
 * ======================================================
 *
 * 1. Download and install Code:Blocks with MinGW from this website: http://www.codeblocks.org/downloads/26
 *    (Choose the file "codeblocks-12.11mingw-setup.exe".)
 *
 * 2. Download and install MinGW 64-bit from this website: http://mingw-w64.sourceforge.net/
 *    (Choose the latest automated build for Windows from "WIN64 Downloads", it should be a file
 *    like this "mingw-w64-bin_i686-mingw_20111220.zip".)
 *
 * 3. Extract the ZIP archive containing MinGW64 to the path "P:\MinGW64\", or any other path you like.
 *
 * 4. Start Code:Blocks, click on "Settings" -> "Compiler ..."
 *
 * 5. Copy the settings "GNU GCC Compiler" and name the new setting "gcc64".
 *
 * 6. Change the names and paths inside the tab "toolchain executables" to point to MinGW64.
 *    The names have to start with "x86_64-w64-mingw32-", as you can check out in "P:\MinGW64\bin\".
 *
 * 7. Jump to tab "Additional path" and add this path: "P:\MinGW64\libexec\gcc\x86_64-w64-mingw32\4.7.0"
 *    or the path with the version number of your MinGW64.
 *
 * 8. Now add this paths at "Search directories" -> "Compiler":
 *       P:\MinGW64\include
 *       P:\MinGW64\x86_64-w64-mingw32\include
 *       P:\MinGW64\x86_64-w64-mingw32\include\c++\<VERSION>
 *       P:\MinGW64\x86_64-w64-mingw32\include\c++\<VERSION>\backward
 *       P:\MinGW64\x86_64-w64-mingw32\include\c++\<VERSION>\x86_64-w64-mingw32
 *       P:\MinGW64\lib\gcc\x86_64-w64-mingw32\<VERSION>\include
 *
 * 9. Add this to "Search directories" -> "Linker":
 *       P:\MinGW64\lib
 *       P:\MinGW64\x86_64-w64-mingw32\lib
 *
 * 10. Close Code:Blocks now.
 *
 * 11. Create a new folder "P:\Plugin Development\".
 *
 * 12. Go to this website and download the X-Plane SDK: http://www.xsquawkbox.net/xpsdk/mediawiki/Download
 *     Download the latest version and the 2.0 version too.
 *
 * 13. Create a subfolder "P:\Plugin Development\SDK201\" and an equivalent subfolder for the latest version (like "P:\Plugin Development\SDK212\").
 *
 * 14. Extract the ZIP files containing the X-Plane SDKs into the corresponding subfolders.
 *     (There is now a file like this "P:\Plugin Development\SDK201\SDK\Libraries\Win\XPLM.lib".)
 *
 * 15. Extract all files from "sourcecode.zip" into a new folder "P:\Plugin Development\FlyWithLua2\".
 *
 * 16. Step into "P:\Plugin Development\FlyWithLua2\" and double-click on "FlyWithLua.cbp".
 *
 * 17. Click on "Project" -> "Build options..." -> "WIN64 SDK212" -> "Pre/post build steps" and setup all post build steps as you like.
 *     You may want to erase all of them, as they didn't fit your paths.
 */

#if IBM
#include <windows.h>
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return true;
}
#include <unistd.h>
#endif

// OK, load as much as you can ;)

#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"
#include "XPLMNavigation.h"
#include "XPLMPlanes.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <wchar.h>
#include "XSBComDefs.h"

// include OpenGL
#if IBM
#include <gl/GL.h>
#include <gl/glut.h>
#else
#if LIN
#define TRUE 1
#define FALSE 0
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
// #include <libudev.h>

#else
#include <gl.h>
#include <glu.h>
#include <glut.h>
#include <Carbon/Carbon.h>
#endif

#endif

//get low level access to HID devices

// disable HID in OSX until be able to create binaries.
//#ifndef APL
#include <hidapi.h>
//#endif
// include the extern command provided by the LUA team
#include <lua.hpp>

namespace flywithlua
{
using namespace std; // snagar


// Maybe usefull to be platform independend
#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif


#define NORMALSTRING 250
#define SHORTSRTING 80
#define LONGSTRING 1024
#define MAXDATAREFS 400
#define MAXMACROS 150
#define MAXCOMMANDS 250
#define MAXJOYSTICKBUTTONS 1600  // this value is set by the length of DataRef sim/joystick/joystick_button_values



//Code from Ben Supnik Regarding Luajit in 64bit build
struct lua_alloc_request_t
{
    void *	ud;
    void *	ptr;
    size_t	osize;
    size_t	nsize;
};

#define		ALLOC_OPEN		0x00A110C1
#define		ALLOC_REALLOC	0x00A110C2
#define		ALLOC_CLOSE		0x00A110C3

static void *lj_alloc_create(void)
{
    struct lua_alloc_request_t r = { 0 };
    XPLMSendMessageToPlugin(XPLM_PLUGIN_XPLANE, ALLOC_OPEN,&r);
    return r.ud;
}

static void  lj_alloc_destroy(void *msp)
{
    struct lua_alloc_request_t r = { 0 };
    r.ud = msp;
    XPLMSendMessageToPlugin(XPLM_PLUGIN_XPLANE, ALLOC_CLOSE,&r);
}

static void *lj_alloc_f(void *msp, void *ptr, size_t osize, size_t nsize)
{
    struct lua_alloc_request_t r = { 0 };
    r.ud = msp;
    r.ptr = ptr;
    r.osize = osize;
    r.nsize = nsize;
    XPLMSendMessageToPlugin(XPLM_PLUGIN_XPLANE, ALLOC_REALLOC,&r);
    return r.ptr;
}


// Added since compiler argued about duplicate definitions on OSX
#ifndef APL
struct RGBColor
{
    float   red;
    float   green;
    float   blue;
};
#endif

// qsort needs a compare function, let's use strcmp for it
// as qsort gives void pointers, we have to convert them to char pointers
static int compare_filenames(const void *a, const void *b)
{
    return strcmp (*(const char **) a, *(const char **) b);
}

//#ifndef APL

#define MAXHIDDEVICES 127
void*   HIDSloppyTable[MAXHIDDEVICES];
int     LAST_SLOPPY_HID = -1;

void CloseAllOpenHIDDevices( void )
{
    if (LAST_SLOPPY_HID >= 0)
    {
        for (int i=0; i<=LAST_SLOPPY_HID; i++)
        {
            hid_close((hid_device *) HIDSloppyTable[i]);
        }
    }
    LAST_SLOPPY_HID = -1;
    return;
}
//#endif

struct  DataRefTableStructure
{
    char            DataRefName[NORMALSTRING];
    char            LuaVariable[NORMALSTRING];
    bool            IsReadOnly;
    XPLMDataRef     DataRefId;
    int             Index;
    XPLMDataTypeID  DataRefTypeId;
};

static DataRefTableStructure       DataRefTable[MAXDATAREFS];
static int                         DataRefTableLastElement = -1;

struct  MacroTableStructure
{
    bool        IsSwitch;
    string      MacroName;
    string      ActivateCommand;
    string      DeactivateCommand;
    int         XPLM_Index;
};

static MacroTableStructure     MacroTable[MAXMACROS];
static int                     MacroTableLastElement = -1;

enum    SwitchTypes     {Switch, PositiveEdge, NegativeEdge, PositiveIncrement, NegativeIncrement, PositiveDecrement, NegativeDecrement, ABCEncoder};

struct  SwitchTableStructure
{
    SwitchTypes     SwitchType;
    XPLMDataRef     DataRefID;
    XPLMDataTypeID  DataRefType;
    string          DataRefName;
    int             button;
    int             button2;
    int             index;
    int             on_int;
    int             off_int;
    float           on_float;
    float           off_float;
    double          on_double;
    double          off_double;
    float           upper_limit_float;
    float           lower_limit_float;
    float           stepping_float;
    float           round;
    int             upper_limit_int;
    int             lower_limit_int;
    int             stepping_int;
};

static SwitchTableStructure     SwitchTable[MAXDATAREFS];
static int                      SwitchTableLastElement = -1;


bool            CrashReportDisplayed = false;
static int      LuaResetCount = 0;


void EraseDataRefTable( void )
{
    XPLMDataRef     DoNothing = XPLMFindDataRef("sim/none/none");

    for (int i=0; i<MAXDATAREFS; i++)
    {
        DataRefTable[i].IsReadOnly = true;
        DataRefTable[i].DataRefId = DoNothing;
        strcpy(DataRefTable[i].DataRefName, "");
        DataRefTable[i].DataRefTypeId = xplmType_Unknown;
        DataRefTable[i].Index = 0;
        strcpy(DataRefTable[i].LuaVariable, "");
    }
    DataRefTableLastElement = -1;

    for (int i=0; i<MAXMACROS; i++)
    {
        MacroTable[i].IsSwitch = false;
        MacroTable[i].MacroName.clear();
        MacroTable[i].ActivateCommand.clear();
        MacroTable[i].DeactivateCommand.clear();
        MacroTable[i].XPLM_Index = 0;
    }

    MacroTableLastElement = -1;

    for (int i=0; i<MAXDATAREFS; i++)
    {
        SwitchTable[i].button = 0;
        SwitchTable[i].DataRefID = DoNothing;
        SwitchTable[i].DataRefName.clear();
        SwitchTable[i].DataRefType = xplmType_Unknown;
        SwitchTable[i].off_int = 0;
        SwitchTable[i].off_float = 0;
        SwitchTable[i].off_double = 0;
        SwitchTable[i].on_int = 1;
        SwitchTable[i].on_float = 1;
        SwitchTable[i].on_double = 1;
    }

    SwitchTableLastElement = -1;
}

struct      CommandTableStructure
{
    XPLMCommandRef      Reference;
    string              Name;
    string              Description;
    string              BeginCommand;
    string              ContinueCommand;
    string              EndCommand;
};

static CommandTableStructure    CommandTable[MAXCOMMANDS];
static int                      CommandTableLastElement = -1;

lua_State   *FWLLua;
void        *ud;

// Modified by Snagar
std::string              pluginMainDir;
std::string              scriptDir;
// END modified by Snagar

// The user will be able to handle the plugin with commands
XPLMCommandRef MyReloadScriptsCommand = NULL;

int    MyReloadScriptsCommandHandler(XPLMCommandRef        inCommand,
                                     XPLMCommandPhase      inPhase,
                                     void *                inRefcon);

//Teddii: Enum fuer "logMsg"
enum ELogType
{
    logToAll    = 0,
    logToDevCon = 1,
    logToSqkBox = 2
};

void logMsg (ELogType logType, std::string message ); //Teddii: added parameter logType //void logMsg ( std::string message );
void initPluginDirectory ( ); // snagar

void ResetLuaEngine( void );
bool RunLuaString(string LuaCommandString);
void CopyDataRefsToLua( void );
void CopyDataRefsToXPlane( void );
bool ReadScriptFile(char *FileNameToRead);

// new way to handle classic and modern DataRaf access
void update_Lua_dataref_variables(XPLMDataRef DataRefID, int Index, float Value);
void update_Lua_dataref_strings(XPLMDataRef DataRefID, int Index, char * ValueString);

// These things will run periodically
float	MyEveryFrameLoopCallback(
    float                inElapsedSinceLastCall,
    float                inElapsedTimeSinceLastFlightLoop,
    int                  inCounter,
    void *               inRefcon);

float	MyFastLoopCallback(
    float                inElapsedSinceLastCall,
    float                inElapsedTimeSinceLastFlightLoop,
    int                  inCounter,
    void *               inRefcon);

float	MySlowLoopCallback(
    float                inElapsedSinceLastCall,
    float                inElapsedTimeSinceLastFlightLoop,
    int                  inCounter,
    void *               inRefcon);



// let's give the last Metar to Lua
XPLMDataRef gXSBMetarStringXDataRef = NULL;


// Some variables used global in this plugin
static string      EveryFrameCallbackCommand = "";
static string      CallbackCommand = "";
static string      LongTimeCallbackCommand = "";
static string      KeyEventCommand = "";
static string      NewMetarCommand = "";

float TimeBetweenCallbacks = 1.0;                // processed every second
float LongTimeBetweenCallbacks = 10.0;           // not so often processed
// to increase performance
bool LuaIsRunning = false;                       // Are we working with Lua?

int             JoystickButtonValues[MAXJOYSTICKBUTTONS];
int             JoystickButtonLastValues[MAXJOYSTICKBUTTONS];

// We need some DataRefs
XPLMDataRef     gJoystickButtonAssignments;
XPLMDataRef     gJoystickButtonValues;
XPLMDataRef     gJoystickAxisAssignments;
XPLMDataRef     gJoystickAxisReverse;
XPLMDataRef     gPlaneICAO;
XPLMDataRef     gPlaneTailNumber;


// We interact with XSquawkBox
XPLMPluginID XSBPluginId;
XPLMDataRef  XSBInputUsrMsgXDataRef;
XPLMDataRef  XSBInputStringXDataRef;
XPLMDataRef  XSBDestinationXDataRef;
XPLMDataRef  XSBAlternativeXDataRef;
XPLMDataRef  XSBStartAirportXDataRef;
XPLMDataRef  XSBATCFreqXDataRef;
XPLMDataRef  XSBATCCallsignXDataRef;
static bool  WeHaveXSB = false;
static int   NewMetarCountdown = 0;

// DataRefs used by set_pilots_head()
static XPLMDataRef	FWLPilotsHeadX = XPLMFindDataRef("sim/graphics/view/pilots_head_x");
static XPLMDataRef	FWLPilotsHeadY = XPLMFindDataRef("sim/graphics/view/pilots_head_y");
static XPLMDataRef	FWLPilotsHeadZ = XPLMFindDataRef("sim/graphics/view/pilots_head_z");
static XPLMDataRef	FWLPilotsHeadHeading = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
static XPLMDataRef	FWLPilotsHeadPitch = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
static XPLMDataRef	FWLViewType = XPLMFindDataRef("sim/graphics/view/view_type");

// Don't want to use buttons? Here is a nice little menu
void         FlyWithLuaMenuHandler(void *, void *);
XPLMMenuID   FlyWithLuaMenuId;
int          FlyWithLuaMenuItem;
void         MacroMenuHandler(void *, void *);
XPLMMenuID   MacroMenuId;
XPLMMenuID   ATCMenuId;
int          MacroMenuItem;
int          ATCMenuItem;

// to be able to draw text we need a container to carry Lua commands
string          LuaDrawCommand;

// and we need a window to catch mouse events
XPLMWindowID    FWLMouseEventWindowID;
string          LuaMouseClickCommand;
string          LuaMouseWheelCommand;

int             LAST_SCREEN_WIDTH, LAST_SCREEN_HIGHT;

void FWLMouseEventWindowDraw(XPLMWindowID inWindowID, void * inRefcon)
{
    // we have nothing to draw inside the mouse event window
}

void FWLMouseEventWindowKey(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char vkey, void * inRefcon, int losingFocus)
{
    // no keyboard handling to catch mouse events
}

int	FWLMouseEventWindowMouse(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus isDown, void * inRefcon)
{
    // is Lua running? If not, give the control back to X-Plane
    if (!LuaIsRunning)
    {
        return 0;
    }

    // setup the predefined variables
    lua_pushboolean(FWLLua, false);
    lua_setglobal(FWLLua, "RESUME_MOUSE_CLICK");
    if (isDown == xplm_MouseDown)
    {
        lua_pushstring(FWLLua, "down");
    }
    else if (isDown == xplm_MouseDrag)
    {
        lua_pushstring(FWLLua, "drag");
    }
    else
    {
        lua_pushstring(FWLLua, "up");
    }
    lua_setglobal(FWLLua, "MOUSE_STATUS");

    // let Lua do it's work
    RunLuaString(LuaMouseClickCommand);

    // should we resume the mouse click?
    lua_getglobal(FWLLua, "RESUME_MOUSE_CLICK");
    if (lua_toboolean(FWLLua, 1))
    {
        lua_pop(FWLLua, 1);
        return 1;
    }
    else
    {
        lua_pop(FWLLua, 1);
        return 0;
    }
}

int	FWLMouseEventWindowMouseWheel(XPLMWindowID	inWindowID,
                                  int            x,
                                  int            y,
                                  int            wheel,
                                  int            clicks,
                                  void *         inRefcon)
{
    // is Lua running? If not, give the control back to X-Plane
    if (!LuaIsRunning)
    {
        return 0;
    }

    // setup the predefined variables
    lua_pushboolean(FWLLua, false);
    lua_setglobal(FWLLua, "RESUME_MOUSE_WHEEL");
    lua_pushnumber(FWLLua, wheel);
    lua_setglobal(FWLLua, "MOUSE_WHEEL_NUMBER");
    lua_pushnumber(FWLLua, clicks);
    lua_setglobal(FWLLua, "MOUSE_WHEEL_CLICKS");

    // let Lua do it's work
    RunLuaString(LuaMouseWheelCommand);

    // should we resume the mouse wheel?
    lua_getglobal(FWLLua, "RESUME_MOUSE_WHEEL");
    if (lua_toboolean(FWLLua, 1))
    {
        lua_pop(FWLLua, 1);
        return 1;
    }
    else
    {
        lua_pop(FWLLua, 1);
        return 0;
    }
}

XPLMCursorStatus FWLMouseEventWindowCursor(XPLMWindowID	inWindowID,
        int           x,
        int           y,
        void *        inRefcon)
{
    // noting to do with the cursor
}



int            FWLCommandCallback(
    XPLMCommandRef       inCommand,
    XPLMCommandPhase     inPhase,
    void *               inRefcon)
{
    int     CommandNumber = -1;

    if (LuaIsRunning == false) return 1;  // no Lua - no fun
    if (CommandTableLastElement == -1) return 1; // no Lua commands defined
    for (int i=0; i<=CommandTableLastElement; i++)
    {
        if (CommandTable[i].Reference == inCommand)
        {
            CommandNumber = i;
        }
    }
    if (CommandNumber == -1) // no Lua Command found
    {
        logMsg(logToAll, "FlyWithLua Error: Somebody has stolen a command. Ask your admin or call the police!");
        LuaIsRunning = false;
        return 1;
    }
    CopyDataRefsToXPlane();
    CopyDataRefsToLua();
    lua_pushstring(FWLLua, "continue");
    lua_setglobal(FWLLua, "COMMAND_PROCESSING");
    if (inPhase == xplm_CommandBegin)
    {
        if (CommandTable[CommandNumber].BeginCommand != "") luaL_dostring(FWLLua, CommandTable[CommandNumber].BeginCommand.c_str());
    }
    if (inPhase == xplm_CommandContinue)
    {
        if (CommandTable[CommandNumber].ContinueCommand != "") luaL_dostring(FWLLua, CommandTable[CommandNumber].ContinueCommand.c_str());
    }
    if (inPhase == xplm_CommandEnd)
    {
        if (CommandTable[CommandNumber].EndCommand != "") luaL_dostring(FWLLua, CommandTable[CommandNumber].EndCommand.c_str());
    }
    CopyDataRefsToXPlane();
    lua_getglobal(FWLLua, "COMMAND_PROCESSING");
    if (strcmp(luaL_checkstring(FWLLua, -1), "halt") == 0)
    {
        lua_pop(FWLLua, 1);
        return 0;
    }
    else
    {
        lua_pop(FWLLua, 1);
        return 1;
    }
}

int FWLDrawWindowCallback(XPLMDrawingPhase     inPhase,
                          int                  inIsBefore,
                          void *               inRefcon)
{
    char buffer[LONGSTRING] = {"Lua stopped!"};
    //sprintf(buffer, "%.255s", "Lua stopped!");

    // is Lua running?
    if (!LuaIsRunning)
    {

        float   ColorWanted[] = { 1.0, 0.0, 0.0 }; 	     // RGB Red
        XPLMGetScreenSize(&LAST_SCREEN_WIDTH, &LAST_SCREEN_HIGHT);
        XPLMDrawString(ColorWanted, LAST_SCREEN_WIDTH - 100, LAST_SCREEN_HIGHT - 10, buffer, NULL, xplmFont_Proportional);
        int StackSize = lua_gettop(FWLLua);
        int     NUMBER_OF_CR = 0;
        if (StackSize > 0)
        {
            for (int i=1; i<=StackSize; i++)
            {
                if (lua_isstring(FWLLua, i))
                {
                    // get the string from the stack
                    strcpy(buffer, lua_tostring(FWLLua, i));

                    // set color and position to OpenGL
                    XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);
                    glColor4f(0.0, 1.0, 0.0, 1.0);
                    glRasterPos2f (20, LAST_SCREEN_HIGHT - 20 * i - 15 * NUMBER_OF_CR - 20);

                    // print each letter using a GLUT loop
                    for (int j=0; j<LONGSTRING; j++)
                    {
                        if (buffer[j] == '\n')
                        {
                            glRasterPos2f (20, LAST_SCREEN_HIGHT - 20 * i - 15 * ++NUMBER_OF_CR - 20);
                        }
                        else
                        {
                            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, buffer[j]);
                        }
                    }
                }
            }
        }
        return 1;
    }

    // we do not want to update the DataRefs <-> variables when drawing a window, to not waste CPU time
    // but we inform the script about the mouse position
    int     mouse_x, mouse_y;
    XPLMGetMouseLocation(&mouse_x, &mouse_y);
    XPLMGetScreenSize(&LAST_SCREEN_WIDTH, &LAST_SCREEN_HIGHT);
    // we will have to check the mouse position (is it still inside X-Plane's window?)
    if ((mouse_x >= 0) && (mouse_x <= LAST_SCREEN_WIDTH) && (mouse_y >= 0) && (mouse_y <= LAST_SCREEN_HIGHT) )
    {
        lua_pushnumber(FWLLua, mouse_x);
        lua_setglobal(FWLLua, "MOUSE_X");
        lua_pushnumber(FWLLua, mouse_y);
        lua_setglobal(FWLLua, "MOUSE_Y");
    }
    lua_pushnumber(FWLLua, LAST_SCREEN_WIDTH);
    lua_setglobal(FWLLua, "SCREEN_WIDTH");
    lua_pushnumber(FWLLua, LAST_SCREEN_HIGHT);
    lua_setglobal(FWLLua, "SCREEN_HIGHT");

    // restore the window size of the mouse event window and bring it to the front
    XPLMSetWindowGeometry(FWLMouseEventWindowID, 0, LAST_SCREEN_HIGHT, LAST_SCREEN_WIDTH, 0);
    // XPLMSetWindowIsVisible(FWLMouseEventWindowID, 1);
    // XPLMBringWindowToFront(FWLMouseEventWindowID);

    // set a default state for drawing
    XPLMSetGraphicsState(0,  // enables or disables fog, equivalent to: glEnable(GL_FOG);
                         0,  // enables or disables a number of multitexturing units. If the number is 0, 2d texturing is disabled entirely,
                         // as in glDisable(GL_TEXTURE_2D); Otherwise, 2d texturing is enabled, and a number of multitexturing units are enabled
                         // sequentially, starting with unit 0, e.g. glActiveTextureARB(GL_TEXTURE0_ARB); glEnable (GL_TEXTURE_2D);
                         0,  // enables or disables OpenGL lighting, e.g. glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
                         1,  // enables or disables the alpha test per pixel, e.g. glEnable(GL_ALPHA_TEST);
                         1,  // enables or disables alpha blending per pixel, e.g. glEnable(GL_BLEND);
                         0,  // enables per pixel depth testing, as in glEnable(GL_DEPTH_TEST);
                         0); // enables writing back of depth information to the depth bufffer, as in glDepthMask(GL_TRUE);

    if (luaL_dostring(FWLLua, LuaDrawCommand.c_str()))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Can't execute window draw callback command string. The string who failed is:");
        logMsg(logToDevCon, LuaDrawCommand);
        LuaIsRunning = false;
    }

    return 1;
};

void FWLHandleKeyCallback(
    XPLMWindowID         inWindowID,
    char                 inKey,
    XPLMKeyFlags         inFlags,
    char                 inVirtualKey,
    void *               inRefcon,
    int                  losingFocus)
{
    // nothing to handle here, we use a seperate sniffer
}

int FWLKeySniffer(
    char                 inChar,
    XPLMKeyFlags         inFlags,
    char                 inVirtualKey,
    void *               inRefcon)
{
    // is Lua running? If not, give the control back to X-Plane
    if (!LuaIsRunning)
    {
        return 1;
    }

    // setup the predefined variables
    lua_pushboolean(FWLLua, false);
    lua_setglobal(FWLLua, "RESUME_KEY");
    lua_pushnumber(FWLLua, inVirtualKey);
    lua_setglobal(FWLLua, "VKEY");
    lua_pushlstring(FWLLua, &inChar, 1);
    lua_setglobal(FWLLua, "CKEY");
    lua_pushboolean(FWLLua, (inFlags & xplm_ShiftFlag));
    lua_setglobal(FWLLua, "SHIFT_KEY");
    if (inFlags & xplm_DownFlag)
    {
        lua_pushstring(FWLLua, "pressed");
    }
    else
    {
        lua_pushstring(FWLLua, "released");
    }
    lua_setglobal(FWLLua, "KEY_ACTION");

    // let Lua do it's work
    RunLuaString(KeyEventCommand);

    // should we resume the keystroke?
    lua_getglobal(FWLLua, "RESUME_KEY");
    if (lua_toboolean(FWLLua, 1))
    {
        lua_pop(FWLLua, 1);
        return 0;
    }
    else
    {
        lua_pop(FWLLua, 1);
        return 1;
    }
};

void XSBSpeakString(std::string StringToSpeak)
{
    std::size_t ptr = StringToSpeak.find('\n');
    if (WeHaveXSB)
    {
        if (ptr != std::string::npos)
        {
            XSBSpeakString(StringToSpeak.substr(0, ptr));
            XSBSpeakString(StringToSpeak.substr(ptr+1));
            return;
        }
        else
        {
            XPLMSetDatab(XSBInputUsrMsgXDataRef, (void *) StringToSpeak.c_str(),
                         0, StringToSpeak.length());
            XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_USER_MSG, NULL);
        }
    }
    return;
}


//#ifndef APL
/***************************************************************
 ** All stuff taken from Kein-Hong Man's luahidapi            **
 ** https://sites.google.com/site/rubblepiles/hardware-stuff  **
 ***************************************************************/

#define USB_STR_MAXLEN 255      /* max USB string length */

/*----------------------------------------------------------------------
 * simple wchar_t[] to char[] conversion, returns a string
 * - exposing wchar_t[] to Lua is messy, must consider cross-platform
 *   sizes of wchar_t, so here is a quickie but crippled solution...
 *----------------------------------------------------------------------
 */

void push_forced_ascii(lua_State *L, const wchar_t *s)
{
    std::size_t i;
    char d[USB_STR_MAXLEN + 1];

    if (!s)                     /* check for NULL case */
    {
        d[0] = '\0';
        lua_pushstring(L, d);
        return;
    }
    std::size_t n = wcslen(s);
    if (n > USB_STR_MAXLEN) n = USB_STR_MAXLEN;

    for (i = 0; i < n; i++)
    {
        wchar_t wc = s[i];
        char c = wc & 0x7F;     /* zap all de funny chars */
        if (wc > 127 || (wc > 0 && wc < 32))
        {
            c = '?';
        }
        d[i] = c;
    }
    d[i] = '\0';
    lua_pushstring(L, d);
}

/*********************************************
 ** All own functions to handle HID devices **
 *********************************************/

static int      Luacreate_HID_table(lua_State *L)
{
    int                     number_of_HID_devices = 0;
    struct hid_device_info  *devs, *cur_dev;

    devs = hid_enumerate(0x0, 0x0);
    cur_dev = devs;

    // create the main table (we do not know the size of the table)
    lua_createtable(L, 0, 0);

    while (cur_dev)   // we found a device, let's create a subtable
    {
        number_of_HID_devices++;
        lua_pushnumber(L, number_of_HID_devices);  // the index of the device in main table
        lua_createtable(L, 8, 0);  // each subtable contains 8 named elements and no indexed elements

        // get vendor_id
        lua_pushstring(L, "vendor_id");
        lua_pushnumber(L, cur_dev->vendor_id);
        lua_settable(L, -3);

        // get product_id
        lua_pushstring(L, "product_id");
        lua_pushnumber(L, cur_dev->product_id);
        lua_settable(L, -3);

        // get release_number
        lua_pushstring(L, "release_number");
        lua_pushnumber(L, cur_dev->release_number);
        lua_settable(L, -3);

        // get interface_number
        lua_pushstring(L, "interface_number");
        lua_pushnumber(L, cur_dev->interface_number);
        lua_settable(L, -3);

        // get usage_page
        lua_pushstring(L, "usage_page");
        lua_pushnumber(L, cur_dev->usage_page);
        lua_settable(L, -3);

        // get usage
        lua_pushstring(L, "usage");
        lua_pushnumber(L, cur_dev->usage);
        lua_settable(L, -3);

        // get path
        lua_pushstring(L, "path");
        lua_pushstring(L, cur_dev->path);
        lua_settable(L, -3);

        // get serial_number
        lua_pushstring(L, "serial_number");
        push_forced_ascii(L, cur_dev->serial_number);
        lua_settable(L, -3);

        // get manufacturer_string
        lua_pushstring(L, "manufacturer_string");
        push_forced_ascii(L, cur_dev->manufacturer_string);
        lua_settable(L, -3);

        // get product_string
        lua_pushstring(L, "product_string");
        push_forced_ascii(L, cur_dev->product_string);
        lua_settable(L, -3);

        // get the device table into the main table
        lua_settable(L, -3);

        // is there a next device?
        cur_dev = cur_dev->next;
    }
    // free RAM
    hid_free_enumeration(devs);

    // return the main table plus the number of devices found
    lua_pushnumber(L, number_of_HID_devices);
    return 2;
}

static int      Luahid_open(lua_State *L)
{
    // usage: device = hid_open( vendor_id, product_id )
    // returns: pointer to device or nil if no device found
    // parameter serial_number not supported in Lua (Lua has no wchar_t type)

    hid_device      *device_pointer;
    lua_Integer     id;

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToAll, "FlyWithLua Error: hid_open() need two numbers as arguments.");
        LuaIsRunning = false;
        lua_pushnil(L);
        return 1;
    }

    // check argument range
    id = luaL_checkinteger(L, 1);
    if (id < 0 || id > 0xFFFF)
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong argument range for hid_open().");
        LuaIsRunning = false;
        lua_pushnil(L);
        return 1;
    }
    unsigned short vendor_id = (unsigned short)id;

    id = luaL_checkinteger(L, 2);
    if (id < 0 || id > 0xFFFF)
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong argument range for hid_open().");
        LuaIsRunning = false;
        lua_pushnil(L);
        return 1;
    }
    unsigned short product_id = (unsigned short)id;

    // space enough in the sloppy table?
    if (++LAST_SLOPPY_HID >= MAXHIDDEVICES)
    {
        logMsg(logToAll, "FlyWithLua Error: Max devices to be open at the same time reached.");
        LuaIsRunning = false;
        lua_pushnil(L);
        LAST_SLOPPY_HID--;
        return 1;
    }

    // open the device
    device_pointer = hid_open(vendor_id, product_id, NULL);
    HIDSloppyTable[LAST_SLOPPY_HID] = device_pointer;

    // push pointer to lua
    if (device_pointer != NULL)
    {
        lua_pushlightuserdata(L, device_pointer);
        return 1;
    }
    else
    {
        // no device found
        LAST_SLOPPY_HID--;
        lua_pushnil(L);
        return 1;
    }
}

static int      Luahid_open_path(lua_State *L)
{
    // usage: hid_open_path( "path_string" )
    // returns: pointer to device or nil if no device found

    hid_device      *device_pointer;

    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong argument for hid_open_path().");
        LuaIsRunning = false;
        lua_pushnil(L);
        return 1;
    }

    // space enough in the sloppy table?
    if (++LAST_SLOPPY_HID >= MAXHIDDEVICES)
    {
        logMsg(logToAll, "FlyWithLua Error: Max devices to be open at the same time reached.");
        LuaIsRunning = false;
        lua_pushnil(L);
        LAST_SLOPPY_HID--;
        return 1;
    }

    // get path string and call the C function
    const char      *device_path = lua_tostring(L, 1);
    device_pointer = hid_open_path( device_path );
    HIDSloppyTable[LAST_SLOPPY_HID] = device_pointer;

    // push pointer to lua
    if (device_pointer != NULL)
    {
        lua_pushlightuserdata(L, device_pointer);
        return 1;
    }
    else
    {
        // no device found
        LAST_SLOPPY_HID--;
        lua_pushnil(L);
        return 1;
    }
}

static int      Luahid_close(lua_State *L)
{
    if (lua_islightuserdata(L, 1))
    {
        hid_device*     device_pointer;

        // close the device
        device_pointer = (hid_device *) lua_touserdata(L, 1);
        hid_close(device_pointer);

        // clean up sloppy table
        if (LAST_SLOPPY_HID < 0) return 0;
        int i = 0;
        while ((i <= LAST_SLOPPY_HID) && (HIDSloppyTable[i] != device_pointer)) i++;
        if (i>LAST_SLOPPY_HID) return 0; // no match found (user closed a closed or unknown device)
        for (int j = i; j < LAST_SLOPPY_HID; j++)
        {
            HIDSloppyTable[j] = HIDSloppyTable[j+1];
        }
        LAST_SLOPPY_HID--;
        return 0;
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function hid_close().");
        LuaIsRunning = false;
        return 0;
    }
}

static int      Luahid_write(lua_State *L)
{
    // usage: hid_write( device_pointer, int, int, int, ... )

    unsigned char       BlockToWrite[USB_STR_MAXLEN];

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function hid_write().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    int noa = lua_gettop(L);  // number of arguments
    if (noa > USB_STR_MAXLEN)
    {
        logMsg(logToAll, "FlyWithLua Error: Too many arguments to function hid_write().");
        LuaIsRunning = false;
        return 0;
    }
    // collect values to write
    for (int i = 2; i <= noa; i++)
    {
        BlockToWrite[i-2] = luaL_checknumber(L, i);
    }
    // write values to HID device
    int result = hid_write((hid_device *) lua_touserdata(L, 1), BlockToWrite, noa-1);
    if (result == -1)
    {
        logMsg(logToAll, "FlyWithLua Error: hid_write() failed.");
        LuaIsRunning = false;
    }
    lua_pushnumber(L, result);
    return 1;
}

static int      Luahid_read_timeout(lua_State *L)
{
    // usage: number_of_values_read, value, value, value, ... = hid_read_timeout( device_pointer, number_of_values, milliseconds )

    unsigned char       BlockToRead[USB_STR_MAXLEN];

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function hid_read_timeout().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    int nov = lua_tonumber(L, 2);  // number of values wanted
    if ((nov >= USB_STR_MAXLEN) || (nov < 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Sorry, hid_read_timeout() can not handle the number of values you want to get.");
        LuaIsRunning = false;
        return 0;
    }
    // get the values into the block
    int result = hid_read_timeout((hid_device *) lua_touserdata(L, 1), BlockToRead, nov, lua_tonumber(L, 3));

    // push number of values reeded to Lua
    lua_pushnumber(L, result);

    // push values to Lua
    if (result < 1)
    {
        return 1;
    }
    for (int i=1; i<=result; i++)
    {
        lua_pushnumber(L, BlockToRead[i-1]);
    }
    return result+1;
}

static int      Luahid_read(lua_State *L)
{
    // usage: number_of_values_read, value, value, value, ... = hid_read( device_pointer, number_of_values )

    unsigned char       BlockToRead[USB_STR_MAXLEN];

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function hid_read().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    int nov = lua_tonumber(L, 2);  // number of values wanted
    if ((nov >= USB_STR_MAXLEN) || (nov < 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Sorry, hid_read() can not handle the number of values you want to get.");
        LuaIsRunning = false;
        return 0;
    }
    // get the values into the block
    int result = hid_read((hid_device *) lua_touserdata(L, 1), BlockToRead, nov);

    // push number of values reeded to Lua
    lua_pushnumber(L, result);

    // push values to Lua
    if (result < 1)
    {
        return 1;
    }
    for (int i=1; i<=result; i++)
    {
        lua_pushnumber(L, BlockToRead[i-1]);
    }
    return result+1;
}

static int      Luahid_set_nonblocking(lua_State *L)
{
    // usage: success = hid_set_nonblocking( device_pointer, nonblock )
    // nonblock can be value 1 or 0

    // check arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function hid_set_nonblocking().");
        LuaIsRunning = false;
        return 0;
    }
    int blockornot = lua_tonumber(L, 2);
    if ((blockornot == 0) || (blockornot == 1))
    {
        int success = hid_set_nonblocking((hid_device *) lua_touserdata(L, 1), blockornot);
        lua_pushnumber(L, success);
        return 1;
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: The second argument to function hid_set_nonblocking() must be 1 or 0.");
        LuaIsRunning = false;
        return 0;
    }
}

static int      Luahid_send_feature_report(lua_State *L)
{
    // usage: number_of_values_written = hid_send_feature_report( device_pointer, int, int, int, ... )

    unsigned char       BlockToWrite[USB_STR_MAXLEN];

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function hid_send_feature_report().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    int noa = lua_gettop(L);  // number of arguments
    if (noa > USB_STR_MAXLEN)
    {
        logMsg(logToAll, "FlyWithLua Error: Too many arguments to function hid_send_feature_report().");
        LuaIsRunning = false;
        return 0;
    }
    // collect values to write
    for (int i = 2; i <= noa; i++)
    {
        BlockToWrite[i-2] = luaL_checknumber(L, i);
    }
    // write values to HID device
    int result = hid_send_feature_report((hid_device *) lua_touserdata(L, 1), BlockToWrite, noa-1);
    if (result == -1)
    {
        logMsg(logToAll, "FlyWithLua Error: hid_send_feature_report() failed.");
        LuaIsRunning = false;
    }
    lua_pushnumber(L, result);
    return 1;
}

static int      Luahid_get_feature_report(lua_State *L)
{
    // usage: number_of_values_read, value, value, value, ... = hid_get_feature_report( device_pointer, number_of_values )

    unsigned char       BlockToRead[USB_STR_MAXLEN];

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function hid_get_feature_report().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    int nov = lua_tonumber(L, 2);  // number of values wanted
    if ((nov >= USB_STR_MAXLEN) || (nov < 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Sorry, hid_get_feature_report() can not handle the number of values you want to get.");
        LuaIsRunning = false;
        return 0;
    }
    // get the values into the block
    int result = hid_get_feature_report((hid_device *) lua_touserdata(L, 1), BlockToRead, nov+1);

    // push number of values reeded to Lua
    lua_pushnumber(L, result);

    // push values to Lua
    if (result < 1)
    {
        return 1;
    }
    for (int i=1; i<result; i++)
    {
        lua_pushnumber(L, BlockToRead[i-1]);
    }
    return result+1;
}

/*******************************************
 ** End of HID functions provided for Lua **
 *******************************************/

//#endif // end of disable APL HID implementation

void PushDataRefToLuaVariable(  char*           VariableWantedCString,
                                XPLMDataRef     DataRefIdWanted,
                                XPLMDataTypeID  DataRefTypeIdWanted,
                                int             IndexWanted)
{
    if (!LuaIsRunning) return;

    if (DataRefTypeIdWanted == xplmType_Int)
    {
        int ValueOfDataRef = XPLMGetDatai(DataRefIdWanted);
        lua_pushnumber(FWLLua, ValueOfDataRef);
        lua_setglobal(FWLLua, VariableWantedCString);
        return;
    }

    if (DataRefTypeIdWanted == xplmType_Double)
    {
        double ValueOfDataRef = XPLMGetDatad(DataRefIdWanted);
        lua_pushnumber(FWLLua, ValueOfDataRef);
        lua_setglobal(FWLLua, VariableWantedCString);
        return;
    }

    if (DataRefTypeIdWanted == xplmType_Float)
    {
        float ValueOfDataRef = XPLMGetDataf(DataRefIdWanted);
        lua_pushnumber(FWLLua, ValueOfDataRef);
        lua_setglobal(FWLLua, VariableWantedCString);
        return;
    }

    if (DataRefTypeIdWanted == xplmType_FloatArray)
    {
        float ValueOfDataRef;
        XPLMGetDatavf(DataRefIdWanted, &ValueOfDataRef, IndexWanted, 1);
        lua_pushnumber(FWLLua, ValueOfDataRef);
        lua_setglobal(FWLLua, VariableWantedCString);
        return;
    }

    if (DataRefTypeIdWanted == xplmType_IntArray)
    {
        int ValueOfDataRef;
        XPLMGetDatavi(DataRefIdWanted, &ValueOfDataRef, IndexWanted, 1);
        lua_pushnumber(FWLLua, ValueOfDataRef);
        lua_setglobal(FWLLua, VariableWantedCString);
        return;
    }

    if (DataRefTypeIdWanted == xplmType_Data)
    {
        char ValueOfDataRef[LONGSTRING];
        XPLMGetDatab(DataRefIdWanted, &ValueOfDataRef, IndexWanted, LONGSTRING);
        lua_pushstring(FWLLua, ValueOfDataRef);
        lua_setglobal(FWLLua, VariableWantedCString);
        return;
    }

    logMsg(logToAll, string("FlyWithLua Error: The type of the DataRef variable \"").append(VariableWantedCString).append("\" is unknown or impossible."));
    LuaIsRunning = false;
    return;
}

// define the core C functions to use them inside the Lua state

static int LuaXSBSpeakString(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToSqkBox, "FlyWithLua Error: nothing to say."); //fallback to DevCon
        return 0;
    }
    string LuaWantsToSpeak = lua_tostring(L, 1);
    logMsg(logToSqkBox, LuaWantsToSpeak); //fallback to DevCon
    return 0;
}

static int      LuaXSBShowFlightplan(lua_State *L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SHOW_FP, NULL);
    return 0;
}

static int      LuaXSBSendFlightplan(lua_State *L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SEND_FP, NULL);
    return 0;
}

static int      LuaXSBUserLogin(lua_State *L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_USER_LOGIN, NULL);
    return 0;
}

static int      LuaXSBConnect(lua_State *L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_CONNECT, NULL);
    return 0;
}

static int      LuaXSBDisconnect(lua_State *L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_DISCONNECT, NULL);
    return 0;
}

static int      LuaXSBLookupATC(lua_State *L)
{
    char    ATC_wanted[NORMALSTRING];

    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong argument to XSBLookupATC, we need a string to search for.");
        return 0;
    }
    strcpy(ATC_wanted, lua_tostring(L, 1));
    XPLMSetDatab(XSBATCCallsignXDataRef, ATC_wanted, 0, strlen(ATC_wanted));
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_LOOKUP_ATC, NULL);
    lua_pushnumber(L, XPLMGetDatai(XSBATCFreqXDataRef)/10);
    return 1;
}

static int LualogMsg(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: nothing to log.");
        return 0;
    }
    string LuaWantsToSpeak = lua_tostring(L, 1);
    logMsg(logToDevCon, LuaWantsToSpeak);
    return 0;
}

static int LuaSpeakString(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToSqkBox, "FlyWithLua Error: nothing to say."); //fallback to DevCon
        return 0;
    }
    string LuaWantsToSpeak = lua_tostring(L, 1);
    logMsg(logToSqkBox, LuaWantsToSpeak.c_str()); //fallback to DevCon
    XPLMSpeakString(LuaWantsToSpeak.c_str());
    return 0;
}

// this is a helper fuction to LuaDrawString() filling the RGB color array
static void fill_RGB_array(float* array_to_fill, float red, float green, float blue)
{
    array_to_fill[0] = red;
    array_to_fill[1] = green;
    array_to_fill[2] = blue;
}

static int LuaDrawString(lua_State *L)
{
    // arguments: int x_pos, int y_pos, string string_to_print, float red, float green, float blue

    // or         int x_pos, int y_pos, string string_to_print, string color_to_print
    //            (color_to_print can be "red", "green", "blue", "yellow", "cyan", "magenta", "grey", "black" or "white")

    // or         int x_pos, int y_pos, string string_to_print
    //            (uses default color = white)

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments given to draw_string().");
        LuaIsRunning = false;
        return 0;
    }
    float   ColorWanted[] = { 1.0, 1.0, 1.0 }; 	     // RGB White

    if (lua_isstring(L, 4))
    {
        string ColorString = lua_tostring(L, 4);

        if (ColorString.compare("red") == 0)     fill_RGB_array(ColorWanted, 1.0, 0.0, 0.0);
        if (ColorString.compare("green") == 0)   fill_RGB_array(ColorWanted, 0.0, 1.0, 0.0);
        if (ColorString.compare("blue") == 0)    fill_RGB_array(ColorWanted, 0.0, 0.0, 1.0);
        if (ColorString.compare("yellow") == 0)  fill_RGB_array(ColorWanted, 1.0, 1.0, 0.0);
        if (ColorString.compare("cyan") == 0)    fill_RGB_array(ColorWanted, 0.0, 1.0, 1.0);
        if (ColorString.compare("magenta") == 0) fill_RGB_array(ColorWanted, 1.0, 0.0, 1.0);
        if (ColorString.compare("grey") == 0)    fill_RGB_array(ColorWanted, 0.5, 0.5, 0.5);
        if (ColorString.compare("black") == 0)   fill_RGB_array(ColorWanted, 0.0, 0.0, 0.0);
    }

    if (lua_isnumber(L, 4) && lua_isnumber(L, 5) && lua_isnumber(L, 6))
    {
        float red = lua_tonumber(L, 4);
        float green = lua_tonumber(L, 5);
        float blue = lua_tonumber(L, 6);

        if (red < 0.0 || red > 1.0) red = 1.0;
        if (green < 0.0 || green > 1.0) green = 1.0;
        if (blue < 0.0 || blue > 1.0) blue = 1.0;

        fill_RGB_array(ColorWanted, red, green, blue);
    }

    int     x_pos = lua_tointeger(L, 1);
    int     y_pos = lua_tointeger(L, 2);
    char  string_to_print[LONGSTRING];

    strcpy(string_to_print, lua_tostring(L, 3));
    XPLMDrawString(ColorWanted, x_pos, y_pos, string_to_print, NULL, xplmFont_Proportional);
    return 0;
}

static int LuaDrawStringHelv18(lua_State *L)
{
    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments given to draw_string_Helv18().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    int     x_pos = lua_tointeger(L, 1);
    int     y_pos = lua_tointeger(L, 2);

    // get string from Lua Script
    char  string_to_print[LONGSTRING];
    strcpy(string_to_print, lua_tostring(L, 3));

    // set position to OpenGL
    glRasterPos2f (x_pos, y_pos);

    // print each letter using a GLUT loop
    for (int i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string_to_print[i]);
    }

    return 0;
}

static int LuaDrawStringHelv12(lua_State *L)
{
    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments given to draw_string_Helv18().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    int     x_pos = lua_tointeger(L, 1);
    int     y_pos = lua_tointeger(L, 2);

    // get string from Lua Script
    char  string_to_print[LONGSTRING];
    strcpy(string_to_print, lua_tostring(L, 3));

    // set position to OpenGL
    glRasterPos2f (x_pos, y_pos);

    // print each letter using a GLUT loop
    for (int i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string_to_print[i]);
    }

    return 0;
}

static int LuaDrawStringHelv10(lua_State *L)
{
    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments given to draw_string_Helv18().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    int     x_pos = lua_tointeger(L, 1);
    int     y_pos = lua_tointeger(L, 2);

    // get string from Lua Script
    char  string_to_print[LONGSTRING];
    strcpy(string_to_print, lua_tostring(L, 3));

    // set position to OpenGL
    glRasterPos2f (x_pos, y_pos);

    // print each letter using a GLUT loop
    for (int i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string_to_print[i]);
    }

    return 0;
}

static int LuaDrawStringTimes10(lua_State *L)
{
    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments given to draw_string_Helv18().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    int     x_pos = lua_tointeger(L, 1);
    int     y_pos = lua_tointeger(L, 2);

    // get string from Lua Script
    char  string_to_print[LONGSTRING];
    strcpy(string_to_print, lua_tostring(L, 3));

    // set position to OpenGL
    glRasterPos2f (x_pos, y_pos);

    // print each letter using a GLUT loop
    for (int i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, string_to_print[i]);
    }

    return 0;
}

static int LuaDrawStringTimes24(lua_State *L)
{
    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments given to draw_string_Helv18().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    int     x_pos = lua_tointeger(L, 1);
    int     y_pos = lua_tointeger(L, 2);

    // get string from Lua Script
    char  string_to_print[LONGSTRING];
    strcpy(string_to_print, lua_tostring(L, 3));

    // set position to OpenGL
    glRasterPos2f (x_pos, y_pos);

    // print each letter using a GLUT loop
    for (int i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string_to_print[i]);
    }

    return 0;
}

static int LuaMeasureString(lua_State *L)
{
    // arguments:  string: string to measure
    // output:     float:  length of string in screen pixel
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguemtns given to measure_string().");
        LuaIsRunning = false;
        return 0;
    }
    char  string_to_measure[LONGSTRING];
    strcpy(string_to_measure, lua_tostring(L, 1));
    if (lua_isstring(L, 2))
    {
        int result = 0;
        char  font_to_measure[LONGSTRING];
        strcpy(font_to_measure, lua_tostring(L, 2));
        if (strcmp(font_to_measure, "Helvetica_10") == 0)
        {
            for (int i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_HELVETICA_10, string_to_measure[i]);
            }
        }
        if (strcmp(font_to_measure, "Helvetica_12") == 0)
        {
            for (int i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, string_to_measure[i]);
            }
        }
        if (strcmp(font_to_measure, "Helvetica_18") == 0)
        {
            for (int i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, string_to_measure[i]);
            }
        }
        if (strcmp(font_to_measure, "Times_Roman_10") == 0)
        {
            for (int i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_10, string_to_measure[i]);
            }
        }
        if (strcmp(font_to_measure, "Times_Roman_24") == 0)
        {
            for (int i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, string_to_measure[i]);
            }
        }
        lua_pushnumber(L, result);
    }
    else
    {
        float result = XPLMMeasureString(xplmFont_Proportional, string_to_measure, strlen(string_to_measure));
        lua_pushnumber(L, result);
    }
    return 1;
}

static int LuaCrashTheSim(lua_State *L)
{
    XPLMSpeakString("Oh, no! Please replace the user. This one is too stupid! Do you really think we crash the simulater? Killing your joysticks instead.");
    int EveryThingIsZero[1600] = {0};
    XPLMSetDatavi(gJoystickButtonAssignments, EveryThingIsZero, 0, 1600);
    XPLMSetDatavi(gJoystickAxisAssignments, EveryThingIsZero, 0, 100);
    return 0;
}

static int LuaCommandOnce(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: nothing to do. You will have to give a string.");
        return 0;
    }
    char LuaWantsToDo[NORMALSTRING];
    strcpy(LuaWantsToDo, lua_tostring(L, 1));
    XPLMCommandRef CommandId = XPLMFindCommand(LuaWantsToDo);
    if (CommandId == NULL)
    {
        logMsg(logToAll, "FlyWithLua Error: nothing to do. The command is unknown.");
        return 0;
    }
    XPLMCommandOnce(CommandId);
    return 0;
}

static int LuaAddMacro(lua_State *L)
{
    if (++MacroTableLastElement >= MAXMACROS)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want too much macro menu entries.");
        MacroTableLastElement--;
        return 0;
    }
    if ( lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3) )
    {
        string MacroName = lua_tostring(L, 1);
        MacroTable[MacroTableLastElement].MacroName = MacroName;
        MacroTable[MacroTableLastElement].ActivateCommand.append(lua_tostring(L, 2));
        MacroTable[MacroTableLastElement].DeactivateCommand.append(lua_tostring(L, 3));
        MacroTable[MacroTableLastElement].XPLM_Index = XPLMAppendMenuItem(MacroMenuId, MacroName.c_str(), (void *) MacroTableLastElement, 0);
        MacroTable[MacroTableLastElement].IsSwitch = true;
        string StartActive = "";
        if (lua_isstring(L, 4))
        {
            StartActive = lua_tostring(L, 4);
        }
        if (StartActive == "activate")
        {
            XPLMCheckMenuItem(MacroMenuId, MacroTable[MacroTableLastElement].XPLM_Index, xplm_Menu_Checked);
            RunLuaString(MacroTable[MacroTableLastElement].ActivateCommand);
        }
        else
        {
            XPLMCheckMenuItem(MacroMenuId, MacroTable[MacroTableLastElement].XPLM_Index, xplm_Menu_Unchecked);
            RunLuaString(MacroTable[MacroTableLastElement].DeactivateCommand);
        }
        MacroTable[MacroTableLastElement].IsSwitch = true;
        return 0;
    }
    if ( !(lua_isstring(L, 1) && lua_isstring(L, 2) ) )
    {
        logMsg(logToAll, "FlyWithLua Error: wrong parameters to AddMacro function.");
        return 0;
    }
    string MacroName = lua_tostring(L, 1);
    MacroTable[MacroTableLastElement].MacroName = MacroName;
    MacroTable[MacroTableLastElement].ActivateCommand.append(lua_tostring(L, 2));
    MacroTable[MacroTableLastElement].XPLM_Index = XPLMAppendMenuItem(MacroMenuId, MacroName.c_str(), (void *) MacroTableLastElement, 0);
    MacroTable[MacroTableLastElement].IsSwitch = false;
    return 0;
}

static int LuaAddATCMacro(lua_State *L)
{
    if (++MacroTableLastElement >= MAXMACROS)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want too much ATC macro menu entries.");
        MacroTableLastElement--;
        return 0;
    }
    // ATC Menu Entries are never checkable
    if ( !(lua_isstring(L, 1) && lua_isstring(L, 2) ) )
    {
        logMsg(logToAll, "FlyWithLua Error: wrong parameters to AddATCMacro function.");
        return 0;
    }
    string MacroName = lua_tostring(L, 1);
    MacroTable[MacroTableLastElement].MacroName = MacroName;
    MacroTable[MacroTableLastElement].ActivateCommand.append(lua_tostring(L, 2));
    MacroTable[MacroTableLastElement].XPLM_Index = XPLMAppendMenuItem(ATCMenuId, MacroName.c_str(), (void *) MacroTableLastElement, 1);
    MacroTable[MacroTableLastElement].IsSwitch = false;
    return 0;
}

static int LuaActivateMacro(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Your argument is not a string.");
        return 0;
    }
    if (MacroTableLastElement < 0) return 0;
    string search_string = lua_tostring(L, 1);
    for (int i=0; i<=MacroTableLastElement; i++)
    {
        if ((MacroTable[i].IsSwitch) && (MacroTable[i].MacroName == search_string))
        {
            XPLMCheckMenuItem(MacroMenuId, MacroTable[i].XPLM_Index, xplm_Menu_Checked);
            RunLuaString(MacroTable[i].ActivateCommand);
        }
    }
    return 0;
}

static int LuaDeactivateMacro(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Your argument is not a string.");
        return 0;
    }
    if (MacroTableLastElement < 0) return 0;
    string search_string = lua_tostring(L, 1);
    for (int i=0; i<=MacroTableLastElement; i++)
    {
        if ((MacroTable[i].IsSwitch) && (MacroTable[i].MacroName == search_string))
        {
            XPLMCheckMenuItem(MacroMenuId, MacroTable[i].XPLM_Index, xplm_Menu_Unchecked);
            RunLuaString(MacroTable[i].DeactivateCommand);
        }
    }
    return 0;
}

static int LuaDoEveryKeystroke(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong thing to do? Your command is not a string.");
        return 0;
    }
    string LuaShouldDoCommand = lua_tostring(L, 1);
    KeyEventCommand.append(LuaShouldDoCommand).append("\n");
    return 0;
}

static int LuaDoEveryMouseClick(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong thing to do? Your command is not a string.");
        return 0;
    }
    string LuaShouldDoCommand = lua_tostring(L, 1);
    LuaMouseClickCommand.append(LuaShouldDoCommand).append("\n");
    return 0;
}

static int LuaDoEveryMouseWheel(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong thing to do? Your command is not a string.");
        return 0;
    }
    string LuaShouldDoCommand = lua_tostring(L, 1);
    LuaMouseWheelCommand.append(LuaShouldDoCommand).append("\n");
    return 0;
}

static int LuaDoEveryDrawCallback(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong thing to do? Your command is not a string.");
        return 0;
    }
    string LuaShouldDoCommand = lua_tostring(L, 1);
    LuaDrawCommand.append(LuaShouldDoCommand).append("\n");
    return 0;
}

static int LuaDoEveryMETARCallback(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong thing to do? Your command is not a string.");
        return 0;
    }
    string LuaShouldDoCommand = lua_tostring(L, 1);
    NewMetarCommand.append(LuaShouldDoCommand).append("\n");
    return 0;
}

static int LuaDoEveryFrame(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong thing to do? Your command is not a string.");
        return 0;
    }
    string LuaShouldDoCommand = lua_tostring(L, 1);
    EveryFrameCallbackCommand.append(LuaShouldDoCommand).append("\n");
    return 0;
}

static int LuaDoOften(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong thing to do? Your command is not a string.");
        return 0;
    }
    string LuaShouldDoCommand = lua_tostring(L, 1);
    CallbackCommand.append(LuaShouldDoCommand).append("\n");
    return 0;
}

static int LuaDoSometimes(lua_State *L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong thing to do? Your command is not a string.");
        return 0;
    }
    string LuaShouldDoCommand = lua_tostring(L, 1);
    LongTimeCallbackCommand.append(LuaShouldDoCommand).append("\n");
    return 0;
}

static int LuaClearAllButtonAssignments(lua_State *L)
{
    int EveryThingIsZero[1600] = {0};
    XPLMSetDatavi(gJoystickButtonAssignments, EveryThingIsZero, 0, 1600);
    return 0;
}

static int LuaClearAllAxisAssignments(lua_State *L)
{
    int EveryThingIsZero[100] = {0};
    XPLMSetDatavi(gJoystickAxisAssignments, EveryThingIsZero, 0, 100);
    return 0;
}

static int LuaSetButtonAssignment(lua_State *L)
{
    char    CommandWanted[NORMALSTRING];
    if (!(lua_isstring(L, 2) && lua_isnumber(FWLLua, 1)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong argument types. We need a string and an integer to set a button assignment.");
        return 0;
    }
    int ButtonNumber = lua_tointeger(L, 1);
    if (ButtonNumber < 0 || ButtonNumber > 1599)
    {
        logMsg(logToAll, "FlyWithLua Error: wrong argument range. Button number has to be from 0 to 1599.");
        return 0;
    }
    strcpy(CommandWanted, lua_tostring(L, 2));
    if (strcmp(CommandWanted, "sim/none/none") == 0)
    {
        int set_the_value_to_zero = 0;
        XPLMSetDatavi(gJoystickButtonAssignments, &set_the_value_to_zero, ButtonNumber, 1);
        return 0;
    }
    int CommandRefIdWanted = (std::size_t) XPLMFindCommand(CommandWanted); // snagar
    if (CommandRefIdWanted == 0)
    {
        logMsg(logToDevCon, string("FlyWithLua Error: The Command \"").append(CommandWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetDatavi(gJoystickButtonAssignments, &CommandRefIdWanted, ButtonNumber, 1);
    return 0;
}

// we need two values stored in an integer to point to one of them, to set the axis reverse setting
static int reverse_no = 0;
static int reverse_yes = 1;

static int LuaSetAxisAssignment(lua_State *L)
{
    int CommandRefIdWanted = 0;
    if (!(lua_isstring(L, 3) && lua_isstring(L, 2) && lua_isnumber(FWLLua, 1)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong argument types. We need two strings and an integer to set an axis assignment.");
        return 0;
    }
    int AxisNumber = lua_tointeger(L, 1);
    if (AxisNumber < 0 || AxisNumber > 99)
    {
        logMsg(logToAll, "FlyWithLua Error: wrong argument range. Axis number has to be from 0 to 99.");
        return 0;
    }
    string CommandWanted = lua_tostring(L, 2);
    if (CommandWanted == "none")
        CommandRefIdWanted = 0;
    if (CommandWanted == "pitch")
        CommandRefIdWanted = 1;
    if (CommandWanted == "roll")
        CommandRefIdWanted = 2;
    if (CommandWanted == "yaw")
        CommandRefIdWanted = 3;
    if (CommandWanted == "throttle")
        CommandRefIdWanted = 4;
    if (CommandWanted == "collective")
        CommandRefIdWanted = 5;
    if (CommandWanted == "left toe brake")
        CommandRefIdWanted = 6;
    if (CommandWanted == "right toe brake")
        CommandRefIdWanted = 7;
    if (CommandWanted == "prop")
        CommandRefIdWanted = 8;
    if (CommandWanted == "mixture")
        CommandRefIdWanted = 9;
    if (CommandWanted == "carb heat")
        CommandRefIdWanted = 10;
    if (CommandWanted == "flaps")
        CommandRefIdWanted = 11;
    if (CommandWanted == "thrust vector")
        CommandRefIdWanted = 12;
    if (CommandWanted == "wing sweep")
        CommandRefIdWanted = 13;
    if (CommandWanted == "speedbrakes")
        CommandRefIdWanted = 14;
    if (CommandWanted == "displacement")
        CommandRefIdWanted = 15;
    if (CommandWanted == "reverse")
        CommandRefIdWanted = 16;
    if (CommandWanted == "elev trim")
        CommandRefIdWanted = 17;
    if (CommandWanted == "ailn trim")
        CommandRefIdWanted = 18;
    if (CommandWanted == "rudd trim")
        CommandRefIdWanted = 19;
    if (CommandWanted == "throttle 1")
        CommandRefIdWanted = 20;
    if (CommandWanted == "throttle 2")
        CommandRefIdWanted = 21;
    if (CommandWanted == "throttle 3")
        CommandRefIdWanted = 22;
    if (CommandWanted == "throttle 4")
        CommandRefIdWanted = 23;
    if (CommandWanted == "prop 1")
        CommandRefIdWanted = 24;
    if (CommandWanted == "prop 2")
        CommandRefIdWanted = 25;
    if (CommandWanted == "prop 3")
        CommandRefIdWanted = 26;
    if (CommandWanted == "prop 4")
        CommandRefIdWanted = 27;
    if (CommandWanted == "mixture 1")
        CommandRefIdWanted = 28;
    if (CommandWanted == "mixture 2")
        CommandRefIdWanted = 29;
    if (CommandWanted == "mixture 3")
        CommandRefIdWanted = 30;
    if (CommandWanted == "mixture 4")
        CommandRefIdWanted = 31;
    if (CommandWanted == "reverse 1")
        CommandRefIdWanted = 32;
    if (CommandWanted == "reverse 2")
        CommandRefIdWanted = 33;
    if (CommandWanted == "reverse 3")
        CommandRefIdWanted = 34;
    if (CommandWanted == "reverse 4")
        CommandRefIdWanted = 35;
    if (CommandWanted == "landing gear")
        CommandRefIdWanted = 36;
    if (CommandWanted == "nosewheel tiller")
        CommandRefIdWanted = 37;
    if (CommandWanted == "backup throttle")
        CommandRefIdWanted = 38;
    if (CommandWanted == "auto roll")
        CommandRefIdWanted = 39;
    if (CommandWanted == "auto pitch")
        CommandRefIdWanted = 40;
    if (CommandWanted == "view left/right")
        CommandRefIdWanted = 41;
    if (CommandWanted == "view up/down")
        CommandRefIdWanted = 42;
    if (CommandWanted == "view zoom")
        CommandRefIdWanted = 43;

    XPLMSetDatavi(gJoystickAxisAssignments, &CommandRefIdWanted, AxisNumber, 1);

    string  ReverseOrNot = lua_tostring(L, 3);
    if (ReverseOrNot == "reverse")
    {
        XPLMSetDatavi(gJoystickAxisReverse, &reverse_yes, AxisNumber, 1);
    }
    else
    {
        XPLMSetDatavi(gJoystickAxisReverse, &reverse_no, AxisNumber, 1);
    }
    return 0;
}

static int LuaButton(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: function button() needs an integer argument."); //we really should output a script line number here :-) --Teddii
        LuaIsRunning = false;
        return 0;  // this will force Lua to crash...
    }
    lua_pushboolean(L, JoystickButtonValues[lua_tointeger(L, 1)]);
    return 1;
}

static int LuaLastButton(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: function last_button() needs an integer argument.");
        LuaIsRunning = false;
        return 0;  // this will force Lua to crash...
    }
    lua_pushboolean(L, JoystickButtonLastValues[lua_tointeger(L, 1)]);
    return 1;
}

static int LuaGet(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];
    int     IndexWanted = 0;

    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function get().");
        LuaIsRunning = false;
        return 0;
    }
    strcpy(DataRefWanted, lua_tostring(L, 1));
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    if (lua_isnumber(L, 2))
    {
        IndexWanted = lua_tointeger(L, 2);
    }
    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if ((int)DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;

    // everything is fine, but a FWL-type dataref variable may carry a new value
    // we fix it by pushing all dataref variables to X-Plane before reading
    CopyDataRefsToXPlane();

    if (DataRefTypeIdWanted == xplmType_Int)
    {
        int ValueToWrite = XPLMGetDatai(DataRefIdWanted);
        lua_pushnumber(L, ValueToWrite);
        return 1;
    }
    if (DataRefTypeIdWanted == xplmType_Double)
    {
        double ValueToWrite = XPLMGetDatad(DataRefIdWanted);
        lua_pushnumber(L, ValueToWrite);
        return 1;
    }
    if (DataRefTypeIdWanted == xplmType_Float)
    {
        float ValueToWrite = XPLMGetDataf(DataRefIdWanted);
        lua_pushnumber(L, ValueToWrite);
        return 1;
    }
    if (DataRefTypeIdWanted == xplmType_IntArray)
    {
        int ValueToWrite = 0;
        XPLMGetDatavi(DataRefIdWanted, &ValueToWrite, IndexWanted, 1);
        lua_pushnumber(L, ValueToWrite);
        return 1;
    }
    if (DataRefTypeIdWanted == xplmType_FloatArray)
    {
        float ValueToWrite = 0;
        XPLMGetDatavf(DataRefIdWanted, &ValueToWrite, IndexWanted, 1);
        lua_pushnumber(L, ValueToWrite);
        return 1;
    }
    if (DataRefTypeIdWanted == xplmType_Data)
    {
        char ValueToWrite[LONGSTRING];
        XPLMGetDatab(DataRefIdWanted, ValueToWrite, 0, LONGSTRING);
        lua_pushstring(L, ValueToWrite);
        return 1;
    }
    return 1;
}

static int LuaSet(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];

    if (!(lua_isstring(L, 1) && lua_isnumber(L, 2)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function set().");
        LuaIsRunning = false;
        return 0;
    }
    strcpy(DataRefWanted, lua_tostring(L, 1));
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }

    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if ((int) DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;
    if (DataRefTypeIdWanted == xplmType_Int)
    {
        int ValueToWrite = (int)lua_tonumber(L, 2);
        XPLMSetDatai(DataRefIdWanted, ValueToWrite);
        update_Lua_dataref_variables(DataRefIdWanted, 0, (float) ValueToWrite);
    }
    if (DataRefTypeIdWanted == xplmType_Double)
    {
        double ValueToWrite = (double)lua_tonumber(L, 2);
        XPLMSetDatad(DataRefIdWanted, ValueToWrite);
        update_Lua_dataref_variables(DataRefIdWanted, 0, (float) ValueToWrite);
    }
    if (DataRefTypeIdWanted == xplmType_Float)
    {
        float ValueToWrite = (float)lua_tonumber(L, 2);
        XPLMSetDataf(DataRefIdWanted, ValueToWrite);
        update_Lua_dataref_variables(DataRefIdWanted, 0, ValueToWrite);
    }

    return 0;
}

static int LuaSetArray(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];

    if (!(lua_isstring(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function set_array().");
        LuaIsRunning = false;
        return 0;
    }
    strcpy(DataRefWanted, lua_tostring(L, 1));
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }

    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if (DataRefTypeIdWanted == xplmType_IntArray)
    {
        int ValueWanted = (int) lua_tonumber(L, 3);
        XPLMSetDatavi(DataRefIdWanted, &ValueWanted, (int)lua_tonumber(L, 2), 1);
        update_Lua_dataref_variables(DataRefIdWanted, (int)lua_tonumber(L, 2), (float) ValueWanted);
    }
    if (DataRefTypeIdWanted == xplmType_FloatArray)
    {
        float ValueWanted = (float) lua_tonumber(L, 3);
        XPLMSetDatavf(DataRefIdWanted, &ValueWanted, (int)lua_tonumber(L, 2), 1);
        update_Lua_dataref_variables(DataRefIdWanted, (int)lua_tonumber(L, 2), ValueWanted);
    }

    return 0;
}

static int LuaGetDataRefBinding(lua_State *L)
{
    char    VariableWanted[NORMALSTRING];

    if (!lua_isstring(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong argument to function get_dataref_binding().");
        LuaIsRunning = false;
        return 0;
    }
    strcpy(VariableWanted, lua_tostring(L, 1));
    if (DataRefTableLastElement>-1)
    {
        for (int i=0;i<=DataRefTableLastElement;i++)
        {
            if (strcmp(VariableWanted, DataRefTable[i].LuaVariable) == 0)
            {
                lua_pushstring(L, DataRefTable[i].DataRefName);
                lua_pushinteger(L, DataRefTable[i].Index);
                lua_pushboolean(L, DataRefTable[i].IsReadOnly);
                lua_pushinteger(L, DataRefTable[i].DataRefTypeId);
                lua_pushlightuserdata(L, DataRefTable[i].DataRefId);
                return 5;
            }
        }
    }
    // nothing found
    return 0;
}

static int LuaDataRef(lua_State *L)
{
    char    VariableWanted[NORMALSTRING];
    char    DataRefWanted[NORMALSTRING];
    char    ReadOnlyOrNot[NORMALSTRING];
    int     ReadOnlyWanted;
    int     IndexWanted = 0;

    if (!(lua_isstring(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function DataRef().");
        LuaIsRunning = false;
        return 0;
    }
    if ( !lua_isstring(L, 3))
    {
        ReadOnlyWanted = true;
    }
    else
    {
        strcpy(ReadOnlyOrNot, lua_tostring(L, 3));
        if ((strcmp(ReadOnlyOrNot, "writable") == 0) || (strcmp(ReadOnlyOrNot, "writeable") == 0))  // allow a often used mistake in spelling writable
        {
            ReadOnlyWanted = false;
        }
        else
        {
            ReadOnlyWanted = true;
        }
    }
    strcpy(VariableWanted, lua_tostring(L, 1));
    strcpy(DataRefWanted, lua_tostring(L, 2));

    if (!lua_isnumber(L, 4))
    {
        IndexWanted = 0;
    }
    else
    {
        IndexWanted = lua_tointeger(L, 4);
    }

    // Did we already know the variable?
    if (DataRefTableLastElement >= 0)
    {
        for (int i=0; i<=DataRefTableLastElement; i++)
        {
            if (strcmp(DataRefTable[i].LuaVariable, VariableWanted) == 0)
            {
                if (strcmp(DataRefTable[i].DataRefName, DataRefWanted) == 0)
                {
                    // everything is fine?
                    if (IndexWanted != DataRefTable[i].Index)
                    {
                        logMsg(logToAll, string("FlyWithLua Error: The variable ").append(VariableWanted).append("\" is still defined as DataRef \"").append(DataRefTable[i].DataRefName).append("\", but the index is different."));
                        LuaIsRunning = false;
                        return 0;
                    }
                    else
                    {
                        return 0;
                    }
                }
                else
                {
                    logMsg(logToAll, string("FlyWithLua Error: The variable \"").append(VariableWanted).append("\" is still defined as DataRef \"").append(DataRefTable[i].DataRefName).append("\"."));
                    LuaIsRunning = false;
                    return 0;
                }
            }
        }
    }

    // Did we already know the DataRef?
    if (DataRefTableLastElement >= 0)
    {
        for (int i=0; i<=DataRefTableLastElement; i++)
        {
            if ((strcmp(DataRefTable[i].DataRefName, DataRefWanted) == 0) && (DataRefTable[i].Index == IndexWanted))
            {
                logMsg(logToDevCon, string("FlyWithLua Info: The DataRef ") + DataRefWanted + string(" is already handled in variable ") + DataRefTable[i].LuaVariable + string(", you want to use it with ") + VariableWanted);
                logMsg(logToDevCon, "FlyWithLua Info: As long as only one variable wants writable access to the DataRef, it is okay (but you loose performance).");
                if (ReadOnlyWanted == false && DataRefTable[i].IsReadOnly == false)
                {
                    logMsg(logToAll, string("FlyWithLua Error: The variable \"").append(VariableWanted).append("\" wants to write a DataRef, that is already handled by another variable."));
                    LuaIsRunning = false;
                    return 0;
                }
            }
        }
    }

    // let's search for the DataRef
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 4))
        {
            logMsg(logToAll, string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(" should have an index. I set it to 0."));
        }
    }
    if ((int)DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;
    if ((ReadOnlyWanted == false) && (XPLMCanWriteDataRef(DataRefIdWanted) == false))
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" is not writeable."));
        LuaIsRunning = false;
        return 0;
    }

    // add it to the table
    if (++DataRefTableLastElement >= MAXDATAREFS)
    {
        logMsg(logToAll, "FlyWithLua Error: You want more DataRefs than I can handle!");
        LuaIsRunning = false;
        DataRefTableLastElement = MAXDATAREFS-1;
    }
    else
    {
        DataRefTable[DataRefTableLastElement].DataRefId = DataRefIdWanted;
        DataRefTable[DataRefTableLastElement].DataRefTypeId = DataRefTypeIdWanted;
        DataRefTable[DataRefTableLastElement].Index = IndexWanted;
        DataRefTable[DataRefTableLastElement].IsReadOnly = ReadOnlyWanted;
        strcpy(DataRefTable[DataRefTableLastElement].DataRefName, DataRefWanted);
        strcpy(DataRefTable[DataRefTableLastElement].LuaVariable, VariableWanted);
    }

    PushDataRefToLuaVariable(VariableWanted, DataRefIdWanted, DataRefTypeIdWanted, IndexWanted);

    return 0;
}

static int LuaCreateSwitch(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];
    int     IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function create_switch().");
        LuaIsRunning = false;
        return 0;
    }

    strcpy(DataRefWanted, lua_tostring(L, 2));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    }
    else
    {
        IndexWanted = lua_tointeger(L, 3);
    }

    // let's search for the DataRef
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToAll, string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(" should have an index. I set it to 0."));
        }
    }
    if ((int)DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;

    // add it to the table
    if (++SwitchTableLastElement >= MAXDATAREFS)
    {
        logMsg(logToAll, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning = false;
        SwitchTableLastElement = MAXDATAREFS-1;
    }
    else
    {
        SwitchTable[SwitchTableLastElement].SwitchType = Switch;
        SwitchTable[SwitchTableLastElement].button = lua_tointeger(L, 1);
        SwitchTable[SwitchTableLastElement].DataRefID = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].off_int = lua_tointeger(L, 4);
            SwitchTable[SwitchTableLastElement].off_float = lua_tonumber(L, 4);
            SwitchTable[SwitchTableLastElement].off_double = lua_tonumber(L, 4);
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].on_int = lua_tointeger(L, 5);
            SwitchTable[SwitchTableLastElement].on_float = lua_tonumber(L, 5);
            SwitchTable[SwitchTableLastElement].on_double = lua_tonumber(L, 5);
        }
    }
    return 0;
}

static int LuaCreatePositiveEdgeTrigger(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];
    int     IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function create_positive_edge_trigger().");
        LuaIsRunning = false;
        return 0;
    }

    strcpy(DataRefWanted, lua_tostring(L, 2));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    }
    else
    {
        IndexWanted = lua_tointeger(L, 3);
    }

    // let's search for the DataRef
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToAll, string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(" should have an index. I set it to 0."));
        }
    }
    if ((int)DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;

    // add it to the table
    if (++SwitchTableLastElement >= MAXDATAREFS)
    {
        logMsg(logToAll, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning = false;
        SwitchTableLastElement = MAXDATAREFS-1;
    }
    else
    {
        SwitchTable[SwitchTableLastElement].SwitchType = PositiveEdge;
        SwitchTable[SwitchTableLastElement].button = lua_tointeger(L, 1);
        SwitchTable[SwitchTableLastElement].DataRefID = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].on_int = lua_tointeger(L, 4);
            SwitchTable[SwitchTableLastElement].on_float = lua_tonumber(L, 4);
            SwitchTable[SwitchTableLastElement].on_double = lua_tonumber(L, 4);
        }
    }
    return 0;
}

static int LuaCreateNegativeEdgeTrigger(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];
    int     IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function create_positive_edge_trigger().");
        LuaIsRunning = false;
        return 0;
    }

    strcpy(DataRefWanted, lua_tostring(L, 2));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    }
    else
    {
        IndexWanted = lua_tointeger(L, 3);
    }

    // let's search for the DataRef
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToAll, string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(" should have an index. I set it to 0."));
        }
    }
    if ((int)DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;

    // add it to the table
    if (++SwitchTableLastElement >= MAXDATAREFS)
    {
        logMsg(logToAll, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning = false;
        SwitchTableLastElement = MAXDATAREFS-1;
    }
    else
    {
        SwitchTable[SwitchTableLastElement].SwitchType = NegativeEdge;
        SwitchTable[SwitchTableLastElement].button = lua_tointeger(L, 1);
        SwitchTable[SwitchTableLastElement].DataRefID = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].on_int = lua_tointeger(L, 4);
            SwitchTable[SwitchTableLastElement].on_float = lua_tonumber(L, 4);
            SwitchTable[SwitchTableLastElement].on_double = lua_tonumber(L, 4);
        }
    }
    return 0;
}

static int LuaCreatePositiveEdgeIncrement(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];
    int     IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function create_positive_edge_increment().");
        LuaIsRunning = false;
        return 0;
    }

    strcpy(DataRefWanted, lua_tostring(L, 2));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    }
    else
    {
        IndexWanted = lua_tointeger(L, 3);
    }

    // let's search for the DataRef
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToAll, string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(" should have an index. I set it to 0."));
        }
    }
    if ((int)DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;

    // add it to the table
    if (++SwitchTableLastElement >= MAXDATAREFS)
    {
        logMsg(logToAll, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning = false;
        SwitchTableLastElement = MAXDATAREFS-1;
    }
    else
    {
        SwitchTable[SwitchTableLastElement].SwitchType = PositiveIncrement;
        SwitchTable[SwitchTableLastElement].button = lua_tointeger(L, 1);
        SwitchTable[SwitchTableLastElement].DataRefID = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].stepping_int = lua_tointeger(L, 4);
            SwitchTable[SwitchTableLastElement].stepping_float = lua_tonumber(L, 4);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].stepping_int = 1;
            SwitchTable[SwitchTableLastElement].stepping_float = 1;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].upper_limit_int = lua_tointeger(L, 5);
            SwitchTable[SwitchTableLastElement].upper_limit_float = lua_tonumber(L, 5);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].upper_limit_int = 999999999;
            SwitchTable[SwitchTableLastElement].upper_limit_float = 999999999;
        }
        if (lua_isnumber(L, 6))
        {
            SwitchTable[SwitchTableLastElement].round = lua_tonumber(L, 6);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].round = 1;
        }
    }
    return 0;
}

static int LuaCreateNegativeEdgeIncrement(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];
    int     IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function create_negative_edge_increment().");
        LuaIsRunning = false;
        return 0;
    }

    strcpy(DataRefWanted, lua_tostring(L, 2));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    }
    else
    {
        IndexWanted = lua_tointeger(L, 3);
    }

    // let's search for the DataRef
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToAll, string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(" should have an index. I set it to 0."));
        }
    }
    if ((int)DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;

    // add it to the table
    if (++SwitchTableLastElement >= MAXDATAREFS)
    {
        logMsg(logToAll, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning = false;
        SwitchTableLastElement = MAXDATAREFS-1;
    }
    else
    {
        SwitchTable[SwitchTableLastElement].SwitchType = NegativeIncrement;
        SwitchTable[SwitchTableLastElement].button = lua_tointeger(L, 1);
        SwitchTable[SwitchTableLastElement].DataRefID = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].stepping_int = lua_tointeger(L, 4);
            SwitchTable[SwitchTableLastElement].stepping_float = lua_tonumber(L, 4);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].stepping_int = 1;
            SwitchTable[SwitchTableLastElement].stepping_float = 1;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].upper_limit_int = lua_tointeger(L, 5);
            SwitchTable[SwitchTableLastElement].upper_limit_float = lua_tonumber(L, 5);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].upper_limit_int = 999999999;
            SwitchTable[SwitchTableLastElement].upper_limit_float = 999999999;
        }
        if (lua_isnumber(L, 6))
        {
            SwitchTable[SwitchTableLastElement].round = lua_tonumber(L, 6);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].round = 1;
        }
    }
    return 0;
}

static int LuaCreatePositiveEdgeDecrement(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];
    int     IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function create_positive_edge_decrement().");
        LuaIsRunning = false;
        return 0;
    }

    strcpy(DataRefWanted, lua_tostring(L, 2));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    }
    else
    {
        IndexWanted = lua_tointeger(L, 3);
    }

    // let's search for the DataRef
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToAll, string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(" should have an index. I set it to 0."));
        }
    }
    if ((int)DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;

    // add it to the table
    if (++SwitchTableLastElement >= MAXDATAREFS)
    {
        logMsg(logToAll, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning = false;
        SwitchTableLastElement = MAXDATAREFS-1;
    }
    else
    {
        SwitchTable[SwitchTableLastElement].SwitchType = PositiveDecrement;
        SwitchTable[SwitchTableLastElement].button = lua_tointeger(L, 1);
        SwitchTable[SwitchTableLastElement].DataRefID = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].stepping_int = lua_tointeger(L, 4);
            SwitchTable[SwitchTableLastElement].stepping_float = lua_tonumber(L, 4);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].stepping_int = 1;
            SwitchTable[SwitchTableLastElement].stepping_float = 1;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].lower_limit_int = lua_tointeger(L, 5);
            SwitchTable[SwitchTableLastElement].lower_limit_float = lua_tonumber(L, 5);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].lower_limit_int = 0;
            SwitchTable[SwitchTableLastElement].lower_limit_float = 0;
        }
        if (lua_isnumber(L, 6))
        {
            SwitchTable[SwitchTableLastElement].round = lua_tonumber(L, 6);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].round = 1;
        }
    }
    return 0;
}

static int LuaCreateNegativeEdgeDecrement(lua_State *L)
{
    char    DataRefWanted[NORMALSTRING];
    int     IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToAll, "FlyWithLua Error: wrong arguments to function create_negative_edge_decrement().");
        LuaIsRunning = false;
        return 0;
    }

    strcpy(DataRefWanted, lua_tostring(L, 2));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    }
    else
    {
        IndexWanted = lua_tointeger(L, 3);
    }

    // let's search for the DataRef
    XPLMDataRef     DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == NULL)
    {
        logMsg(logToAll, string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID  DataRefTypeIdWanted = XPLMGetDataRefTypes(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToAll, string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(" should have an index. I set it to 0."));
        }
    }
    if ((int)DataRefTypeIdWanted == 6) DataRefTypeIdWanted = xplmType_Double;

    // add it to the table
    if (++SwitchTableLastElement >= MAXDATAREFS)
    {
        logMsg(logToAll, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning = false;
        SwitchTableLastElement = MAXDATAREFS-1;
    }
    else
    {
        SwitchTable[SwitchTableLastElement].SwitchType = NegativeDecrement;
        SwitchTable[SwitchTableLastElement].button = lua_tointeger(L, 1);
        SwitchTable[SwitchTableLastElement].DataRefID = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].stepping_int = lua_tointeger(L, 4);
            SwitchTable[SwitchTableLastElement].stepping_float = lua_tonumber(L, 4);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].stepping_int = 1;
            SwitchTable[SwitchTableLastElement].stepping_float = 1;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].lower_limit_int = lua_tointeger(L, 5);
            SwitchTable[SwitchTableLastElement].lower_limit_float = lua_tonumber(L, 5);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].lower_limit_int = 0;
            SwitchTable[SwitchTableLastElement].lower_limit_float = 0;
        }
        if (lua_isnumber(L, 6))
        {
            SwitchTable[SwitchTableLastElement].round = lua_tonumber(L, 6);
        }
        else
        {
            SwitchTable[SwitchTableLastElement].round = 1;
        }
    }
    return 0;
}


static int LuaSetPilotsHead(lua_State *L)
{
    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_isnumber(L, 5)))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function set_pilots_head().");
        LuaIsRunning = false;
        return 0;
    }
    // set the view to 3D
    XPLMSetDatai(FWLViewType, 1026);
    update_Lua_dataref_variables(FWLViewType, 0, 1026);

    // set pilot's head
    XPLMSetDataf(FWLPilotsHeadX, lua_tonumber(L, 1));
    XPLMSetDataf(FWLPilotsHeadY, lua_tonumber(L, 2));
    XPLMSetDataf(FWLPilotsHeadZ, lua_tonumber(L, 3));
    XPLMSetDataf(FWLPilotsHeadHeading, lua_tonumber(L, 4));
    XPLMSetDataf(FWLPilotsHeadPitch, lua_tonumber(L, 5));
    return 0;
}

static int LuaGetPilotsHead(lua_State *L)
{
    float X = XPLMGetDataf(FWLPilotsHeadX);
    float Y = XPLMGetDataf(FWLPilotsHeadY);
    float Z = XPLMGetDataf(FWLPilotsHeadZ);
    float Heading = XPLMGetDataf(FWLPilotsHeadHeading);
    float Pitch = XPLMGetDataf(FWLPilotsHeadPitch);
    lua_pushnumber(L, X);
    lua_pushnumber(L, Y);
    lua_pushnumber(L, Z);
    lua_pushnumber(L, Heading);
    lua_pushnumber(L, Pitch);
    return 5;
}

static int LuaXPLMSetGraphicsState(lua_State *L)
{
    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_isnumber(L, 5) && lua_isnumber(L, 6) && lua_isnumber(L, 7) ))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetGraphicsState.");
        LuaIsRunning = false;
    }
    XPLMSetGraphicsState(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4), lua_tointeger(L, 5), lua_tointeger(L, 6), lua_tointeger(L, 7) );
    return 0;
}

// now follows the OpenGL part of FlyWithLua
// all these functions will *not* check for errors!!!
// error handling is made by the module written in Lua

static int LuaglBegin_POINTS(lua_State *L)
{
    glBegin(GL_POINTS);
    return 0;
}

static int LuaglBegin_LINES(lua_State *L)
{
    glBegin(GL_LINES);
    return 0;
}

static int LuaglBegin_LINE_STRIP(lua_State *L)
{
    glBegin(GL_LINE_STRIP);
    return 0;
}

static int LuaglBegin_LINE_LOOP(lua_State *L)
{
    glBegin(GL_LINE_LOOP);
    return 0;
}

static int LuaglBegin_POLYGON(lua_State *L)
{
    glBegin(GL_POLYGON);
    return 0;
}

static int LuaglBegin_TRIANGLES(lua_State *L)
{
    glBegin(GL_TRIANGLES);
    return 0;
}

static int LuaglBegin_TRIANGLE_STRIP(lua_State *L)
{
    glBegin(GL_TRIANGLE_STRIP);
    return 0;
}

static int LuaglBegin_TRIANGLE_FAN(lua_State *L)
{
    glBegin(GL_TRIANGLE_FAN);
    return 0;
}

static int LuaglBegin_QUADS(lua_State *L)
{
    glBegin(GL_QUADS);
    return 0;
}

static int LuaglBegin_QUAD_STRIP(lua_State *L)
{
    glBegin(GL_QUAD_STRIP);
    return 0;
}

static int LuaglEnd(lua_State *L)
{
    glEnd();
    return 0;
}

static int LuaglVertex2f(lua_State *L)
{
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    glVertex2f(x, y);
    return 0;
}

static int LuaglVertex3f(lua_State *L)
{
    float x = lua_tonumber(L, 1);
    float y = lua_tonumber(L, 2);
    float z = lua_tonumber(L, 3);
    glVertex3f(x, y, z);
    return 0;
}

static int LuaglLineWidth(lua_State *L)
{
    float width = lua_tonumber(L, 1);
    glLineWidth(width);
    return 0;
}

static int LuaglColor3f(lua_State *L)
{
    float red = lua_tonumber(L, 1);
    float green = lua_tonumber(L, 2);
    float blue = lua_tonumber(L, 3);
    glColor3f(red, green, blue);
    return 0;
}

static int LuaglColor4f(lua_State *L)
{
    float red = lua_tonumber(L, 1);
    float green = lua_tonumber(L, 2);
    float blue = lua_tonumber(L, 3);
    float alpha = lua_tonumber(L, 4);
    glColor4f(red, green, blue, alpha);
    return 0;
}

static int LuaglRectf(lua_State *L)
{
    float x1 = lua_tonumber(L, 1);
    float y1 = lua_tonumber(L, 2);
    float x2 = lua_tonumber(L, 3);
    float y2 = lua_tonumber(L, 4);

    // correct position to avoid invisible rectangles
    if (x1 < x2)
    {
        float xs = x1;
        x1 = x2;
        x2 = xs;
    }
    if (y1 > y2)
    {
        float ys = y1;
        y1 = y2;
        y2 = ys;
    }

    glRectf(x1, y1, x2, y2);
    return 0;
}

// The XPLMNavigation commands
static int      LuaXPLMGetFirstNavAid(lua_State *L)
{
    lua_pushnumber(L, XPLMGetFirstNavAid());
    return 1;
}

static int      LuaXPLMFindFirstNavAidOfType(lua_State *L)
{
    int     NavType;
    if (lua_isnumber(L, 1))
    {
        NavType = lua_tointeger(L, 1);
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMFindFirstNavAidOfType.");
        LuaIsRunning = false;
        return 0;
    }
    lua_pushnumber(L, XPLMFindFirstNavAidOfType(NavType));
    return 1;
}

static int      LuaXPLMFindLastNavAidOfType(lua_State *L)
{
    int     NavType;
    if (lua_isnumber(L, 1))
    {
        NavType = lua_tointeger(L, 1);
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMFindLastNavAidOfType.");
        LuaIsRunning = false;
        return 0;
    }
    lua_pushnumber(L, XPLMFindLastNavAidOfType(NavType));
    return 1;
}

static int      LuaXPLMGetNextNavAid(lua_State *L)
{
    int     NavID;
    if (lua_isnumber(L, 1))
    {
        NavID = lua_tointeger(L, 1);
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMGetNextNavAid.");
        LuaIsRunning = false;
        return 0;
    }
    lua_pushnumber(L, XPLMGetNextNavAid(NavID));
    return 1;
}

static int      LuaXPLMFindNavAid(lua_State *L)
{
    const char *         inNameFragment;    /* Can be NULL */
    const char *         inIDFragment;    /* Can be NULL */
    float *              inLat;    /* Can be NULL */
    float *              inLon;    /* Can be NULL */
    int *                inFrequency;    /* Can be NULL */
    XPLMNavType          inType;
    int                  Frequency;
    float                Lat, Lon;
    char                 NameFragment[NORMALSTRING];
    char                 IDFragment[NORMALSTRING];

    if (lua_isstring(L, 1))
    {
        strcpy(NameFragment, lua_tostring(L, 1));
        inNameFragment = NameFragment;
    }
    else
    {
        inNameFragment = NULL;
    }
    if (lua_isstring(L, 2))
    {
        strcpy(IDFragment, lua_tostring(L, 2));
        inIDFragment = IDFragment;
    }
    else
    {
        inIDFragment = NULL;
    }
    if (lua_isnumber(L, 3))
    {
        Lat = lua_tonumber(L, 3);
        inLat = &Lat;
    }
    else
    {
        inLat = NULL;
    }
    if (lua_isnumber(L, 4))
    {
        Lon = lua_tonumber(L, 4);
        inLon = &Lon;
    }
    else
    {
        inLon = NULL;
    }
    if (lua_isnumber(L, 5))
    {
        Frequency = lua_tonumber(L, 5);
        inFrequency = &Frequency;
    }
    else
    {
        inFrequency = NULL;
    }
    if (lua_isnumber(L, 6))
    {
        inType = lua_tonumber(L, 6);
    }
    else
    {
        inType = xplm_Nav_Airport;
    }
    lua_pushnumber(L, XPLMFindNavAid(inNameFragment, inIDFragment, inLat, inLon, inFrequency, inType));
    return 1;
}

static int      LuaXPLMGetNavAidInfo(lua_State *L)
{
    XPLMNavType        outType = xplm_Nav_Unknown;        /* Can be NULL */
    float              outLatitude = 0;    /* Can be NULL */
    float              outLongitude = 0;   /* Can be NULL */
    float              outHeight = 0;      /* Can be NULL */
    int                outFrequency = 0;   /* Can be NULL */
    float              outHeading = 0;     /* Can be NULL */
    char               outID[32] = "";     /* Can be NULL */
    char               outName[256] = "";

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMGetNavAidInfo.");
        LuaIsRunning = false;
        return 0;
    }

    XPLMGetNavAidInfo(lua_tointeger(L,1), &outType, &outLatitude, &outLongitude, &outHeight, &outFrequency, &outHeading, outID, outName, NULL);

    lua_pushnumber(L, outType);
    lua_pushnumber(L, outLatitude);
    lua_pushnumber(L, outLongitude);
    lua_pushnumber(L, outHeight);
    lua_pushnumber(L, outFrequency);
    lua_pushnumber(L, outHeading);
    lua_pushstring(L, outID);
    lua_pushstring(L, outName);
    return 8;
}

static int      LuaXPLMCountFMSEntries(lua_State *L)
{
    lua_pushnumber(L, XPLMCountFMSEntries());
    return 1;
}

static int      LuaXPLMGetDisplayedFMSEntry(lua_State *L)
{
    lua_pushnumber(L, XPLMGetDisplayedFMSEntry());
    return 1;
}

static int      LuaXPLMGetDestinationFMSEntry(lua_State *L)
{
    lua_pushnumber(L, XPLMGetDestinationFMSEntry());
    return 1;
}

static int      LuaXPLMSetDisplayedFMSEntry(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetDisplayedFMSEntry.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetDisplayedFMSEntry(lua_tointeger(L, 1));
    return 0;
}

static int      LuaXPLMSetDestinationFMSEntry(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetDestinationFMSEntry.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetDestinationFMSEntry(lua_tointeger(L, 1));
    return 0;
}

static int      LuaXPLMClearFMSEntry(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMClearFMSEntry.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMClearFMSEntry(lua_tointeger(L, 1));
    return 0;
}

static int      LuaXPLMSetFMSEntryLatLon(lua_State *L)
{
    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4)))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetFMSEntryLatLon.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetFMSEntryLatLon(lua_tointeger(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tointeger(L, 4));
    return 0;
}

static int      LuaXPLMSetFMSEntryInfo(lua_State *L)
{
    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetFMSEntryInfo.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetFMSEntryInfo(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3));
    return 0;
}

static int      LuaXPLMGetFMSEntryInfo(lua_State *L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetFMSEntryInfo.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMNavType         outType = xplm_Nav_Unknown;
    char                outID[256] = "";
    XPLMNavRef          outRef = XPLM_NAV_NOT_FOUND;
#if XPLM210
    int                 outAltitude = 0;
#else
    intptr_t            outAltitude = 0;
#endif
    float               outLat = 0;
    float               outLon = 0;

    XPLMGetFMSEntryInfo(lua_tointeger(L, 1), &outType, outID, &outRef, &outAltitude, &outLat, &outLon);

    lua_pushnumber(L, outType);
    lua_pushstring(L, outID);
    lua_pushnumber(L, outRef);
    lua_pushnumber(L, outAltitude);
    lua_pushnumber(L, outLat);
    lua_pushnumber(L, outLon);
    return 6;
}

// The core access to DataRefs (for advanced users of FlyWithLua)
static int      LuaXPLMFindDataRef(lua_State *L)
{
    char            DataRefWanted[NORMALSTRING];
    XPLMDataRef     DataRefID;

    strcpy(DataRefWanted, luaL_checkstring(L, 1));
    DataRefID = XPLMFindDataRef(DataRefWanted);
    if (DataRefID != NULL)
    {
        lua_pushlightuserdata(L, DataRefID);
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

static int      LuaXPLMGetDatai(lua_State *L)
{
    if (lua_islightuserdata(L, 1))
    {
        lua_pushnumber(L, XPLMGetDatai(lua_touserdata(L, 1)));
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMGetDatai.");
        LuaIsRunning = false;
        return 0;
    }
    return 1;
}

static int      LuaXPLMGetDataf(lua_State *L)
{
    if (lua_islightuserdata(L, 1))
    {
        lua_pushnumber(L, XPLMGetDataf(lua_touserdata(L, 1)));
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMGetDataf.");
        LuaIsRunning = false;
        return 0;
    }
    return 1;
}

static int      LuaXPLMGetDatad(lua_State *L)
{
    if (lua_islightuserdata(L, 1))
    {
        lua_pushnumber(L, XPLMGetDatad(lua_touserdata(L, 1)));
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMGetDatad.");
        LuaIsRunning = false;
        return 0;
    }
    return 1;
}

static int      LuaXPLMGetDatavf(lua_State *L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        float       SpaceForValues[100] = {0};
        int         inMax = lua_tointeger(L, 3);
        int         StartFrom = lua_tointeger(L, 2);
        if (inMax > 100)
        {
            logMsg(logToAll, "FlyWithLua Error: Can't handle more than 100 values in function XPLMGetDatavf.");
            LuaIsRunning = false;
            return 0;
        }
        lua_createtable(L, inMax, 1);
        XPLMGetDatavf(lua_touserdata(L, 1), SpaceForValues, StartFrom, inMax);
        for (int i=0; i<inMax; i++)
        {
            lua_pushnumber(L, StartFrom+i);
            lua_pushnumber(L, SpaceForValues[i]);
            lua_settable(L, -3);
        }
        return 1;
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMGetDatavf.");
        LuaIsRunning = false;
        return 0;
    }
}

static int      LuaXPLMGetDatavi(lua_State *L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        int         SpaceForValues[100] = {0};
        int         inMax = lua_tointeger(L, 3);
        int         StartFrom = lua_tointeger(L, 2);
        if (inMax > 100)
        {
            logMsg(logToAll, "FlyWithLua Error: Can't handle more than 100 values in function XPLMGetDatavi.");
            LuaIsRunning = false;
            return 0;
        }
        lua_createtable(L, inMax, 1);
        XPLMGetDatavi(lua_touserdata(L, 1), SpaceForValues, StartFrom, inMax);
        for (int i=0; i<inMax; i++)
        {
            lua_pushnumber(L, StartFrom+i);
            lua_pushnumber(L, SpaceForValues[i]);
            lua_settable(L, -3);
        }
        return 1;
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMGetDatavi.");
        LuaIsRunning = false;
        return 0;
    }
}

static int      LuaXPLMSetDatai(lua_State *L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2))
    {
        XPLMSetDatai(lua_touserdata(L, 1), lua_tointeger(L, 2));
        update_Lua_dataref_variables(lua_touserdata(L, 1), 0, lua_tonumber(L, 2));
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetDatai.");
        LuaIsRunning = false;
    }
    return 0;
}

static int      LuaXPLMSetDataf(lua_State *L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2))
    {
        XPLMSetDataf(lua_touserdata(L, 1), lua_tonumber(L, 2));
        update_Lua_dataref_variables(lua_touserdata(L, 1), 0, lua_tonumber(L, 2));
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetDataf.");
        LuaIsRunning = false;
    }
    return 0;
}

static int      LuaXPLMSetDatad(lua_State *L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2))
    {
        XPLMSetDatad(lua_touserdata(L, 1), lua_tonumber(L, 2));
        update_Lua_dataref_variables(lua_touserdata(L, 1), 0, lua_tonumber(L, 2));
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetDatad.");
        LuaIsRunning = false;
    }
    return 0;
}

static int      LuaXPLMSetDatavi(lua_State *L)
{
    if (lua_islightuserdata(L, 1) && lua_istable(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
    {
        int         SpaceForValues[100] = {0};
        int         inMax = lua_tointeger(L, 4);
        int         StartFrom = lua_tointeger(L, 3);
        XPLMDataRef DataRefWanted = lua_touserdata(L, 1);
        if (inMax > 100)
        {
            logMsg(logToAll, "FlyWithLua Error: Can't handle more than 100 values in function XPLMSetDatavi.");
            LuaIsRunning = false;
            return 0;
        }
        lua_pushvalue(L, 2);
        for (int i=0; i<inMax; i++)
        {
            lua_pushnumber(L, StartFrom+i);
            lua_gettable(L, -2);
            SpaceForValues[i] = lua_tonumber(L, -1);
            lua_pop(L, 1);
            update_Lua_dataref_variables(DataRefWanted, StartFrom+i, (float) SpaceForValues[i]);
        }
        XPLMSetDatavi(DataRefWanted, SpaceForValues, StartFrom, inMax);
        return 0;
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetDatavi.");
        LuaIsRunning = false;
        return 0;
    }
}

static int      LuaXPLMSetDatavf(lua_State *L)
{
    if (lua_islightuserdata(L, 1) && lua_istable(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
    {
        float       SpaceForValues[100] = {0};
        int         inMax = lua_tointeger(L, 4);
        int         StartFrom = lua_tointeger(L, 3);
        XPLMDataRef DataRefWanted = lua_touserdata(L, 1);
        if (inMax > 100)
        {
            logMsg(logToAll, "FlyWithLua Error: Can't handle more than 100 values in function XPLMSetDatavf.");
            LuaIsRunning = false;
            return 0;
        }
        lua_pushvalue(L, 2);
        for (int i=0; i<inMax; i++)
        {
            lua_pushnumber(L, StartFrom+i);
            lua_gettable(L, -2);
            SpaceForValues[i] = lua_tonumber(L, -1);
            lua_pop(L, 1);
            update_Lua_dataref_variables(DataRefWanted, StartFrom+i, SpaceForValues[i]);
        }
        XPLMSetDatavf(DataRefWanted, SpaceForValues, StartFrom, inMax);
        return 0;
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function XPLMSetDatavf.");
        LuaIsRunning = false;
        return 0;
    }
}

static int      LuaXPLMGetDataRefTypes(lua_State *L)
{
    int         ReferenceType = xplmType_Unknown;

    if (lua_islightuserdata(L, 1))
    {
        ReferenceType = XPLMGetDataRefTypes(lua_touserdata(L, 1));
        lua_pushnumber(L, ReferenceType);
        return 1;
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong argument to function XPLMGetDataRefTypes, expecting a single userdata as a reference.");
        LuaIsRunning = false;
        return 0;
    }
}

static int      Luapeek(lua_State *L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        XPLMDataRef     DataRefIdWanted;
        int             DataRefTypeIdWanted;
        int             IndexWanted;

        DataRefIdWanted     = lua_touserdata(L, 1);
        DataRefTypeIdWanted = lua_tointeger(L, 2);
        IndexWanted         = lua_tointeger(L, 3);

        if (DataRefTypeIdWanted == xplmType_Float)
        {
            float ValueOfDataRef = XPLMGetDataf(DataRefIdWanted);
            lua_pushnumber(FWLLua, ValueOfDataRef);
            return 1;
        }

        if (DataRefTypeIdWanted == xplmType_Int)
        {
            int ValueOfDataRef = XPLMGetDatai(DataRefIdWanted);
            lua_pushnumber(FWLLua, ValueOfDataRef);
            return 1;
        }

        if ((DataRefTypeIdWanted == xplmType_Double) || (DataRefTypeIdWanted == 6))
        {
            double ValueOfDataRef = XPLMGetDatad(DataRefIdWanted);
            lua_pushnumber(FWLLua, ValueOfDataRef);
            return 1;
        }

        if (DataRefTypeIdWanted == xplmType_FloatArray)
        {
            float ValueOfDataRef;
            XPLMGetDatavf(DataRefIdWanted, &ValueOfDataRef, IndexWanted, 1);
            lua_pushnumber(FWLLua, ValueOfDataRef);
            return 1;
        }

        if (DataRefTypeIdWanted == xplmType_IntArray)
        {
            int ValueOfDataRef;
            XPLMGetDatavi(DataRefIdWanted, &ValueOfDataRef, IndexWanted, 1);
            lua_pushnumber(FWLLua, ValueOfDataRef);
            return 1;
        }

        if (DataRefTypeIdWanted == xplmType_Data)
        {
            char ValueOfDataRef[LONGSTRING];
            XPLMGetDatab(DataRefIdWanted, &ValueOfDataRef, IndexWanted, LONGSTRING);
            lua_pushstring(FWLLua, ValueOfDataRef);
            return 1;
        }
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong argument to function peek, expecting a userdata as a reference, the type of the DataRef and an index.");
        LuaIsRunning = false;
        return 0;
    }

    return 1; // snagar: added.
}

static int      Luapoke(lua_State *L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        XPLMDataRef     DataRefId;
        int             DataRefTypeId;
        int             Index;

        DataRefId     = lua_touserdata(L, 1);
        DataRefTypeId = lua_tointeger(L, 2);
        Index         = lua_tointeger(L, 3);

        if ((DataRefTypeId == xplmType_Int) && lua_isnumber(L, 4))
        {
            int DataRefValue = (int) lua_tonumber(FWLLua, 4);
            XPLMSetDatai(DataRefId, DataRefValue);
            update_Lua_dataref_variables(DataRefId, Index, (float) DataRefValue);
        }
        if ((DataRefTypeId == xplmType_Float) && lua_isnumber(L, 4))
        {
            float DataRefValue = (float) lua_tonumber(FWLLua, 4);
            XPLMSetDataf(DataRefId, DataRefValue);
            update_Lua_dataref_variables(DataRefId, Index, (float) DataRefValue);
        }
        if ((DataRefTypeId == xplmType_Double) && lua_isnumber(L, 4))
        {
            double DataRefValue = (double) lua_tonumber(FWLLua, 4);
            XPLMSetDatad(DataRefId, DataRefValue);
            update_Lua_dataref_variables(DataRefId, Index, (float) DataRefValue);
        }
        if ((DataRefTypeId == xplmType_IntArray) && lua_isnumber(L, 4))
        {
            int DataRefValue = lua_tointeger(FWLLua, -1);
            XPLMSetDatavi(DataRefId, &DataRefValue, Index, 1);
            update_Lua_dataref_variables(DataRefId, Index, (float) DataRefValue);
        }
        if ((DataRefTypeId == xplmType_FloatArray) && lua_isnumber(L, 4))
        {
            float DataRefValue = lua_tonumber(FWLLua, 4);
            XPLMSetDatavf(DataRefId, &DataRefValue, Index, 1);
            update_Lua_dataref_variables(DataRefId, Index, (float) DataRefValue);
        }
        if ((DataRefTypeId == xplmType_Data) && lua_isstring(L, 4))
        {
            char    DataRefValue[LONGSTRING];
            strcpy(DataRefValue, lua_tostring(FWLLua, 4));
            XPLMSetDatab(DataRefId, DataRefValue, Index, sizeof(DataRefValue));
            update_Lua_dataref_strings(DataRefId, Index, DataRefValue);
        }
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong argument to function poke, expecting a userdata as a reference, the type of the DataRef, an index and the value to set.");
        LuaIsRunning = false;
        return 0;
    }
    return 1; // snagar: added.
}

// we leave these functions inside FlyWithLua 2.2.1 to be compatible to older versions
static int      Luabegin_classic_mode(lua_State *L)
{
    logMsg(logToDevCon, "FlyWithLua Info: Since version 2.2.1 we do not differ in classic or modern scripts, begin_classic_mode() will be ignored.");
    return 0;
}

static int      Luaend_classic_mode(lua_State *L)
{
    logMsg(logToDevCon, "FlyWithLua Info: Since version 2.2.1 we do not differ in classic or modern scripts, end_classic_mode() will be ignored.");
    return 0;
}

static int      LuaCreateCommand(lua_State *L)
{
    // usage: create_command( "name", "description", "begin command", "continue command", "end command" )
    if (!(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3) && lua_isstring(L, 4) && lua_isstring(L, 5)))
    {
        logMsg(logToAll, "FlyWithLua Error: Wrong arguments to function create_command().");
        LuaIsRunning = false;
        return 0;
    }
    if (++CommandTableLastElement >= MAXCOMMANDS)
    {
        logMsg(logToAll, "FlyWithLua Error: Too much functions create_command().");
        LuaIsRunning = false;
        return 0;
    }
    if (strncmp(lua_tostring(L, 1), "sim/", 4) == 0)
    {
        logMsg(logToAll, "FlyWithLua Error: Custom commands are prohibited to start with \"sim/\".");
        LuaIsRunning = false;
        return 0;
    }
    CommandTable[CommandTableLastElement].Name = lua_tostring(L, 1);
    CommandTable[CommandTableLastElement].Description = lua_tostring(L, 2);
    CommandTable[CommandTableLastElement].BeginCommand = lua_tostring(L, 3);
    CommandTable[CommandTableLastElement].ContinueCommand = lua_tostring(L, 4);
    CommandTable[CommandTableLastElement].EndCommand = lua_tostring(L, 5);
    CommandTable[CommandTableLastElement].Reference = XPLMCreateCommand(CommandTable[CommandTableLastElement].Name.c_str(), CommandTable[CommandTableLastElement].Description.c_str());
    XPLMRegisterCommandHandler(CommandTable[CommandTableLastElement].Reference, FWLCommandCallback, 1, (void *) "FlyWithLuaCommand");
    return 0;
}

static int      Luadirectory_to_table(lua_State *L)
{
    char            DirectoryPath[LONGSTRING] = "";
    int    k;
    char            FilesInFolder[15000];
#if XPLM210
    int             NumberOfFiles;
    int             TotalNumberOfFiles;
#else
    intptr_t        NumberOfFiles;
    intptr_t        TotalNumberOfFiles;
#endif
    char*           FileIndex[500];

    strcpy(DirectoryPath, luaL_checkstring(L, 1));

    if (XPLMGetDirectoryContents(DirectoryPath, 0, FilesInFolder, sizeof(FilesInFolder), FileIndex, 500, &TotalNumberOfFiles, &NumberOfFiles))
    {
        // create an empty table on stack
        lua_createtable(L, NumberOfFiles, 0);

        // are enough files in the folder to read them out?
        if (NumberOfFiles == 0)
        {
            // just return an empty table
            return 1;
        }

        // sorting the files
        qsort(FileIndex, NumberOfFiles, sizeof(char *), compare_filenames);

        // reading ot the files into a Lua table step by step
        for (k = 0; k < NumberOfFiles; k++)
        {
            lua_pushnumber(L, k+1);
            lua_pushstring(L, FileIndex[k]);
            lua_settable(L, -3);
        }
    }
    else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Can't read out the subfolder given to function directory_to_table().");
        LuaIsRunning = false;
        return 0;
    }

    return true; // snagar
}


void RegisterCoreCFunctionsToLua(lua_State *L)
{
    lua_register(L, "XSBSpeakString", LuaXSBSpeakString);
    lua_register(L, "XSBShowFlightplan", LuaXSBShowFlightplan);
    lua_register(L, "XSBSendFlightplan", LuaXSBSendFlightplan);
    lua_register(L, "XSBUserLogin", LuaXSBUserLogin);
    lua_register(L, "XSBConnect", LuaXSBConnect);
    lua_register(L, "XSBDisconnect", LuaXSBDisconnect);
    lua_register(L, "XSBLookupATC", LuaXSBLookupATC);
    lua_register(L, "logMsg", LualogMsg);
    lua_register(L, "XPLMSpeakString", LuaSpeakString);
    lua_register(L, "DataRef", LuaDataRef);
    lua_register(L, "dataref", LuaDataRef);
    lua_register(L, "get_dataref_binding", LuaGetDataRefBinding);
    lua_register(L, "get_DataRef_binding", LuaGetDataRefBinding);
    lua_register(L, "create_switch", LuaCreateSwitch);
    lua_register(L, "create_positive_edge_trigger", LuaCreatePositiveEdgeTrigger);
    lua_register(L, "create_negative_edge_trigger", LuaCreateNegativeEdgeTrigger);
    lua_register(L, "create_positive_edge_increment", LuaCreatePositiveEdgeIncrement);
    lua_register(L, "create_negative_edge_increment", LuaCreateNegativeEdgeIncrement);
    lua_register(L, "create_positive_edge_decrement", LuaCreatePositiveEdgeDecrement);
    lua_register(L, "create_negative_edge_decrement", LuaCreateNegativeEdgeDecrement);
    lua_register(L, "do_on_new_metar", LuaDoEveryMETARCallback);
    lua_register(L, "do_on_keystroke", LuaDoEveryKeystroke);
    lua_register(L, "do_on_mouse_click", LuaDoEveryMouseClick);
    lua_register(L, "do_on_mouse_wheel", LuaDoEveryMouseWheel);
    lua_register(L, "do_every_draw", LuaDoEveryDrawCallback);
    lua_register(L, "do_every_frame", LuaDoEveryFrame);
    lua_register(L, "do_often", LuaDoOften);
    lua_register(L, "do_sometimes", LuaDoSometimes);
    lua_register(L, "add_macro", LuaAddMacro);
    lua_register(L, "add_ATC_macro", LuaAddATCMacro);
    lua_register(L, "add_atc_macro", LuaAddATCMacro);
    lua_register(L, "activate_macro", LuaActivateMacro);
    lua_register(L, "deactivate_macro", LuaDeactivateMacro);
    lua_register(L, "set_button_assignment", LuaSetButtonAssignment);
    lua_register(L, "set_axis_assignment", LuaSetAxisAssignment);
    lua_register(L, "clear_all_button_assignments", LuaClearAllButtonAssignments);
    lua_register(L, "clear_all_axis_assignments", LuaClearAllAxisAssignments);
    lua_register(L, "button", LuaButton);
    lua_register(L, "last_button", LuaLastButton);
    lua_register(L, "get", LuaGet);
    lua_register(L, "set", LuaSet);
    lua_register(L, "set_array", LuaSetArray);
    lua_register(L, "command_once", LuaCommandOnce);
    lua_register(L, "crash_the_sim", LuaCrashTheSim);
    lua_register(L, "draw_string", LuaDrawString);
    lua_register(L, "draw_string_Helvetica_18", LuaDrawStringHelv18);
    lua_register(L, "draw_string_Helvetica_12", LuaDrawStringHelv12);
    lua_register(L, "draw_string_Helvetica_10", LuaDrawStringHelv10);
    lua_register(L, "draw_string_Times_Roman_10", LuaDrawStringTimes10);
    lua_register(L, "draw_string_Times_Roman_24", LuaDrawStringTimes24);
    lua_register(L, "measure_string", LuaMeasureString);
    lua_register(L, "XPLMSetGraphicsState", LuaXPLMSetGraphicsState);
    lua_register(L, "glBegin_POINTS", LuaglBegin_POINTS);
    lua_register(L, "glBegin_LINES", LuaglBegin_LINES);
    lua_register(L, "glBegin_LINE_STRIP", LuaglBegin_LINE_STRIP);
    lua_register(L, "glBegin_LINE_LOOP", LuaglBegin_LINE_LOOP);
    lua_register(L, "glBegin_POLYGON", LuaglBegin_POLYGON);
    lua_register(L, "glBegin_TRIANGLES", LuaglBegin_TRIANGLES);
    lua_register(L, "glBegin_TRIANGLE_STRIP", LuaglBegin_TRIANGLE_STRIP);
    lua_register(L, "glBegin_TRIANGLE_FAN", LuaglBegin_TRIANGLE_FAN);
    lua_register(L, "glBegin_QUADS", LuaglBegin_QUADS);
    lua_register(L, "glBegin_QUAD_STRIP", LuaglBegin_QUAD_STRIP);
    lua_register(L, "glEnd", LuaglEnd);
    lua_register(L, "glVertex2f", LuaglVertex2f);
    lua_register(L, "glVertex3f", LuaglVertex3f);
    lua_register(L, "glLineWidth", LuaglLineWidth);
    lua_register(L, "glColor3f", LuaglColor3f);
    lua_register(L, "glColor4f", LuaglColor4f);
    lua_register(L, "glRectf", LuaglRectf);
    lua_register(L, "XPLMGetFirstNavAid", LuaXPLMGetFirstNavAid);
    lua_register(L, "XPLMGetNextNavAid", LuaXPLMGetNextNavAid);
    lua_register(L, "XPLMFindFirstNavAidOfType", LuaXPLMFindFirstNavAidOfType);
    lua_register(L, "XPLMFindLastNavAidOfType", LuaXPLMFindLastNavAidOfType);
    lua_register(L, "XPLMFindNavAid", LuaXPLMFindNavAid);
    lua_register(L, "XPLMGetNavAidInfo", LuaXPLMGetNavAidInfo);
    lua_register(L, "XPLMCountFMSEntries", LuaXPLMCountFMSEntries);
    lua_register(L, "XPLMGetDisplayedFMSEntry", LuaXPLMGetDisplayedFMSEntry);
    lua_register(L, "XPLMGetDestinationFMSEntry", LuaXPLMGetDestinationFMSEntry);
    lua_register(L, "XPLMSetDisplayedFMSEntry", LuaXPLMSetDisplayedFMSEntry);
    lua_register(L, "XPLMSetDestinationFMSEntry", LuaXPLMSetDestinationFMSEntry);
    lua_register(L, "XPLMClearFMSEntry", LuaXPLMClearFMSEntry);
    lua_register(L, "XPLMSetFMSEntryLatLon", LuaXPLMSetFMSEntryLatLon);
    lua_register(L, "XPLMSetFMSEntryInfo", LuaXPLMSetFMSEntryInfo);
    lua_register(L, "XPLMGetFMSEntryInfo", LuaXPLMGetFMSEntryInfo);
    lua_register(L, "begin_classic_mode", Luabegin_classic_mode);
    lua_register(L, "end_classic_mode", Luaend_classic_mode);
    lua_register(L, "XPLMFindDataRef", LuaXPLMFindDataRef);
    lua_register(L, "XPLMGetDatai", LuaXPLMGetDatai);
    lua_register(L, "XPLMGetDataf", LuaXPLMGetDataf);
    lua_register(L, "XPLMGetDatad", LuaXPLMGetDatad);
    lua_register(L, "XPLMGetDatavi", LuaXPLMGetDatavi);
    lua_register(L, "XPLMGetDatavf", LuaXPLMGetDatavf);
    lua_register(L, "XPLMSetDatai", LuaXPLMSetDatai);
    lua_register(L, "XPLMSetDataf", LuaXPLMSetDataf);
    lua_register(L, "XPLMSetDatad", LuaXPLMSetDatad);
    lua_register(L, "XPLMSetDatavi", LuaXPLMSetDatavi);
    lua_register(L, "XPLMSetDatavf", LuaXPLMSetDatavf);
    lua_register(L, "XPLMGetDataRefTypes", LuaXPLMGetDataRefTypes);
    lua_register(L, "create_command", LuaCreateCommand);
    lua_register(L, "directory_to_table", Luadirectory_to_table);
    lua_register(L, "peek", Luapeek);
    lua_register(L, "poke", Luapoke);
    lua_register(L, "set_pilots_head", LuaSetPilotsHead);
    lua_register(L, "get_pilots_head", LuaGetPilotsHead);

    // function to access HID devices (new since FWL2.1)
    lua_register(L, "create_HID_table", Luacreate_HID_table);
    lua_register(L, "hid_open", Luahid_open);
    lua_register(L, "hid_open_path", Luahid_open_path);
    lua_register(L, "hid_close", Luahid_close);
    lua_register(L, "hid_write", Luahid_write);
    lua_register(L, "hid_read_timeout", Luahid_read_timeout);
    lua_register(L, "hid_read", Luahid_read);
    lua_register(L, "hid_set_nonblocking", Luahid_set_nonblocking);
    lua_register(L, "hid_send_feature_report", Luahid_send_feature_report);
    lua_register(L, "hid_get_feature_report", Luahid_get_feature_report);

}

// sort DataRefTable by name and index of the DataRef
typedef int (*compfn)(const void*, const void*);

static int compare_datareftable(struct DataRefTableStructure *a, struct DataRefTableStructure *b)
{
    int     result = strcmp(a->DataRefName, b->DataRefName);
    if (result == 0)
    {
        if (a->Index > b->Index) return 1;
        else if (a->Index < b->Index) return -1;
        else return 0;
    }
    else return result;
}

// sort SwitchTable by number of first button
static int compare_switchtable(struct SwitchTableStructure *a, struct SwitchTableStructure *b)
{
    if (a->button > b->button) return 1;
    else if (a->button < b->button) return -1;
    else return 0;
}

void DebugLua( void )
{
    time_t      DebugZeit;
    time(&DebugZeit);
    ofstream DebugFile("FlyWithLua_Debug.txt");
    if (DebugFile.is_open() != true)
    {
        logMsg(logToDevCon, "FlyWithLua Error: Unable to write a debug file.");
        return;
    }
    DebugFile << "-- FlyWithLua v" << PLUGIN_VERSION << " Debug File\n-- " << ctime(&DebugZeit) << "-- Memory in use: " << lua_gc(FWLLua, LUA_GCCOUNT, 0) << " kb";
    DebugFile << "\n*** Lua stack ***\n";
    int StackSize = lua_gettop(FWLLua);
    if (StackSize == 0)
    {
        DebugFile << "The stack is empty.\n";
    }
    else
    {
        DebugFile << "Number of elements: " << StackSize << "\n";
        for (int i=1; i<=StackSize; i++)
        {
            if (lua_isstring(FWLLua, i))
            {
                DebugFile << "Element no. " << i << " is: " << lua_tostring(FWLLua, i) << "\n";
            }
        }
    }
    DebugFile << "\n\n*** Every draw loop callback ***\n";
    DebugFile << LuaDrawCommand;
    DebugFile << "\n\n*** Every frame loop callback ***\n";
    DebugFile << EveryFrameCallbackCommand;
    DebugFile << "\n\n*** Often loop callback ***\n";
    DebugFile << CallbackCommand;
    DebugFile << "\n\n*** Sometimes loop callback ***\n";
    DebugFile << LongTimeCallbackCommand;
    DebugFile << "\n\n*** Keystroke callback ***\n";
    DebugFile << KeyEventCommand;
    DebugFile << "\n\n*** Mouse click callback ***\n";
    DebugFile << LuaMouseClickCommand;
    DebugFile << "\n\n*** Mouse wheel callback ***\n";
    DebugFile << LuaMouseWheelCommand;
    DebugFile << "\n\n*** New METAR callback ***\n";
    DebugFile << NewMetarCommand;
    DebugFile << "\n\n*** command callbacks ***\n";
    if (CommandTableLastElement >= 0)
    {
        for (int i=0; i<=CommandTableLastElement; i++)
        {
            DebugFile << "Command    --> \"" << CommandTable[i].Name << "\" (" << CommandTable[i].Reference << ")\n";
            DebugFile << "Description--> \"" << CommandTable[i].Description << "\"\n";
            DebugFile << "Begin      --> " << CommandTable[i].BeginCommand << "\n";
            DebugFile << "Continue   --> " << CommandTable[i].ContinueCommand << "\n";
            DebugFile << "End        --> " << CommandTable[i].EndCommand << "\n\n";
        }
    }
    else
    {
        DebugFile << "No custom commands defined.\n";
    }

    DebugFile << "\n\n*** DataRefs stored in Lua variables ***\n";
    if (DataRefTableLastElement >= 0)
    {
        // sort the DataRef table
        qsort((void *) &DataRefTable, DataRefTableLastElement+1, (size_t) sizeof(struct DataRefTableStructure), (compfn) compare_datareftable);

        for (int i=0; i<=DataRefTableLastElement; i++)
        {
            DebugFile << "The DataRef \"" << DataRefTable[i].DataRefName << "\" is stored in \"" << DataRefTable[i].LuaVariable;
            if (DataRefTable[i].IsReadOnly)
            {
                DebugFile << "\" (read only, type = ";
            }
            else
            {
                DebugFile << "\" (writeable, type = ";
            }
            if (DataRefTable[i].DataRefTypeId == xplmType_Int)
            {
                DebugFile << "int).\n";
            }
            else if (DataRefTable[i].DataRefTypeId == xplmType_Float)
            {
                DebugFile << "float).\n";
            }
            else if (DataRefTable[i].DataRefTypeId == xplmType_Double)
            {
                DebugFile << "double).\n";
            }
            else if (DataRefTable[i].DataRefTypeId == xplmType_IntArray)
            {
                DebugFile << "int array, index = " << DataRefTable[i].Index << ").\n";
            }
            else if (DataRefTable[i].DataRefTypeId == xplmType_FloatArray)
            {
                DebugFile << "float array, index = " << DataRefTable[i].Index << ").\n";
            }
            else if (DataRefTable[i].DataRefTypeId == xplmType_Data)
            {
                DebugFile << "string).\n";
            }
            else
            {
                DebugFile << "unknown, " << (int) DataRefTable[i].DataRefTypeId << ").\n";
            }
        }
    }
    else
    {
        DebugFile << "No DataRefs stored in Lua variables.\n";
    }

    DebugFile << "\n\n*** Switches created by Lua ***\n";
    if (SwitchTableLastElement >= 0)
    {
        char      WhatToSay[LONGSTRING];

        // sort the switch table
        qsort((void *) &SwitchTable, SwitchTableLastElement+1, (size_t) sizeof(struct SwitchTableStructure), (compfn) compare_switchtable);

        for (int i=0; i<=SwitchTableLastElement; i++)
        {
            if (SwitchTable[i].SwitchType == Switch)
            {
                sprintf(WhatToSay, "The button %i switches %s[%i] to %f (if off) or to %f (if on).\n",
                        SwitchTable[i].button, SwitchTable[i].DataRefName.c_str(), SwitchTable[i].index, SwitchTable[i].off_float, SwitchTable[i].on_float);
            }
            if (SwitchTable[i].SwitchType == PositiveEdge)
            {
                sprintf(WhatToSay, "The positive edge of button %i sets %s[%i] to %f.\n",
                        SwitchTable[i].button, SwitchTable[i].DataRefName.c_str(), SwitchTable[i].index, SwitchTable[i].on_float);
            }
            if (SwitchTable[i].SwitchType == NegativeEdge)
            {
                sprintf(WhatToSay, "The negative edge of button %i sets %s[%i] to %f.\n",
                        SwitchTable[i].button, SwitchTable[i].DataRefName.c_str(), SwitchTable[i].index, SwitchTable[i].on_float);
            }
            if (SwitchTable[i].SwitchType == PositiveIncrement)
            {
                sprintf(WhatToSay, "The positive edge of button %i increments %s[%i] by %f (limit = %f, round to %f).\n",
                        SwitchTable[i].button, SwitchTable[i].DataRefName.c_str(), SwitchTable[i].index,
                        SwitchTable[i].stepping_float, SwitchTable[i].upper_limit_float, SwitchTable[i].round);
            }
            if (SwitchTable[i].SwitchType == NegativeIncrement)
            {
                sprintf(WhatToSay, "The negative edge of button %i increments %s[%i] by %f (limit = %f, round to %f).\n",
                        SwitchTable[i].button, SwitchTable[i].DataRefName.c_str(), SwitchTable[i].index,
                        SwitchTable[i].stepping_float, SwitchTable[i].upper_limit_float, SwitchTable[i].round);
            }
            if (SwitchTable[i].SwitchType == PositiveDecrement)
            {
                sprintf(WhatToSay, "The positive edge of button %i decrements %s[%i] by %f (limit = %f, round to %f).\n",
                        SwitchTable[i].button, SwitchTable[i].DataRefName.c_str(), SwitchTable[i].index,
                        SwitchTable[i].stepping_float, SwitchTable[i].lower_limit_float, SwitchTable[i].round);
            }
            if (SwitchTable[i].SwitchType == NegativeDecrement)
            {
                sprintf(WhatToSay, "The negative edge of button %i decrements %s[%i] by %f (limit = %f, round to %f).\n",
                        SwitchTable[i].button, SwitchTable[i].DataRefName.c_str(), SwitchTable[i].index,
                        SwitchTable[i].stepping_float, SwitchTable[i].lower_limit_float, SwitchTable[i].round);
            }
            DebugFile << WhatToSay;
        }
    }
    else
    {
        DebugFile << "No switches created by Lua.\n";
    }
    DebugFile << "\n\n*** Macros from macro and ATC memu are stored in a table ***\n";
    if (MacroTableLastElement >= 0)
    {
        for (int i=0; i<=MacroTableLastElement; i++)
        {
            DebugFile << "<<< Macro table element no. " << i;
            if (MacroTable[i].IsSwitch)
            {
                DebugFile << " is a switch. >>>\n";
                DebugFile << "name      -->\"" << MacroTable[i].MacroName << "\"\n";
                DebugFile << "activate  -->" << MacroTable[i].ActivateCommand << "\n";
                DebugFile << "deactivate-->" << MacroTable[i].DeactivateCommand << "\n\n";
            }
            else
            {
                DebugFile << " is not a switch. >>>\n";
                DebugFile << "name      -->\"" << MacroTable[i].MacroName << "\"\n";
                DebugFile << "always    -->" << MacroTable[i].ActivateCommand << "\n\n";
            }
        }
    }
    DebugFile.close();
}

void CopyDataRefsToLua( void )
{
    if (LuaIsRunning == false) return;
    if (DataRefTableLastElement >= 0)
    {
        for (int i=0; i<= DataRefTableLastElement; i++)
        {
            PushDataRefToLuaVariable(   DataRefTable[i].LuaVariable,
                                        DataRefTable[i].DataRefId,
                                        DataRefTable[i].DataRefTypeId,
                                        DataRefTable[i].Index);
        }
    }
    return;
}

void CopyDataRefsToXPlane( void )
{
    if (LuaIsRunning == false) return;
    if (DataRefTableLastElement >= 0)
    {
        for (int i=0; i<= DataRefTableLastElement; i++)
        {
            if (!DataRefTable[i].IsReadOnly)
            {
                if (DataRefTable[i].DataRefTypeId == xplmType_Int)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    int DataRefValue = (int) lua_tonumber(FWLLua, -1);
                    lua_pop(FWLLua, 1);
                    XPLMSetDatai(DataRefTable[i].DataRefId, DataRefValue);
                }
                if (DataRefTable[i].DataRefTypeId == xplmType_Float)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    float DataRefValue = (float) lua_tonumber(FWLLua, -1);
                    lua_pop(FWLLua, 1);
                    XPLMSetDataf(DataRefTable[i].DataRefId, DataRefValue);
                }
                if (DataRefTable[i].DataRefTypeId == xplmType_Double)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    double DataRefValue = (double) lua_tonumber(FWLLua, -1);
                    lua_pop(FWLLua, 1);
                    XPLMSetDatad(DataRefTable[i].DataRefId, DataRefValue);
                }
                if (DataRefTable[i].DataRefTypeId == xplmType_IntArray)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    int DataRefValue = lua_tointeger(FWLLua, -1);
                    lua_pop(FWLLua, 1);
                    XPLMSetDatavi(DataRefTable[i].DataRefId, &DataRefValue, DataRefTable[i].Index, 1);
                }
                if (DataRefTable[i].DataRefTypeId == xplmType_FloatArray)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    float DataRefValue = lua_tonumber(FWLLua, -1);
                    lua_pop(FWLLua, 1);
                    XPLMSetDatavf(DataRefTable[i].DataRefId, &DataRefValue, DataRefTable[i].Index, 1);
                }
                if (DataRefTable[i].DataRefTypeId == xplmType_Data)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    char    DataRefValue[LONGSTRING];
                    strcpy(DataRefValue, lua_tostring(FWLLua, -1));
                    lua_pop(FWLLua, 1);
                    XPLMSetDatab(DataRefTable[i].DataRefId, DataRefValue, 0, sizeof(DataRefValue));
                }
            }
        }
    }
    return;
}

bool RunLuaString(std::string LuaCommandString)
{
    if (LuaIsRunning == false) return false;
    CopyDataRefsToLua();
    if (luaL_dostring(FWLLua, LuaCommandString.c_str()))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Can't execute command string. The string who failed is:");
        logMsg(logToDevCon, LuaCommandString);
        LuaIsRunning = false;
    }
    CopyDataRefsToXPlane();
    return true;
}

void ResetLuaEngine( void )
{
    char    success_cstring[NORMALSTRING];

    // run through the exit script
    if ((LuaResetCount>0) && !ReadScriptFile("Resources/plugins/FlyWithLua/Internals/FlyWithLua.exit"))
    {
        logMsg(logToAll, "FlyWithLua Error: Unable to load exit file.");
    }

    LuaIsRunning = false;

    // killing all commands
    if (CommandTableLastElement > -1)
    {
        for (int i=0; i<= CommandTableLastElement; i++)
        {
            XPLMUnregisterCommandHandler(CommandTable[i].Reference, FWLCommandCallback, 1, (void *) "FlyWithLuaCommand");
            CommandTable[i].Reference = NULL;
            CommandTable[i].Name.erase();
            CommandTable[i].Description.erase();
            CommandTable[i].BeginCommand.erase();
            CommandTable[i].ContinueCommand.erase();
            CommandTable[i].EndCommand.erase();
        }
    }
    CommandTableLastElement = -1;

    lua_close(FWLLua);
    lj_alloc_destroy(ud);

    XPLMDataRef lua_alloc_ref = XPLMFindDataRef("sim/operation/prefs/misc/has_lua_alloc");
    if(lua_alloc_ref && XPLMGetDatai(lua_alloc_ref))
    {
        /* X-Plane has an allocator for us - we _must_ use it. */
        ud = lj_alloc_create();
        if (ud == NULL)
        {
            logMsg(logToDevCon, "FlyWithLua Error: LuaJIT Kernel Panic! No space to create the Lua instance!");
        }
        FWLLua = lua_newstate(lj_alloc_f, ud);
    }
    else
    {
        /* use the default allocator. */
        FWLLua = luaL_newstate();
    }

    LuaDrawCommand.clear();
    EveryFrameCallbackCommand.clear();
    CallbackCommand.clear();
    LongTimeCallbackCommand.clear();
    NewMetarCommand.clear();
    KeyEventCommand.clear();
    LuaMouseClickCommand.clear();
    LuaMouseWheelCommand.clear();

    // clean up HID connections
    CloseAllOpenHIDDevices();
    hid_exit();

    // init the XSB connection
    XSBPluginId = XPLMFindPluginBySignature("vatsim.protodev.clients.xsquawkbox");
    XSBInputUsrMsgXDataRef = XPLMFindDataRef(XSB_INPUT_USER_MSG);
    XSBInputStringXDataRef = XPLMFindDataRef(XSB_INPUT_STRING);
    XSBDestinationXDataRef = XPLMFindDataRef(XSB_FP_ARRIVAL_AIRPORT);
    XSBAlternativeXDataRef = XPLMFindDataRef(XSB_FP_ALTERNATE_AIRPORT);
    XSBStartAirportXDataRef = XPLMFindDataRef(XSB_FP_DEPARTURE_AIRPORT);
    XSBATCFreqXDataRef = XPLMFindDataRef(XSB_ATC_FREQ);
    XSBATCCallsignXDataRef = XPLMFindDataRef(XSB_ATC_CALLSIGN);
    if (XSBPluginId != XPLM_NO_PLUGIN_ID)
    {
        WeHaveXSB = true;
        XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SUBSCRIBE, (void *) XSB_TEXT);
        XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SUBSCRIBE, (void *) XSB_WEATHER);
        logMsg(logToAll,    PLUGIN_NAME " V" PLUGIN_VERSION " succesfully connected to XSquawkBox.");
        logMsg(logToSqkBox, "Type in \">helpme\" to learn about using Lua code in XSB.");
    }
    else
    {
        WeHaveXSB = false;
    }




    luaL_openlibs(FWLLua);

    RegisterCoreCFunctionsToLua(FWLLua);
    lua_pushnumber(FWLLua, ++LuaResetCount);
    lua_setglobal(FWLLua, "LUA_RUN");

    int     mouse_x, mouse_y;

    // if a script wants MOUSE or SCREEN during load, we must provide it
    XPLMGetMouseLocation(&mouse_x, &mouse_y);
    lua_pushnumber(FWLLua, mouse_x);
    lua_setglobal(FWLLua, "MOUSE_X");
    lua_pushnumber(FWLLua, mouse_y);
    lua_setglobal(FWLLua, "MOUSE_Y");
    lua_pushnumber(FWLLua, LAST_SCREEN_WIDTH);
    lua_setglobal(FWLLua, "SCREEN_WIDTH");
    lua_pushnumber(FWLLua, LAST_SCREEN_HIGHT);
    lua_setglobal(FWLLua, "SCREEN_HIGHT");

    // inform the script what system we are using
#if IBM
    lua_pushstring(FWLLua, "IBM");
#elif APL
    lua_pushstring(FWLLua, "APL");
#else
    lua_pushstring(FWLLua, "LIN");
#endif
    lua_setglobal(FWLLua, "SYSTEM");
#if defined(__LP64__) || defined(_WIN64)
    lua_pushnumber(FWLLua, 64);
#else
    lua_pushnumber(FWLLua, 32);
#endif
    lua_setglobal(FWLLua, "SYSTEM_ARCHITECTURE");
    switch (XPLMGetLanguage())
    {
    case 1:
        lua_pushstring(FWLLua, "English");
        break;
    case 2:
        lua_pushstring(FWLLua, "French");
        break;
    case 3:
        lua_pushstring(FWLLua, "German");
        break;
    case 4:
        lua_pushstring(FWLLua, "Italian");
        break;
    case 5:
        lua_pushstring(FWLLua, "Spanish");
        break;
    case 6:
        lua_pushstring(FWLLua, "Korean");
        break;
    case 7:
        lua_pushstring(FWLLua, "Russian");
        break;
    case 8:
        lua_pushstring(FWLLua, "Greek");
        break;
    case 9:
        lua_pushstring(FWLLua, "Japanese");
        break;
    case 10:
        lua_pushstring(FWLLua, "Chinese");
        break;
    default:
        lua_pushstring(FWLLua, "Unknown");
    }
    lua_setglobal(FWLLua, "XPLANE_LANGUAGE");
    int                         VersionXP, VersionSDK;
    XPLMHostApplicationID       HostID;
    XPLMGetVersions(&VersionXP, &VersionSDK, &HostID);
    lua_pushnumber(FWLLua, VersionXP);
    lua_setglobal(FWLLua, "XPLANE_VERSION");
    lua_pushnumber(FWLLua, VersionSDK);
    lua_setglobal(FWLLua, "SDK_VERSION");
    lua_pushnumber(FWLLua, HostID);
    lua_setglobal(FWLLua, "XPLANE_HOSTID");
    char dirSep[2];
    strcpy( dirSep, XPLMGetDirectorySeparator() ); // correct for each OS
    lua_pushstring(FWLLua, dirSep);
    lua_setglobal(FWLLua, "DIRECTORY_SEPARATOR");
    lua_pushstring(FWLLua, scriptDir.c_str());
    lua_setglobal(FWLLua, "SCRIPT_DIRECTORY");
    char AircraftFileName[256];
    char AircraftPath[512];
    XPLMGetNthAircraftModel(0, AircraftFileName, AircraftPath);
    lua_pushstring(FWLLua, AircraftFileName);
    lua_setglobal(FWLLua, "AIRCRAFT_FILENAME");
    lua_pushstring(FWLLua, AircraftPath);
    lua_setglobal(FWLLua, "AIRCRAFT_PATH");

    LuaIsRunning = true;
    lua_getglobal(FWLLua, "XPLANE_LANGUAGE");
    sprintf(success_cstring, "FlyWithLua Info: Lua engine (re)started. LUA_RUN = %i, SDK_VERSION = %i, XPLANE_VERSION = %i, XPLANE_LANGUAGE = %s and XPLANE_HOSTID = %i",
            LuaResetCount, VersionSDK, VersionXP, lua_tostring(FWLLua, -1), HostID);
    logMsg(logToDevCon, success_cstring);
    lua_settop(FWLLua, 0);
}

bool ReadScriptFile(char *FileNameToRead)
{
    if (LuaIsRunning == false)
    {
        logMsg(logToDevCon, "FlyWithLua: Lua is not running, can't load script file.");
        logMsg(logToDevCon, FileNameToRead);
        return false;
    }


    CopyDataRefsToLua();
    if (luaL_dofile(FWLLua, FileNameToRead))
    {
        logMsg(logToDevCon, string("FlyWithLua: Lua has crashed, can't execute script file: ").append(FileNameToRead));
        return false;
    }
    CopyDataRefsToXPlane();
    lua_gc(FWLLua, LUA_GCCOLLECT, 0);

    return true;
}

bool ReadAllScriptFiles(void)
{
    char            FileToLoad[SHORTSRTING] = "";
    char            PathAndName[NORMALSTRING] = "";
    char            PlaneICAO[SHORTSRTING] = "";
    char            PlaneTailNumber[SHORTSRTING] = "";
    int    k;
    char            FilesInFolder[5000];
#if XPLM210
    int             NumberOfFiles;
    int             TotalNumberOfFiles;
#else
    intptr_t        NumberOfFiles;
    intptr_t        TotalNumberOfFiles;
#endif
    char*           FileIndex[250];

    // starting the engine
    LuaIsRunning = true;
    CrashReportDisplayed = false;

    // reset macro and ATC menu
    XPLMClearAllMenuItems(MacroMenuId);
    XPLMClearAllMenuItems(ATCMenuId);

    // reset Lua engine
    ResetLuaEngine();

    // reset DataRefTable (and MacroTable and SwitchTable)
    EraseDataRefTable();

    // init the metar callback
    if (luaL_dostring(FWLLua, "XSB_METAR = \"Sorry, no METAR\"\nfunction XSB_METAR_CALLBACK()\nreturn\nend\n"))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Can't init the METAR callback.");
        LuaIsRunning = false;
    }

    // init some pre defined variables
    XPLMGetDatab(gPlaneICAO, PlaneICAO, 0, SHORTSRTING);
    lua_pushstring(FWLLua, PlaneICAO);
    lua_setglobal(FWLLua, "PLANE_ICAO");
    XPLMGetDatab(gPlaneTailNumber, PlaneTailNumber, 0, SHORTSRTING);
    lua_pushstring(FWLLua, PlaneTailNumber);
    lua_setglobal(FWLLua, "PLANE_TAILNUMBER");

    // if we are still in boot phase of X-Plane, we do not want to load files
    if (XPLMInitialized() == 0)
    {
        logMsg(logToDevCon, "FlyWithLua Info: X-Plane is still booting, we do not want to read files during startup.");
        CrashReportDisplayed = false;
        return true;
    }

    // run through the init script
    if (ReadScriptFile("Resources/plugins/FlyWithLua/Internals/FlyWithLua.ini"))
    {
        logMsg(logToDevCon, "FlyWithLua Info: Load ini file.");
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Unable to load ini file.");
        LuaIsRunning = false;
        return false;
    }

    if (XPLMGetDirectoryContents(scriptDir.c_str(), 0, FilesInFolder, sizeof(FilesInFolder), FileIndex, 250, &TotalNumberOfFiles, &NumberOfFiles))
    {
        logMsg(logToDevCon, "FlyWithLua Info: Searching for Lua script files");

        // are enough files in the folder to read them out?
        if (NumberOfFiles == 0)
        {
            // nothing to load, let's start Lua without reading files
            logMsg(logToAll, "FlyWithLua Info: The folder /Resources/plugins/FlyWithLua/Scripts/ does not exist or it is empty.");
            logMsg(logToAll, "FlyWithLua Info: Nothing to load at all, starting without reading files.");
            CrashReportDisplayed = false;
            return true;
        }

        logMsg(logToDevCon, "FlyWithLua Info: Sorting Lua script files");
        // sorting the files
        qsort(FileIndex, NumberOfFiles, sizeof(char *), compare_filenames);

        // reading out the files into a string step by step
        for (k = 0; k < NumberOfFiles; k++)
        {
            strcpy(FileToLoad, FileIndex[k]);
            if ((FileToLoad[0] != '.') and ((strstr(FileToLoad, ".fwl") != NULL and strlen(strstr(FileToLoad, ".fwl")) == 4)
                                            or (strstr(FileToLoad, ".FWL") != NULL and strlen(strstr(FileToLoad, ".FWL")) == 4)
#if defined(__LP64__) || defined(_WIN64)
                                            or (strstr(FileToLoad, ".lua64") != NULL and strlen(strstr(FileToLoad, ".lua64")) == 6)
                                            or (strstr(FileToLoad, ".Lua64") != NULL and strlen(strstr(FileToLoad, ".Lua64")) == 6)
                                            or (strstr(FileToLoad, ".LUA64") != NULL and strlen(strstr(FileToLoad, ".LUA64")) == 6)
#else
                                            or (strstr(FileToLoad, ".lua32") != NULL and strlen(strstr(FileToLoad, ".lua32")) == 6)
                                            or (strstr(FileToLoad, ".Lua32") != NULL and strlen(strstr(FileToLoad, ".Lua32")) == 6)
                                            or (strstr(FileToLoad, ".LUA32") != NULL and strlen(strstr(FileToLoad, ".LUA32")) == 6)
#endif
                                            or (strstr(FileToLoad, ".lua") != NULL and strlen(strstr(FileToLoad, ".lua")) == 4)
                                            or (strstr(FileToLoad, ".Lua") != NULL and strlen(strstr(FileToLoad, ".Lua")) == 4)
                                            or (strstr(FileToLoad, ".LUA") != NULL and strlen(strstr(FileToLoad, ".LUA")) == 4)))
            {
                // load the script file
                sprintf(PathAndName, "Resources/plugins/FlyWithLua/Scripts/%s", FileToLoad);
                logMsg(logToDevCon, std::string("FlyWithLua Info: Start loading script file ").append(PathAndName) );
                if (ReadScriptFile(PathAndName))
                {
                    logMsg(logToDevCon, std::string("FlyWithLua Info: Finished loading script file ").append(PathAndName) );
                }
                else
                {
                    logMsg(logToAll, std::string("FlyWithLua Error: Unable to load script file ").append(PathAndName) );
                    LuaIsRunning = false;
                    break;
                }

                // is Lua still running, or are there any problems?
                if (!LuaIsRunning)
                {
                    logMsg(logToAll, std::string("FlyWithLua Error: The error seems to be inside of script file ").append(PathAndName) );
                    return false;
                }
            }
        }
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Can't read out the subfolder Resources/plugins/FlyWithLua/Scripts.");
        logMsg(logToAll, "FlyWithLua Info: To fix this problem, simply create the subfolder Resources/plugins/FlyWithLua/Scripts.");
        LuaIsRunning = false;
        return false;
    }

    if (LuaIsRunning) logMsg(logToDevCon, "FlyWithLua Info: All script files loaded successfully.");

    return true; // snagar
}

void update_Lua_dataref_variables(XPLMDataRef DataRefID, int Index, float Value)
{
    if (!LuaIsRunning) return;
    if (DataRefTableLastElement < 0) return;
    for (int i=0; i<= DataRefTableLastElement; i++)
    {
        if ((DataRefTable[i].DataRefId == DataRefID) && (DataRefTable[i].Index == Index))
        {
            lua_pushnumber(FWLLua, Value);
            lua_setglobal(FWLLua, DataRefTable[i].LuaVariable);
        }
    }
    return;
}

void update_Lua_dataref_strings(XPLMDataRef DataRefID, int Index, char * ValueString)
{
    if (!LuaIsRunning) return;
    if (DataRefTableLastElement < 0) return;
    for (int i=0; i<= DataRefTableLastElement; i++)
    {
        if ((DataRefTable[i].DataRefId == DataRefID) && (DataRefTable[i].Index == Index))
        {
            lua_pushstring(FWLLua, ValueString);
            lua_setglobal(FWLLua, DataRefTable[i].LuaVariable);
        }
    }
    return;
}


PLUGIN_API int XPluginStart(
    char *        outName,
    char *        outSig,
    char *        outDesc)
{
    // int    i;


    // Plugin Info
    strcpy(outName, PLUGIN_NAME " " PLUGIN_VERSION);
    strcpy(outSig, "CarstenLynker." PLUGIN_NAME);
    strcpy(outDesc, PLUGIN_DESCRIPTION);

    initPluginDirectory(); // snagar

//#ifndef APL
    // init the HID access
    hid_init();
//#endif

    // DataRefs we want to use
    gXSBMetarStringXDataRef = XPLMFindDataRef(XSB_WEATHER_METAR);
    gJoystickAxisAssignments = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
    gJoystickButtonAssignments = XPLMFindDataRef("sim/joystick/joystick_button_assignments");
    gJoystickAxisReverse = XPLMFindDataRef("sim/joystick/joystick_axis_reverse");
    gJoystickButtonValues = XPLMFindDataRef("sim/joystick/joystick_button_values");
    gPlaneICAO = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
    gPlaneTailNumber = XPLMFindDataRef("sim/aircraft/view/acf_tailnum");

    // Lua run numbering starts at zero (no runs) when X-Plane starts
    LuaResetCount = 0;

    // init the menu item index
    FlyWithLuaMenuItem = -1;
    ATCMenuItem = -1;
    MacroMenuItem = -1;

    return 1;
}

PLUGIN_API void    XPluginStop(void)
{
//#ifndef APL
    // clean HID device connections and free memory
    hid_exit();
//#endif

}


PLUGIN_API void XPluginDisable(void)
{
    // run through the exit script
    if (ReadScriptFile("Resources/plugins/FlyWithLua/Internals/FlyWithLua.exit"))
    {
        logMsg(logToDevCon, "FlyWithLua Info: Load exit file.");
    }
    else
    {
        logMsg(logToAll, "FlyWithLua Error: Unable to load exit file.");
    }

    // close Lua
    LuaIsRunning = false;
    lua_close( FWLLua );
    lj_alloc_destroy(ud);

    // killing all commands
    if (CommandTableLastElement > -1)
    {
        for (int i=0; i<= CommandTableLastElement; i++)
        {
            XPLMUnregisterCommandHandler(CommandTable[i].Reference, FWLCommandCallback, 1, (void *) "FlyWithLuaCommand");
            CommandTable[i].Reference = NULL;
            CommandTable[i].Name.erase();
            CommandTable[i].Description.erase();
            CommandTable[i].BeginCommand.erase();
            CommandTable[i].ContinueCommand.erase();
            CommandTable[i].EndCommand.erase();
        }
    }
    CommandTableLastElement = -1;

    // clear Lua action string
    LuaDrawCommand.clear();
    EveryFrameCallbackCommand.clear();
    CallbackCommand.clear();
    LongTimeCallbackCommand.clear();
    NewMetarCommand.clear();
    KeyEventCommand.clear();
    LuaMouseClickCommand.clear();
    LuaMouseWheelCommand.clear();

    // clean up HID connections
    CloseAllOpenHIDDevices();
    hid_exit();

    // unsubscribe messages from/to XSquawkBox
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_UNSUBSCRIBE, (void *) XSB_TEXT);
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_UNSUBSCRIBE, (void *) XSB_WEATHER);

    // destroy the mouse event window
    XPLMDestroyWindow(FWLMouseEventWindowID);

    // remove the menus
    XPLMDestroyMenu(FlyWithLuaMenuId);
    XPLMDestroyMenu(ATCMenuId);
    XPLMDestroyMenu(MacroMenuId);
    FlyWithLuaMenuId = NULL;
    ATCMenuId = NULL;
    MacroMenuId = NULL;

    // unregister callbacks
    XPLMUnregisterDrawCallback(FWLDrawWindowCallback, xplm_Phase_Window, 0, (void *) "FWLWindowDrawer");
    XPLMUnregisterKeySniffer(FWLKeySniffer, 0, (void *) "FWLKeySniffer");
    XPLMUnregisterCommandHandler(MyReloadScriptsCommand, MyReloadScriptsCommandHandler, 0, 0);
    XPLMUnregisterFlightLoopCallback(MyFastLoopCallback, NULL);
    XPLMUnregisterFlightLoopCallback(MySlowLoopCallback, NULL);
    XPLMUnregisterFlightLoopCallback(MyEveryFrameLoopCallback, NULL);

    // write to Log.txt
    logMsg(logToDevCon, "FlyWithLua Info: FlyWithLua plugin disabled.");
}

PLUGIN_API int XPluginEnable(void)
{
    // init screen size
    XPLMGetScreenSize(&LAST_SCREEN_WIDTH, &LAST_SCREEN_HIGHT);

    // init buttons
    for (int i=0; i< MAXJOYSTICKBUTTONS; i++)
    {
        JoystickButtonValues[i] = 0;
        JoystickButtonLastValues[i] = 0;
    }

    // init CommandTable
    for (int i=0; i< MAXCOMMANDS; i++)
    {
        CommandTable[i].Reference = NULL;
        CommandTable[i].Name.erase();
        CommandTable[i].Description.erase();
        CommandTable[i].BeginCommand.erase();
        CommandTable[i].ContinueCommand.erase();
        CommandTable[i].EndCommand.erase();
    }

    // The commands
    MyReloadScriptsCommand = XPLMCreateCommand("FlyWithLua/debugging/reload_scripts", "Reload all Lua script files.");


    // Now tell X-Plane about our commands
    XPLMRegisterCommandHandler(MyReloadScriptsCommand,            // in Command name
                               MyReloadScriptsCommandHandler,     // in Handler
                               0,                                 // Receive input when plugin windows.
                               (void *) 0);                       // inRefcon.

    // Check in the main funktions
    XPLMRegisterFlightLoopCallback(
        MyFastLoopCallback,	    /* Callback */
        TimeBetweenCallbacks,	/* Interval */
        NULL);  	     	    /* refcon not used. */

    XPLMRegisterFlightLoopCallback(
        MySlowLoopCallback, 	/* Callback */
        TimeBetweenCallbacks,	/* Interval */
        NULL);  	        	/* refcon not used. */

    XPLMRegisterFlightLoopCallback(
        MyEveryFrameLoopCallback,  /* Callback */
        -1,                        /* every frame */
        NULL);                     /* refcon not used. */

    XPLMRegisterDrawCallback(FWLDrawWindowCallback, xplm_Phase_Window, 0, (void *) "FWLWindowDrawer");

    // create the plugin menu
    if (FlyWithLuaMenuItem < 0) FlyWithLuaMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "FlyWithLua", NULL, 1);
    FlyWithLuaMenuId = XPLMCreateMenu("FlyWithLua", XPLMFindPluginsMenu(), FlyWithLuaMenuItem, FlyWithLuaMenuHandler, NULL);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Reload all Lua script files", (void *)"Reload", 1);
    XPLMAppendMenuSeparator(FlyWithLuaMenuId);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Write Debug file", (void *)"Debug", 1);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Stop the Lua engine", (void *)"Stop", 1);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Try to resume Lua engine (not recommended)", (void *)"Resume", 1);

    // create the macro and ATC menus
    if (MacroMenuItem < 0) MacroMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "FlyWithLua Macros", NULL, 1);
    MacroMenuId = XPLMCreateMenu("FlyWithLua Macros", XPLMFindPluginsMenu(), MacroMenuItem, MacroMenuHandler, NULL);
    if (ATCMenuItem < 0) ATCMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "FlyWithLua ATC", NULL, 1);
    ATCMenuId = XPLMCreateMenu("FlyWithLua ATC", XPLMFindPluginsMenu(), ATCMenuItem, MacroMenuHandler, NULL);

    // init the XSB connection
    XSBPluginId = XPLMFindPluginBySignature("vatsim.protodev.clients.xsquawkbox");
    XSBInputUsrMsgXDataRef = XPLMFindDataRef(XSB_INPUT_USER_MSG);
    XSBInputStringXDataRef = XPLMFindDataRef(XSB_INPUT_STRING);
    XSBDestinationXDataRef = XPLMFindDataRef(XSB_FP_ARRIVAL_AIRPORT);
    XSBAlternativeXDataRef = XPLMFindDataRef(XSB_FP_ALTERNATE_AIRPORT);
    XSBStartAirportXDataRef = XPLMFindDataRef(XSB_FP_DEPARTURE_AIRPORT);
    XSBATCFreqXDataRef = XPLMFindDataRef(XSB_ATC_FREQ);
    XSBATCCallsignXDataRef = XPLMFindDataRef(XSB_ATC_CALLSIGN);
    if (XSBPluginId != XPLM_NO_PLUGIN_ID)
    {
        WeHaveXSB = true;
        XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SUBSCRIBE, (void *) XSB_TEXT);
        XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SUBSCRIBE, (void *) XSB_WEATHER);
        logMsg(logToAll,    PLUGIN_NAME " V" PLUGIN_VERSION " succesfully connected to XSquawkBox.");
        logMsg(logToSqkBox, "Type in \">helpme\" to learn about using Lua code in XSB.");
    }
    else
    {
        WeHaveXSB = false;
    }

    // Starting the Lua engine
    XPLMDataRef lua_alloc_ref = XPLMFindDataRef("sim/operation/prefs/misc/has_lua_alloc");
    if(lua_alloc_ref && XPLMGetDatai(lua_alloc_ref))
    {
        /* X-Plane has an allocator for us - we _must_ use it. */
        ud = lj_alloc_create();
        if (ud == NULL)
        {
            logMsg(logToDevCon, "FlyWithLua Error: LuaJIT Panic! No space to create the Lua instance!");
        }
        FWLLua = lua_newstate(lj_alloc_f, ud);
    }
    else
    {
        /* use the default allocator. */
        FWLLua = luaL_newstate();
    }

    luaL_openlibs(FWLLua);

    // reload all script files, if the plugin is re-enabled via X-Plane's plugin manager menu
    if (LuaResetCount > 0)
    {
        ReadAllScriptFiles();
    }

    //register the keysniffer
    XPLMRegisterKeySniffer(FWLKeySniffer, 0, (void *) "FWLKeySniffer");

    //register the mouse capture window
    XPLMCreateWindow_t MouseWindowData;
    MouseWindowData.structSize = sizeof(MouseWindowData);
    MouseWindowData.left = 0;
    MouseWindowData.bottom = 0;
    XPLMGetScreenSize(&MouseWindowData.right, &MouseWindowData.top);
    MouseWindowData.visible= 1;
    MouseWindowData.drawWindowFunc = FWLMouseEventWindowDraw;
    MouseWindowData.handleKeyFunc = FWLMouseEventWindowKey;
    MouseWindowData.handleMouseClickFunc = FWLMouseEventWindowMouse;
    MouseWindowData.handleMouseWheelFunc = FWLMouseEventWindowMouseWheel;
    MouseWindowData.handleCursorFunc = FWLMouseEventWindowCursor;
    MouseWindowData.refcon = NULL;
    FWLMouseEventWindowID = XPLMCreateWindowEx(&MouseWindowData);

    return 1;
}

#if XPLM210
PLUGIN_API void XPluginReceiveMessage(
    XPLMPluginID    inFromWho,
    intptr_t        inMessage,
    void *          inParam)
#else
PLUGIN_API void XPluginReceiveMessage(
    XPLMPluginID    inFromWho,
    intptr_t        inMessage,
    void *          inParam)
#endif
{


    char    XSBString[500];
    //char    ReadText[NORMALSTRING];
    //char    ReadText2[NORMALSTRING];
    //int     ReadInt;
    //float   ReadFloat;
    std::string  StringToChange;
    //bool    CanNotFindTheCommand;
    //int     i;

    /* switching the plane will force switching the airport too, we do not need to restart Lua twice
     if ((inMessage == XPLM_MSG_PLANE_LOADED) and ((int) inParam == 0))
     {
     logMsg(logToDevCon, "FlyWithLua: User switched to a new plane. Script files have to be reloaded.");
     ReadAllScriptFiles();
     }
     */
    if (inMessage == XPLM_MSG_AIRPORT_LOADED)
    {
        logMsg(logToDevCon, "FlyWithLua: User switched to a new airport (or changed the plane). Script files have to be reloaded.");
        ReadAllScriptFiles();
    }
    if (inMessage == XSB_MSG_METAR)
    {
        char    ReadMetar[NORMALSTRING];
        XPLMGetDatab(gXSBMetarStringXDataRef, ReadMetar, 0, NORMALSTRING);
        if (LuaIsRunning)
        {
            lua_pushstring(FWLLua, ReadMetar);
            lua_setglobal(FWLLua, "XSB_METAR");
        }
        NewMetarCountdown = 0;
    }
    if (inMessage == XSB_MSG_USER_COMMAND)
    {
        XSBString[XPLMGetDatab(XSBInputStringXDataRef, XSBString, 0, sizeof(XSBString)-1)] = 0;
        std::string  StringToChange(XSBString);
        if (StringToChange == ">helpme")
        {
            logMsg(logToSqkBox, "---------8<------------------------------------------------------------------------------");
            logMsg(logToSqkBox, "You can directly enter a Lua line of code, just by starting with \">\"");
            logMsg(logToSqkBox, "(a greater than char followed by the Lua code).");
            logMsg(logToSqkBox, "Example:");
            logMsg(logToSqkBox, ">DataRef( \"Strobe\", \"sim/cockpit/electrical/strobe_lights_on\", \"writable\" )");
            logMsg(logToSqkBox, ">Strobe = 1");
            logMsg(logToSqkBox, "-------------------------------------------------------------------->8-------------------");
            XPLMSetDatab(XSBInputStringXDataRef, (void *) "", 0, 0);
            return;
        }
        if (XSBString[0] == '>')
        {
            if ( !StringToChange.empty() )
            {
                RunLuaString( StringToChange.substr(1) );
                XPLMSetDatab(XSBInputStringXDataRef, (void *) "", 0, 0);
            }
            return;
        }
    }
}


// to be processed not so often
float	MySlowLoopCallback(
    float                inElapsedSinceLastCall,
    float                inElapsedTimeSinceLastFlightLoop,
    int                  inCounter,
    void *               inRefcon)
{
    if ((LongTimeCallbackCommand.length() > 1) && (LuaIsRunning == true))
    {
        RunLuaString(LongTimeCallbackCommand);
    }
    return LongTimeBetweenCallbacks;
}

// Now follows the fast part of it
float	MyFastLoopCallback(
    float                inElapsedSinceLastCall,
    float                inElapsedTimeSinceLastFlightLoop,
    int                  inCounter,
    void *               inRefcon)
{
    // stack overflow? pull the emergency brake!
    if ((lua_gettop(FWLLua) > 100) && (LuaIsRunning == true))
    {
        logMsg(logToAll, "FlyWithLua Debug Info: Stack overflow, more than 100 elements on stack.");
        LuaIsRunning = false;
    }
    if ((CallbackCommand.length() > 1) && (LuaIsRunning == true))
    {
        RunLuaString(CallbackCommand);
    }
    if ((LuaIsRunning == false) && (CrashReportDisplayed == false))
    {
        CrashReportDisplayed = true;
        int StackSize = lua_gettop(FWLLua);
        if (StackSize == 0)
        {
            logMsg(logToAll, "FlyWithLua Debug Info: Sorry, no debug Info on stack.");
        }
        else
        {
            logMsg(logToAll, "FlyWithLua Debug Info: The Lua stack contains the following elements:");
            for (int i=1; i<=StackSize; i++)
            {
                if (lua_isstring(FWLLua, i))
                {
                    logMsg(logToAll, lua_tostring(FWLLua, i) );
                }
            }
        }
        DebugLua();
        luaL_dostring(FWLLua, "FLYWITHLUA_DEBUG()");
    }
    return TimeBetweenCallbacks;
}

// switch some DataRefs
void ExecuteSwitches( void )
{
    float       FloatToSet;
    int         IntToSet;

    if (SwitchTableLastElement < 0) return;

    for (int i=0; i<=SwitchTableLastElement; i++)
    {
        if (SwitchTable[i].SwitchType == Switch)
        {
            if (JoystickButtonValues[SwitchTable[i].button] == 0)
            {
                if (SwitchTable[i].DataRefType == xplmType_Int) XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].off_int);
                if (SwitchTable[i].DataRefType == xplmType_Float) XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].off_float);
                if (SwitchTable[i].DataRefType == xplmType_Double) XPLMSetDatad(SwitchTable[i].DataRefID, SwitchTable[i].off_double);
                if (SwitchTable[i].DataRefType == xplmType_IntArray) XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].off_int, SwitchTable[i].index, 1);
                if (SwitchTable[i].DataRefType == xplmType_FloatArray) XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].off_float, SwitchTable[i].index, 1);
            }
            else
            {
                if (SwitchTable[i].DataRefType == xplmType_Int) XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].on_int);
                if (SwitchTable[i].DataRefType == xplmType_Float) XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].on_float);
                if (SwitchTable[i].DataRefType == xplmType_Double) XPLMSetDatad(SwitchTable[i].DataRefID, SwitchTable[i].on_double);
                if (SwitchTable[i].DataRefType == xplmType_IntArray) XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].on_int, SwitchTable[i].index, 1);
                if (SwitchTable[i].DataRefType == xplmType_FloatArray) XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].on_float, SwitchTable[i].index, 1);
            }
        }
        if ((SwitchTable[i].SwitchType == PositiveEdge) && (JoystickButtonValues[SwitchTable[i].button] == 1) && (JoystickButtonLastValues[SwitchTable[i].button] == 0))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int) XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].on_int);
            if (SwitchTable[i].DataRefType == xplmType_Float) XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].on_float);
            if (SwitchTable[i].DataRefType == xplmType_Double) XPLMSetDatad(SwitchTable[i].DataRefID, SwitchTable[i].on_double);
            if (SwitchTable[i].DataRefType == xplmType_IntArray) XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].on_int, SwitchTable[i].index, 1);
            if (SwitchTable[i].DataRefType == xplmType_FloatArray) XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].on_float, SwitchTable[i].index, 1);
        }
        if ((SwitchTable[i].SwitchType == NegativeEdge) && (JoystickButtonValues[SwitchTable[i].button] == 0) && (JoystickButtonLastValues[SwitchTable[i].button] == 1))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int) XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].on_int);
            if (SwitchTable[i].DataRefType == xplmType_Float) XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].on_float);
            if (SwitchTable[i].DataRefType == xplmType_Double) XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].on_double);
            if (SwitchTable[i].DataRefType == xplmType_IntArray) XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].on_int, SwitchTable[i].index, 1);
            if (SwitchTable[i].DataRefType == xplmType_FloatArray) XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].on_float, SwitchTable[i].index, 1);
        }
        if ((SwitchTable[i].SwitchType == PositiveIncrement) && (JoystickButtonValues[SwitchTable[i].button] == 1) && (JoystickButtonLastValues[SwitchTable[i].button] == 0))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
            {
                IntToSet = XPLMGetDatai(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_int;
                if (IntToSet > SwitchTable[i].upper_limit_int) IntToSet = SwitchTable[i].upper_limit_int;
                XPLMSetDatai(SwitchTable[i].DataRefID, IntToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_Float)
            {
                FloatToSet = XPLMGetDataf(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDataf(SwitchTable[i].DataRefID, FloatToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_Double)
            {
                FloatToSet = (float) XPLMGetDatad(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDatad(SwitchTable[i].DataRefID, (double) FloatToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_IntArray)
            {
                XPLMGetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
                IntToSet += SwitchTable[i].stepping_int;
                if (IntToSet > SwitchTable[i].upper_limit_int) IntToSet = SwitchTable[i].upper_limit_int;
                XPLMSetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
            }
            if (SwitchTable[i].DataRefType == xplmType_FloatArray)
            {
                XPLMGetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
                FloatToSet += SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
            }
        }
        if ((SwitchTable[i].SwitchType == NegativeIncrement) && (JoystickButtonValues[SwitchTable[i].button] == 0) && (JoystickButtonLastValues[SwitchTable[i].button] == 1))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
            {
                IntToSet = XPLMGetDatai(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_int;
                if (IntToSet > SwitchTable[i].upper_limit_int) IntToSet = SwitchTable[i].upper_limit_int;
                XPLMSetDatai(SwitchTable[i].DataRefID, IntToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_Float)
            {
                FloatToSet = XPLMGetDataf(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDataf(SwitchTable[i].DataRefID, FloatToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_Double)
            {
                FloatToSet = (float) XPLMGetDatad(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDatad(SwitchTable[i].DataRefID, (double) FloatToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_IntArray)
            {
                XPLMGetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
                IntToSet += SwitchTable[i].stepping_int;
                if (IntToSet > SwitchTable[i].upper_limit_int) IntToSet = SwitchTable[i].upper_limit_int;
                XPLMSetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
            }
            if (SwitchTable[i].DataRefType == xplmType_FloatArray)
            {
                XPLMGetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
                FloatToSet += SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
            }
        }
        if ((SwitchTable[i].SwitchType == PositiveDecrement) && (JoystickButtonValues[SwitchTable[i].button] == 1) && (JoystickButtonLastValues[SwitchTable[i].button] == 0))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
            {
                IntToSet = XPLMGetDatai(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_int;
                if (IntToSet < SwitchTable[i].lower_limit_int) IntToSet = SwitchTable[i].lower_limit_int;
                XPLMSetDatai(SwitchTable[i].DataRefID, IntToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_Float)
            {
                FloatToSet = XPLMGetDataf(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDataf(SwitchTable[i].DataRefID, FloatToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_Double)
            {
                FloatToSet = (float) XPLMGetDatad(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDatad(SwitchTable[i].DataRefID, (double) FloatToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_IntArray)
            {
                XPLMGetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
                IntToSet -= SwitchTable[i].stepping_int;
                if (IntToSet < SwitchTable[i].lower_limit_int) IntToSet = SwitchTable[i].lower_limit_int;
                XPLMSetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
            }
            if (SwitchTable[i].DataRefType == xplmType_FloatArray)
            {
                XPLMGetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
                FloatToSet -= SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
            }
        }
        if ((SwitchTable[i].SwitchType == NegativeDecrement) && (JoystickButtonValues[SwitchTable[i].button] == 0) && (JoystickButtonLastValues[SwitchTable[i].button] == 1))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
            {
                IntToSet = XPLMGetDatai(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_int;
                if (IntToSet < SwitchTable[i].lower_limit_int) IntToSet = SwitchTable[i].lower_limit_int;
                XPLMSetDatai(SwitchTable[i].DataRefID, IntToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_Float)
            {
                FloatToSet = XPLMGetDataf(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDataf(SwitchTable[i].DataRefID, FloatToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_Double)
            {
                FloatToSet = (float) XPLMGetDatad(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDatad(SwitchTable[i].DataRefID, (double) FloatToSet);
            }
            if (SwitchTable[i].DataRefType == xplmType_IntArray)
            {
                XPLMGetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
                IntToSet -= SwitchTable[i].stepping_int;
                if (IntToSet < SwitchTable[i].lower_limit_int) IntToSet = SwitchTable[i].lower_limit_int;
                XPLMSetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
            }
            if (SwitchTable[i].DataRefType == xplmType_FloatArray)
            {
                XPLMGetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
                FloatToSet -= SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0) FloatToSet = fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round;
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
            }
        }
    }
    return;
}

// Now follows the extreme fast part of it
float	MyEveryFrameLoopCallback(
    float                inElapsedSinceLastCall,
    float                inElapsedTimeSinceLastFlightLoop,
    int                  inCounter,
    void *               inRefcon)
{
    // catching the buttons (only relevant in this callback)
    XPLMGetDatavi(gJoystickButtonValues, JoystickButtonValues, 0, MAXJOYSTICKBUTTONS);

    if (LuaIsRunning == true) ExecuteSwitches();

    if ((EveryFrameCallbackCommand.length() > 1) && (LuaIsRunning == true))
    {
        RunLuaString(EveryFrameCallbackCommand);
    }

    // do the new metar command string?
    if ((NewMetarCountdown++ == 2) && (LuaIsRunning == true))
    {
        if (luaL_dostring(FWLLua, NewMetarCommand.c_str()))
        {
            logMsg(logToDevCon, "FlyWithLua Error: Can't execute command string. The string who failed is:");
            logMsg(logToDevCon, NewMetarCommand);
            LuaIsRunning = false;
        }
    }


    // store the joystick buttons for state analysis
    for (int i=0; i<MAXJOYSTICKBUTTONS; i++)
    {
        JoystickButtonLastValues[i] = JoystickButtonValues[i];
    }

    return -1;  // -1 will set callback to every frame
}


int    MyReloadScriptsCommandHandler(XPLMCommandRef       inCommand,
                                     XPLMCommandPhase     inPhase,
                                     void *               inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        ReadAllScriptFiles();
        XPLMSpeakString("All Lua Script files loaded.");
    }
    return 0; // we resume the command
}

// execute the macro by setting a mission level

void MacroMenuHandler(void * mRef, void * iRef)
{
    int MacroIndex = (std::size_t) iRef; // snagar
    if (MacroTable[MacroIndex].IsSwitch)
    {
        int     ActualState;
        XPLMCheckMenuItemState(MacroMenuId, MacroTable[MacroIndex].XPLM_Index, &ActualState);
        if (ActualState == xplm_Menu_Checked)
        {
            RunLuaString(MacroTable[MacroIndex].DeactivateCommand);
            XPLMCheckMenuItem(MacroMenuId, MacroTable[MacroIndex].XPLM_Index, xplm_Menu_Unchecked);
        }
        else
        {
            RunLuaString(MacroTable[MacroIndex].ActivateCommand);
            XPLMCheckMenuItem(MacroMenuId, MacroTable[MacroIndex].XPLM_Index, xplm_Menu_Checked);
        }
    }
    else
    {
        RunLuaString(MacroTable[MacroIndex].ActivateCommand);
    }
    return;
}

// Functions called by the menu will be send to command handler.
void FlyWithLuaMenuHandler(void * mRef, void * iRef)
{
    if (!strcmp((char *) iRef, "Reload"))
    {
        logMsg(logToDevCon, "FlyWithLua: User forced a script reload.");
        ReadAllScriptFiles();
        return;
    }
    if (!strcmp((char *) iRef, "Debug"))
    {
        DebugLua();
        luaL_dostring(FWLLua, "FLYWITHLUA_DEBUG()");
        return;
    }
    if (!strcmp((char *) iRef, "Resume"))
    {
        DebugLua();
        lua_settop(FWLLua, 0);
        LuaIsRunning = true;
        return;
    }
    if (!strcmp((char *) iRef, "Stop"))
    {
        logMsg(logToAll, "FlyWithLua: Lua Engine stopped by user.");
        LuaIsRunning = false;
        return;
    }
}


/*  Snagar -- for debug */
/*  extended with logType by Tedii */
void logMsg (ELogType logType, std::string message )
{
    std::string out = message + "\n";

    //if XSquawkBox not connected / not installed fall back to DevCon
    if(WeHaveXSB == false && (logType&logToSqkBox) != 0)
    {
        logType = logToDevCon;
    }

    //push message to DevCon
    if(logType == logToAll || (logType&logToDevCon) != 0)
    {
        XPLMDebugString(const_cast<char *> (out.c_str()));
    }

    //push message to Speech
    if(logType == logToAll || (logType&logToSqkBox) != 0)
    {
        XSBSpeakString(message);
    }

}

void initPluginDirectory ( )
{
    char path[1024]; // hopefully path will not be longer then 1024 characters


    XPLMGetSystemPath( path );
    logMsg(logToDevCon, std::string("FlyWithLua: Your system path is ").append(path) ); // debug


    char dirSep[2];
    strcpy( dirSep, XPLMGetDirectorySeparator() ); // correct for each OS

    pluginMainDir.clear();
    scriptDir.clear();

    pluginMainDir.append(path).append("Resources").append(dirSep).append("plugins").append(dirSep).append("FlyWithLua");
    scriptDir.append(pluginMainDir).append(dirSep).append("Scripts");

    logMsg (logToDevCon, std::string("FlyWithLua: Plugin Dir: ") + pluginMainDir ); // debug
    logMsg (logToDevCon, std::string("FlyWithLua: Plugin Scripts Dir: ") + scriptDir ); // debug
}



} // end namespace
