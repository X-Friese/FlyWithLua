// ----------------------------------
//  FlyWithLua Plugin for X-Plane 12
// ----------------------------------

#define PLUGIN_VERSION_NO "2.8.13"
#define PLUGIN_VERSION_BUILD __DATE__ " " __TIME__
#define PLUGIN_VERSION PLUGIN_VERSION_NO " build " PLUGIN_VERSION_BUILD

#define PLUGIN_NAME "FlyWithLua NG+"
#define PLUGIN_DESCRIPTION "Next Generation Plus Version " PLUGIN_VERSION

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
 *  v2.1.29 [linux/osx build] removed disabled HID code for OSX (again)
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
 *  v2.3.0  [added] OpenAL sound support!
 *  v2.3.3  [added] Support for Arcaze USB hardware.
 *  v2.4.2  [changed] More sound files can be loaded into memory
 *  v2.4.3  [added] sounds can be replaced in memory
 *  v2.4.4  [changed] new axis assignments in X-Plane 10.5x added to function set_axis_assignment()
 *  v2.6.0  [added] now we can create custom DataRefs
 *          [changed] from version 2.6.0 this plugin will only support X-Plane 11
 *  v2.6.2  [added] new compiler flag "CREATECOMPLETEEDITION" to get a separated version without restrictions
 *  v2.6.5  [changed] doubled the number of joystick buttons for X-Plane 11.10 and above
 *  v2.6.6  [added] new function do_on_new_XSB_text() to handle XSquawkBox incoming text message events
 *  v2.6.7  [added] support for XPLMCommandBegin and XPLMCommandEnd
 *
 *  v2.7.0  [added]   support for floating windows and VR and new name NG (Next Generation)
 *  v2.7.1  [solved]  issue with not being able to write to xlua datarefs.
 *  v2.7.2  [solved]  issue with not being able to write to xlua array datarefs.
 *          [added]   check for nil in SaveInitialAssignments.ini before writing to initial_assignments.txt
 *  v2.7.3  [added]   being able to set initial window position of non VR floating windows.
 *  v2.7.4  [solved]  issue when loading situation files in Scripts folder might cause CTD.
 *  v2.7.5  [added]   better error reporting.
 *  v2.7.6  [fixed]   package.path and package.cpath in FlyWithLua.ini.
 *          [changed] Replaced strcpy() with strncpy() in most places to avoid buffer overflows.
 *          [changed] Statically link LuaXML_lib into FlyWithLua plugin binary.
 *          [added]   The ability to get the xplm handle from floating window to be used with ffi thanks to IanQ.
 *          [changed] Statically link LuaSocket_lib into FlyWithLua plugin binary.
 *  v2.7.7  [added]   New floating window methods by IanQ.
 *  v2.7.8  [changed] Thanks to Jetbrains used CLion to help refactor FlyWithLua.cpp
 *          [added]   Now using sol2 library for high-level C++ to Lua bindings. Thanks nevkontakte.
 *  v2.7.9  [changed] Restore Lua 5.1 behavior for lua_tointeger() to mitigate issue #22.
 *  v2.7.10 [added]   Added a "Scripts (Quarantine)" folder to put bad scripts found on initial reading.
 *          [added]   When a bad script is found and moved it will speak this and put a information line at the top or the monitor.
 *          [added]   Added the version to the logging in the Log.txt file.
 *          [changed] Now supporting 400 sound files.
 *  v2.7.11 [added]   Added menu item to move scripts from "Scripts (Quarantine)" folder to  "Scripts" folder after you have fixed issues.
 *          [added]   After 20 seconds if there are scripts in the "Scripts (Quarantine)" folder it will say and display "Please check your quarantined scripts folder".
 *  v2.7.12 [changed] Improving error reporting in Log.txt file for items I can't yet put into quarantine.
 *  v2.7.13 [changed] Only speak once no mater how many bad scripts found.
 *          [removed] Unneeded window ID logging info.
 *          [added]   Throw execption after moving bad function script.
 *          [added]   Even if no bad scripts are found on startup look in the "Scripts (Quarantine)" folder and speak if some are found.
 *          [changed] Most of the logToAll to logToDevCon to prevent flooding of XSquawkBox window.
 *  v2.7.14 [changed] from logToAll to logToDevCon for XPLMSpeakString.
 *          [changed] If Devmode enabled do not speak or display messages about quarantine folder.
 *  v2.7.15 [changed] from building on LuaJit 2.0.1 to building on LuaJit 2.0.5.
 *  v2.7.16 [added]   using iniReader for a preference file
 *  v2.7.17 [fixed]   issue when Devmode is enabled still speaking some messages.
 *          [fixed]   issue when Devmode is enabled still moving some scripts to quarantine folder.
 *  v2.7.18 [changed] the size of Luadirectory_to_table to 1500 files or folders and 45000 characters.
 *  v2.7.19 [fixed]   issue with Arcaze not being found.
 *  v2.7.20 [changed] Remove the limit on the number of sounds.
 *  v2.7.21 [added]   Support for XPLMFindCommand.
 *          [removed] Removed support for building core and complete versions as we only build the NG version.
 *  v2.7.22 [added]   Verbose logging mode to keep the Log.txt cleaner unless needed for troubleshooting.
 *          [added]   #define MAXSWITCHES 400.
 *          [changed] #define MAXDATAREFS from 400 to 600.
 *  v2.7.23 [added]   support for Imgui 1.70
 *  v2.7.24 [added]   support for XPLMPlaceUserAtLocation
 *  v2.7.25 [added]   support for Imgui 1.76
 *  v2.7.26 [added]   missing support for two Imgui functions
 *  v2.7.27 [added]   support for Imgui 1.77 Increased number of supported functions.
 *  v2.7.28 [fixed]   missing configurations imconfig.h for #define IM_ASSERT.
 *  v2.7.29 [added]   global varable SCREEN_HEIGHT that has the same value as SCREEN_HIGHT.
 *  v2.7.30 [added]   global varables PLANE_AUTHOR and PLANE_DESCRIP Thanks to Steven L. Goldberg.
 *          [fixed]   bug if you used the reload pluggins function Thanks to Steven L. Goldberg.
 *  v2.7.31 [fixed]   Fix hid_send_filled_feature_report, adjust length check Thanks to Daniel Peukert.
 *  v2.7.32 [fixed]   Fixed Debug output for sound elements so it is written directly to the debug file by Patrick Lang.
 *          [fixed]   Fix bug so we can now use SDK 303.
 *          [fixed]   Fix some typos in CMakeLists.txt.
 *          [added]   Now building with set(CMAKE_CXX_STANDARD 17)
 *          [added]   Better error detection of the Lua allocator.
 *          [added]   sound buffer to the debug output and load_wave function receives a sound pointer by Patrick Lang.
 *          [added]   OpenAL_Test.lua and OpenAL_Test2.lua to the  Scripts (disabled) folder.
 *  v2.7.33 [added]   Updated sol2 to version 3.2.1
 *          [added]   Better error checking for OpenAL source and buffer id issues on start.
 *          [added]   Support for LuaJIT 2.1.0 beta3
 *          [added]   Updated support for OpenAL v1.21.1
 *  v2.7.34 [added]   Support for Imgui 1.85
 *          [added]   Updated sol2 to version 3.2.2
 *  v2.7.35 [changed] lua_pushstring to lua_pushlstring to improve string dataref handling thanks melbo.
 *          [added]   fix for Datab datarefs
 *          [added]   set(xplmType_data) , perf enhancements ( else if )
 *          [added]   Updated SaveInitialAssignments to match XP 11.55r2
 *  v2.8.0  [added]   Support for X-Plane 12
 *          [added]   Support for x86_64 and arm64 for Mac
 *          [added]   Inital support for Fmod with the Radios and Interior bus
 *  v2.8.1  [added]   Updated SaveInitialAssignments to match XP 12.00
 *          [added]   Fmod support for Master bus
 *  v2.8.2  [added]   Fix set_axis_assignment to allow mixed case. Thanks Cedrik Lussier
 *  v2.8.3  [added]   Increased the number of Normal Imgui finctions supported for Imgui 1.85
 *  v2.8.4  [changed] Fixed all issues with OpenAL thanks to Camille Bachmann's help.
 *  v2.8.5  [changed] Made sure Fmod's memory foot print is removed when we reload scripts, disable or stop FlyWithLua.
 *  v2.8.6  [added]   Add Global PLUGIN_VERSION predefined variables PLUGIN_VERSION, PLUGIN_VERSION_NO, PLUGIN_VERSION_BUILD
 *                    Thanks osprey-12
 *  v2.8.7  [changed] Fixed issue with float_wnd_set_position not puting window in correct position.
 *                    Fixed issue to make sure float_wnd window is in correct position after leaving VR going into 2d.
 *  v2.8.8  [added]   Added support for the horizontal scrollbar in Imgui windows.
 *  v2.8.9  [changed] Now using the X-Plane Fmod SDK for the com1, interior, ui and the master buses.
 *  v2.8.10 [changed] Removed X-Plane LuaJIT alloc because it is not needed and was causing issues with Linux.
 *  v2.8.11 [changed] Fixed missing Throttle 9 from SaveInitalAssignments.ini Thanks XPJavelin from x-plane.org
 *  v2.8.12 [changed] Fixed issue with do_on_mouse_click now is only called once per mouse click Thanks apn from x-plane.org
 *	v2.8.13 [Added]   do_every_frame_after() callback to run in FlightLoop Phase 1 (after physics processing, thus eliminating 
 *	                  graphic jitter
 *  Markus (Teddii):
 *  v2.1.20 [changed] bug fixed in Luahid_open() and Luahid_open_path(), setting last HID device index back if no device was found
 *          [changed] extended logMsg() with logType=logToAll|logToDevCon|logToSqkBox. If XSquawkBox is not connected logMsg() will fall back to DevCon
 *          [changed] overworked all logMsg() and XSBSpeakString() calls - so there are no more doubled strings in the code
 *          [fixed]   fixed some copy/pasted logMessages in LuaAddMacro(), LuaLastButton(), LuaSetArray()
 *          [fixed]   fixed a bug in function LuaSpeakString() 
 *
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

#include "FlyWithLua.h"


#if IBM

#include <windows.h>
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
#include <cctype>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>
#include <time.h>
#include <wchar.h>
#include <vector>
#include <sol.hpp>
#include "XSBComDefs.h"
#include "FloatingWindows/FLWIntegration.h"

// include OpenGL
#if IBM

#include <GL/gl.h>
#include <GL/glut.h>

#else
#if LIN
#define TRUE 1
#define FALSE 0

// class chunk_header;

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
// #include <libudev.h>

#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <Carbon/Carbon.h>
#endif

#endif

// include OpenAL
#include "OpenAL/al.h"
#include "OpenAL/alc.h"


//get low level access to HID devices

// disable HID in OSX until be able to create binaries.
//#ifndef APL
#include <hidapi.h>
//#endif
// include the extern command provided by the LUA team
#include <lua.hpp>
using namespace flywithlua;   // Resolve namespace issues when compiling

/// This symbol comes from statically linked LuaXML_lib library.
extern "C" int luaopen_LuaXML_lib(lua_State* L);
/// These symbols come from statically linked LuaSocket libraries.
extern "C" int luaopen_mime_core(lua_State* L);
extern "C" int luaopen_socket_core(lua_State* L);
extern "C" int luaopen_socket_serial(lua_State* L);
extern "C" int luaopen_socket_unix(lua_State* L);

namespace flywithlua
{
// Maybe useful to be platform independent
#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif


#define NORMALSTRING 250
#define SHORTSRTING 80
#define LONGSTRING 1024
#define MAXDATAREFS 600 // sparker256 changed this from 400 to 600 as a stopgap. We need to convert this to a vector.
#define MAXSWITCHES 400 // We also need to convert this to a vector. sparker256 does not have the skillset to do this.
#define MAXMACROS 150
#define MAXCOMMANDS 250
#define MAXJOYSTICKBUTTONS 3200  // this value is set by the length of DataRef sim/joystick/joystick_button_values


// a flag to reload the scenery
bool WeNeedToReloadTheScenery = false;


//Code from Ben Supnik Regarding Luajit in 64bit build
struct lua_alloc_request_t
{
    void* ud;
    void* ptr;
    size_t osize;
    size_t nsize;
};

#define        ALLOC_OPEN        0x00A110C1
#define        ALLOC_REALLOC    0x00A110C2
#define        ALLOC_CLOSE        0x00A110C3

/* new in X-Plane 10.40 */
/* but totally unused in FlyWithLua at the moment */
#define        ALLOC_LOCK        0x00A110C4
#define        ALLOC_UNLOCK    0x00A110C5
#define        ALLOC_LOCK_RO    0x00A110C6

static void* lj_alloc_create()
{
    struct lua_alloc_request_t r = {};
    XPLMSendMessageToPlugin(XPLM_PLUGIN_XPLANE, ALLOC_OPEN, &r);
    return r.ud;
}

static void lj_alloc_destroy(void* msp)
{
    struct lua_alloc_request_t r = {};
    r.ud = msp;
    XPLMSendMessageToPlugin(XPLM_PLUGIN_XPLANE, ALLOC_CLOSE, &r);
}

static void* lj_alloc_f(void* msp, void* ptr, size_t osize, size_t nsize)
{
    struct lua_alloc_request_t r = {};
    r.ud    = msp;
    r.ptr   = ptr;
    r.osize = osize;
    r.nsize = nsize;
    XPLMSendMessageToPlugin(XPLM_PLUGIN_XPLANE, ALLOC_REALLOC, &r);
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
static int compare_filenames(const void* a, const void* b)
{
    return strcmp(*(const char**) a, *(const char**) b);
}

//#ifndef APL

#define MAXHIDDEVICES 127
void* HIDSloppyTable[MAXHIDDEVICES];
int LAST_SLOPPY_HID                                  = -1;

void CloseAllOpenHIDDevices()
{
    if (LAST_SLOPPY_HID >= 0)
    {
        for (int i = 0; i <= LAST_SLOPPY_HID; i++)
        {
            hid_close((hid_device*) HIDSloppyTable[i]);
        }
    }
    LAST_SLOPPY_HID = -1;
}
//#endif

struct  MonitorBoundsStructure
{
    int    MonitorIndex;
    int    inLeft;
    int    inTop;
    int    inRight;
    int    inBottom;
};

struct DataRefTableStructure
{
    char           DataRefName[NORMALSTRING];
    char           LuaVariable[NORMALSTRING];
    bool           IsReadOnly;
    XPLMDataRef    DataRefId;
    int            Index;
    XPLMDataTypeID DataRefTypeId;
};

static DataRefTableStructure DataRefTable[MAXDATAREFS];
static int                   DataRefTableLastElement = -1;

struct MacroTableStructure
{
    bool        IsSwitch{};
    std::string MacroName;
    std::string ActivateCommand;
    std::string DeactivateCommand;
    int         XPLM_Index{};
};

static MacroTableStructure MacroTable[MAXMACROS];
static int                 MacroTableLastElement     = -1;

enum SwitchTypes
{
    Switch,
    PositiveEdge,
    NegativeEdge,
    PositiveIncrement,
    NegativeIncrement,
    PositiveDecrement,
    NegativeDecrement,
    // ABCEncoder,
    PositiveFlip,
    NegativeFlip,
    AxisMedian
};

struct SwitchTableStructure
{
    SwitchTypes    SwitchType{};
    XPLMDataRef    DataRefID{};
    XPLMDataTypeID DataRefType{};
    std::string    DataRefName;
    int            button{};
    int            button2{};
    int            index{};
    int            on_int{};
    int            off_int{};
    float          on_float{};
    float          off_float{};
    double         on_double{};
    double         off_double{};
    float          upper_limit_float{};
    float          lower_limit_float{};
    float          stepping_float{};
    float          round{};
    int            upper_limit_int{};
    int            lower_limit_int{};
    int            stepping_int{};
};

static SwitchTableStructure SwitchTable[MAXSWITCHES];
static int                  SwitchTableLastElement   = -1;


bool       CrashReportDisplayed = false;
static int LuaResetCount        = 0;


bool UserWantsANewPlane         = false;
bool UserWantsToLoadASituation  = false;
bool UserWantsToReplaceAircraft = false;
bool UserWantsToMoveAircraft    = false;
char UserWantedFilename[LONGSTRING];
double UserWantedPosHdgSpd[5];

bool WeAreNotInDrawingState     = true;

/*
 * Since FlyWithLua can't handle datarefs with multiple types,
 * let's do a somewhat sane translation to single types using
 * a priority encoder.
 */
XPLMDataTypeID reduceTypes(XPLMDataTypeID types)
{
    if (types & xplmType_Double)
    {
        return xplmType_Double;

    } else if (types & xplmType_Float)
    {
        return xplmType_Float;

    } else if (types & xplmType_Int)
    {
        return xplmType_Int;

    } else if (types & xplmType_FloatArray)
    {
        return xplmType_FloatArray;

    } else if (types & xplmType_IntArray)
    {
        return xplmType_IntArray;

    } else
    {
        return types;
    }
}

XPLMDataTypeID GetDataRefTypeCompat(XPLMDataRef ref)
{
    XPLMDataTypeID types = XPLMGetDataRefTypes(ref);
    return reduceTypes(types);
}

void EraseDataRefTable()
{
    XPLMDataRef DoNothing = XPLMFindDataRef("sim/none/none");

    for (auto& dataref : DataRefTable)
    {
        dataref.IsReadOnly = true;
        dataref.DataRefId  = DoNothing;
        strcpy(dataref.DataRefName, "");
        dataref.DataRefTypeId = xplmType_Unknown;
        dataref.Index         = 0;
        strcpy(dataref.LuaVariable, "");
    }
    DataRefTableLastElement = -1;

    for (auto& macro : MacroTable)
    {
        macro.IsSwitch = false;
        macro.MacroName.clear();
        macro.ActivateCommand.clear();
        macro.DeactivateCommand.clear();
        macro.XPLM_Index = 0;
    }

    MacroTableLastElement = -1;

    for (auto& sw : SwitchTable)
    {
        sw.button    = 0;
        sw.DataRefID = DoNothing;
        sw.DataRefName.clear();
        sw.DataRefType = xplmType_Unknown;
        sw.off_int     = 0;
        sw.off_float   = 0;
        sw.off_double  = 0;
        sw.on_int      = 1;
        sw.on_float    = 1;
        sw.on_double   = 1;
    }

    SwitchTableLastElement = -1;
}

struct CommandTableStructure
{
    XPLMCommandRef Reference{};
    std::string    Name;
    std::string    Description;
    std::string    BeginCommand;
    std::string    ContinueCommand;
    std::string    EndCommand;
};

static CommandTableStructure CommandTable[MAXCOMMANDS];
static int                   CommandTableLastElement = -1;

struct OpenALSoundsStructure
{
    std::string filename;
    float       pitch{};
    float       gain{};
    bool        loop{};
    ALuint      buffer{};
    ALuint      source{};
};

static std::vector<OpenALSoundsStructure> OpenALSounds;

lua_State* FWLLua;
void     * ud;


// ----8<---- Some code from this example: http://www.xsquawkbox.net/xpsdk/mediawiki/OpenAL_Shared_Example ----

/**************************************************************************************************************
 * WAVE FILE LOADING
**************************************************************************************************************/

// You can just use alutCreateBufferFromFile to load a wave file, but there seems to be a lot of problems with
// alut not beign available, being deprecated, etc.  So...here's a stupid routine to load a wave file.  I have
// tested this only on x86 machines, so if you find a bug on PPC please let me know.

// Macros to swap endian-values.

#define SWAP_32(value)                 \
        (((((unsigned short)(value))<<8) & 0xFF00)   | \
         ((((unsigned short)(value))>>8) & 0x00FF))

#define SWAP_16(value)                     \
        (((((unsigned int)(value))<<24) & 0xFF000000)  | \
         ((((unsigned int)(value))<< 8) & 0x00FF0000)  | \
         ((((unsigned int)(value))>> 8) & 0x0000FF00)  | \
         ((((unsigned int)(value))>>24) & 0x000000FF))

// Wave files are RIFF files, which are "chunky" - each section has an ID and a length.  This lets us skip
// things we can't understand to find the parts we want.  This header is common to all RIFF chunks.
struct chunk_header
{
    int id;
    int size;
};

// WAVE file format info.  We pass this through to OpenAL so we can support mono/stereo, 8/16/bit, etc.
struct format_info
{
    short format;                // PCM = 1, not sure what other values are legal.
    short num_channels;
    int   sample_rate;
    int   byte_rate;
    short block_align;
    short bits_per_sample;
};

// This utility returns the start of data for a chunk given a range of bytes it might be within.  Pass 1 for
// swapped if the machine is not the same endian as the file.
static char* find_chunk(char* file_begin, const char* file_end, int desired_id, int swapped)
{
    while (file_begin < file_end)
    {
        auto * h = (chunk_header*) file_begin;
        if (h->id == desired_id && !swapped)
            return file_begin + sizeof(chunk_header);
        if (h->id == SWAP_32(desired_id) && swapped)
            return file_begin + sizeof(chunk_header);
        int chunk_size = swapped ? SWAP_32(h->size) : h->size;
        char* next = file_begin + chunk_size + sizeof(chunk_header);
        if (next > file_end || next <= file_begin)
            return nullptr;
        file_begin = next;
    }
    return nullptr;
}

// Given a chunk, find its end by going back to the header.
static char* chunk_end(char* chunk_start, int swapped)
{
    auto * h = (chunk_header * )(chunk_start - sizeof(chunk_header));
    return chunk_start + (swapped ? SWAP_32(h->size) : h->size);
}

#define FAIL(X) { XPLMDebugString(X); free(mem); return; }

#define RIFF_ID 0x46464952            // 'RIFF'
#define FMT_ID  0x20746D66            // 'FMT '
#define DATA_ID 0x61746164            // 'DATA'

int openal_source_id_not_found = 0;
int openal_buffer_id_not_found = 0;

void load_wave(const char* file_name, OpenALSoundsStructure* sound)
{
    // First: we open the file and copy it into a single large memory buffer for processing.

    FILE* fi = fopen(file_name, "rb");
    if (fi == nullptr)
    {
        XPLMDebugString("FlyWithLua Error: WAVE file load failed - could not open.\n");
        return;
    }
    fseek(fi, 0, SEEK_END);
    auto file_size = static_cast<int>(ftell(fi));
    fseek(fi, 0, SEEK_SET);
    auto * mem = (char*) malloc(static_cast<size_t>(file_size));
    if (mem == nullptr)
    {
        XPLMDebugString("FlyWithLua Error: WAVE file load failed - could not allocate memory.\n");
        fclose(fi);
        return;
    }
    if (fread(mem, 1, static_cast<size_t>(file_size), fi) != file_size)
    {
        XPLMDebugString("FlyWithLua Error: WAVE file load failed - could not read file.\n");
        free(mem);
        fclose(fi);
        return;
    }
    fclose(fi);
    char* mem_end = mem + file_size;

    // Second: find the RIFF chunk.  Note that by searching for RIFF both normal
    // and reversed, we can automatically determine the endian swap situation for
    // this file regardless of what machine we are on.

    int swapped = 0;
    char* riff = find_chunk(mem, mem_end, RIFF_ID, 0);
    if (riff == nullptr)
    {
        riff        = find_chunk(mem, mem_end, RIFF_ID, 1);
        if (riff)
            swapped = 1;
        else FAIL("FlyWithLua Error: Could not find RIFF chunk in wave file.\n")
    }

    // The wave chunk isn't really a chunk at all. :-(  It's just a "WAVE" tag
    // followed by more chunks.  This strikes me as totally inconsistent, but
    // anyway, confirm the WAVE ID and move on.

    if (riff[0] != 'W' ||
        riff[1] != 'A' ||
        riff[2] != 'V' ||
        riff[3] != 'E') FAIL("FlyWithLua Error: Could not find WAVE signature in wave file.\n")

    char* format = find_chunk(riff + 4, chunk_end(riff, swapped), FMT_ID, swapped);
    if (format == nullptr) FAIL("FlyWithLua Error: Could not find FMT  chunk in wave file.\n")

    // Find the format chunk, and swap the values if needed.  This gives us our real format.

    auto* fmt = (format_info*) format;
    if (swapped)
    {
        fmt->format          = SWAP_16(fmt->format);
        fmt->num_channels    = SWAP_16(fmt->num_channels);
        fmt->sample_rate     = SWAP_32(fmt->sample_rate);
        fmt->byte_rate       = SWAP_32(fmt->byte_rate);
        fmt->block_align     = SWAP_16(fmt->block_align);
        fmt->bits_per_sample = SWAP_16(fmt->bits_per_sample);
    }

    // Reject things we don't understand...expand this code to support weirder audio formats.

    if (fmt->format != 1) FAIL("FlyWithLua Error: Wave file is not PCM format data.\n")
    if (fmt->num_channels != 1 && fmt->num_channels != 2) FAIL("FlyWithLua Error: Must have mono or stereo sound.\n")
    if (fmt->bits_per_sample != 8 && fmt->bits_per_sample != 16) FAIL("FlyWithLua Error: Must have 8 or 16 bit sounds.\n")
    char* data = find_chunk(riff + 4, chunk_end(riff, swapped), DATA_ID, swapped);
    if (data == nullptr) FAIL("FlyWithLua Error: I could not find the DATA chunk.\n")

    int sample_size  = fmt->num_channels * fmt->bits_per_sample / 8;
    auto data_bytes   = static_cast<int>(chunk_end(data, swapped) - data);
    int data_samples = data_bytes / sample_size;

    // If the file is swapped and we have 16-bit audio, we need to endian-swap the audio too or we'll
    // get something that sounds just astoundingly bad!

    if (fmt->bits_per_sample == 16 && swapped)
    {
        auto * ptr = (short*) data;
        int words = data_samples * fmt->num_channels;
        while (words--)
        {
            *ptr = SWAP_16(*ptr);
            ++ptr;
        }
    }

    // Finally, the OpenAL crud.  Build a new OpenAL buffer and send the data to OpenAL, passing in
    // OpenAL format enums based on the format chunk.

    // We expect that the OpenALSounds array has already been expanded to accomodate the new sound.

    alGenBuffers(1, &(sound->buffer));

    if (sound->buffer == 0)
    {
        // Set this flag to true so we know to reload the scripts
        // Since we did not find the buffer id.
        openal_buffer_id_not_found = 1;
        free(mem);
        return;
    }

    if (sound->buffer == 0) FAIL("FlyWithLua Error: Could not generate buffer id.\n");

    alBufferData(sound->buffer, fmt->bits_per_sample == 16 ?
                     (fmt->num_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16):
                     (fmt->num_channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8),
                 data, data_bytes, fmt->sample_rate);
    free(mem);
}


/**************************************************************************************************************
 * SAMPLE OEPNAL PLUGIN:
 **************************************************************************************************************/

// we use our own table for this
// static ALuint			snd_src		=0;				// Sample source and buffer - this is one "sound" we play.
// static ALuint			snd_buffer	=0;
// static float			pitch		= 1.0f;			// Start with 1.0 pitch - no pitch shift.

static ALCdevice * my_device = nullptr;            // We make our own device and context to play sound through.
static ALCcontext* my_context = nullptr;


// This is a stupid logging error function...useful for debugging, but not good error checking.
#define CHECK_ERR() __CHECK_ERR(__FILE__,__LINE__)

static void __CHECK_ERR(const char* f, int l)
{
    auto e = static_cast<ALuint>(alGetError());
    if (e != AL_NO_ERROR)
        printf("ERROR: %d (%s:%d\n", e, f, l);
}

// Mac specific: this converts file paths from HFS (which we get from the SDK) to Unix (which the OS wants).
// See this for more info:
//
// http://www.xsquawkbox.net/xpsdk/mediawiki/FilePathsAndMacho

#if APL
static int ConvertPath(const char * inPath, char * outPath, int outPathMaxLen) {

    CFStringRef inStr = CFStringCreateWithCString(kCFAllocatorDefault, inPath ,kCFStringEncodingMacRoman);
    if (inStr == NULL)
        return -1;
    CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, inStr, kCFURLHFSPathStyle,0);
    CFStringRef outStr = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
    if (!CFStringGetCString(outStr, outPath, outPathMaxLen, kCFURLPOSIXPathStyle))
        return -1;
    CFRelease(outStr);
    CFRelease(url);
    CFRelease(inStr);
    return 0;
}
#endif

// Initialization code.
// Trying to improve how this is done
// William Good 01-07-2023


void init_openal_sound()
{

    CHECK_ERR();

    // We have to save the old context and restore it later, so that we don't interfere with X-Plane
    // and other plugins.

    ALCcontext* old_context = alcGetCurrentContext();
    my_device = alcOpenDevice(nullptr);
    if (nullptr == my_device)
    {
        XPLMDebugString("FlyWithLua Error: Could not open the default OpenAL device.\n");
        return;
    }
    my_context = alcCreateContext(my_device, nullptr);
    if(nullptr == my_context)
    {
        if(old_context)
        {
            alcMakeContextCurrent(old_context);
        }
        alcCloseDevice(my_device);
        my_device = nullptr;
        XPLMDebugString("FlyWithLua Error: Could not create a context");
        // return 0;
        return;
    }

    alcMakeContextCurrent(my_context);

    ALfloat	zero[3] = { 0 } ;

    OpenALSounds.emplace_back(); // Make space to store information about the sound.
    OpenALSoundsStructure* sound = &OpenALSounds.back();

    // Generate 1 source and load a buffer of audio.
    alGenSources(1, &(sound->source));

    // Create and load buffers
    // We expect that the OpenALSounds array has already been expanded to accomodate the new sound.
    alGenBuffers(1, &(sound->buffer));

    // Basic initializtion code to play a sound: specify the buffer the source is playing, as well as some
    // sound parameters. This doesn't play the sound - it's just one-time initialization.
    alSourcei(sound->source, AL_BUFFER, sound->buffer);
    alSourcef(sound->source, AL_PITCH, 1.0f);
    alSourcef(sound->source, AL_GAIN, 1.0f);
    alSourcei(sound->source, AL_LOOPING, 0);
    alSourcefv(sound->source, AL_POSITION, zero);
    alSourcefv(sound->source, AL_VELOCITY, zero);
    CHECK_ERR();

    if(old_context)
    {
        alcMakeContextCurrent(old_context);
    }

    // Load a sound file to initalize
    // ALuint loadedBuffer = loadWave("./Resources/sounds/alert/10ft.wav", 1);

    return;
}

// ----------------- End of code from example --------------->8------------------


// Modified by Snagar
std::string pluginMainDir;
std::string scriptDir;
// END modified by Snagar

// Added by Sparker
std::string systemDir;
std::string internalsDir;
std::string modulesDir;
std::string quarantineDir;
// END added by Sparker

void process_read_ini_file();

// The user will be able to handle the plugin with commands
XPLMCommandRef MyReloadScriptsCommand = nullptr;

int MyReloadScriptsCommandHandler(XPLMCommandRef inCommand,
                                  XPLMCommandPhase inPhase,
                                  void* inRefcon);

void initPluginDirectory(); // snagar

void ResetLuaEngine();

bool RunLuaString(const std::string& LuaCommandString);

bool ReadScriptFile(const char* FileNameToRead);
bool ReadQtScriptFile(const char* QtFileNameToRead);

bool RunLuaChunk(const char* ChunkName);

// new way to handle classic and modern DataRaf access
void update_Lua_dataref_variables(XPLMDataRef DataRefID, int Index, float Value);

void update_Lua_dataref_strings(XPLMDataRef DataRefID, int Index, char* ValueString);

// These things will run periodically
float MyEveryFrameLoopCallback(
        float inElapsedSinceLastCall,
        float inElapsedTimeSinceLastFlightLoop,
        int inCounter,
        void* inRefcon);

float MyFastLoopCallback(
        float inElapsedSinceLastCall,
        float inElapsedTimeSinceLastFlightLoop,
        int inCounter,
        void* inRefcon);

float MySlowLoopCallback(
        float inElapsedSinceLastCall,
        float inElapsedTimeSinceLastFlightLoop,
        int inCounter,
        void* inRefcon);


// let's give the last Metar to Lua
XPLMDataRef gXSBMetarStringXDataRef = nullptr;

// we need to examine a text message from XSB
XPLMDataRef gXSBTextMessageXDataRef = nullptr;
XPLMDataRef gXSBTextFromXDataRef    = nullptr;
XPLMDataRef gXSBTextFreqsXDataRef   = nullptr;
XPLMDataRef gXSBTextUseXDataRef     = nullptr;


// Some variables used global in this plugin
static std::string EveryFrameCallbackCommand;
static std::string CallbackCommand;
static std::string LongTimeCallbackCommand;
static std::string KeyEventCommand;
static std::string NewMetarCommand;
static std::string NewXSBTextCommand;

float TimeBetweenCallbacks     = 1.0;                // processed every second
float LongTimeBetweenCallbacks = 10.0;           // not so often processed
// to increase performance
bool  LuaIsRunning             = false;                       // Are we working with Lua?

int JoystickButtonValues[MAXJOYSTICKBUTTONS];
int JoystickButtonLastValues[MAXJOYSTICKBUTTONS];

// We need some DataRefs
XPLMDataRef gJoystickButtonAssignments;
XPLMDataRef gJoystickButtonValues;
XPLMDataRef gJoystickAxisAssignments;
XPLMDataRef gJoystickAxisReverse;
XPLMDataRef gJoystickAxisValues;
XPLMDataRef gPlaneICAO;
XPLMDataRef gPlaneTailNumber;
XPLMDataRef gPlaneDescrip;
XPLMDataRef gPlaneAuthor;


// We interact with XSquawkBox
XPLMPluginID XSBPluginId;
XPLMDataRef  XSBInputUsrMsgXDataRef;
XPLMDataRef  XSBInputStringXDataRef;
XPLMDataRef  XSBDestinationXDataRef;
XPLMDataRef  XSBAlternativeXDataRef;
XPLMDataRef  XSBStartAirportXDataRef;
XPLMDataRef  XSBATCFreqXDataRef;
XPLMDataRef  XSBATCCallsignXDataRef;
static bool  WeHaveXSB         = false;
static int   NewMetarCountdown = 0;

// DataRefs used by set_pilots_head()
static XPLMDataRef FWLPilotsHeadX       = XPLMFindDataRef("sim/graphics/view/pilots_head_x");
static XPLMDataRef FWLPilotsHeadY       = XPLMFindDataRef("sim/graphics/view/pilots_head_y");
static XPLMDataRef FWLPilotsHeadZ       = XPLMFindDataRef("sim/graphics/view/pilots_head_z");
static XPLMDataRef FWLPilotsHeadHeading = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
static XPLMDataRef FWLPilotsHeadPitch   = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
static XPLMDataRef FWLViewType          = XPLMFindDataRef("sim/graphics/view/view_type");


// Don't want to use buttons? Here is a nice little menu
void FlyWithLuaMenuHandler(void*, void*);

XPLMMenuID FlyWithLuaMenuId;
int        FlyWithLuaMenuItem;

XPLMMenuCheck DevMenuMode;
int DevModeCheckedPosition = 10;

XPLMMenuCheck VerboseMenuMode;
int VerboseModeCheckedPosition = 12;

int bad_script_count = 0;
int bad_scripts_found = 0;

int speak_second_warning = 0;
int first_pass = 0;


void send_delayed_quarantined_message();

int found_bad_function_script = 0;

int developer_mode = 0;
int verbose_logging_mode = 0;

clock_t speak_time;

static float DelayedQuarantinedMessage_Callback(float inElapsed1, float inElapsed2,
                                      int cntr, void *ref);

void MacroMenuHandler(void*, void*);

XPLMMenuID MacroMenuId;
XPLMMenuID ATCMenuId;
int        MacroMenuItem;
int        ATCMenuItem;

// to be able to draw text we need a container to carry Lua commands
std::string LuaDrawCommand;

// and we need a window to catch mouse events
XPLMWindowID FWLMouseEventWindowID;
std::string  LuaMouseClickCommand;
std::string  LuaMouseWheelCommand;

int LAST_SCREEN_WIDTH, LAST_SCREEN_HIGHT, LAST_SCREEN_HEIGHT;

void FWLMouseEventWindowDraw(XPLMWindowID /*inWindowID*/, void* /*inRefcon*/)
{

    // we have nothing to draw inside the mouse event window
}

void FWLMouseEventWindowKey(XPLMWindowID /*inWindowID*/, char /*inKey*/, XPLMKeyFlags /*inFlags*/, char /*vkey*/,
                            void* /*inRefcon*/, int /*losingFocus*/)
{

    // no keyboard handling to catch mouse events
}

int FWLMouseEventWindowRightMouse(XPLMWindowID /*inWindowID*/, int /*x*/, int /*y*/, XPLMMouseStatus /*isDown*/,
                             void* /*inRefcon*/)
{
    return 0;
}

int FWLMouseEventWindowMouse(XPLMWindowID /*inWindowID*/, int /*x*/, int /*y*/, XPLMMouseStatus isDown,
                             void* /*inRefcon*/)
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
    } else if (isDown == xplm_MouseDrag)
    {
        lua_pushstring(FWLLua, "drag");
    } else
    {
        lua_pushstring(FWLLua, "up");
    }
    lua_setglobal(FWLLua, "MOUSE_STATUS");

    // let Lua do it's work
    RunLuaChunk("DO_ON_MOUSE_CLICK_CHUNK");

    // should we resume the mouse click?
    lua_getglobal(FWLLua, "RESUME_MOUSE_CLICK");
    if (lua_toboolean(FWLLua, 1))
    {
        lua_pop(FWLLua, 1);
        return 1;
    } else
    {
        lua_pop(FWLLua, 1);
        return 0;
    }
}

int FWLMouseEventWindowMouseWheel(XPLMWindowID /*inWindowID*/,
                                  int /*x*/,
                                  int /*y*/,
                                  int wheel,
                                  int clicks,
                                  void* /*inRefcon*/)
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
    RunLuaChunk("DO_ON_MOUSE_WHEEL_CHUNK");

    // should we resume the mouse wheel?
    lua_getglobal(FWLLua, "RESUME_MOUSE_WHEEL");
    if (lua_toboolean(FWLLua, 1))
    {
        lua_pop(FWLLua, 1);
        return 1;
    } else
    {
        lua_pop(FWLLua, 1);
        return 0;
    }
}

XPLMCursorStatus FWLMouseEventWindowCursor(XPLMWindowID /*inWindowID*/,
                                           int /*x*/,
                                           int /*y*/,
                                           void* /*inRefcon*/)
{
    // noting to do with the cursor
    return xplm_CursorDefault;
}


int FWLCommandCallback(
        XPLMCommandRef inCommand,
        XPLMCommandPhase inPhase,
        void* /*inRefcon*/)
{
    int CommandNumber = -1;

    if (!LuaIsRunning) return 1;  // no Lua - no fun
    if (CommandTableLastElement == -1) return 1; // no Lua commands defined
    for (auto i = 0; i <= CommandTableLastElement; i++)
    {
        if (CommandTable[i].Reference == inCommand)
        {
            CommandNumber = i;
        }
    }
    if (CommandNumber == -1) // no Lua Command found
    {
        logMsg(logToDevCon, "FlyWithLua Error: Somebody has stolen a command. Ask your admin or call the police!");
        LuaIsRunning = false;
        return 1;
    }
    CopyDataRefsToXPlane();
    CopyDataRefsToLua();
    lua_pushstring(FWLLua, "continue");
    lua_setglobal(FWLLua, "COMMAND_PROCESSING");
    if (inPhase == xplm_CommandBegin)
    {
        if (!CommandTable[CommandNumber].BeginCommand.empty())
            luaL_dostring(FWLLua, CommandTable[CommandNumber].BeginCommand.c_str());
    }
    if (inPhase == xplm_CommandContinue)
    {
        if (!CommandTable[CommandNumber].ContinueCommand.empty())
            luaL_dostring(FWLLua, CommandTable[CommandNumber].ContinueCommand.c_str());
    }
    if (inPhase == xplm_CommandEnd)
    {
        if (!CommandTable[CommandNumber].EndCommand.empty())
            luaL_dostring(FWLLua, CommandTable[CommandNumber].EndCommand.c_str());
    }
    CopyDataRefsToXPlane();
    lua_getglobal(FWLLua, "COMMAND_PROCESSING");
    if (strcmp(luaL_checkstring(FWLLua, -1), "halt") == 0)
    {
        lua_pop(FWLLua, 1);
        return 0;
    } else
    {
        lua_pop(FWLLua, 1);
        return 1;
    }
}

int FWLDrawWindowCallback(XPLMDrawingPhase /*inPhase*/,
                          int /*inIsBefore*/,
                          void* /*inRefcon*/)
{
    // get time before execution
    clock_t time_start = clock();

    char buffer[LONGSTRING] = {"Lua stopped!"};

    // reload the scenery when triggered by flag and reactivate Lua system
    if (WeNeedToReloadTheScenery)
    {
        XPLMReloadScenery();
        WeNeedToReloadTheScenery = false;
    }

    // is Lua running?
    if (!LuaIsRunning)
    {

        float ColorWanted[] = {1.0, 0.0, 0.0};         // RGB Red
        XPLMGetScreenSize(&LAST_SCREEN_WIDTH, &LAST_SCREEN_HIGHT);
        XPLMDrawString(ColorWanted, LAST_SCREEN_WIDTH - 100, LAST_SCREEN_HIGHT - 10, buffer, nullptr,
                       xplmFont_Proportional);
        int StackSize    = lua_gettop(FWLLua);
        int NUMBER_OF_CR = 0;
        if (StackSize > 0)
        {
            for (auto i = 1; i <= StackSize; i++)
            {
                if (lua_isstring(FWLLua, i))
                {
                    // get the string from the stack
                    strncpy(buffer, lua_tostring(FWLLua, i), sizeof(buffer));

                    // set color and position to OpenGL
                    XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);
                    glColor4f(0.0, 1.0, 0.0, 1.0);
                    glRasterPos2f(20, LAST_SCREEN_HIGHT - 20 * i - 15 * NUMBER_OF_CR - 20);

                    // print each letter using a GLUT loop
                    for (char c : buffer)
                    {
                        if (c == '\n')
                        {
                            glRasterPos2f(20, LAST_SCREEN_HIGHT - 20 * i - 15 * ++NUMBER_OF_CR - 20);
                        } else
                        {
                            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
                        }
                    }
                }
            }
        }
        return 1;
    }

    // we do not want to update the DataRefs <-> variables when drawing a window, to not waste CPU time
    // but we inform the script about the mouse position
    int mouse_x, mouse_y;
    XPLMGetMouseLocation(&mouse_x, &mouse_y);
    XPLMGetScreenSize(&LAST_SCREEN_WIDTH, &LAST_SCREEN_HIGHT);
    // we will have to check the mouse position (is it still inside X-Plane's window?)
    if ((mouse_x >= 0) && (mouse_x <= LAST_SCREEN_WIDTH) && (mouse_y >= 0) && (mouse_y <= LAST_SCREEN_HIGHT))
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
    LAST_SCREEN_HEIGHT = LAST_SCREEN_HIGHT;
    lua_pushnumber(FWLLua, LAST_SCREEN_HEIGHT);
    lua_setglobal(FWLLua, "SCREEN_HEIGHT");

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
                         0); // enables writing back of depth information to the depth buffer, as in glDepthMask(GL_TRUE);

    WeAreNotInDrawingState = false;
    if (LuaDrawCommand.length())
    {
        // check if DataRef to variable transfer is done
        if (EveryFrameCallbackCommand.length())
        {
            // transfer DataRefs into variables
            CopyDataRefsToLua();
        }

        // execute the chunk
        lua_getglobal(FWLLua, "DO_EVERY_DRAW_CHUNK");
        if (lua_pcall(FWLLua, 0, LUA_MULTRET, 0))
        {
            logMsg(logToDevCon, "FlyWithLua Error: Can't execute window draw callback chunk.");
            LuaIsRunning = false;
        }
    }
    WeAreNotInDrawingState = true;

    // get time after execution
    clock_t time_end = clock();

    // write time difference and memory usage into global variables
    if (LuaIsRunning)
    {
        lua_pushnumber(FWLLua, (double) (time_end - time_start) / CLOCKS_PER_SEC);
        lua_setglobal(FWLLua, "DO_EVERY_DRAW_TIME_SEC");
    }

    return 1;
};

int FWLKeySniffer(
        char inChar,
        XPLMKeyFlags inFlags,
        char inVirtualKey,
        void* /*inRefcon*/)
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
    lua_pushboolean(FWLLua, (inFlags & xplm_OptionAltFlag));
    lua_setglobal(FWLLua, "OPTION_KEY");
    lua_pushboolean(FWLLua, (inFlags & xplm_ControlFlag));
    lua_setglobal(FWLLua, "CONTROL_KEY");
    if (inFlags & xplm_DownFlag)
    {
        lua_pushstring(FWLLua, "pressed");
    } else
    {
        lua_pushstring(FWLLua, "released");
    }
    lua_setglobal(FWLLua, "KEY_ACTION");

    // let Lua do it's work
    RunLuaChunk("DO_ON_KEYSTROKE_CHUNK");

    // should we resume the keystroke?
    lua_getglobal(FWLLua, "RESUME_KEY");
    if (lua_toboolean(FWLLua, 1))
    {
        lua_pop(FWLLua, 1);
        return 0;
    } else
    {
        lua_pop(FWLLua, 1);
        return 1;
    }
}

void XSBSpeakString(const std::string& StringToSpeak)
{
    std::size_t ptr = StringToSpeak.find('\n');
    if (WeHaveXSB)
    {
        if (ptr != std::string::npos)
        {
            XSBSpeakString(StringToSpeak.substr(0, ptr));
            XSBSpeakString(StringToSpeak.substr(ptr + 1));
            return;
        } else
        {
            XPLMSetDatab(XSBInputUsrMsgXDataRef, (void*) StringToSpeak.c_str(),
                         0, static_cast<int>(StringToSpeak.length()));
            XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_USER_MSG, nullptr);
        }
    }
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

void push_forced_ascii(lua_State* L, const wchar_t* s)
{
    std::size_t i;
    char        d[USB_STR_MAXLEN + 1];

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
        auto c  = static_cast<char>(wc & 0x7F);     /* zap all de funny chars */
        if (wc > 127 || (wc > 0 && wc < 32))
        {
            c = '?';
        }
        d[i] = c;
    }
    d[i]   = '\0';
    lua_pushstring(L, d);
}

/*********************************************
 ** All own functions to handle HID devices **
 *********************************************/

static int Luacreate_HID_table(lua_State* L)
{
    int number_of_HID_devices = 0;
    struct hid_device_info* devs, * cur_dev;

    devs    = hid_enumerate(0x0, 0x0);
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

static int Luahid_open(lua_State* L)
{
    // usage: device = hid_open( vendor_id, product_id )
    // returns: pointer to device or nil if no device found
    // parameter serial_number not supported in Lua (Lua has no wchar_t type)

    hid_device* device_pointer;
    lua_Integer id;

    if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: hid_open() need two numbers as arguments.");
        LuaIsRunning = false;
        lua_pushnil(L);
        return 1;
    }

    // check argument range
    id = luaL_checkinteger(L, 1);
    if (id < 0 || id > 0xFFFF)
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong argument range for hid_open().");
        LuaIsRunning = false;
        lua_pushnil(L);
        return 1;
    }
    auto vendor_id = (unsigned short) id;

    id = luaL_checkinteger(L, 2);
    if (id < 0 || id > 0xFFFF)
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong argument range for hid_open().");
        LuaIsRunning = false;
        lua_pushnil(L);
        return 1;
    }
    auto product_id = (unsigned short) id;

    // space enough in the sloppy table?
    if (++LAST_SLOPPY_HID >= MAXHIDDEVICES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: Max devices to be open at the same time reached.");
        LuaIsRunning = false;
        lua_pushnil(L);
        LAST_SLOPPY_HID--;
        return 1;
    }

    // open the device
    device_pointer = hid_open(vendor_id, product_id, nullptr);
    HIDSloppyTable[LAST_SLOPPY_HID] = device_pointer;

    // push pointer to lua
    if (device_pointer != nullptr)
    {
        lua_pushlightuserdata(L, device_pointer);
        return 1;
    } else
    {
        // no device found
        LAST_SLOPPY_HID--;
        lua_pushnil(L);
        return 1;
    }
}

static int Luahid_open_path(lua_State* L)
{
    // usage: hid_open_path( "path_string" )
    // returns: pointer to device or nil if no device found

    hid_device* device_pointer;

    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong argument for hid_open_path().");
        LuaIsRunning = false;
        lua_pushnil(L);
        return 1;
    }

    // space enough in the sloppy table?
    if (++LAST_SLOPPY_HID >= MAXHIDDEVICES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: Max devices to be open at the same time reached.");
        LuaIsRunning = false;
        lua_pushnil(L);
        LAST_SLOPPY_HID--;
        return 1;
    }

    // get path string and call the C function
    const char* device_path = lua_tostring(L, 1);
    device_pointer = hid_open_path(device_path);
    HIDSloppyTable[LAST_SLOPPY_HID] = device_pointer;

    // push pointer to lua
    if (device_pointer != nullptr)
    {
        lua_pushlightuserdata(L, device_pointer);
        return 1;
    } else
    {
        // no device found
        LAST_SLOPPY_HID--;
        lua_pushnil(L);
        return 1;
    }
}

static int Luahid_close(lua_State* L)
{
    if (lua_islightuserdata(L, 1))
    {
        hid_device* device_pointer;

        // close the device
        device_pointer = (hid_device*) lua_touserdata(L, 1);
        hid_close(device_pointer);

        // clean up sloppy table
        if (LAST_SLOPPY_HID < 0) return 0;
        int i = 0;
        while ((i <= LAST_SLOPPY_HID) && (HIDSloppyTable[i] != device_pointer)) i++;
        if (i > LAST_SLOPPY_HID) return 0; // no match found (user closed a closed or unknown device)
        for (int j = i; j < LAST_SLOPPY_HID; j++)
        {
            HIDSloppyTable[j] = HIDSloppyTable[j + 1];
        }
        LAST_SLOPPY_HID--;
        return 0;
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function hid_close().");
        LuaIsRunning = false;
        return 0;
    }
}

static int Luahid_write(lua_State* L)
{
    // usage: hid_write( device_pointer, int, int, int, ... )

    unsigned char BlockToWrite[USB_STR_MAXLEN];

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function hid_write().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    int noa = lua_gettop(L);  // number of arguments
    if (noa > USB_STR_MAXLEN)
    {
        logMsg(logToDevCon, "FlyWithLua Error: Too many arguments to function hid_write().");
        LuaIsRunning = false;
        return 0;
    }
    // collect values to write
    for (auto i      = 2; i <= noa; i++)
    {
        BlockToWrite[i - 2] = static_cast<unsigned char>(luaL_checknumber(L, i));
    }
    // write values to HID device
    int result = hid_write((hid_device*) lua_touserdata(L, 1), BlockToWrite, static_cast<size_t>(noa - 1));
    if (result == -1)
    {
        logMsg(logToDevCon, "FlyWithLua Error: hid_write() failed.");
        LuaIsRunning = false;
    }
    lua_pushnumber(L, result);
    return 1;
}

static int Luahid_read_timeout(lua_State* L)
{
    // usage: number_of_values_read, value, value, value, ... = hid_read_timeout( device_pointer, number_of_values, milliseconds )

    unsigned char BlockToRead[USB_STR_MAXLEN];

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function hid_read_timeout().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    auto nov = static_cast<int>(lua_tonumber(L, 2));  // number of values wanted
    if ((nov >= USB_STR_MAXLEN) || (nov < 1))
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: Sorry, hid_read_timeout() can not handle the number of values you want to get.");
        LuaIsRunning = false;
        return 0;
    }
    // get the values into the block
    int result = hid_read_timeout((hid_device*) lua_touserdata(L, 1), BlockToRead, static_cast<size_t>(nov),
                                  static_cast<int>(lua_tonumber(L, 3)));

    // push number of values reeded to Lua
    lua_pushnumber(L, result);

    // push values to Lua
    if (result < 1)
    {
        return 1;
    }
    for (auto i = 1; i <= result; i++)
    {
        lua_pushnumber(L, BlockToRead[i - 1]);
    }
    return result + 1;
}

static int Luahid_read(lua_State* L)
{
    // usage: number_of_values_read, value, value, value, ... = hid_read( device_pointer, number_of_values )

    unsigned char BlockToRead[USB_STR_MAXLEN];

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function hid_read().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    auto nov = static_cast<int>(lua_tonumber(L, 2));  // number of values wanted
    if ((nov >= USB_STR_MAXLEN) || (nov < 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Sorry, hid_read() can not handle the number of values you want to get.");
        LuaIsRunning = false;
        return 0;
    }
    // get the values into the block
    int result = hid_read((hid_device*) lua_touserdata(L, 1), BlockToRead, static_cast<size_t>(nov));

    // push number of values read to Lua
    lua_pushnumber(L, result);

    // push values to Lua
    if (result < 1)
    {
        return 1;
    }
    for (auto i = 1; i <= result; i++)
    {
        lua_pushnumber(L, BlockToRead[i - 1]);
    }
    return result + 1;
}

static int Luahid_set_nonblocking(lua_State* L)
{
    // usage: success = hid_set_nonblocking( device_pointer, nonblock )
    // nonblock can be value 1 or 0

    // check arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function hid_set_nonblocking().");
        LuaIsRunning = false;
        return 0;
    }
    auto blockornot = static_cast<int>(lua_tonumber(L, 2));
    if ((blockornot == 0) || (blockornot == 1))
    {
        auto success = hid_set_nonblocking((hid_device*) lua_touserdata(L, 1), blockornot);
        lua_pushnumber(L, success);
        return 1;
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: The second argument to function hid_set_nonblocking() must be 1 or 0.");
        LuaIsRunning = false;
        return 0;
    }
}

static int Luahid_send_feature_report(lua_State* L)
{
    // usage: number_of_values_written = hid_send_feature_report( device_pointer, int, int, int, ... )

    unsigned char BlockToWrite[USB_STR_MAXLEN];
    // cleanup the memory
    memset(BlockToWrite, 0, sizeof(BlockToWrite));

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function hid_send_feature_report().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    int noa = lua_gettop(L);  // number of arguments
    if (noa - 1 > USB_STR_MAXLEN)
    {
        logMsg(logToDevCon, "FlyWithLua Error: Too many arguments to function hid_send_feature_report().");
        LuaIsRunning = false;
        return 0;
    }
    // collect values to write
    for (auto i      = 2; i <= noa; i++)
    {
        BlockToWrite[i - 2] = static_cast<unsigned char>(luaL_checknumber(L, i));
    }
    // write values to HID device
    int result = hid_send_feature_report((hid_device*) lua_touserdata(L, 1), BlockToWrite,
                                              static_cast<size_t>(noa - 1));
    if (result == -1)
    {
        logMsg(logToDevCon, "FlyWithLua Error: hid_send_feature_report() failed.");
        LuaIsRunning = false;
    }
    lua_pushnumber(L, result);
    return 1;
}

static int Luahid_send_filled_feature_report(lua_State* L)
{
    // usage: number_of_values_written = hid_send_feature_report( device_pointer, int, int, int, ... )

    unsigned char BlockToWrite[USB_STR_MAXLEN];
    // cleanup the memory
    memset(BlockToWrite, 0, sizeof(BlockToWrite));

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function hid_send_feature_report().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    int noa = lua_gettop(L);  // number of arguments
    if ((noa - 2 > USB_STR_MAXLEN) || (luaL_checknumber(L, 3) > USB_STR_MAXLEN))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Too many arguments to function hid_send_feature_report().");
        LuaIsRunning = false;
        return 0;
    }

    // write report ID outside of the loop, as we need to skip nobts
    BlockToWrite[0] = static_cast<unsigned char>(luaL_checknumber(L, 2));

    // collect values to write
    for (auto i = 4; i <= noa; i++)
    {
        BlockToWrite[i - 3] = static_cast<unsigned char>(luaL_checknumber(L, i));
    }
    // write values to HID device
    int      result = hid_send_feature_report((hid_device*) lua_touserdata(L, 1), BlockToWrite,
                                              static_cast<size_t>(luaL_checknumber(L, 3)));
    if (result == -1)
    {
        logMsg(logToDevCon, "FlyWithLua Error: hid_send_feature_report() failed.");
        LuaIsRunning = false;
    }
    lua_pushnumber(L, result);
    return 1;
}

static int Luahid_get_feature_report(lua_State* L)
{
    // usage: number_of_values_read, value, value, value, ... = hid_get_feature_report( device_pointer, number_of_values )

    unsigned char BlockToRead[USB_STR_MAXLEN];
    // cleanup the memory
    memset(BlockToRead, 0, sizeof(BlockToRead));

    // check minimum arguments
    if (!lua_islightuserdata(L, 1) || !lua_isnumber(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function hid_get_feature_report().");
        LuaIsRunning = false;
        return 0;
    }
    // check max number of arguments
    auto nov = static_cast<int>(lua_tonumber(L, 2));  // number of values wanted
    if ((nov >= USB_STR_MAXLEN) || (nov < 1))
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: Sorry, hid_get_feature_report() can not handle the number of values you want to get.");
        LuaIsRunning = false;
        return 0;
    }
    // get the values into the block
    int result = hid_get_feature_report((hid_device*) lua_touserdata(L, 1), BlockToRead, static_cast<size_t>(nov + 1));

    // push number of values reeded to Lua
    lua_pushnumber(L, result);

    // push values to Lua
    if (result < 1)
    {
        return 1;
    }
    for (auto i = 1; i < result; i++)
    {
        lua_pushnumber(L, BlockToRead[i - 1]);
    }
    return result + 1;
}

/*******************************************
 ** End of HID functions provided for Lua **
 *******************************************/

//#endif // end of disable APL HID implementation

void PushDataRefToLuaVariable(char* VariableWantedCString,
                              XPLMDataRef DataRefIdWanted,
                              XPLMDataTypeID DataRefTypeIdWanted,
                              int IndexWanted)
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
        // %%%
        int DatabLen;
        char ValueOfDataRef[LONGSTRING];
        DatabLen = XPLMGetDatab(DataRefIdWanted, &ValueOfDataRef, IndexWanted, LONGSTRING);
        if ( DatabLen > 0 ) {
           DatabLen = strnlen(ValueOfDataRef,DatabLen);
        }
        lua_pushlstring(FWLLua, ValueOfDataRef, DatabLen);
        lua_setglobal(FWLLua, VariableWantedCString);
        return;
    }

    // we have a new mixed DataRef type "number" since X-Plane 11
    if (DataRefTypeIdWanted == (xplmType_Int | xplmType_Double | xplmType_Float))
    {
        auto ValueOfDataRef = XPLMGetDataf(DataRefIdWanted);
        lua_pushnumber(FWLLua, ValueOfDataRef);
        lua_setglobal(FWLLua, VariableWantedCString);
        return;
    }


    if (DataRefTypeIdWanted == (xplmType_FloatArray | xplmType_IntArray))
    {
        float ValueOfDataRef;
        XPLMGetDatavf(DataRefIdWanted, &ValueOfDataRef, IndexWanted, 1);
        lua_pushnumber(FWLLua, ValueOfDataRef);
        lua_setglobal(FWLLua, VariableWantedCString);
        return;
    }


    logMsg(logToDevCon,
           std::string("FlyWithLua Error: The type of the DataRef variable \"").append(VariableWantedCString).append(
                   "\" is unknown or impossible."));
    LuaIsRunning = false;
}

// define the core C functions to use them inside the Lua state

static int LuaPlaceUserAtAirport(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToSqkBox, "FlyWithLua Error: No airport ICAO code to replace the plane."); //fallback to DevCon
        LuaIsRunning = false;
        return 0;
    }
    strncpy(UserWantedFilename, lua_tostring(L, 1), sizeof(UserWantedFilename));
    logMsg(logToSqkBox,
           std::string("FlyWithLua Info: Placing user's aircraft at ").append(UserWantedFilename)); //fallback to DevCon
    UserWantsANewPlane         = false;
    UserWantsToLoadASituation  = false;
    UserWantsToReplaceAircraft = true;
    UserWantsToMoveAircraft    = false;
    LuaIsRunning               = false;
    return 0;
}

static int LuaPlaceUserAtLocation(lua_State* L)
{
    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_isnumber(L, 5)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMPlaceUserAtLocation.");
        LuaIsRunning = false;
        return 0;
    }

    UserWantedPosHdgSpd[0] = static_cast<double>(lua_tonumber(L, 1));
    UserWantedPosHdgSpd[1] = static_cast<double>(lua_tonumber(L, 2));
    UserWantedPosHdgSpd[2] = static_cast<float>(lua_tonumber(L, 3));
    UserWantedPosHdgSpd[3] = static_cast<float>(lua_tonumber(L, 4));
    UserWantedPosHdgSpd[4] = static_cast<float>(lua_tonumber(L, 5));

    UserWantsANewPlane         = false;
    UserWantsToLoadASituation  = false;
    UserWantsToReplaceAircraft = false;
    UserWantsToMoveAircraft    = true;
    LuaIsRunning               = false;
    return 0;
}

static int LuaLoadAircraft(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToSqkBox, "FlyWithLua Error: No aircraft filename to load."); //fallback to DevCon
        LuaIsRunning = false;
        return 0;
    }
    strncpy(UserWantedFilename, lua_tostring(L, 1), sizeof(UserWantedFilename));
    logMsg(logToSqkBox,
           std::string("FlyWithLua Info: Loading aircraft ").append(UserWantedFilename)); //fallback to DevCon
    UserWantsANewPlane         = true;
    UserWantsToLoadASituation  = false;
    UserWantsToReplaceAircraft = false;
    UserWantsToMoveAircraft    = false;
    LuaIsRunning               = false;
    return 0;
}

static int LuaLoadSituation(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToSqkBox, "FlyWithLua Error: No situation filename to load."); //fallback to DevCon
        LuaIsRunning = false;
        return 0;
    }
    strncpy(UserWantedFilename, lua_tostring(L, 1), sizeof(UserWantedFilename));
    logMsg(logToSqkBox,
           std::string("FlyWithLua Info: Loading situation ").append(UserWantedFilename)); //fallback to DevCon
    UserWantsANewPlane         = false;
    UserWantsToLoadASituation  = true;
    UserWantsToReplaceAircraft = false;
    UserWantsToMoveAircraft    = false;
    LuaIsRunning               = false;
    return 0;
}

static int LuaSaveSituation(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToSqkBox, "FlyWithLua Error: No situation filename to save to."); //fallback to DevCon
        LuaIsRunning = false;
        return 0;
    }
    strncpy(UserWantedFilename, lua_tostring(L, 1), sizeof(UserWantedFilename));
    logMsg(logToSqkBox,
           std::string("FlyWithLua Info: Saving situation ").append(UserWantedFilename)); //fallback to DevCon
    XPLMSaveDataFile(xplm_DataFile_Situation, UserWantedFilename);
    return 0;
}

static int LuaXSBSpeakString(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToSqkBox, "FlyWithLua Error: nothing to say."); //fallback to DevCon
        return 0;
    }
    std::string LuaWantsToSpeak = lua_tostring(L, 1);
    logMsg(logToSqkBox, LuaWantsToSpeak); //fallback to DevCon
    return 0;
}

static int LuaXSBShowFlightplan(lua_State* L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SHOW_FP, nullptr);
    return 0;
}

static int LuaXSBSendFlightplan(lua_State* L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SEND_FP, nullptr);
    return 0;
}

static int LuaXSBUserLogin(lua_State* L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_USER_LOGIN, nullptr);
    return 0;
}

static int LuaXSBConnect(lua_State* L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_CONNECT, nullptr);
    return 0;
}

static int LuaXSBDisconnect(lua_State* L)
{
    CopyDataRefsToXPlane();
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_DISCONNECT, nullptr);
    return 0;
}

static int LuaXSBLookupATC(lua_State* L)
{
    char ATC_wanted[NORMALSTRING];

    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong argument to XSBLookupATC, we need a string to search for.");
        return 0;
    }
    strncpy(ATC_wanted, lua_tostring(L, 1), sizeof(ATC_wanted));
    XPLMSetDatab(XSBATCCallsignXDataRef, ATC_wanted, 0, static_cast<int>(strlen(ATC_wanted)));
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_LOOKUP_ATC, nullptr);
    lua_pushnumber(L, XPLMGetDatai(XSBATCFreqXDataRef) / 10);
    return 1;
}

static int LualogMsg(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: nothing to log.");
        return 0;
    }
    std::string LuaWantsToSpeak = lua_tostring(L, 1);
    logMsg(logToDevCon, LuaWantsToSpeak);
    return 0;
}

static int LuaSpeakString(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: nothing to say."); //fallback to DevCon
        return 0;
    }
    std::string LuaWantsToSpeak = lua_tostring(L, 1);
    logMsg(logToDevCon, LuaWantsToSpeak); //fallback to DevCon
    XPLMSpeakString(LuaWantsToSpeak.c_str());
    return 0;
}

// this is a helper function to LuaDrawString() filling the RGB color array
static void fill_RGB_array(float* array_to_fill, float red, float green, float blue)
{
    array_to_fill[0] = red;
    array_to_fill[1] = green;
    array_to_fill[2] = blue;
}

static int LuaDrawString(lua_State* L)
{
    // arguments: int x_pos, int y_pos, string string_to_print, float red, float green, float blue

    // or         int x_pos, int y_pos, string string_to_print, string color_to_print
    //            (color_to_print can be "red", "green", "blue", "yellow", "cyan", "magenta", "grey", "black" or "white")

    // or         int x_pos, int y_pos, string string_to_print
    //            (uses default color = white)

    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: draw_string() cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments given to draw_string().");
        LuaIsRunning = false;
        return 0;
    }
    float ColorWanted[] = {1.0, 1.0, 1.0};         // RGB White

    if (lua_isstring(L, 4))
    {
        std::string ColorString = lua_tostring(L, 4);

        if (ColorString == "red") fill_RGB_array(ColorWanted, 1.0, 0.0, 0.0);
        else if (ColorString == "green") fill_RGB_array(ColorWanted, 0.0, 1.0, 0.0);
        else if (ColorString == "blue") fill_RGB_array(ColorWanted, 0.0, 0.0, 1.0);
        else if (ColorString == "yellow") fill_RGB_array(ColorWanted, 1.0, 1.0, 0.0);
        else if (ColorString == "cyan") fill_RGB_array(ColorWanted, 0.0, 1.0, 1.0);
        else if (ColorString == "magenta") fill_RGB_array(ColorWanted, 1.0, 0.0, 1.0);
        else if (ColorString == "grey") fill_RGB_array(ColorWanted, 0.5, 0.5, 0.5);
        else if (ColorString == "black") fill_RGB_array(ColorWanted, 0.0, 0.0, 0.0);
    }

    if (lua_isnumber(L, 4) && lua_isnumber(L, 5) && lua_isnumber(L, 6))
    {
        auto red   = static_cast<float>(lua_tonumber(L, 4));
        auto green = static_cast<float>(lua_tonumber(L, 5));
        auto blue  = static_cast<float>(lua_tonumber(L, 6));

        if (red < 0.0 || red > 1.0) red       = 1.0;
        if (green < 0.0 || green > 1.0) green = 1.0;
        if (blue < 0.0 || blue > 1.0) blue    = 1.0;

        fill_RGB_array(ColorWanted, red, green, blue);
    }

    auto x_pos = static_cast<int>(lua_tointeger(L, 1));
    auto y_pos = static_cast<int>(lua_tointeger(L, 2));
    char string_to_print[LONGSTRING];

    strncpy(string_to_print, lua_tostring(L, 3), sizeof(string_to_print));
    XPLMDrawString(ColorWanted, x_pos, y_pos, string_to_print, nullptr, xplmFont_Proportional);
    return 0;
}

static int LuaDrawStringHelv18(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: draw_string_Helvetica_18() cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments given to draw_string_Helvetica_18().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    auto x_pos = static_cast<int>(lua_tointeger(L, 1));
    auto y_pos = static_cast<int>(lua_tointeger(L, 2));

    // get string from Lua Script
    char string_to_print[LONGSTRING];
    strncpy(string_to_print, lua_tostring(L, 3), sizeof(string_to_print));

    // set position to OpenGL
    glRasterPos2f(x_pos, y_pos);

    // print each letter using a GLUT loop
    for (auto i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, string_to_print[i]);
    }

    return 0;
}

static int LuaDrawStringHelv12(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: draw_string_Helvetica_12() cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments given to draw_string_Helvetica_12().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    auto x_pos = static_cast<int>(lua_tointeger(L, 1));
    auto y_pos = static_cast<int>(lua_tointeger(L, 2));

    // get string from Lua Script
    char string_to_print[LONGSTRING];
    strncpy(string_to_print, lua_tostring(L, 3), sizeof(string_to_print));

    // set position to OpenGL
    glRasterPos2f(x_pos, y_pos);

    // print each letter using a GLUT loop
    for (auto i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string_to_print[i]);
    }

    return 0;
}

static int LuaDrawStringHelv10(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: draw_string_Helvetica_10() cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments given to draw_string_Helvetica_10().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    auto x_pos = static_cast<int>(lua_tointeger(L, 1));
    auto y_pos = static_cast<int>(lua_tointeger(L, 2));

    // get string from Lua Script
    char string_to_print[LONGSTRING];
    strncpy(string_to_print, lua_tostring(L, 3), sizeof(string_to_print));

    // set position to OpenGL
    glRasterPos2f(x_pos, y_pos);

    // print each letter using a GLUT loop
    for (auto i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, string_to_print[i]);
    }

    return 0;
}

static int LuaDrawStringTimes10(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: draw_string_Times_Roman_10() cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments given to draw_string_Times_Roman_10().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    auto x_pos = static_cast<int>(lua_tointeger(L, 1));
    auto y_pos = static_cast<int>(lua_tointeger(L, 2));

    // get string from Lua Script
    char string_to_print[LONGSTRING];
    strncpy(string_to_print, lua_tostring(L, 3), sizeof(string_to_print));

    // set position to OpenGL
    glRasterPos2f(x_pos, y_pos);

    // print each letter using a GLUT loop
    for (auto i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, string_to_print[i]);
    }

    return 0;
}

static int LuaDrawStringTimes24(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: draw_string_Times_Roman_24() cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    // arguments: int x_pos, int y_pos, string string_to_print

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isstring(L, 3)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments given to draw_string_Times_Roman_24().");
        LuaIsRunning = false;
        return 0;
    }

    // get position from Lua script
    auto x_pos = static_cast<int>(lua_tointeger(L, 1));
    auto y_pos = static_cast<int>(lua_tointeger(L, 2));

    // get string from Lua Script
    char string_to_print[LONGSTRING];
    strncpy(string_to_print, lua_tostring(L, 3), sizeof(string_to_print));

    // set position to OpenGL
    glRasterPos2f(x_pos, y_pos);

    // print each letter using a GLUT loop
    for (auto i = 0; string_to_print[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string_to_print[i]);
    }

    return 0;
}

static int LuaMeasureString(lua_State* L)
{
    // arguments:  string: string to measure
    // output:     float:  length of string in screen pixel
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments given to measure_string().");
        LuaIsRunning = false;
        return 0;
    }
    char string_to_measure[LONGSTRING];
    strncpy(string_to_measure, lua_tostring(L, 1), sizeof(string_to_measure));
    if (lua_isstring(L, 2))
    {
        int  result = 0;
        char font_to_measure[LONGSTRING];
        strncpy(font_to_measure, lua_tostring(L, 2), sizeof(font_to_measure));
        if (strcmp(font_to_measure, "Helvetica_10") == 0)
        {
            for (auto i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_HELVETICA_10, string_to_measure[i]);
            }

        } else if (strcmp(font_to_measure, "Helvetica_12") == 0)
        {
            for (auto i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, string_to_measure[i]);
            }

        } else if (strcmp(font_to_measure, "Helvetica_18") == 0)
        {
            for (auto i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, string_to_measure[i]);
            }

        } else if (strcmp(font_to_measure, "Times_Roman_10") == 0)
        {
            for (auto i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_10, string_to_measure[i]);
            }

        } else if (strcmp(font_to_measure, "Times_Roman_24") == 0)
        {
            for (auto i = 0; string_to_measure[i] != '\0'; i++)
            {
                result += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, string_to_measure[i]);
            }
        }
        lua_pushnumber(L, result);
    } else
    {
        auto result = XPLMMeasureString(xplmFont_Proportional, string_to_measure,
                                        static_cast<int>(strlen(string_to_measure)));
        lua_pushnumber(L, result);
    }
    return 1;
}

static int LuaCrashTheSim(lua_State* L)
{
    XPLMSpeakString(
            "Oh, no! Please replace the user. This one is too stupid! Do you really think we crash the simulator? Killing your joysticks instead.");
    int EveryThingIsZero[1600] = {0};
    XPLMSetDatavi(gJoystickButtonAssignments, EveryThingIsZero, 0, 1600);
    XPLMSetDatavi(gJoystickAxisAssignments, EveryThingIsZero, 0, 100);
    return 0;
}

static int LuaCommandOnce(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: nothing to do. You will have to give a string for CommandOnce.");
        return 0;
    }
    char LuaWantsToDo[NORMALSTRING];
    strncpy(LuaWantsToDo, lua_tostring(L, 1), sizeof(LuaWantsToDo));
    XPLMCommandRef CommandId = XPLMFindCommand(LuaWantsToDo);
    if (CommandId == nullptr)
    {
        logMsg(logToDevCon, std::string("FlyWithLua Error: nothing to do. The command \"").append(LuaWantsToDo).append(
                "\" is unknown."));
        return 0;
    }
    XPLMCommandOnce(CommandId);
    return 0;
}

static int LuaCommandBegin(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: nothing to do. You will have to give a string for CommandBegin.");
        return 0;
    }
    char LuaWantsToDo[NORMALSTRING];
    strncpy(LuaWantsToDo, lua_tostring(L, 1), sizeof(LuaWantsToDo));
    XPLMCommandRef CommandId = XPLMFindCommand(LuaWantsToDo);
    if (CommandId == nullptr)
    {
        logMsg(logToDevCon, std::string("FlyWithLua Error: nothing to do. The command \"").append(LuaWantsToDo).append(
                "\" is unknown."));
        return 0;
    }
    XPLMCommandBegin(CommandId);
    return 0;
}

static int LuaCommandEnd(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: nothing to do. You will have to give a string for CommandEnd.");
        return 0;
    }
    char LuaWantsToDo[NORMALSTRING];
    strncpy(LuaWantsToDo, lua_tostring(L, 1), sizeof(LuaWantsToDo));
    XPLMCommandRef CommandId = XPLMFindCommand(LuaWantsToDo);
    if (CommandId == nullptr)
    {
        logMsg(logToDevCon, std::string("FlyWithLua Error: nothing to do. The command \"").append(LuaWantsToDo).append(
                "\" is unknown."));
        return 0;
    }
    XPLMCommandEnd(CommandId);
    return 0;
}

static int LuaAddMacro(lua_State* L)
{
    if (++MacroTableLastElement >= MAXMACROS)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want too much macro menu entries.");
        MacroTableLastElement--;
        return 0;
    }
    if (lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3))
    {
        std::string MacroName = lua_tostring(L, 1);
        MacroTable[MacroTableLastElement].MacroName = MacroName;
        MacroTable[MacroTableLastElement].ActivateCommand.append(lua_tostring(L, 2));
        MacroTable[MacroTableLastElement].DeactivateCommand.append(lua_tostring(L, 3));
        MacroTable[MacroTableLastElement].XPLM_Index = XPLMAppendMenuItem(MacroMenuId, MacroName.c_str(),
                                                                          (void*) MacroTableLastElement, 0);
        MacroTable[MacroTableLastElement].IsSwitch   = true;
        std::string StartActive;
        if (lua_isstring(L, 4))
        {
            StartActive = lua_tostring(L, 4);
        }
        if (StartActive == "activate")
        {
            XPLMCheckMenuItem(MacroMenuId, MacroTable[MacroTableLastElement].XPLM_Index, xplm_Menu_Checked);
            RunLuaString(MacroTable[MacroTableLastElement].ActivateCommand);
        } else
        {
            XPLMCheckMenuItem(MacroMenuId, MacroTable[MacroTableLastElement].XPLM_Index, xplm_Menu_Unchecked);
            RunLuaString(MacroTable[MacroTableLastElement].DeactivateCommand);
        }
        MacroTable[MacroTableLastElement].IsSwitch = true;
        return 0;
    }
    if (!(lua_isstring(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong parameters to AddMacro function.");
        return 0;
    }
    std::string MacroName = lua_tostring(L, 1);
    MacroTable[MacroTableLastElement].MacroName = MacroName;
    MacroTable[MacroTableLastElement].ActivateCommand.append(lua_tostring(L, 2));
    MacroTable[MacroTableLastElement].XPLM_Index = XPLMAppendMenuItem(MacroMenuId, MacroName.c_str(),
                                                                      (void*) MacroTableLastElement, 0);
    MacroTable[MacroTableLastElement].IsSwitch   = false;
    return 0;
}

static int LuaAddATCMacro(lua_State* L)
{
    if (++MacroTableLastElement >= MAXMACROS)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want too much ATC macro menu entries.");
        MacroTableLastElement--;
        return 0;
    }
    // ATC Menu Entries are never checkable
    if (!(lua_isstring(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong parameters to AddATCMacro function.");
        return 0;
    }
    std::string MacroName = lua_tostring(L, 1);
    MacroTable[MacroTableLastElement].MacroName = MacroName;
    MacroTable[MacroTableLastElement].ActivateCommand.append(lua_tostring(L, 2));
    MacroTable[MacroTableLastElement].XPLM_Index = XPLMAppendMenuItem(ATCMenuId, MacroName.c_str(),
                                                                      (void*) MacroTableLastElement, 1);
    MacroTable[MacroTableLastElement].IsSwitch   = false;
    return 0;
}

static int LuaActivateMacro(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Your argument is not a string.");
        return 0;
    }
    if (MacroTableLastElement < 0) return 0;
    std::string search_string = lua_tostring(L, 1);
    for (auto i = 0; i <= MacroTableLastElement; i++)
    {
        if ((MacroTable[i].IsSwitch) && (MacroTable[i].MacroName == search_string))
        {
            XPLMCheckMenuItem(MacroMenuId, MacroTable[i].XPLM_Index, xplm_Menu_Checked);
            RunLuaString(MacroTable[i].ActivateCommand);
        }
    }
    return 0;
}

static int LuaDeactivateMacro(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Your argument is not a string.");
        return 0;
    }
    if (MacroTableLastElement < 0) return 0;
    std::string search_string = lua_tostring(L, 1);
    for (auto i = 0; i <= MacroTableLastElement; i++)
    {
        if ((MacroTable[i].IsSwitch) && (MacroTable[i].MacroName == search_string))
        {
            XPLMCheckMenuItem(MacroMenuId, MacroTable[i].XPLM_Index, xplm_Menu_Unchecked);
            RunLuaString(MacroTable[i].DeactivateCommand);
        }
    }
    return 0;
}

bool StoreLuaChunk(const std::string& LuaCommandString, const char* ChunkName)
{
    if (!LuaIsRunning) return false;
    if (luaL_loadstring(FWLLua, LuaCommandString.c_str()))
    {
        logMsg(logToDevCon, std::string(
                "FlyWithLua Error: Can't store command string into a Lua chunk. The string should be stored into: ").append(
                ChunkName));
        logMsg(logToDevCon, LuaCommandString);
        LuaIsRunning = false;
        return false;
    }
    lua_setglobal(FWLLua, ChunkName);
    return true;
}

static int LuaDoEveryKeystroke(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong thing to do? Your DoEveryKeystroke command is not a string.");
        return 0;
    }
    std::string LuaShouldDoCommand = lua_tostring(L, 1);
    KeyEventCommand.append(LuaShouldDoCommand).append("\n");
    StoreLuaChunk(KeyEventCommand, "DO_ON_KEYSTROKE_CHUNK");
    return 0;
}

static int LuaDoEveryMouseClick(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong thing to do? Your DoEveryMouseClick command is not a string.");
        return 0;
    }
    std::string LuaShouldDoCommand = lua_tostring(L, 1);
    LuaMouseClickCommand.append(LuaShouldDoCommand).append("\n");
    StoreLuaChunk(LuaMouseClickCommand, "DO_ON_MOUSE_CLICK_CHUNK");
    return 0;
}

static int LuaDoEveryMouseWheel(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong thing to do? Your DoEveryMouseClick command is not a string.");
        return 0;
    }
    std::string LuaShouldDoCommand = lua_tostring(L, 1);
    LuaMouseWheelCommand.append(LuaShouldDoCommand).append("\n");
    StoreLuaChunk(LuaMouseWheelCommand, "DO_ON_MOUSE_WHEEL_CHUNK");
    return 0;
}

static int LuaDoEveryDrawCallback(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong thing to do? Your DoEveryDrawCallback command is not a string.");
        return 0;
    }
    std::string LuaShouldDoCommand = lua_tostring(L, 1);
    LuaDrawCommand.append(LuaShouldDoCommand).append("\n");
    StoreLuaChunk(LuaDrawCommand, "DO_EVERY_DRAW_CHUNK");
    return 0;
}

static int LuaDoEveryMETARCallback(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong thing to do? Your DoEveryMETARCallback command is not a string.");
        return 0;
    }
    std::string LuaShouldDoCommand = lua_tostring(L, 1);
    NewMetarCommand.append(LuaShouldDoCommand).append("\n");
    StoreLuaChunk(NewMetarCommand, "DO_ON_NEW_METAR_CHUNK");
    return 0;
}

static int LuaDoOnNewXSBTextCallback(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong thing to do? Your DoOnNewXSBTextCallback command is not a string.");
        return 0;
    }
    std::string LuaShouldDoCommand = lua_tostring(L, 1);
    NewXSBTextCommand.append(LuaShouldDoCommand).append("\n");
    StoreLuaChunk(NewXSBTextCommand, "DO_ON_NEW_XSB_TEXT_CHUNK");
    return 0;
}

static int LuaDoEveryFrame(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong thing to do? Your DoEveryFrame command is not a string.");
        return 0;
    }
    std::string LuaShouldDoCommand = lua_tostring(L, 1);
    EveryFrameCallbackCommand.append(LuaShouldDoCommand).append("\n");
    StoreLuaChunk(EveryFrameCallbackCommand, "DO_EVERY_FRAME_CHUNK");
    return 0;
}

// --- MULTI-CALLBACK VERSION OF: FlightLoop Callback AfterFlightModel ---

static XPLMFlightLoopID g_DoEveryFrameAfter_ID = nullptr;
static std::vector<std::string> do_every_frame_after_code;

// Called every frame after flight model is updated
float Do_Every_Frame_After(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop,
                           int inCounter, void* inRefcon)
{
    if (!LuaIsRunning) return -1.0f;

    CopyDataRefsToLua();

    for (const auto& code : do_every_frame_after_code)
    {
        if (luaL_loadstring(FWLLua, code.c_str()) == LUA_OK)
        {
            if (lua_pcall(FWLLua, 0, 0, 0) != LUA_OK)
            {
                const char* err = lua_tostring(FWLLua, -1);
                logMsg(logToDevCon, std::string("Error in do_every_frame_after(): ") + err);
                lua_pop(FWLLua, 1);
            }
        }
        else
        {
            const char* err = lua_tostring(FWLLua, -1);
            logMsg(logToDevCon, std::string("Syntax error in do_every_frame_after(): ") + err);
            lua_pop(FWLLua, 1);
        }
    }

    CopyDataRefsToXPlane();

    return -1.0f; // every frame
}

void Register_Do_Every_Frame_After()
{
    if (g_DoEveryFrameAfter_ID != nullptr)
        return;

    XPLMCreateFlightLoop_t loop_params{};
    loop_params.structSize = sizeof(XPLMCreateFlightLoop_t);
    loop_params.phase = xplm_FlightLoop_Phase_AfterFlightModel;
    loop_params.callbackFunc = Do_Every_Frame_After;
    loop_params.refcon = nullptr;

    g_DoEveryFrameAfter_ID = XPLMCreateFlightLoop(&loop_params);
    XPLMScheduleFlightLoop(g_DoEveryFrameAfter_ID, -1.0, 0);
}

static int LuaDoEveryFrameAfter(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: do_every_frame_after() needs a string of Lua code.");
        LuaIsRunning = false;
        return 0;
    }

    std::string lua_code = lua_tostring(L, 1);
    do_every_frame_after_code.push_back(lua_code);

    return 0;
}

// ---  End DoEveryFrameAfter routines


static int LuaDoOften(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong thing to do? Your DoOften command is not a string.");
        return 0;
    }
    std::string LuaShouldDoCommand = lua_tostring(L, 1);
    CallbackCommand.append(LuaShouldDoCommand).append("\n");
    StoreLuaChunk(CallbackCommand, "DO_OFTEN_CHUNK");
    return 0;
}

static int LuaDoSometimes(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong thing to do? Your DoSometimes command is not a string.");
        return 0;
    }
    std::string LuaShouldDoCommand = lua_tostring(L, 1);
    LongTimeCallbackCommand.append(LuaShouldDoCommand).append("\n");
    StoreLuaChunk(LongTimeCallbackCommand, "DO_SOMETIMES_CHUNK");
    return 0;
}

static int LuaClearAllButtonAssignments(lua_State* L)
{
    int EveryThingIsZero[1600] = {0};
    XPLMSetDatavi(gJoystickButtonAssignments, EveryThingIsZero, 0, 1600);
    return 0;
}

static int LuaClearAllAxisAssignments(lua_State* L)
{
    int VersionXP, VersionSDK;
    XPLMHostApplicationID HostID;
    XPLMGetVersions(&VersionXP, &VersionSDK, &HostID);

    if (VersionXP < 11000)
    {
        int EveryThingIsZero[100] = {0};
        XPLMSetDatavi(gJoystickAxisAssignments, EveryThingIsZero, 0, 100);
    } else if (VersionXP < 11100)
    {
        int EveryThingIsZero[250] = {0};
        XPLMSetDatavi(gJoystickAxisAssignments, EveryThingIsZero, 0, 250);
    } else
    {
        int EveryThingIsZero[500] = {0};
        XPLMSetDatavi(gJoystickAxisAssignments, EveryThingIsZero, 0, 500);
    }
    return 0;
}

static int LuaSetButtonAssignment(lua_State* L)
{
    char CommandWanted[NORMALSTRING];
    if (!(lua_isstring(L, 2) && lua_isnumber(FWLLua, 1)))
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: wrong argument types. We need a string and an integer to set a button assignment.");
        return 0;
    }
    auto ButtonNumber = static_cast<int>(lua_tointeger(L, 1));
    if (ButtonNumber < 0 || ButtonNumber > 3199)
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong argument range. Button number has to be from 0 to 3199.");
        return 0;
    }
    strncpy(CommandWanted, lua_tostring(L, 2), sizeof(CommandWanted));
    if (strcmp(CommandWanted, "sim/none/none") == 0)
    {
        int set_the_value_to_zero = 0;
        XPLMSetDatavi(gJoystickButtonAssignments, &set_the_value_to_zero, ButtonNumber, 1);
        return 0;
    }
    auto CommandRefIdWanted = static_cast<int>((std::size_t) XPLMFindCommand(CommandWanted)); // snagar
    if (CommandRefIdWanted == 0)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The Command \"").append(CommandWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetDatavi(gJoystickButtonAssignments, &CommandRefIdWanted, ButtonNumber, 1);
    return 0;
}

// we need two values stored in an integer to point to one of them, to set the axis reverse setting
static int reverse_no  = 0;
static int reverse_yes = 1;

static int LuaSetAxisAssignment(lua_State* L)
{
    int VersionXP, VersionSDK;
    XPLMHostApplicationID HostID;
    XPLMGetVersions(&VersionXP, &VersionSDK, &HostID);

    int CommandRefIdWanted = 0;
    if (!(lua_isstring(L, 3) && lua_isstring(L, 2) && lua_isnumber(FWLLua, 1)))
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: wrong argument types. We need two strings and an integer to set an axis assignment.");
        return 0;
    }
    auto AxisNumber = static_cast<int>(lua_tointeger(L, 1));

    if ((AxisNumber < 0 || AxisNumber > 99) && (VersionXP < 11000))
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: wrong argument range. Before X-Plane 11 axis number has to be from 0 to 99.");
        return 0;
    } else if (AxisNumber < 0 || AxisNumber > 499)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: wrong argument range. Since X-Plane 11 axis number has to be from 0 to 499.");
        return 0;
    }
    std::string CommandWanted = lua_tostring(L, 2);
    std::transform(CommandWanted.begin(), CommandWanted.end(), CommandWanted.begin(), [](unsigned char c){ return std::tolower(c); });
    if (CommandWanted == "none")
        CommandRefIdWanted = 0;
    else if (CommandWanted == "pitch")
        CommandRefIdWanted = 1;
    else if (CommandWanted == "roll")
        CommandRefIdWanted = 2;
    else if (CommandWanted == "yaw")
        CommandRefIdWanted = 3;
    else if (CommandWanted == "throttle")
        CommandRefIdWanted = 4;
    else if (CommandWanted == "collective")
        CommandRefIdWanted = 5;
    else if (CommandWanted == "left toe brake")
        CommandRefIdWanted = 6;
    else if (CommandWanted == "right toe brake")
        CommandRefIdWanted = 7;
    else if (CommandWanted == "prop")
        CommandRefIdWanted = 8;
    else if (CommandWanted == "mixture")
        CommandRefIdWanted = 9;
    else if (CommandWanted == "carb heat")
        CommandRefIdWanted = 10;
    else if (CommandWanted == "flaps")
        CommandRefIdWanted = 11;
    else if (CommandWanted == "thrust vector")
        CommandRefIdWanted = 12;
    else if (CommandWanted == "wing sweep")
        CommandRefIdWanted = 13;
    else if (CommandWanted == "speedbrakes")
        CommandRefIdWanted = 14;
    else if (CommandWanted == "displacement")
        CommandRefIdWanted = 15;
    else if (CommandWanted == "reverse")
        CommandRefIdWanted = 16;
    else if (CommandWanted == "elevator trim")
        CommandRefIdWanted = 17;
    else if (CommandWanted == "aileron trim")
        CommandRefIdWanted = 18;
    else if (CommandWanted == "rudder trim")
        CommandRefIdWanted = 19;
    else if (CommandWanted == "throttle 1")
        CommandRefIdWanted = 20;
    else if (CommandWanted == "throttle 2")
        CommandRefIdWanted = 21;
    else if (CommandWanted == "throttle 3")
        CommandRefIdWanted = 22;
    else if (CommandWanted == "throttle 4")
        CommandRefIdWanted = 23;
    else if (CommandWanted == "prop 1")
        CommandRefIdWanted = 24;
    else if (CommandWanted == "prop 2")
        CommandRefIdWanted = 25;
    else if (CommandWanted == "prop 3")
        CommandRefIdWanted = 26;
    else if (CommandWanted == "prop 4")
        CommandRefIdWanted = 27;
    else if (CommandWanted == "mixture 1")
        CommandRefIdWanted = 28;
    else if (CommandWanted == "mixture 2")
        CommandRefIdWanted = 29;
    else if (CommandWanted == "mixture 3")
        CommandRefIdWanted = 30;
    else if (CommandWanted == "mixture 4")
        CommandRefIdWanted = 31;
    else if (CommandWanted == "reverse 1")
        CommandRefIdWanted = 32;
    else if (CommandWanted == "reverse 2")
        CommandRefIdWanted = 33;
    else if (CommandWanted == "reverse 3")
        CommandRefIdWanted = 34;
    else if (CommandWanted == "reverse 4")
        CommandRefIdWanted = 35;
    else if (CommandWanted == "landing gear")
        CommandRefIdWanted = 36;
    else if (CommandWanted == "nosewheel tiller")
        CommandRefIdWanted = 37;
    else if (CommandWanted == "backup throttle")
        CommandRefIdWanted = 38;

    // the next two axis functions changed in X-Plane 11.02b1
    // if the scripts wants the no longer active functions, we set it to "none"
    else if (CommandWanted == "auto roll")
        CommandRefIdWanted = 0;
    else if (CommandWanted == "auto pitch")
        CommandRefIdWanted = 0;

    // instead we have a new function
    else if (CommandWanted == "cowl flaps")
        CommandRefIdWanted = 39;

    // and nothing for the index value 40

    else if (CommandWanted == "view left/right")
        CommandRefIdWanted = 41;
    else if (CommandWanted == "view up/down")
        CommandRefIdWanted = 42;
    else if (CommandWanted == "view zoom")
        CommandRefIdWanted = 43;
    else if (CommandWanted == "camera left/right")
        CommandRefIdWanted = 44;
    else if (CommandWanted == "camera up/down")
        CommandRefIdWanted = 45;
    else if (CommandWanted == "camera zoom")
        CommandRefIdWanted = 46;
    else if (CommandWanted == "gun/bomb left/right")
        CommandRefIdWanted = 47;
    else if (CommandWanted == "gun/bomb up/down")
        CommandRefIdWanted = 48;

    // added to support for X-Plane VR (version 11.20+)

    else if (CommandWanted == "vr touchpad x")
        CommandRefIdWanted = 49;
    else if (CommandWanted == "vr touchpad y")
        CommandRefIdWanted = 50;
    else if (CommandWanted == "vr trigger")
        CommandRefIdWanted = 51;

    // Missing axis up to 11.55r2

    else if (CommandWanted == "custom command(s)")
        CommandRefIdWanted = 52;
    else if (CommandWanted == "throttle 5")
        CommandRefIdWanted = 53;
    else if (CommandWanted == "throttle 6")
        CommandRefIdWanted = 54;
    else if (CommandWanted == "throttle 7")
        CommandRefIdWanted = 55;
    else if (CommandWanted == "throttle 8")
        CommandRefIdWanted = 56;
    else if (CommandWanted == "cowl flaps 1")
        CommandRefIdWanted = 57;
    else if (CommandWanted == "cowl flaps 2")
        CommandRefIdWanted = 58;
    else if (CommandWanted == "cowl flaps 3")
        CommandRefIdWanted = 59;
    else if (CommandWanted == "cowl flaps 4")
        CommandRefIdWanted = 60;
    else if (CommandWanted == "cowl flaps 5")
        CommandRefIdWanted = 61;
    else if (CommandWanted == "cowl flaps 6")
        CommandRefIdWanted = 62;
    else if (CommandWanted == "cowl flaps 7")
        CommandRefIdWanted = 63;
    else if (CommandWanted == "cowl flaps 8")
        CommandRefIdWanted = 64;
    else if (CommandWanted == "throttle vertical")
        CommandRefIdWanted = 65;
    else if (CommandWanted == "throttle horizontal")
        CommandRefIdWanted = 66;
    else if (CommandWanted == "throttle 9")
        CommandRefIdWanted = 67;
    else if (CommandWanted == "copilot pitch")
        CommandRefIdWanted = 68;
    else if (CommandWanted == "copilot roll")
        CommandRefIdWanted = 69;
    else if (CommandWanted == "copilot yaw")
        CommandRefIdWanted = 70;
    else if (CommandWanted == "copilot left toe brake")
        CommandRefIdWanted = 71;
    else if (CommandWanted == "copilot right toe brake")
        CommandRefIdWanted = 72;
    else if (CommandWanted == "copilot nosewheel tiller")
        CommandRefIdWanted = 73;

    XPLMSetDatavi(gJoystickAxisAssignments, &CommandRefIdWanted, AxisNumber, 1);

    std::string ReverseOrNot = lua_tostring(L, 3);
    std::transform(ReverseOrNot.begin(), ReverseOrNot.end(), ReverseOrNot.begin(), [](unsigned char c){ return std::tolower(c); });
    if (ReverseOrNot == "reverse")
    {
        XPLMSetDatavi(gJoystickAxisReverse, &reverse_yes, AxisNumber, 1);
    } else
    {
        XPLMSetDatavi(gJoystickAxisReverse, &reverse_no, AxisNumber, 1);
    }
    return 0;
}

static int LuaButton(lua_State* L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: function button() needs an integer argument."); //we really should output a script line number here :-) --Teddii
        LuaIsRunning = false;
        return 0;  // this will force Lua to crash...
    }
    lua_pushboolean(L, JoystickButtonValues[lua_tointeger(L, 1)]);
    return 1;
}

static int LuaLastButton(lua_State* L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: function last_button() needs an integer argument.");
        LuaIsRunning = false;
        return 0;  // this will force Lua to crash...
    }
    lua_pushboolean(L, JoystickButtonLastValues[lua_tointeger(L, 1)]);
    return 1;
}

static int LuaGet(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;
    char buf[2048];

    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function get().");
        LuaIsRunning = false;
        return 0;
    }
    strncpy(DataRefWanted, lua_tostring(L, 1), sizeof(DataRefWanted));

    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    if (lua_isnumber(L, 2))
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 2));
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);

    // everything is fine, but a FWL-type dataref variable may carry a new value
    // we fix it by pushing all dataref variables to X-Plane before reading
    CopyDataRefsToXPlane();

    if (DataRefTypeIdWanted == xplmType_Int)
    {
        auto ValueToWrite = XPLMGetDatai(DataRefIdWanted);
        lua_pushnumber(L, ValueToWrite);
        return 1;
    } 
    if (DataRefTypeIdWanted == xplmType_Double)
    {
        auto ValueToWrite = XPLMGetDatad(DataRefIdWanted);
        lua_pushnumber(L, ValueToWrite);
        return 1;
    } 
    if (DataRefTypeIdWanted == xplmType_Float)
    {
        auto ValueToWrite = XPLMGetDataf(DataRefIdWanted);
        lua_pushnumber(L, ValueToWrite);
        return 1;
    } 
    if (DataRefTypeIdWanted == xplmType_IntArray)
    {
        auto ValueToWrite = 0;
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
        // %%%
        int DatabLen; 
        char ValueToWrite[LONGSTRING];
        DatabLen = XPLMGetDatab(DataRefIdWanted, ValueToWrite, 0, LONGSTRING);

        // for debug only
        //sprintf(buf,"%%%%%% len = %d , strlen = %d",DatabLen,strlen(ValueToWrite));
        //logMsg(logToDevCon, buf);
        
        if ( DatabLen > 0 ) {
           DatabLen = strnlen(ValueToWrite,DatabLen);
        }

        lua_pushlstring(L, ValueToWrite,DatabLen);
        return 1;
    }
    return 1;
}

static int LuaSet(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];

    if (!(lua_isstring(L, 1) && (lua_isnumber(L, 2) || lua_isstring(L, 2)) ))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function set().");
        LuaIsRunning = false;
        return 0;
    }
    strncpy(DataRefWanted, lua_tostring(L, 1), sizeof(DataRefWanted));

    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }

    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted == xplmType_Int)
    {
        auto ValueToWrite = (int) lua_tonumber(L, 2);
        XPLMSetDatai(DataRefIdWanted, ValueToWrite);
        update_Lua_dataref_variables(DataRefIdWanted, 0, (float) ValueToWrite);

    } else if (DataRefTypeIdWanted == xplmType_Double)
    {
        auto ValueToWrite = (double) lua_tonumber(L, 2);
        XPLMSetDatad(DataRefIdWanted, ValueToWrite);
        update_Lua_dataref_variables(DataRefIdWanted, 0, (float) ValueToWrite);

    } else if (DataRefTypeIdWanted == xplmType_Float)
    {
        auto ValueToWrite = (float) lua_tonumber(L, 2);
        XPLMSetDataf(DataRefIdWanted, ValueToWrite);
        update_Lua_dataref_variables(DataRefIdWanted, 0, ValueToWrite);

    } else if (DataRefTypeIdWanted == xplmType_Data)
    {
        // %%%
        char DataRefValue[LONGSTRING];

        strncpy(DataRefValue, lua_tostring(L, 2), sizeof(DataRefValue));
        XPLMSetDatab(DataRefIdWanted, DataRefValue, 0, sizeof(DataRefValue));
        update_Lua_dataref_strings(DataRefIdWanted, 0, DataRefValue);
    }


    return 0;
}

static int LuaSetArray(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];

    if (!(lua_isstring(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function set_array().");
        LuaIsRunning = false;
        return 0;
    }
    strncpy(DataRefWanted, lua_tostring(L, 1), sizeof(DataRefWanted));

    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }

    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted == xplmType_IntArray)
    {
        auto ValueWanted = (int) lua_tonumber(L, 3);
        XPLMSetDatavi(DataRefIdWanted, &ValueWanted, (int) lua_tonumber(L, 2), 1);
        update_Lua_dataref_variables(DataRefIdWanted, (int) lua_tonumber(L, 2), (float) ValueWanted);
    }
    if (DataRefTypeIdWanted == xplmType_FloatArray)
    {
        auto ValueWanted = (float) lua_tonumber(L, 3);
        XPLMSetDatavf(DataRefIdWanted, &ValueWanted, (int) lua_tonumber(L, 2), 1);
        update_Lua_dataref_variables(DataRefIdWanted, (int) lua_tonumber(L, 2), ValueWanted);
    }

    return 0;
}

static int LuaGetDataRefBinding(lua_State* L)
{
    char VariableWanted[NORMALSTRING];

    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong argument to function get_dataref_binding().");
        LuaIsRunning = false;
        return 0;
    }
    strncpy(VariableWanted, lua_tostring(L, 1), sizeof(VariableWanted));
    if (DataRefTableLastElement > -1)
    {
        for (auto i = 0; i <= DataRefTableLastElement; i++)
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

static int LuaDefineSharedDataRef(lua_State* L)
{
    char           DataRefNameWanted[NORMALSTRING];
    char           DataRefTypeWanted[NORMALSTRING];
    XPLMDataTypeID XPLMDataRefTypeWanted;

    if (!(lua_isstring(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function define_shared_DataRef().");
        LuaIsRunning = false;
        return 0;
    }
    strncpy(DataRefNameWanted, lua_tostring(L, 1), sizeof(DataRefNameWanted));
    strncpy(DataRefTypeWanted, lua_tostring(L, 2), sizeof(DataRefTypeWanted));

    // check if the name is okay
    if (strncmp(DataRefNameWanted, "sim/", 4) == 0)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You are not allowed to define a shared DataRef starting with \"sim/\".");
        LuaIsRunning = false;
        return 0;
    }

    // check the type
    if (strcmp(DataRefTypeWanted, "Int") == 0)
    {
        XPLMDataRefTypeWanted = xplmType_Int;
    } 
    else if (strcmp(DataRefTypeWanted, "IntArray") == 0)
    {
        XPLMDataRefTypeWanted = xplmType_IntArray;
    } 
    else if (strcmp(DataRefTypeWanted, "Float") == 0)
    {
        XPLMDataRefTypeWanted = xplmType_Float;
    } 
    else if (strcmp(DataRefTypeWanted, "FloatArray") == 0)
    {
        XPLMDataRefTypeWanted = xplmType_FloatArray;
    } 
    else if (strcmp(DataRefTypeWanted, "Double") == 0)
    {
        XPLMDataRefTypeWanted = xplmType_Double;
    } 
    else if (strcmp(DataRefTypeWanted, "Data") == 0)
    {
        XPLMDataRefTypeWanted = xplmType_Data;
    } 
    else
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: Unknown type for the shared DataRef \"").append(DataRefNameWanted).append(
                       "\"."));
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: We do not know what \"").append(DataRefTypeWanted).append("\" is."));
        LuaIsRunning = false;
        return 0;
    }

    // share the data
    if (XPLMShareData(DataRefNameWanted, XPLMDataRefTypeWanted, nullptr, nullptr) == 0)
    {
        logMsg(logToDevCon, std::string("FlyWithLua Error: Wrong type for the existing shared DataRef \"").append(
                DataRefNameWanted).append("\"."));
        LuaIsRunning = false;
        return 0;
    }

    // publish the new shared DataRef to the DataRefEditor
#define MSG_ADD_DATAREF 0x01000000
    XPLMPluginID DREPluginID = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");
    if (DREPluginID != XPLM_NO_PLUGIN_ID)
    {
        XPLMSendMessageToPlugin(DREPluginID, MSG_ADD_DATAREF, DataRefNameWanted);
    }

    return 0;
}

static int LuaDataRef(lua_State* L)
{
    char VariableWanted[NORMALSTRING];
    char DataRefWanted[NORMALSTRING];
    char ReadOnlyOrNot[NORMALSTRING];
    int  ReadOnlyWanted;
    int  IndexWanted = 0;

    if (!(lua_isstring(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function DataRef().");
        LuaIsRunning = false;
        return 0;
    }
    if (!lua_isstring(L, 3))
    {
        ReadOnlyWanted = true;
    } else
    {
        strncpy(ReadOnlyOrNot, lua_tostring(L, 3), sizeof(ReadOnlyOrNot));
        ReadOnlyWanted = !((strcmp(ReadOnlyOrNot, "writable") == 0) || (strcmp(ReadOnlyOrNot, "writeable") == 0));
    }
    strncpy(VariableWanted, lua_tostring(L, 1), sizeof(VariableWanted));
    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 4))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 4));
    }


    // Did we already know the variable?
    if (DataRefTableLastElement >= 0)
    {
        for (auto i = 0; i <= DataRefTableLastElement; i++)
        {
            if (strcmp(DataRefTable[i].LuaVariable, VariableWanted) == 0)
            {
                if (strcmp(DataRefTable[i].DataRefName, DataRefWanted) == 0)
                {
                    // everything is fine?
                    if (IndexWanted != DataRefTable[i].Index)
                    {
                        logMsg(logToDevCon, std::string("FlyWithLua Error: The variable \"").append(VariableWanted).append(
                                "\" is still defined as DataRef \"").append(DataRefTable[i].DataRefName).append(
                                "\", but the index is different."));
                        LuaIsRunning = false;
                        return 0;
                    } else
                    {
                        return 0;
                    }
                } else
                {
                    logMsg(logToDevCon, std::string("FlyWithLua Error: The variable \"").append(VariableWanted).append(
                            "\" is still defined as DataRef \"").append(DataRefTable[i].DataRefName).append("\"."));
                    LuaIsRunning = false;
                    return 0;
                }
            }
        }
    }

    // Did we already know the DataRef?
    if (DataRefTableLastElement >= 0)
    {
        for (auto i = 0; i <= DataRefTableLastElement; i++)
        {
            if ((strcmp(DataRefTable[i].DataRefName, DataRefWanted) == 0) && (DataRefTable[i].Index == IndexWanted))
            {
                logMsg(logToDevCon, std::string("FlyWithLua Info: The DataRef \"") + DataRefWanted +
                                    std::string("\" is already handled in variable \"") + DataRefTable[i].LuaVariable +
                                    std::string("\", you want to use it with \"") + VariableWanted + "\".");
                logMsg(logToDevCon,
                       "FlyWithLua Info: As long as only one variable wants writable access to the DataRef, it is okay (but you loose performance).");
                if (ReadOnlyWanted == 0 && !DataRefTable[i].IsReadOnly)
                {
                    logMsg(logToDevCon, std::string("FlyWithLua Error: The variable \"").append(VariableWanted).append(
                            "\" wants to write a DataRef, that is already handled by another variable."));
                    LuaIsRunning = false;
                    return 0;
                }
            }
        }
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    //if (DataRefTypeIdWanted >= xplmType_FloatArray)
    if (DataRefTypeIdWanted & (xplmType_FloatArray+xplmType_IntArray))    // allow xplmType_Data to have no index
    {
        if (!lua_isnumber(L, 4))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef \"").append(DataRefWanted).append(
                    "\" should have an index. I set it to 0."));
        }
    }
    if (ReadOnlyWanted == 0 && XPLMCanWriteDataRef(DataRefIdWanted) == 0)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" is not writeable."));
        LuaIsRunning = false;
        return 0;
    }

    // add it to the table
    if (++DataRefTableLastElement >= MAXDATAREFS)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more DataRefs than I can handle!");
        LuaIsRunning            = false;
        DataRefTableLastElement = MAXDATAREFS - 1;
    } else
    {
        DataRefTable[DataRefTableLastElement].DataRefId     = DataRefIdWanted;
        DataRefTable[DataRefTableLastElement].DataRefTypeId = DataRefTypeIdWanted;
        DataRefTable[DataRefTableLastElement].Index         = IndexWanted;
        DataRefTable[DataRefTableLastElement].IsReadOnly    = ReadOnlyWanted != 0;
        strncpy(DataRefTable[DataRefTableLastElement].DataRefName, DataRefWanted,
                sizeof(DataRefTable[DataRefTableLastElement].DataRefName));
        strncpy(DataRefTable[DataRefTableLastElement].LuaVariable, VariableWanted,
                sizeof(DataRefTable[DataRefTableLastElement].LuaVariable));
    }

    PushDataRefToLuaVariable(VariableWanted, DataRefIdWanted, DataRefTypeIdWanted, IndexWanted);

    return 0;
}

static int LuaCreateSwitch(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_switch().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 3));
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(
                    " should have an index. I set it to 0."));
        }
    }

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType  = Switch;
        SwitchTable[SwitchTableLastElement].button      = static_cast<int>(lua_tointeger(L, 1));
        SwitchTable[SwitchTableLastElement].DataRefID   = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index       = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].off_int    = static_cast<int>(lua_tointeger(L, 4));
            SwitchTable[SwitchTableLastElement].off_float  = static_cast<float>(lua_tonumber(L, 4));
            SwitchTable[SwitchTableLastElement].off_double = lua_tonumber(L, 4);
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].on_int    = static_cast<int>(lua_tointeger(L, 5));
            SwitchTable[SwitchTableLastElement].on_float  = static_cast<float>(lua_tonumber(L, 5));
            SwitchTable[SwitchTableLastElement].on_double = lua_tonumber(L, 5);
        }
    }
    return 0;
}

static int LuaCreateAxisMedian(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_axis_median().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    IndexWanted = static_cast<int>(lua_tointeger(L, 1));

    // get the actual value of the axis
    float axis_value;
    XPLMGetDatavf(gJoystickAxisValues, &axis_value, IndexWanted, 1);

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType        = AxisMedian;
        SwitchTable[SwitchTableLastElement].DataRefName       = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index             = IndexWanted;
        SwitchTable[SwitchTableLastElement].on_float          = axis_value;
        SwitchTable[SwitchTableLastElement].off_float         = axis_value;
        SwitchTable[SwitchTableLastElement].upper_limit_float = axis_value;
        SwitchTable[SwitchTableLastElement].lower_limit_float = axis_value;
        SwitchTable[SwitchTableLastElement].stepping_float    = axis_value;
    }
    return 0;
}

static int LuaCreatePositiveEdgeTrigger(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_positive_edge_trigger().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 3));
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(
                    " should have an index. I set it to 0."));
        }
    }

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType  = PositiveEdge;
        SwitchTable[SwitchTableLastElement].button      = static_cast<int>(lua_tointeger(L, 1));
        SwitchTable[SwitchTableLastElement].DataRefID   = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index       = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].on_int    = static_cast<int>(lua_tointeger(L, 4));
            SwitchTable[SwitchTableLastElement].on_float  = static_cast<float>(lua_tonumber(L, 4));
            SwitchTable[SwitchTableLastElement].on_double = lua_tonumber(L, 4);
        }
    }
    return 0;
}

static int LuaCreateNegativeEdgeTrigger(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_positive_edge_trigger().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 3));
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(
                    " should have an index. I set it to 0."));
        }
    }

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType  = NegativeEdge;
        SwitchTable[SwitchTableLastElement].button      = static_cast<int>(lua_tointeger(L, 1));
        SwitchTable[SwitchTableLastElement].DataRefID   = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index       = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].on_int    = static_cast<int>(lua_tointeger(L, 4));
            SwitchTable[SwitchTableLastElement].on_float  = static_cast<float>(lua_tonumber(L, 4));
            SwitchTable[SwitchTableLastElement].on_double = lua_tonumber(L, 4);
        }
    }
    return 0;
}

static int LuaCreatePositiveEdgeFlip(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_positive_edge_increment().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 3));
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(
                    " should have an index. I set it to 0."));
        }
    }

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType  = PositiveFlip;
        SwitchTable[SwitchTableLastElement].button      = static_cast<int>(lua_tointeger(L, 1));
        SwitchTable[SwitchTableLastElement].DataRefID   = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index       = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].off_int    = static_cast<int>(lua_tointeger(L, 4));
            SwitchTable[SwitchTableLastElement].off_float  = static_cast<float>(lua_tonumber(L, 4));
            SwitchTable[SwitchTableLastElement].off_double = lua_tonumber(L, 4);
        } else
        {
            SwitchTable[SwitchTableLastElement].off_int    = 0;
            SwitchTable[SwitchTableLastElement].off_float  = 0;
            SwitchTable[SwitchTableLastElement].off_double = 0;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].on_int    = static_cast<int>(lua_tointeger(L, 5));
            SwitchTable[SwitchTableLastElement].on_float  = static_cast<float>(lua_tonumber(L, 5));
            SwitchTable[SwitchTableLastElement].on_double = lua_tonumber(L, 5);
        } else
        {
            SwitchTable[SwitchTableLastElement].on_int    = 1;
            SwitchTable[SwitchTableLastElement].on_float  = 1;
            SwitchTable[SwitchTableLastElement].on_double = 1;
        }
    }
    return 0;
}

static int LuaCreateNegativeEdgeFlip(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_positive_edge_increment().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 3));
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(
                    " should have an index. I set it to 0."));
        }
    }

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType  = NegativeFlip;
        SwitchTable[SwitchTableLastElement].button      = static_cast<int>(lua_tointeger(L, 1));
        SwitchTable[SwitchTableLastElement].DataRefID   = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index       = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].off_int    = static_cast<int>(lua_tointeger(L, 4));
            SwitchTable[SwitchTableLastElement].off_float  = static_cast<float>(lua_tonumber(L, 4));
            SwitchTable[SwitchTableLastElement].off_double = lua_tonumber(L, 4);
        } else
        {
            SwitchTable[SwitchTableLastElement].off_int    = 0;
            SwitchTable[SwitchTableLastElement].off_float  = 0;
            SwitchTable[SwitchTableLastElement].off_double = 0;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].on_int    = static_cast<int>(lua_tointeger(L, 5));
            SwitchTable[SwitchTableLastElement].on_float  = static_cast<float>(lua_tonumber(L, 5));
            SwitchTable[SwitchTableLastElement].on_double = lua_tonumber(L, 5);
        } else
        {
            SwitchTable[SwitchTableLastElement].on_int    = 1;
            SwitchTable[SwitchTableLastElement].on_float  = 1;
            SwitchTable[SwitchTableLastElement].on_double = 1;
        }
    }
    return 0;
}

static int LuaCreatePositiveEdgeIncrement(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_positive_edge_increment().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 3));
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(
                    " should have an index. I set it to 0."));
        }
    }

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType  = PositiveIncrement;
        SwitchTable[SwitchTableLastElement].button      = static_cast<int>(lua_tointeger(L, 1));
        SwitchTable[SwitchTableLastElement].DataRefID   = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index       = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].stepping_int   = static_cast<int>(lua_tointeger(L, 4));
            SwitchTable[SwitchTableLastElement].stepping_float = static_cast<float>(lua_tonumber(L, 4));
        } else
        {
            SwitchTable[SwitchTableLastElement].stepping_int   = 1;
            SwitchTable[SwitchTableLastElement].stepping_float = 1;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].upper_limit_int   = static_cast<int>(lua_tointeger(L, 5));
            SwitchTable[SwitchTableLastElement].upper_limit_float = static_cast<float>(lua_tonumber(L, 5));
        } else
        {
            SwitchTable[SwitchTableLastElement].upper_limit_int   = 999999999;
            SwitchTable[SwitchTableLastElement].upper_limit_float = 999999999;
        }
        if (lua_isnumber(L, 6))
        {
            SwitchTable[SwitchTableLastElement].round = static_cast<float>(lua_tonumber(L, 6));
        } else
        {
            SwitchTable[SwitchTableLastElement].round = 1;
        }
    }
    return 0;
}

static int LuaCreateNegativeEdgeIncrement(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_negative_edge_increment().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 3));
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(
                    " should have an index. I set it to 0."));
        }
    }

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType  = NegativeIncrement;
        SwitchTable[SwitchTableLastElement].button      = static_cast<int>(lua_tointeger(L, 1));
        SwitchTable[SwitchTableLastElement].DataRefID   = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index       = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].stepping_int   = static_cast<int>(lua_tointeger(L, 4));
            SwitchTable[SwitchTableLastElement].stepping_float = static_cast<float>(lua_tonumber(L, 4));
        } else
        {
            SwitchTable[SwitchTableLastElement].stepping_int   = 1;
            SwitchTable[SwitchTableLastElement].stepping_float = 1;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].upper_limit_int   = static_cast<int>(lua_tointeger(L, 5));
            SwitchTable[SwitchTableLastElement].upper_limit_float = static_cast<float>(lua_tonumber(L, 5));
        } else
        {
            SwitchTable[SwitchTableLastElement].upper_limit_int   = 999999999;
            SwitchTable[SwitchTableLastElement].upper_limit_float = 999999999;
        }
        if (lua_isnumber(L, 6))
        {
            SwitchTable[SwitchTableLastElement].round = static_cast<float>(lua_tonumber(L, 6));
        } else
        {
            SwitchTable[SwitchTableLastElement].round = 1;
        }
    }
    return 0;
}

static int LuaCreatePositiveEdgeDecrement(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_positive_edge_decrement().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 3));
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(
                    " should have an index. I set it to 0."));
        }
    }

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType  = PositiveDecrement;
        SwitchTable[SwitchTableLastElement].button      = static_cast<int>(lua_tointeger(L, 1));
        SwitchTable[SwitchTableLastElement].DataRefID   = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index       = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].stepping_int   = static_cast<int>(lua_tointeger(L, 4));
            SwitchTable[SwitchTableLastElement].stepping_float = static_cast<float>(lua_tonumber(L, 4));
        } else
        {
            SwitchTable[SwitchTableLastElement].stepping_int   = 1;
            SwitchTable[SwitchTableLastElement].stepping_float = 1;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].lower_limit_int   = static_cast<int>(lua_tointeger(L, 5));
            SwitchTable[SwitchTableLastElement].lower_limit_float = static_cast<float>(lua_tonumber(L, 5));
        } else
        {
            SwitchTable[SwitchTableLastElement].lower_limit_int   = 0;
            SwitchTable[SwitchTableLastElement].lower_limit_float = 0;
        }
        if (lua_isnumber(L, 6))
        {
            SwitchTable[SwitchTableLastElement].round = static_cast<float>(lua_tonumber(L, 6));
        } else
        {
            SwitchTable[SwitchTableLastElement].round = 1;
        }
    }
    return 0;
}

static int LuaCreateNegativeEdgeDecrement(lua_State* L)
{
    char DataRefWanted[NORMALSTRING];
    int  IndexWanted = 0;

    if (!(lua_isnumber(L, 1) && lua_isstring(L, 2)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: wrong arguments to function create_negative_edge_decrement().");
        LuaIsRunning = false;
        return 0;
    }

    strncpy(DataRefWanted, lua_tostring(L, 2), sizeof(DataRefWanted));

    if (!lua_isnumber(L, 3))
    {
        IndexWanted = 0;
    } else
    {
        IndexWanted = static_cast<int>(lua_tointeger(L, 3));
    }

    // let's search for the DataRef
    XPLMDataRef DataRefIdWanted = XPLMFindDataRef(DataRefWanted);
    if (DataRefIdWanted == nullptr)
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: The DataRef \"").append(DataRefWanted).append("\" does not exist."));
        LuaIsRunning = false;
        return 0;
    }
    XPLMDataTypeID DataRefTypeIdWanted = GetDataRefTypeCompat(DataRefIdWanted);
    if (DataRefTypeIdWanted >= xplmType_FloatArray)
    {
        if (!lua_isnumber(L, 3))
        {
            logMsg(logToDevCon, std::string("FlyWithLua Warning: The DataRef ").append(DataRefWanted).append(
                    " should have an index. I set it to 0."));
        }
    }

    // add it to the table
    if (++SwitchTableLastElement >= MAXSWITCHES)
    {
        logMsg(logToDevCon, "FlyWithLua Error: You want more switches than I can handle!");
        LuaIsRunning           = false;
        SwitchTableLastElement = MAXSWITCHES - 1;
    } else
    {
        SwitchTable[SwitchTableLastElement].SwitchType  = NegativeDecrement;
        SwitchTable[SwitchTableLastElement].button      = static_cast<int>(lua_tointeger(L, 1));
        SwitchTable[SwitchTableLastElement].DataRefID   = DataRefIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefType = DataRefTypeIdWanted;
        SwitchTable[SwitchTableLastElement].DataRefName = DataRefWanted;
        SwitchTable[SwitchTableLastElement].index       = IndexWanted;
        if (lua_isnumber(L, 4))
        {
            SwitchTable[SwitchTableLastElement].stepping_int   = static_cast<int>(lua_tointeger(L, 4));
            SwitchTable[SwitchTableLastElement].stepping_float = static_cast<float>(lua_tonumber(L, 4));
        } else
        {
            SwitchTable[SwitchTableLastElement].stepping_int   = 1;
            SwitchTable[SwitchTableLastElement].stepping_float = 1;
        }
        if (lua_isnumber(L, 5))
        {
            SwitchTable[SwitchTableLastElement].lower_limit_int   = static_cast<int>(lua_tointeger(L, 5));
            SwitchTable[SwitchTableLastElement].lower_limit_float = static_cast<float>(lua_tonumber(L, 5));
        } else
        {
            SwitchTable[SwitchTableLastElement].lower_limit_int   = 0;
            SwitchTable[SwitchTableLastElement].lower_limit_float = 0;
        }
        if (lua_isnumber(L, 6))
        {
            SwitchTable[SwitchTableLastElement].round = static_cast<float>(lua_tonumber(L, 6));
        } else
        {
            SwitchTable[SwitchTableLastElement].round = 1;
        }
    }
    return 0;
}


static int LuaSetPilotsHead(lua_State* L)
{
    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_isnumber(L, 5)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function set_pilots_head().");
        LuaIsRunning = false;
        return 0;
    }
    // set the view to 3D
    XPLMSetDatai(FWLViewType, 1026);
    update_Lua_dataref_variables(FWLViewType, 0, 1026);

    // set pilot's head
    XPLMSetDataf(FWLPilotsHeadX, static_cast<float>(lua_tonumber(L, 1)));
    XPLMSetDataf(FWLPilotsHeadY, static_cast<float>(lua_tonumber(L, 2)));
    XPLMSetDataf(FWLPilotsHeadZ, static_cast<float>(lua_tonumber(L, 3)));
    XPLMSetDataf(FWLPilotsHeadHeading, static_cast<float>(lua_tonumber(L, 4)));
    XPLMSetDataf(FWLPilotsHeadPitch, static_cast<float>(lua_tonumber(L, 5)));
    return 0;
}

static int LuaGetPilotsHead(lua_State* L)
{
    auto X       = XPLMGetDataf(FWLPilotsHeadX);
    auto Y       = XPLMGetDataf(FWLPilotsHeadY);
    auto Z       = XPLMGetDataf(FWLPilotsHeadZ);
    auto Heading = XPLMGetDataf(FWLPilotsHeadHeading);
    auto Pitch   = XPLMGetDataf(FWLPilotsHeadPitch);
    lua_pushnumber(L, X);
    lua_pushnumber(L, Y);
    lua_pushnumber(L, Z);
    lua_pushnumber(L, Heading);
    lua_pushnumber(L, Pitch);
    return 5;
}

static int LuaXPLMSetGraphicsState(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: XPLMSetGraphicsState() cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4) && lua_isnumber(L, 5) &&
          lua_isnumber(L, 6) && lua_isnumber(L, 7)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetGraphicsState.");
        LuaIsRunning = false;
    }
    XPLMSetGraphicsState(static_cast<int>(lua_tointeger(L, 1)), static_cast<int>(lua_tointeger(L, 2)),
                         static_cast<int>(lua_tointeger(L, 3)), static_cast<int>(lua_tointeger(L, 4)),
                         static_cast<int>(lua_tointeger(L, 5)), static_cast<int>(lua_tointeger(L, 6)),
                         static_cast<int>(lua_tointeger(L, 7)));
    return 0;
}

// now follows the OpenGL part of FlyWithLua
// all these functions will *not* check for errors!!!
// error handling is made by the module written in Lua

static int LuaglBegin_POINTS(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_POINTS);
    return 0;
}

static int LuaglBegin_LINES(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_LINES);
    return 0;
}

static int LuaglBegin_LINE_STRIP(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_LINE_STRIP);
    return 0;
}

static int LuaglBegin_LINE_LOOP(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_LINE_LOOP);
    return 0;
}

static int LuaglBegin_POLYGON(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_POLYGON);
    return 0;
}

static int LuaglBegin_TRIANGLES(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_TRIANGLES);
    return 0;
}

static int LuaglBegin_TRIANGLE_STRIP(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_TRIANGLE_STRIP);
    return 0;
}

static int LuaglBegin_TRIANGLE_FAN(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_TRIANGLE_FAN);
    return 0;
}

static int LuaglBegin_QUADS(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_QUADS);
    return 0;
}

static int LuaglBegin_QUAD_STRIP(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glBegin(GL_QUAD_STRIP);
    return 0;
}

static int LuaglEnd(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    glEnd();
    return 0;
}

static int LuaglVertex2f(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    auto x = static_cast<float>(lua_tonumber(L, 1));
    auto y = static_cast<float>(lua_tonumber(L, 2));
    glVertex2f(x, y);
    return 0;
}

static int LuaglVertex3f(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    auto x = static_cast<float>(lua_tonumber(L, 1));
    auto y = static_cast<float>(lua_tonumber(L, 2));
    auto z = static_cast<float>(lua_tonumber(L, 3));
    glVertex3f(x, y, z);
    return 0;
}

static int LuaglLineWidth(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    auto width = static_cast<float>(lua_tonumber(L, 1));
    glLineWidth(width);
    return 0;
}

static int LuaglColor3f(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    auto red   = static_cast<float>(lua_tonumber(L, 1));
    auto green = static_cast<float>(lua_tonumber(L, 2));
    auto blue  = static_cast<float>(lua_tonumber(L, 3));
    glColor3f(red, green, blue);
    return 0;
}

static int LuaglColor4f(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    auto red   = static_cast<float>(lua_tonumber(L, 1));
    auto green = static_cast<float>(lua_tonumber(L, 2));
    auto blue  = static_cast<float>(lua_tonumber(L, 3));
    auto alpha = static_cast<float>(lua_tonumber(L, 4));
    glColor4f(red, green, blue, alpha);
    return 0;
}

static int LuaglRectf(lua_State* L)
{
    if (WeAreNotInDrawingState)
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: An OpenGL function cannot be executed outside a drawing loopback. Put the function call inside the do_every_draw() string argument to solve this issue.");
        LuaIsRunning = false;
        return 0;
    }

    auto x1 = static_cast<float>(lua_tonumber(L, 1));
    auto y1 = static_cast<float>(lua_tonumber(L, 2));
    auto x2 = static_cast<float>(lua_tonumber(L, 3));
    auto y2 = static_cast<float>(lua_tonumber(L, 4));

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
        y1       = y2;
        y2       = ys;
    }

    glRectf(x1, y1, x2, y2);
    return 0;
}

// The XPLMNavigation commands
static int LuaXPLMGetFirstNavAid(lua_State* L)
{
    lua_pushnumber(L, XPLMGetFirstNavAid());
    return 1;
}

static int LuaXPLMFindFirstNavAidOfType(lua_State* L)
{
    int NavType;
    if (lua_isnumber(L, 1))
    {
        NavType = static_cast<int>(lua_tointeger(L, 1));
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMFindFirstNavAidOfType.");
        LuaIsRunning = false;
        return 0;
    }
    lua_pushnumber(L, XPLMFindFirstNavAidOfType(NavType));
    return 1;
}

static int LuaXPLMFindLastNavAidOfType(lua_State* L)
{
    int NavType;
    if (lua_isnumber(L, 1))
    {
        NavType = static_cast<int>(lua_tointeger(L, 1));
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMFindLastNavAidOfType.");
        LuaIsRunning = false;
        return 0;
    }
    lua_pushnumber(L, XPLMFindLastNavAidOfType(NavType));
    return 1;
}

static int LuaXPLMGetNextNavAid(lua_State* L)
{
    int NavID;
    if (lua_isnumber(L, 1))
    {
        NavID = static_cast<int>(lua_tointeger(L, 1));
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMGetNextNavAid.");
        LuaIsRunning = false;
        return 0;
    }
    lua_pushnumber(L, XPLMGetNextNavAid(NavID));
    return 1;
}

static int LuaXPLMFindNavAid(lua_State* L)
{
    const char* inNameFragment;    /* Can be NULL */
    const char* inIDFragment;    /* Can be NULL */
    float     * inLat;    /* Can be NULL */
    float     * inLon;    /* Can be NULL */
    int       * inFrequency;    /* Can be NULL */
    XPLMNavType inType;
    int         Frequency;
    float       Lat, Lon;
    char        NameFragment[NORMALSTRING];
    char        IDFragment[NORMALSTRING];

    if (lua_isstring(L, 1))
    {
        strncpy(NameFragment, lua_tostring(L, 1), sizeof(NameFragment));
        inNameFragment = NameFragment;
    } else
    {
        inNameFragment = nullptr;
    }
    if (lua_isstring(L, 2))
    {
        strncpy(IDFragment, lua_tostring(L, 2), sizeof(IDFragment));
        inIDFragment = IDFragment;
    } else
    {
        inIDFragment = nullptr;
    }
    if (lua_isnumber(L, 3))
    {
        Lat   = static_cast<float>(lua_tonumber(L, 3));
        inLat = &Lat;
    } else
    {
        inLat = nullptr;
    }
    if (lua_isnumber(L, 4))
    {
        Lon   = static_cast<float>(lua_tonumber(L, 4));
        inLon = &Lon;
    } else
    {
        inLon = nullptr;
    }
    if (lua_isnumber(L, 5))
    {
        Frequency   = static_cast<int>(lua_tonumber(L, 5));
        inFrequency = &Frequency;
    } else
    {
        inFrequency = nullptr;
    }
    if (lua_isnumber(L, 6))
    {
        inType = static_cast<XPLMNavType>(lua_tonumber(L, 6));
    } else
    {
        inType = xplm_Nav_Airport;
    }
    lua_pushnumber(L, XPLMFindNavAid(inNameFragment, inIDFragment, inLat, inLon, inFrequency, inType));
    return 1;
}

static int LuaXPLMGetNavAidInfo(lua_State* L)
{
    XPLMNavType outType      = xplm_Nav_Unknown;        /* Can be NULL */
    float       outLatitude  = 0;    /* Can be NULL */
    float       outLongitude = 0;   /* Can be NULL */
    float       outHeight    = 0;      /* Can be NULL */
    int         outFrequency = 0;   /* Can be NULL */
    float       outHeading   = 0;     /* Can be NULL */
    char        outID[32]    = "";     /* Can be NULL */
    char        outName[256] = "";

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMGetNavAidInfo.");
        LuaIsRunning = false;
        return 0;
    }

    XPLMGetNavAidInfo(static_cast<XPLMNavRef>(lua_tointeger(L, 1)), &outType, &outLatitude, &outLongitude, &outHeight, &outFrequency,
                      &outHeading, outID, outName,
                      nullptr);

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

static int LuaXPLMCountFMSEntries(lua_State* L)
{
    lua_pushnumber(L, XPLMCountFMSEntries());
    return 1;
}

static int LuaXPLMGetDisplayedFMSEntry(lua_State* L)
{
    lua_pushnumber(L, XPLMGetDisplayedFMSEntry());
    return 1;
}

static int LuaXPLMGetDestinationFMSEntry(lua_State* L)
{
    lua_pushnumber(L, XPLMGetDestinationFMSEntry());
    return 1;
}

static int LuaXPLMSetDisplayedFMSEntry(lua_State* L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetDisplayedFMSEntry.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetDisplayedFMSEntry(static_cast<int>(lua_tointeger(L, 1)));
    return 0;
}

static int LuaXPLMSetDestinationFMSEntry(lua_State* L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetDestinationFMSEntry.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetDestinationFMSEntry(static_cast<int>(lua_tointeger(L, 1)));
    return 0;
}

static int LuaXPLMClearFMSEntry(lua_State* L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMClearFMSEntry.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMClearFMSEntry(static_cast<int>(lua_tointeger(L, 1)));
    return 0;
}

static int LuaXPLMSetFMSEntryLatLon(lua_State* L)
{
    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetFMSEntryLatLon.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetFMSEntryLatLon(static_cast<int>(lua_tointeger(L, 1)), static_cast<float>(lua_tonumber(L, 2)), static_cast<float>(lua_tonumber(L, 3)),
                          static_cast<int>(lua_tointeger(L, 4)));
    return 0;
}

static int LuaXPLMSetFMSEntryInfo(lua_State* L)
{
    if (!(lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetFMSEntryInfo.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMSetFMSEntryInfo(static_cast<int>(lua_tointeger(L, 1)), static_cast<XPLMNavRef>(lua_tointeger(L, 2)), static_cast<int>(lua_tointeger(L, 3)));
    return 0;
}

static int LuaXPLMGetFMSEntryInfo(lua_State* L)
{
    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetFMSEntryInfo.");
        LuaIsRunning = false;
        return 0;
    }
    XPLMNavType outType    = xplm_Nav_Unknown;
    char        outID[256] = "";
    XPLMNavRef  outRef     = XPLM_NAV_NOT_FOUND;
    int outAltitude = 0; // modified by saar
    float outLat = 0;
    float outLon = 0;

    XPLMGetFMSEntryInfo(static_cast<int>(lua_tointeger(L, 1)), &outType, outID, &outRef, reinterpret_cast<int *>(&outAltitude), &outLat,
                        &outLon); // disabled by saar for debug

    lua_pushnumber(L, outType);
    lua_pushstring(L, outID);
    lua_pushnumber(L, outRef);
    lua_pushnumber(L, outAltitude);
    lua_pushnumber(L, outLat);
    lua_pushnumber(L, outLon);
    return 6;
}

// The core access to DataRefs (for advanced users of FlyWithLua)
static int LuaXPLMFindDataRef(lua_State* L)
{
    char        DataRefWanted[NORMALSTRING];
    XPLMDataRef DataRefID;

    strncpy(DataRefWanted, luaL_checkstring(L, 1), sizeof(DataRefWanted));

    DataRefID = XPLMFindDataRef(DataRefWanted);
    if (DataRefID != nullptr)
    {
        lua_pushlightuserdata(L, DataRefID);
    } else
    {
        lua_pushnil(L);
    }
    return 1;
}

// The core access to Commands (for advanced users of FlyWithLua)
static int LuaXPLMFindCommand(lua_State* L)
{
    char        CommandWanted[NORMALSTRING];
    XPLMCommandRef CommandID;

    strncpy(CommandWanted, luaL_checkstring(L, 1), sizeof(CommandWanted));

    CommandID = XPLMFindCommand(CommandWanted);
    if (CommandID != nullptr)
    {
        lua_pushlightuserdata(L, CommandID);
    } else
    {
        lua_pushnil(L);
    }
    return 1;
}

static int LuaXPLMGetDatai(lua_State* L)
{
    if (lua_islightuserdata(L, 1))
    {
        lua_pushnumber(L, XPLMGetDatai(lua_touserdata(L, 1)));
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMGetDatai.");
        LuaIsRunning = false;
        return 0;
    }
    return 1;
}

static int LuaXPLMGetDataf(lua_State* L)
{
    if (lua_islightuserdata(L, 1))
    {
        lua_pushnumber(L, XPLMGetDataf(lua_touserdata(L, 1)));
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMGetDataf.");
        LuaIsRunning = false;
        return 0;
    }
    return 1;
}

static int LuaXPLMGetDatad(lua_State* L)
{
    if (lua_islightuserdata(L, 1))
    {
        lua_pushnumber(L, XPLMGetDatad(lua_touserdata(L, 1)));
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMGetDatad.");
        LuaIsRunning = false;
        return 0;
    }
    return 1;
}

static int LuaXPLMGetDatavf(lua_State* L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        float SpaceForValues[100] = {0};
        auto inMax                = static_cast<int>(lua_tointeger(L, 3));
        auto StartFrom            = static_cast<int>(lua_tointeger(L, 2));
        if (inMax > 100)
        {
            logMsg(logToDevCon, "FlyWithLua Error: Can't handle more than 100 values in function XPLMGetDatavf.");
            LuaIsRunning = false;
            return 0;
        }
        lua_createtable(L, inMax, 1);
        XPLMGetDatavf(lua_touserdata(L, 1), SpaceForValues, StartFrom, inMax);
        for (int i = 0; i < inMax; i++)
        {
            lua_pushnumber(L, StartFrom + i);
            lua_pushnumber(L, SpaceForValues[i]);
            lua_settable(L, -3);
        }
        return 1;
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMGetDatavf.");
        LuaIsRunning = false;
        return 0;
    }
}

static int LuaXPLMGetDatavi(lua_State* L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        int SpaceForValues[100] = {0};
        auto inMax              = static_cast<int>(lua_tointeger(L, 3));
        auto StartFrom          = static_cast<int>(lua_tointeger(L, 2));
        if (inMax > 100)
        {
            logMsg(logToDevCon, "FlyWithLua Error: Can't handle more than 100 values in function XPLMGetDatavi.");
            LuaIsRunning = false;
            return 0;
        }
        lua_createtable(L, inMax, 1);
        XPLMGetDatavi(lua_touserdata(L, 1), SpaceForValues, StartFrom, inMax);
        for (int i = 0; i < inMax; i++)
        {
            lua_pushnumber(L, StartFrom + i);
            lua_pushnumber(L, SpaceForValues[i]);
            lua_settable(L, -3);
        }
        return 1;
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMGetDatavi.");
        LuaIsRunning = false;
        return 0;
    }
}

static int LuaXPLMSetDatai(lua_State* L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2))
    {
        XPLMSetDatai(lua_touserdata(L, 1), static_cast<int>(lua_tointeger(L, 2)));
        update_Lua_dataref_variables(lua_touserdata(L, 1), 0, static_cast<float>(lua_tonumber(L, 2)));
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetDatai.");
        LuaIsRunning = false;
    }
    return 0;
}

static int LuaXPLMSetDataf(lua_State* L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2))
    {
        XPLMSetDataf(lua_touserdata(L, 1), static_cast<float>(lua_tonumber(L, 2)));
        update_Lua_dataref_variables(lua_touserdata(L, 1), 0, static_cast<float>(lua_tonumber(L, 2)));
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetDataf.");
        LuaIsRunning = false;
    }
    return 0;
}

static int LuaXPLMSetDatad(lua_State* L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2))
    {
        XPLMSetDatad(lua_touserdata(L, 1), lua_tonumber(L, 2));
        update_Lua_dataref_variables(lua_touserdata(L, 1), 0, static_cast<float>(lua_tonumber(L, 2)));
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetDatad.");
        LuaIsRunning = false;
    }
    return 0;
}

static int LuaXPLMSetDatavi(lua_State* L)
{
    if (lua_islightuserdata(L, 1) && lua_istable(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
    {
        int         SpaceForValues[100] = {0};
        auto inMax                      = static_cast<int>(lua_tointeger(L, 4));
        auto StartFrom                  = static_cast<int>(lua_tointeger(L, 3));
        XPLMDataRef DataRefWanted       = lua_touserdata(L, 1);
        if (inMax > 100)
        {
            logMsg(logToDevCon, "FlyWithLua Error: Can't handle more than 100 values in function XPLMSetDatavi.");
            LuaIsRunning = false;
            return 0;
        }
        lua_pushvalue(L, 2);
        for (int i = 0; i < inMax; i++)
        {
            lua_pushnumber(L, StartFrom + i);
            lua_gettable(L, -2);
            SpaceForValues[i] = static_cast<int>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            update_Lua_dataref_variables(DataRefWanted, StartFrom + i, (float) SpaceForValues[i]);
        }
        XPLMSetDatavi(DataRefWanted, SpaceForValues, StartFrom, inMax);
        return 0;
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetDatavi.");
        LuaIsRunning = false;
        return 0;
    }
}

static int LuaXPLMSetDatavf(lua_State* L)
{
    if (lua_islightuserdata(L, 1) && lua_istable(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
    {
        float       SpaceForValues[100] = {0};
        auto inMax                      = static_cast<int>(lua_tointeger(L, 4));
        auto StartFrom                  = static_cast<int>(lua_tointeger(L, 3));
        XPLMDataRef DataRefWanted       = lua_touserdata(L, 1);
        if (inMax > 100)
        {
            logMsg(logToDevCon, "FlyWithLua Error: Can't handle more than 100 values in function XPLMSetDatavf.");
            LuaIsRunning = false;
            return 0;
        }
        lua_pushvalue(L, 2);
        for (int i = 0; i < inMax; i++)
        {
            lua_pushnumber(L, StartFrom + i);
            lua_gettable(L, -2);
            SpaceForValues[i] = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            update_Lua_dataref_variables(DataRefWanted, StartFrom + i, SpaceForValues[i]);
        }
        XPLMSetDatavf(DataRefWanted, SpaceForValues, StartFrom, inMax);
        return 0;
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function XPLMSetDatavf.");
        LuaIsRunning = false;
        return 0;
    }
}

static int LuaXPLMGetDataRefTypes(lua_State* L)
{
    int ReferenceType = xplmType_Unknown;

    if (lua_islightuserdata(L, 1))
    {
        ReferenceType = XPLMGetDataRefTypes(lua_touserdata(L, 1));
        lua_pushnumber(L, ReferenceType);
        return 1;
    } else
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: Wrong argument to function XPLMGetDataRefTypes, expecting a single userdata as a reference.");
        LuaIsRunning = false;
        return 0;
    }
}

static int LuaXPLMGetMouseLocationGlobal(lua_State *L)
{
    int sGlobalx, sGlobaly;
    XPLMGetMouseLocationGlobal(&sGlobalx, &sGlobaly);

    lua_pushnumber(L, sGlobalx);
    lua_pushnumber(L, sGlobaly);

    return 2;
}

static int LuaXPLMGetScreenSize(lua_State *L)
{
    int sWidth, sHeight;
    XPLMGetScreenSize(&sWidth, &sHeight);

    lua_pushnumber(L, sWidth);
    lua_pushnumber(L, sHeight);

    return 2;
}

static int LuaXPLMGetScreenBoundsGlobal(lua_State *L)
{
    int sLeft, sTop, sRight, sBottom;
    XPLMGetScreenBoundsGlobal(&sLeft, &sTop, &sRight, &sBottom);

    lua_pushnumber(L, sLeft);
    lua_pushnumber(L, sTop);
    lua_pushnumber(L, sRight);
    lua_pushnumber(L, sBottom);

    return 4;
}

static void LuaReceiveMonitorBounds_f(int inMonitorIndex, int inLeft, int inTop, int inRight, int inBottom, void * refcon)
{
    auto MonitorBounds = reinterpret_cast<std::vector<MonitorBoundsStructure>*>(refcon);

    MonitorBounds->emplace_back(
        MonitorBoundsStructure{
            inMonitorIndex,
            inLeft,
            inTop,
            inRight,
            inBottom
        }
    );
}

static int LuaXPLMGetAllMonitorBoundsOS(lua_State *L)
{
    std::vector<MonitorBoundsStructure> MonitorBounds;

    XPLMGetAllMonitorBoundsOS(LuaReceiveMonitorBounds_f, &MonitorBounds);

    int monitorCount = 0;

    lua_newtable(L);                        // create parent table, size unknown

    for (auto& m : MonitorBounds)
    {
        monitorCount++;
        lua_pushnumber(L, monitorCount);    // puts key of the first child table on-top of Lua VM stack
        lua_createtable(L, 5, 0);           // creates first child table of size 5 array elements

        lua_pushnumber(L, m.MonitorIndex);  // fills child table
        lua_setfield(L, -2, "MonIndex");    // setfield() pops the value from Lua VM stack

        lua_pushnumber(L, m.inLeft);
        lua_setfield(L, -2, "inLeft");

        lua_pushnumber(L, m.inTop);
        lua_setfield(L, -2, "inTop");

        lua_pushnumber(L, m.inRight);
        lua_setfield(L, -2, "inRight");

        lua_pushnumber(L, m.inBottom);
        lua_setfield(L, -2, "inBottom");

        lua_settable(L, -3);                // lua_settable() pops key, value pair from Lua VM stack
    }                                       // and puts child table into the parent

    return 1;
}

static int LuaXPLMGetAllMonitorBoundsGlobal(lua_State *L)
{
    std::vector<MonitorBoundsStructure> MonitorBounds;

    XPLMGetAllMonitorBoundsGlobal(LuaReceiveMonitorBounds_f, &MonitorBounds);

    int monitorCount = 0;

    lua_newtable(L);                        // create parent table, size unknown

    for (auto& m : MonitorBounds)
    {
        monitorCount++;
        lua_pushnumber(L, monitorCount);    // puts key of the first child table on-top of Lua VM stack
        lua_createtable(L, 5, 0);           // creates first child table of size 5 array elements

        lua_pushnumber(L, m.MonitorIndex);  // fills child table
        lua_setfield(L, -2, "MonIndex");    // setfield() pops the value from Lua VM stack

        lua_pushnumber(L, m.inLeft);
        lua_setfield(L, -2, "inLeft");

        lua_pushnumber(L, m.inTop);
        lua_setfield(L, -2, "inTop");

        lua_pushnumber(L, m.inRight);
        lua_setfield(L, -2, "inRight");

        lua_pushnumber(L, m.inBottom);
        lua_setfield(L, -2, "inBottom");

        lua_settable(L, -3);                // lua_settable() pops key, value pair from Lua VM stack
    }                                       // and puts child table into the parent

    return 1;
}

static int Luapeek(lua_State* L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        XPLMDataRef DataRefIdWanted;
        int         DataRefTypeIdWanted;
        int         IndexWanted;

        DataRefIdWanted     = lua_touserdata(L, 1);
        DataRefTypeIdWanted = static_cast<int>(lua_tointeger(L, 2));
        IndexWanted         = static_cast<int>(lua_tointeger(L, 3));

        DataRefTypeIdWanted = reduceTypes(DataRefTypeIdWanted);

        if (DataRefTypeIdWanted == xplmType_Float)
        {
            auto ValueOfDataRef = XPLMGetDataf(DataRefIdWanted);
            lua_pushnumber(FWLLua, ValueOfDataRef);
            return 1;
        }

        if (DataRefTypeIdWanted == xplmType_Int)
        {
            auto ValueOfDataRef = XPLMGetDatai(DataRefIdWanted);
            lua_pushnumber(FWLLua, ValueOfDataRef);
            return 1;
        }

        if (DataRefTypeIdWanted == xplmType_Double)
        {
            auto ValueOfDataRef = XPLMGetDatad(DataRefIdWanted);
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
            // %%%
            int DatabLen;
            char ValueOfDataRef[LONGSTRING];
            DatabLen = XPLMGetDatab(DataRefIdWanted, &ValueOfDataRef, IndexWanted, LONGSTRING);
            if ( DatabLen > 0 ) {
               DatabLen = strnlen(ValueOfDataRef,DatabLen);
            }
            lua_pushlstring(FWLLua, ValueOfDataRef, DatabLen);
            return 1;
        }
    } else
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: Wrong argument to function peek, expecting a userdata as a reference, the type of the DataRef and an index.");
        LuaIsRunning = false;
        return 0;
    }

    return 1; // snagar: added.
}

static int Luapoke(lua_State* L)
{
    if (lua_islightuserdata(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
        XPLMDataRef DataRefId;
        int         DataRefTypeId;
        int         Index;

        DataRefId     = lua_touserdata(L, 1);
        DataRefTypeId = static_cast<int>(lua_tointeger(L, 2));
        Index         = static_cast<int>(lua_tointeger(L, 3));

        DataRefTypeId = reduceTypes(DataRefTypeId);

        if ((DataRefTypeId == xplmType_Int) && lua_isnumber(L, 4))
        {
            auto DataRefValue = (int) lua_tonumber(FWLLua, 4);
            XPLMSetDatai(DataRefId, DataRefValue);
            update_Lua_dataref_variables(DataRefId, Index, (float) DataRefValue);

        } else if ((DataRefTypeId == xplmType_Float) && lua_isnumber(L, 4))
        {
            auto DataRefValue = (float) lua_tonumber(FWLLua, 4);
            XPLMSetDataf(DataRefId, DataRefValue);
            update_Lua_dataref_variables(DataRefId, Index, DataRefValue);

        } else if ((DataRefTypeId == xplmType_Double) && lua_isnumber(L, 4))
        {
            auto DataRefValue = (double) lua_tonumber(FWLLua, 4);
            XPLMSetDatad(DataRefId, DataRefValue);
            update_Lua_dataref_variables(DataRefId, Index, (float) DataRefValue);

        } else if ((DataRefTypeId == xplmType_IntArray) && lua_isnumber(L, 4))
        {
            auto DataRefValue = static_cast<int>(lua_tointeger(FWLLua, -1));
            XPLMSetDatavi(DataRefId, &DataRefValue, Index, 1);
            update_Lua_dataref_variables(DataRefId, Index, (float) DataRefValue);

        } else if ((DataRefTypeId == xplmType_FloatArray) && lua_isnumber(L, 4))
        {
            auto DataRefValue = static_cast<float>(lua_tonumber(FWLLua, 4));
            XPLMSetDatavf(DataRefId, &DataRefValue, Index, 1);
            update_Lua_dataref_variables(DataRefId, Index, DataRefValue);

        } else if ((DataRefTypeId == xplmType_Data) && lua_isstring(L, 4))
        {
            // %%%
            char DataRefValue[LONGSTRING];
            strncpy(DataRefValue, lua_tostring(FWLLua, 4), sizeof(DataRefValue));
            XPLMSetDatab(DataRefId, DataRefValue, Index, sizeof(DataRefValue));
            update_Lua_dataref_strings(DataRefId, Index, DataRefValue);
        }
    } else
    {
        logMsg(logToDevCon,
               "FlyWithLua Error: Wrong argument to function poke, expecting a userdata as a reference, the type of the DataRef, an index and the value to set.");
        LuaIsRunning = false;
        return 0;
    }
    return 1; // snagar: added.
}

// we leave these functions inside FlyWithLua 2.2.1 to be compatible to older versions
static int Luabegin_classic_mode(lua_State* L)
{
    logMsg(logToDevCon,
           "FlyWithLua Info: Since version 2.2.1 we do not differ in classic or modern scripts, begin_classic_mode() will be ignored.");
    return 0;
}

static int Luaend_classic_mode(lua_State* L)
{
    logMsg(logToDevCon,
           "FlyWithLua Info: Since version 2.2.1 we do not differ in classic or modern scripts, end_classic_mode() will be ignored.");
    return 0;
}

static int LuaCreateCommand(lua_State* L)
{
    // usage: create_command( "name", "description", "begin command", "continue command", "end command" )
    if (!(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3) && lua_isstring(L, 4) && lua_isstring(L, 5)))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Wrong arguments to function create_command().");
        LuaIsRunning = false;
        return 0;
    }
    if (++CommandTableLastElement >= MAXCOMMANDS)
    {
        logMsg(logToDevCon, "FlyWithLua Error: Too much functions create_command().");
        LuaIsRunning = false;
        return 0;
    }
    if (strncmp(lua_tostring(L, 1), "sim/", 4) == 0)
    {
        logMsg(logToDevCon, "FlyWithLua Error: Custom commands are prohibited to start with \"sim/\".");
        LuaIsRunning = false;
        return 0;
    }
    CommandTable[CommandTableLastElement].Name            = lua_tostring(L, 1);
    CommandTable[CommandTableLastElement].Description     = lua_tostring(L, 2);
    CommandTable[CommandTableLastElement].BeginCommand    = lua_tostring(L, 3);
    CommandTable[CommandTableLastElement].ContinueCommand = lua_tostring(L, 4);
    CommandTable[CommandTableLastElement].EndCommand      = lua_tostring(L, 5);
    CommandTable[CommandTableLastElement].Reference       = XPLMCreateCommand(
            CommandTable[CommandTableLastElement].Name.c_str(),
            CommandTable[CommandTableLastElement].Description.c_str());
    XPLMRegisterCommandHandler(CommandTable[CommandTableLastElement].Reference, FWLCommandCallback, 1,
                               (void*) "FlyWithLuaCommand");
    return 0;
}

static int Luadirectory_to_table(lua_State* L)
{
    char DirectoryPath[LONGSTRING] = "";
    int  k;
    // max number of characters of the files or folders names in directory modified by sparker
    // this is a average of 30 characters for each name * the number of names.
    char FilesInFolder[45000];
    int NumberOfFiles; // modified by saar -- debug
    int TotalNumberOfFiles; // modified by saar -- debug

    // max number of files or folders in directory modified by sparker
    char* FileIndex[1500];

    strncpy(DirectoryPath, luaL_checkstring(L, 1), sizeof(DirectoryPath));

    if (XPLMGetDirectoryContents(DirectoryPath, 0, FilesInFolder, sizeof(FilesInFolder), FileIndex,
                                 sizeof(FileIndex) / sizeof(char *),
                                 &TotalNumberOfFiles, &NumberOfFiles))
    {
        // create an empty table on stack
        lua_createtable(L, static_cast<int>(NumberOfFiles), 0);

        // are enough files in the folder to read them out?
        if (NumberOfFiles == 0)
        {
            // just return an empty table
            return 1;
        }

        // sorting the files
        qsort(FileIndex, static_cast<size_t>(NumberOfFiles), sizeof(char*), compare_filenames);

        // reading ot the files into a Lua table step by step
        for (k = 0; k < NumberOfFiles; k++)
        {
            lua_pushnumber(L, k + 1);
            lua_pushstring(L, FileIndex[k]);
            lua_settable(L, -3);
        }
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Can't read out the subfolder given to function directory_to_table().");
        LuaIsRunning = false;
        return 0;
    }

    return true; // snagar
}

static int LuaLoadWAVFile(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing filename of WAV file to load. You will have to give a string.");
        LuaIsRunning = false;
        return 0;
    }
    char FileNameToLoad[NORMALSTRING];
    strncpy(FileNameToLoad, lua_tostring(L, 1), sizeof(FileNameToLoad));

    ALCcontext* old_context = alcGetCurrentContext();
    OpenALSounds.emplace_back(); // Make space to store information about the sound.
    OpenALSoundsStructure* sound = &OpenALSounds.back();

    // fill the debug table with information
    sound->filename = FileNameToLoad;
    sound->pitch    = 1.0f;
    sound->gain     = 1.0f;
    sound->loop     = false;

    // the following code comes from the SDK example
    ALfloat zero[3] = {0};

    // Generate source and load a buffer of audio.
    alGenSources(1, &(sound->source));
    if (sound->source == 0)
    {
        // Set this flag to true so we know to reload the scripts
        // Since we did not find the source id.
        openal_source_id_not_found = 1;
        // Not sure if this is needed but it does not like it here.
        // I have it where the if (sound->buffer == 0) check is.
        // free(mem);
        // return;
    }

    load_wave(FileNameToLoad, sound);
    if (verbose_logging_mode == 1)
    {
        logMsg(logToDevCon, std::string("FlyWithLua: Loaded sound file \"").append(FileNameToLoad).append("\"."));
    }
    CHECK_ERR();

    // Basic initialization code to play a sound: specify the buffer the source is playing, as well as some
    // sound parameters. This doesn't play the sound - it's just one-time initialization.
    alSourcei(sound->source, AL_BUFFER, sound->buffer);
    alSourcef(sound->source, AL_PITCH, 1.0f);
    alSourcef(sound->source, AL_GAIN, 1.0f);
    alSourcei(sound->source, AL_LOOPING, 0);
    alSourcefv(sound->source, AL_POSITION, zero);
    alSourcefv(sound->source, AL_VELOCITY, zero);
    CHECK_ERR();

    // give back to source number
    lua_pushnumber(FWLLua, OpenALSounds.size() - 1);
    if(old_context) { alcMakeContextCurrent(old_context); }
    return 1;
}

static int LuaPlaySound(lua_State* L)
{
    int SourceNo;

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing Play Sound source number. You will have to give an integer.");
        LuaIsRunning = false;
        return 0;
    }
    SourceNo = static_cast<int>(lua_tointeger(L, 1));
    if ((SourceNo < 0) || (SourceNo >= int(OpenALSounds.size())))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Play Sound source number out of range.");
        LuaIsRunning = false;
        return 0;
    }
    ALCcontext* old_context = alcGetCurrentContext();
    alSourcePlay(OpenALSounds[SourceNo].source);
    if(old_context) { alcMakeContextCurrent(old_context); }
    return 0;
}

static int LuaStopSound(lua_State* L)
{
    int SourceNo;

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing Stop Sound source number. You will have to give an integer.");
        LuaIsRunning = false;
        return 0;
    }
    SourceNo = static_cast<int>(lua_tointeger(L, 1));
    if ((SourceNo < 0) || (SourceNo >= int(OpenALSounds.size())))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Stop Sound source number out of range.");
        LuaIsRunning = false;
        return 0;
    }
    ALCcontext* old_context = alcGetCurrentContext();
    alSourceStop(OpenALSounds[SourceNo].source);
    if(old_context) { alcMakeContextCurrent(old_context); }
    return 0;
}

static int LuaRewindSound(lua_State* L)
{
    int SourceNo;

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing Rewind Sound source number. You will have to give an integer.");
        LuaIsRunning = false;
        return 0;
    }
    SourceNo = static_cast<int>(lua_tointeger(L, 1));
    if ((SourceNo < 0) || (SourceNo >= int(OpenALSounds.size())))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Rewind Sound source number out of range.");
        LuaIsRunning = false;
        return 0;
    }
    ALCcontext* old_context = alcGetCurrentContext();
    alSourceRewind(OpenALSounds[SourceNo].source);
    if(old_context) { alcMakeContextCurrent(old_context); }
    return 0;
}

static int LuaPauseSound(lua_State* L)
{
    int SourceNo;

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing Pause Sound source number. You will have to give an integer.");
        LuaIsRunning = false;
        return 0;
    }
    SourceNo = static_cast<int>(lua_tointeger(L, 1));
    if ((SourceNo < 0) || (SourceNo >= int(OpenALSounds.size())))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Pause Sound source number out of range.");
        LuaIsRunning = false;
        return 0;
    }
    ALCcontext* old_context = alcGetCurrentContext();
    alSourcePause(OpenALSounds[SourceNo].source);
    if(old_context) { alcMakeContextCurrent(old_context); }
    return 0;
}

static int LuaLetSoundLoop(lua_State* L)
{
    int SourceNo;

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing Let Sound Loop source number. You will have to give an integer.");
        LuaIsRunning = false;
        return 0;
    }
    SourceNo = static_cast<int>(lua_tointeger(L, 1));
    if ((SourceNo < 0) || (SourceNo >= int(OpenALSounds.size())))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Let Sound Loop source number out of range.");
        LuaIsRunning = false;
        return 0;
    }
    if (!lua_isboolean(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing boolean value if sound should loop.");
        LuaIsRunning = false;
        return 0;
    }
    ALCcontext* old_context = alcGetCurrentContext();
    bool LoopOrNot;
    LoopOrNot = static_cast<bool>(lua_toboolean(L, 2));
    if (LoopOrNot)
    {
        alSourcei(OpenALSounds[SourceNo].source, AL_LOOPING, 1);
        OpenALSounds[SourceNo].loop = true;
    } else
    {
        alSourcei(OpenALSounds[SourceNo].source, AL_LOOPING, 0);
        OpenALSounds[SourceNo].loop = false;
    }
    if(old_context) { alcMakeContextCurrent(old_context); }
    return 0;
}

static int LuaSetSoundPitch(lua_State* L)
{
    int SourceNo;

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing Set Sound Pitch source number. You will have to give an integer.");
        LuaIsRunning = false;
        return 0;
    }
    SourceNo = static_cast<int>(lua_tointeger(L, 1));
    if ((SourceNo < 0) || (SourceNo >= int(OpenALSounds.size())))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Set Sound Pitch source number out of range.");
        LuaIsRunning = false;
        return 0;
    }
    if (!lua_isnumber(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing float value to set the sound pitch.");
        LuaIsRunning = false;
        return 0;
    }
    float PitchToSet;
    PitchToSet = static_cast<float>(lua_tonumber(L, 2));
    if (PitchToSet > 0)
    {
        ALCcontext* old_context = alcGetCurrentContext();
        alSourcef(OpenALSounds[SourceNo].source, AL_PITCH, PitchToSet);
        OpenALSounds[SourceNo].pitch = PitchToSet;
        if(old_context) { alcMakeContextCurrent(old_context); }
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Float value to set the sound pitch must be greater than zero.");
        LuaIsRunning = false;
    }
    return 0;
}

static int LuaSetSoundGain(lua_State* L)
{
    int SourceNo;

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing Set Sound Gain source number. You will have to give an integer.");
        LuaIsRunning = false;
        return 0;
    }
    SourceNo = static_cast<int>(lua_tointeger(L, 1));
    if ((SourceNo < 0) || (SourceNo >= int(OpenALSounds.size())))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Set Sound Gain source number out of range.");
        LuaIsRunning = false;
        return 0;
    }
    if (!lua_isnumber(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing float value to set the sound gain.");
        LuaIsRunning = false;
        return 0;
    }
    float GainToSet;
    GainToSet = static_cast<float>(lua_tonumber(L, 2));
    if (GainToSet > 0)
    {
        ALCcontext* old_context = alcGetCurrentContext();
        alSourcef(OpenALSounds[SourceNo].source, AL_GAIN, GainToSet);
        OpenALSounds[SourceNo].gain = GainToSet;
        if(old_context) { alcMakeContextCurrent(old_context); }
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Float value to set the sound gain must be greater than zero.");
        LuaIsRunning = false;
    }
    return 0;
}

static int LuaUnloadAllSounds(lua_State* L)
{
    // release memory for OpenAL buffers
    ALCcontext* old_context = alcGetCurrentContext();
    for (const OpenALSoundsStructure & sound: OpenALSounds)
    {
        alDeleteSources(1, &sound.source);
        alDeleteBuffers(1, &sound.buffer);
    }
    OpenALSounds.clear();
    if(old_context) { alcMakeContextCurrent(old_context); }
    return 0;
}

static int LuaReplaceWAVFile(lua_State* L)
{
    int SourceNo;

    if (!lua_isnumber(L, 1))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing ReplaceWAVFile Sounds source number. You will have to give an integer.");
        LuaIsRunning = false;
        return 0;
    }
    SourceNo = static_cast<int>(lua_tointeger(L, 1));
    if ((SourceNo < 0) || (SourceNo >= int(OpenALSounds.size())))
    {
        logMsg(logToDevCon, "FlyWithLua Error: ReplaceWAVFile Sounds source number out of range.");
        LuaIsRunning = false;
        return 0;
    }
    if (!lua_isstring(L, 2))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Missing filename of WAV file to load. You will have to give a string.");
        LuaIsRunning = false;
        return 0;
    }
    char FileNameToLoad[NORMALSTRING];
    strncpy(FileNameToLoad, lua_tostring(L, 2), sizeof(FileNameToLoad));

    ALCcontext* old_context = alcGetCurrentContext();
    OpenALSoundsStructure* sound = &OpenALSounds[SourceNo];

    // free memory
    alDeleteSources(1, &(sound->source));
    alDeleteBuffers(1, &(sound->buffer));

    // fill the debug table with information
    sound->filename = FileNameToLoad;

    // the following code comes from the SDK example
    ALfloat zero[3] = {0};

    // Generate source and load a buffer of audio.
    alGenSources(1, &(sound->source));
    load_wave(FileNameToLoad, sound);
    logMsg(logToDevCon, std::string("FlyWithLua: Replaced sound by new file \"").append(FileNameToLoad).append("\"."));
    CHECK_ERR();

    // Basic initialization code to play a sound: specify the buffer the source is playing, as well as some
    // sound parameters. This doesn't play the sound - it's just one-time initialization.
    alSourcei(sound->source, AL_BUFFER, sound->buffer);
    alSourcef(sound->source, AL_PITCH, sound->pitch);
    alSourcef(sound->source, AL_GAIN, sound->gain);
    alSourcei(sound->source, AL_LOOPING, sound->loop);
    alSourcefv(sound->source, AL_POSITION, zero);
    alSourcefv(sound->source, AL_VELOCITY, zero);
    CHECK_ERR();
    if(old_context) { alcMakeContextCurrent(old_context); }

    return 0;
}

static int LuaReloadScenery(lua_State* L)
{
    // to reload the scenery, we need to be free from jumping back
    // so we raise a flag to let the reload be done
    // by the per drawing call
    WeNeedToReloadTheScenery = true;
    return 0;
}

/**
 * Register initialization functions of Lua modules compiled into the plugin.
 *
 * This function stores references to module initialization functions inside
 * package.preload table, so that calling `require` in a script would be able
 * to find the module.
 *
 * This approach allows us to compile binary modules into the plugin binary
 * instead of having dynamically loadable libraries in Modules directory, at
 * least for modules supported out of box. This is necessary to ensure that
 * binary modules are compiled and linked against exactly the same version of
 * LuaJIT that is shipped with FlyWithLua.
 *
 * While having dynamic libraries *may* work, this causes a risk of LuaJIT
 * version mismatch, leading to difficult to debug errors. It also doesn't work
 * well on UNIX platforms, where FlyWithLua plugin doesn't export any Lua
 * symbols.
 */
void RegisterEmbeddedModules(lua_State* L)
{
    auto const preload_libs = std::map<std::string, lua_CFunction>{
            {"LuaXML_lib", ::luaopen_LuaXML_lib},
            {"mime.core", ::luaopen_mime_core},
            {"socket.core", ::luaopen_socket_core},
#ifndef IBM
    {"socket.serial", ::luaopen_socket_serial},
    {"socket.unix", ::luaopen_socket_unix},
#endif
    };
    for (auto& lib : preload_libs)
    {
        lua_getfield(L, LUA_GLOBALSINDEX, "package");
        lua_getfield(L, -1, "preload");
        lua_pushcfunction(L, lib.second);
        lua_setfield(L, -2, lib.first.c_str());
    }
}

void RegisterCoreCFunctionsToLua(lua_State* L)
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
    lua_register(L, "define_shared_DataRef", LuaDefineSharedDataRef);
    lua_register(L, "define_shared_dataref", LuaDefineSharedDataRef);
    lua_register(L, "get_dataref_binding", LuaGetDataRefBinding);
    lua_register(L, "get_DataRef_binding", LuaGetDataRefBinding);
    lua_register(L, "create_switch", LuaCreateSwitch);
    lua_register(L, "create_axis_median", LuaCreateAxisMedian);
    lua_register(L, "create_positive_edge_trigger", LuaCreatePositiveEdgeTrigger);
    lua_register(L, "create_negative_edge_trigger", LuaCreateNegativeEdgeTrigger);
    lua_register(L, "create_positive_edge_increment", LuaCreatePositiveEdgeIncrement);
    lua_register(L, "create_negative_edge_increment", LuaCreateNegativeEdgeIncrement);
    lua_register(L, "create_positive_edge_decrement", LuaCreatePositiveEdgeDecrement);
    lua_register(L, "create_negative_edge_decrement", LuaCreateNegativeEdgeDecrement);
    lua_register(L, "create_positive_edge_flip", LuaCreatePositiveEdgeFlip);
    lua_register(L, "create_negative_edge_flip", LuaCreateNegativeEdgeFlip);
    lua_register(L, "do_on_new_metar", LuaDoEveryMETARCallback);
    lua_register(L, "do_on_new_XSB_text", LuaDoOnNewXSBTextCallback);
    lua_register(L, "do_on_keystroke", LuaDoEveryKeystroke);
    lua_register(L, "do_on_mouse_click", LuaDoEveryMouseClick);
    lua_register(L, "do_on_mouse_wheel", LuaDoEveryMouseWheel);
    lua_register(L, "do_every_draw", LuaDoEveryDrawCallback);
    lua_register(L, "do_every_frame", LuaDoEveryFrame);
    lua_register(L, "do_every_frame_after", LuaDoEveryFrameAfter);	 // Callback after FlightModel
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
    lua_register(L, "command_begin", LuaCommandBegin);
    lua_register(L, "command_end", LuaCommandEnd);
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
    lua_register(L, "XPLMFindCommand", LuaXPLMFindCommand);
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
    lua_register(L, "place_aircraft_at", LuaPlaceUserAtAirport);
    lua_register(L, "place_aircraft_at_location", LuaPlaceUserAtLocation);
    lua_register(L, "load_situation", LuaLoadSituation);
    lua_register(L, "save_situation", LuaSaveSituation);
    lua_register(L, "load_aircraft", LuaLoadAircraft);
    lua_register(L, "reload_scenery", LuaReloadScenery);
    lua_register(L, "XPLMGetMouseLocationGlobal", LuaXPLMGetMouseLocationGlobal);
    lua_register(L, "XPLMGetScreenSize", LuaXPLMGetScreenSize);
    lua_register(L, "XPLMGetScreenBoundsGlobal", LuaXPLMGetScreenBoundsGlobal);
    lua_register(L, "XPLMGetAllMonitorBoundsOS", LuaXPLMGetAllMonitorBoundsOS);
    lua_register(L, "XPLMGetAllMonitorBoundsGlobal", LuaXPLMGetAllMonitorBoundsGlobal);

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
    lua_register(L, "hid_send_filled_feature_report", Luahid_send_filled_feature_report);
    lua_register(L, "hid_get_feature_report", Luahid_get_feature_report);

    // function to access OpenAL sound
    lua_register(L, "load_WAV_file", LuaLoadWAVFile);
    lua_register(L, "play_sound", LuaPlaySound);
    lua_register(L, "stop_sound", LuaStopSound);
    lua_register(L, "rewind_sound", LuaRewindSound);
    lua_register(L, "pause_sound", LuaPauseSound);
    lua_register(L, "let_sound_loop", LuaLetSoundLoop);
    lua_register(L, "set_sound_pitch", LuaSetSoundPitch);
    lua_register(L, "set_sound_gain", LuaSetSoundGain);
    lua_register(L, "unload_all_sounds", LuaUnloadAllSounds);
    lua_register(L, "replace_WAV_file", LuaReplaceWAVFile);
}

// sort DataRefTable by name and index of the DataRef
typedef int (* compfn)(const void*, const void*);

static int compare_datareftable(struct DataRefTableStructure* a, struct DataRefTableStructure* b)
{
    auto result = strcmp(a->DataRefName, b->DataRefName);
    if (result == 0)
    {
        if (a->Index > b->Index) return 1;
        else if (a->Index < b->Index) return -1;
        else return 0;
    } else return result;
}

// sort SwitchTable by number of first button
static int compare_switchtable(struct SwitchTableStructure* a, struct SwitchTableStructure* b)
{
    if (a->button > b->button) return 1;
    else if (a->button < b->button) return -1;
    else return 0;
}

void DebugLua()
{
    time_t DebugZeit;
    time(&DebugZeit);
    logMsg(logToDevCon, std::string("FlyWithLua Debug Info From Plugin: SystemPath \"").append(systemDir).append("\""));
    std::ofstream DebugFile(systemDir + "FlyWithLua_Debug.txt");
    if (!DebugFile.is_open())
    {
        logMsg(logToDevCon, "FlyWithLua Error: Unable to write a debug file.");
        return;
    }
    DebugFile << "-- " << PLUGIN_NAME << " v" << PLUGIN_VERSION << " Debug File\n-- " << ctime(&DebugZeit)
              << "-- Memory in use: " << lua_gc(FWLLua, LUA_GCCOUNT, 0) << " kb";
    DebugFile << "\n*** Lua stack ***\n";
    auto StackSize = lua_gettop(FWLLua);
    if (StackSize == 0)
    {
        DebugFile << "The stack is empty.\n";
    } else
    {
        DebugFile << "Number of elements: " << StackSize << "\n";
        for (int i = 1; i <= StackSize; i++)
        {
            if (lua_isstring(FWLLua, i))
            {
                DebugFile << "Element no. " << i << " is: " << lua_tostring(FWLLua, i) << "\n";
            }
        }
        if (found_bad_function_script)
        {
            found_bad_function_script = 0;
            ReadAllScriptFiles();
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
    DebugFile << "\n\n*** New XSB Text callback ***\n";
    DebugFile << NewXSBTextCommand;
    DebugFile << "\n\n*** command callbacks ***\n";
    if (CommandTableLastElement >= 0)
    {
        for (auto i = 0; i <= CommandTableLastElement; i++)
        {
            DebugFile << "Command    --> \"" << CommandTable[i].Name << "\" (" << CommandTable[i].Reference << ")\n";
            DebugFile << "Description--> \"" << CommandTable[i].Description << "\"\n";
            DebugFile << "Begin      --> " << CommandTable[i].BeginCommand << "\n";
            DebugFile << "Continue   --> " << CommandTable[i].ContinueCommand << "\n";
            DebugFile << "End        --> " << CommandTable[i].EndCommand << "\n\n";
        }
    } else
    {
        DebugFile << "No custom commands defined.\n";
    }

    DebugFile << "\n\n*** DataRefs stored in Lua variables ***\n";
    if (DataRefTableLastElement >= 0)
    {
        // sort the DataRef table
        qsort((void*) &DataRefTable, static_cast<size_t>(DataRefTableLastElement + 1), (size_t) sizeof(struct DataRefTableStructure),
              (compfn) compare_datareftable);

        for (auto i = 0; i <= DataRefTableLastElement; i++)
        {
            DebugFile << "The DataRef \"" << DataRefTable[i].DataRefName << "\" is stored in \""
                      << DataRefTable[i].LuaVariable;
            if (DataRefTable[i].IsReadOnly)
            {
                DebugFile << "\" (read only, type = ";
            } else
            {
                DebugFile << "\" (writeable, type = ";
            }
            if (DataRefTable[i].DataRefTypeId == xplmType_Int)
            {
                DebugFile << "int).\n";
            } else if (DataRefTable[i].DataRefTypeId == xplmType_Float)
            {
                DebugFile << "float).\n";
            } else if (DataRefTable[i].DataRefTypeId == xplmType_Double)
            {
                DebugFile << "double).\n";
            } else if (DataRefTable[i].DataRefTypeId == xplmType_IntArray)
            {
                DebugFile << "int array, index = " << DataRefTable[i].Index << ").\n";
            } else if (DataRefTable[i].DataRefTypeId == xplmType_FloatArray)
            {
                DebugFile << "float array, index = " << DataRefTable[i].Index << ").\n";
            } else if (DataRefTable[i].DataRefTypeId == xplmType_Data)
            {
                DebugFile << "string).\n";
            } else if (DataRefTable[i].DataRefTypeId == (xplmType_Int | xplmType_Float | xplmType_Double))
            {
                DebugFile << "number).\n";
            } else if (DataRefTable[i].DataRefTypeId == (xplmType_IntArray | xplmType_FloatArray))
            {
                DebugFile << "number array, index = " << DataRefTable[i].Index << ").\n";
            } else
            {
                DebugFile << "unknown, " << (int) DataRefTable[i].DataRefTypeId << ").\n";
            }
        }
    } else
    {
        DebugFile << "No DataRefs stored in Lua variables.\n";
    }

    DebugFile << "\n\n*** Switches created by Lua ***\n";
    if (SwitchTableLastElement >= 0)
    {
        std::ostringstream oss_WhatToSay;
        std::string WhatToSay;

        // sort the switch table
        qsort((void*) &SwitchTable, static_cast<size_t>(SwitchTableLastElement + 1), (size_t) sizeof(struct SwitchTableStructure),
              (compfn) compare_switchtable);

        for (auto i = 0; i <= SwitchTableLastElement; i++)
        {
            if (SwitchTable[i].SwitchType == Switch)
            {
                oss_WhatToSay << "The button " << SwitchTable[i].button << " switches " <<  SwitchTable[i].DataRefName.c_str() << "[" <<
                        SwitchTable[i].index << "] to " << SwitchTable[i].off_float << " (if off) or to " <<
                        SwitchTable[i].on_float << " (if on).\n";
                WhatToSay = oss_WhatToSay.str();
            } else if (SwitchTable[i].SwitchType == PositiveFlip)
            {
                oss_WhatToSay << "The positive edge of button " << SwitchTable[i].button << " flips " <<  SwitchTable[i].DataRefName.c_str() <<
                        "[" <<  SwitchTable[i].index << "] between " << SwitchTable[i].off_float << " and " <<
                        SwitchTable[i].on_float << ".\n";
                WhatToSay = oss_WhatToSay.str();
            } else if (SwitchTable[i].SwitchType == NegativeFlip)
            {
                oss_WhatToSay << "The negative edge of button " << SwitchTable[i].button << " flips " <<  SwitchTable[i].DataRefName.c_str() <<
                        "[" <<  SwitchTable[i].index << "] between " << SwitchTable[i].off_float << " and " <<
                        SwitchTable[i].on_float << ".\n";
                WhatToSay = oss_WhatToSay.str();

            } else if (SwitchTable[i].SwitchType == PositiveEdge)
            {
                oss_WhatToSay << "The positive edge of button " << SwitchTable[i].button << " sets " <<  SwitchTable[i].DataRefName.c_str() <<
                        "[" <<  SwitchTable[i].index << "] to " << SwitchTable[i].on_float << ".\n";
                WhatToSay = oss_WhatToSay.str();

            } else if (SwitchTable[i].SwitchType == NegativeEdge)
            {
                oss_WhatToSay << "The negative edge of button " << SwitchTable[i].button << " sets " <<  SwitchTable[i].DataRefName.c_str() <<
                        "[" <<  SwitchTable[i].index << "] to " << SwitchTable[i].on_float << ".\n";
                WhatToSay = oss_WhatToSay.str();

            } else if (SwitchTable[i].SwitchType == PositiveIncrement)
            {
                oss_WhatToSay << "The positive edge of button " << SwitchTable[i].button << " increments " <<  SwitchTable[i].DataRefName.c_str() <<
                        "[" <<  SwitchTable[i].index << "] by " << SwitchTable[i].stepping_float << " (limit = " <<
                        SwitchTable[i].upper_limit_float << ", round to " << SwitchTable[i].round << ").\n";
                WhatToSay = oss_WhatToSay.str();

            } else if (SwitchTable[i].SwitchType == NegativeIncrement)
            {
                oss_WhatToSay << "The negative edge of button " << SwitchTable[i].button << " increments " <<  SwitchTable[i].DataRefName.c_str() <<
                        "[" <<  SwitchTable[i].index << "] by " << SwitchTable[i].stepping_float << " (limit = " <<
                        SwitchTable[i].upper_limit_float << ", round to " << SwitchTable[i].round << ").\n";
                WhatToSay = oss_WhatToSay.str();

            } else if (SwitchTable[i].SwitchType == PositiveDecrement)
            {
                oss_WhatToSay << "The positive edge of button " << SwitchTable[i].button << " decrements " <<  SwitchTable[i].DataRefName.c_str() <<
                        "[" <<  SwitchTable[i].index << "] by " << SwitchTable[i].stepping_float << " (limit = " <<
                        SwitchTable[i].lower_limit_float << ", round to " << SwitchTable[i].round << ").\n";
                WhatToSay = oss_WhatToSay.str();

            } else if (SwitchTable[i].SwitchType == NegativeDecrement)
            {
                oss_WhatToSay << "The negative edge of button " << SwitchTable[i].button << " decrements " <<  SwitchTable[i].DataRefName.c_str() <<
                        "[" <<  SwitchTable[i].index << "] by " << SwitchTable[i].stepping_float << " (limit = " <<
                        SwitchTable[i].lower_limit_float << ", round to " << SwitchTable[i].round << ").\n";
                WhatToSay = oss_WhatToSay.str();

            }
            DebugFile << WhatToSay.c_str();
        }
    } else
    {
        DebugFile << "No switches created by Lua.\n";
    }
    DebugFile << "\n\n*** Macros from macro and ATC memu are stored in a table ***\n";
    if (MacroTableLastElement >= 0)
    {
        for (auto i = 0; i <= MacroTableLastElement; i++)
        {
            DebugFile << "<<< Macro table element no. " << i;
            if (MacroTable[i].IsSwitch)
            {
                DebugFile << " is a switch. >>>\n";
                DebugFile << "name      -->\"" << MacroTable[i].MacroName << "\"\n";
                DebugFile << "activate  -->" << MacroTable[i].ActivateCommand << "\n";
                DebugFile << "deactivate-->" << MacroTable[i].DeactivateCommand << "\n\n";
            } else
            {
                DebugFile << " is not a switch. >>>\n";
                DebugFile << "name      -->\"" << MacroTable[i].MacroName << "\"\n";
                DebugFile << "always    -->" << MacroTable[i].ActivateCommand << "\n\n";
            }
        }
    }
    DebugFile << "\n\n*** OpenAL sound files are stored in a table ***\n";
    if (!OpenALSounds.empty())
    {
        for (const OpenALSoundsStructure & sound : OpenALSounds)
        {
            DebugFile << "<<< Sound table element >>>\n";
            DebugFile << "filename --> \"" << sound.filename << "\"\n";
            if (sound.loop)
            {
                DebugFile << "pitch    --> " << sound.pitch << "\ngain     --> " << sound.gain <<
                                 "\nloop     --> true\nsource   --> " << sound.source << "\nbuffer   --> " << sound.buffer << "\n\n";
            } else
            {
                DebugFile << "pitch    --> " << sound.pitch << "\ngain     --> " << sound.gain <<
                                 "\nloop     --> false\nsource   --> " << sound.source << "\nbuffer   --> " << sound.buffer << "\n\n";
            }
        }
    } else
    {
        DebugFile << "No OpenAL sounds created by Lua.\n\n";
    }
    DebugFile.close();
}

void CopyDataRefsToLua()
{
    if (!LuaIsRunning) return;
    if (DataRefTableLastElement >= 0)
    {
        for (auto i = 0; i <= DataRefTableLastElement; i++)
        {
            PushDataRefToLuaVariable(DataRefTable[i].LuaVariable,
                                     DataRefTable[i].DataRefId,
                                     DataRefTable[i].DataRefTypeId,
                                     DataRefTable[i].Index);
        }
    }
}

void CopyDataRefsToXPlane()
{
    if (!LuaIsRunning) return;
    if (DataRefTableLastElement >= 0)
    {
        for (auto i = 0; i <= DataRefTableLastElement; i++)
        {
            if (!DataRefTable[i].IsReadOnly)
            {
                if (DataRefTable[i].DataRefTypeId == xplmType_Int)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    auto DataRefValue = (int) lua_tonumber(FWLLua, -1);
                    lua_pop(FWLLua, 1);
                    XPLMSetDatai(DataRefTable[i].DataRefId, DataRefValue);
                } else if (DataRefTable[i].DataRefTypeId == xplmType_Float)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    auto DataRefValue = (float) lua_tonumber(FWLLua, -1);
                    lua_pop(FWLLua, 1);
                    XPLMSetDataf(DataRefTable[i].DataRefId, DataRefValue);
                } else if (DataRefTable[i].DataRefTypeId == xplmType_Double)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    auto DataRefValue = (double) lua_tonumber(FWLLua, -1);
                    lua_pop(FWLLua, 1);
                    XPLMSetDatad(DataRefTable[i].DataRefId, DataRefValue);
                } else if (DataRefTable[i].DataRefTypeId == xplmType_IntArray)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    auto DataRefValue = static_cast<int>(lua_tointeger(FWLLua, -1));
                    lua_pop(FWLLua, 1);
                    XPLMSetDatavi(DataRefTable[i].DataRefId, &DataRefValue, DataRefTable[i].Index, 1);
                } else if (DataRefTable[i].DataRefTypeId == xplmType_FloatArray)
                {
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    auto DataRefValue = static_cast<float>(lua_tonumber(FWLLua, -1));
                    lua_pop(FWLLua, 1);
                    XPLMSetDatavf(DataRefTable[i].DataRefId, &DataRefValue, DataRefTable[i].Index, 1);
                } else if (DataRefTable[i].DataRefTypeId == xplmType_Data)
                {
                    // %%%
                    lua_getglobal(FWLLua, DataRefTable[i].LuaVariable);
                    char DataRefValue[LONGSTRING];
                    strncpy(DataRefValue, lua_tostring(FWLLua, -1), sizeof(DataRefValue));
                    lua_pop(FWLLua, 1);
                    XPLMSetDatab(DataRefTable[i].DataRefId, DataRefValue, 0, sizeof(DataRefValue));
                }
            }
        }
    }
}

bool RunLuaString(const std::string& LuaCommandString)
{
    if (!LuaIsRunning) return false;
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

bool RunLuaChunk(const char* ChunkName)
{
    if (!LuaIsRunning) return false;
    CopyDataRefsToLua();
    lua_getglobal(FWLLua, ChunkName);
    if (!lua_isfunction(FWLLua, 1))
    {
        lua_pop(FWLLua, 1);
        return false;
    }
    if (lua_pcall(FWLLua, 0, LUA_MULTRET, 0))
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: Can't execute Lua chunk. The chunk who failed is: ").append(ChunkName));
        LuaIsRunning = false;
    }
    CopyDataRefsToXPlane();
    return true;
}


void ResetLuaEngine()
{
    // define some DataRefs
    gXSBMetarStringXDataRef = XPLMFindDataRef(XSB_WEATHER_METAR);
    gXSBTextMessageXDataRef = XPLMFindDataRef(XSB_TEXT_MESSAGE);
    gXSBTextFromXDataRef    = XPLMFindDataRef(XSB_TEXT_FROM);
    gXSBTextFreqsXDataRef   = XPLMFindDataRef(XSB_TEXT_FREQS);
    gXSBTextUseXDataRef     = XPLMFindDataRef(XSB_TEXT_USE);


    // run through the exit script
    if ((LuaResetCount > 0) && !ReadScriptFile("Resources/plugins/FlyWithLua/Internals/FlyWithLua.exit"))
    {
        logMsg(logToDevCon, "FlyWithLua Error: Unable to load exit file.");
    }

    LuaIsRunning = false;

    // killing all commands
    if (CommandTableLastElement > -1)
    {
        for (auto i = 0; i <= CommandTableLastElement; i++)
        {
            XPLMUnregisterCommandHandler(CommandTable[i].Reference, FWLCommandCallback, 1,
                                         (void*) "FlyWithLuaCommand");
            CommandTable[i].Reference = nullptr;
            CommandTable[i].Name.erase();
            CommandTable[i].Description.erase();
            CommandTable[i].BeginCommand.erase();
            CommandTable[i].ContinueCommand.erase();
            CommandTable[i].EndCommand.erase();
        }
    }
    CommandTableLastElement = -1;

    // release memory for OpenAL buffers
    for (const OpenALSoundsStructure & sound: OpenALSounds)
    {
        alDeleteSources(1, &sound.source);
        alDeleteBuffers(1, &sound.buffer);
    }
    OpenALSounds.clear();

    // release memory for Fmod buffers
    fmodint::deinitFmodSupport();
    // This does not work here and not wure why
    // I think after this call fmod is not getting initalized.
    // fmodint::fmod_uninitialize();

    // XPLMDataRef lua_alloc_ref = XPLMFindDataRef("sim/operation/prefs/misc/has_lua_alloc");
    lua_close(FWLLua);

    /* use the default allocator. */
    FWLLua = luaL_newstate();

    sol::set_default_state(FWLLua);

    LuaDrawCommand.clear();
    EveryFrameCallbackCommand.clear();
    CallbackCommand.clear();
    LongTimeCallbackCommand.clear();
    NewMetarCommand.clear();
    NewXSBTextCommand.clear();
    KeyEventCommand.clear();
    LuaMouseClickCommand.clear();
    LuaMouseWheelCommand.clear();

    // create empty chunks (functions)
    StoreLuaChunk(LuaDrawCommand, "DO_EVERY_DRAW_CHUNK");
    StoreLuaChunk(EveryFrameCallbackCommand, "DO_EVERY_FRAME_CHUNK");
    StoreLuaChunk(CallbackCommand, "DO_OFTEN_CHUNK");
    StoreLuaChunk(LongTimeCallbackCommand, "DO_SOMETIMES_CHUNK");
    StoreLuaChunk(NewMetarCommand, "DO_ON_NEW_METAR_CHUNK");
    StoreLuaChunk(NewXSBTextCommand, "DO_ON_NEW_XSB_TEXT_CHUNK");
    StoreLuaChunk(KeyEventCommand, "DO_ON_KEYSTROKE_CHUNK");
    StoreLuaChunk(LuaMouseClickCommand, "DO_ON_MOUSE_CLICK_CHUNK");
    StoreLuaChunk(LuaMouseWheelCommand, "DO_ON_MOUSE_WHEEL_CHUNK");

    // clean up HID connections
    CloseAllOpenHIDDevices();
    hid_exit();

    // init the XSB connection
    XSBPluginId             = XPLMFindPluginBySignature("vatsim.protodev.clients.xsquawkbox");
    XSBInputUsrMsgXDataRef  = XPLMFindDataRef(XSB_INPUT_USER_MSG);
    XSBInputStringXDataRef  = XPLMFindDataRef(XSB_INPUT_STRING);
    XSBDestinationXDataRef  = XPLMFindDataRef(XSB_FP_ARRIVAL_AIRPORT);
    XSBAlternativeXDataRef  = XPLMFindDataRef(XSB_FP_ALTERNATE_AIRPORT);
    XSBStartAirportXDataRef = XPLMFindDataRef(XSB_FP_DEPARTURE_AIRPORT);
    XSBATCFreqXDataRef      = XPLMFindDataRef(XSB_ATC_FREQ);
    XSBATCCallsignXDataRef  = XPLMFindDataRef(XSB_ATC_CALLSIGN);
    if (XSBPluginId != XPLM_NO_PLUGIN_ID)
    {
        WeHaveXSB = true;
        XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SUBSCRIBE, (void*) XSB_TEXT);
        XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SUBSCRIBE, (void*) XSB_WEATHER);
        logMsg(logToAll, PLUGIN_NAME " V" PLUGIN_VERSION " successfully connected to XSquawkBox.");
        logMsg(logToSqkBox, "Type in \">helpme\" to learn about using Lua code in XSB.");
    } else
    {
        WeHaveXSB = false;
    }

    UserWantsANewPlane         = false;
    UserWantsToLoadASituation  = false;
    UserWantsToReplaceAircraft = false;
    UserWantsToMoveAircraft    = false;

    luaL_openlibs(FWLLua);
    RegisterEmbeddedModules(FWLLua);

    RegisterCoreCFunctionsToLua(FWLLua);


    // functions for fmod
    fmodint::RegisterFmodFunctionsToLua(FWLLua);

    // functions to operate on floating windows
    flwnd::initFloatingWindowSupport();

    lua_pushnumber(FWLLua, ++LuaResetCount);
    lua_setglobal(FWLLua, "LUA_RUN");

    int mouse_x, mouse_y;

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
    LAST_SCREEN_HEIGHT = LAST_SCREEN_HIGHT;
    lua_pushnumber(FWLLua, LAST_SCREEN_HEIGHT);
    lua_setglobal(FWLLua, "SCREEN_HEIGHT");

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
    int                   VersionXP, VersionSDK;
    XPLMHostApplicationID HostID;
    XPLMGetVersions(&VersionXP, &VersionSDK, &HostID);
    lua_pushnumber(FWLLua, VersionXP);
    lua_setglobal(FWLLua, "XPLANE_VERSION");
    lua_pushnumber(FWLLua, VersionSDK);
    lua_setglobal(FWLLua, "SDK_VERSION");
    lua_pushnumber(FWLLua, HostID);
    lua_setglobal(FWLLua, "XPLANE_HOSTID");
    lua_pushstring(FWLLua, PLUGIN_VERSION);
    lua_setglobal(FWLLua, "PLUGIN_VERSION");
    lua_pushstring(FWLLua, PLUGIN_VERSION_NO);
    lua_setglobal(FWLLua, "PLUGIN_VERSION_NO");
    lua_pushstring(FWLLua, PLUGIN_VERSION_BUILD);
    lua_setglobal(FWLLua, "PLUGIN_VERSION_BUILD");
    char dirSep[2];
    strcpy(dirSep, XPLMGetDirectorySeparator()); // correct for each OS
    lua_pushstring(FWLLua, dirSep);
    lua_setglobal(FWLLua, "DIRECTORY_SEPARATOR");
    lua_pushstring(FWLLua, systemDir.c_str());
    lua_setglobal(FWLLua, "SYSTEM_DIRECTORY");
    lua_pushstring(FWLLua, pluginMainDir.c_str());
    lua_setglobal(FWLLua, "PLUGIN_MAIN_DIRECTORY");
    lua_pushstring(FWLLua, scriptDir.c_str());
    lua_setglobal(FWLLua, "SCRIPT_DIRECTORY");
    lua_pushstring(FWLLua, internalsDir.c_str());
    lua_setglobal(FWLLua, "INTERNALS_DIRECTORY");
    lua_pushstring(FWLLua, modulesDir.c_str());
    lua_setglobal(FWLLua, "MODULES_DIRECTORY");
    lua_pushstring(FWLLua, quarantineDir.c_str());
    lua_setglobal(FWLLua, "QUARANTINE_DIRECTORY");
    char AircraftFileName[256];
    char AircraftPath[512];
    XPLMGetNthAircraftModel(0, AircraftFileName, AircraftPath);
    lua_pushstring(FWLLua, AircraftFileName);
    lua_setglobal(FWLLua, "AIRCRAFT_FILENAME");
    lua_pushstring(FWLLua, AircraftPath);
    lua_setglobal(FWLLua, "AIRCRAFT_PATH");

    LuaIsRunning = true;
    lua_getglobal(FWLLua, "XPLANE_LANGUAGE");

    std::ostringstream oss_success_cstring;
    oss_success_cstring << "FlyWithLua Info: Lua engine (re)started. LUA_RUN =" << LuaResetCount << ", SDK_VERSION = " << VersionSDK <<
                           ", XPLANE_VERSION = " << VersionXP << ", XPLANE_LANGUAGE = " << lua_tostring(FWLLua, -1) <<
                           " and XPLANE_HOSTID = " << HostID;
    logMsg(logToDevCon, oss_success_cstring.str());
    lua_settop(FWLLua, 0);

    // restart the HID device API
    int hid_checker = 0;
    hid_checker = hid_init();
    if (hid_checker == 0)
    {
        logMsg(logToDevCon, "FlyWithLua Info: HID access initialized.");
    } else
    {
        logMsg(logToDevCon, "FlyWithLua error: Can't initialize HIDAPI.");
    }
}

bool ReadPrefFile()
{
  process_read_ini_file();
  if (developer_mode == 0)
  {
    // No developer mode from prefs file so uncheck menu item.
    XPLMCheckMenuItem(FlyWithLuaMenuId, DevModeCheckedPosition, 1);
  } else {
    // Developer mode on from prefs file so check menu item.
    XPLMCheckMenuItem(FlyWithLuaMenuId, DevModeCheckedPosition, 2);
  }

  if (verbose_logging_mode == 0)
  {
     // No verbose mode from prefs file so uncheck menu item.
     XPLMCheckMenuItem(FlyWithLuaMenuId, VerboseModeCheckedPosition, 1);
  } else {
     // Developer mode on from prefs file so check menu item.
     XPLMCheckMenuItem(FlyWithLuaMenuId, VerboseModeCheckedPosition, 2);
  }
  return true;
}

bool ReadScriptFile(const char* FileNameToRead)
{
    if (!LuaIsRunning)
    {
        logMsg(logToDevCon, "FlyWithLua Error: ReadScriptFile() failed, Lua is not running, can't load script file.");
        logMsg(logToDevCon, FileNameToRead);
        return false;
    }

    CopyDataRefsToLua();
    if (luaL_dofile(FWLLua, FileNameToRead))
    {
        logMsg(logToDevCon,
               std::string("FlyWithLua Error: CopyDataRefsToLua() failed, can't execute script file: ").append(FileNameToRead));
        return false;
    }
    CopyDataRefsToXPlane();
    lua_gc(FWLLua, LUA_GCCOLLECT, 0);

    return true;
}

bool ReadAllScriptFiles()
{
    char FileToLoad[SHORTSRTING]      = "";
    char PlaneICAO[SHORTSRTING]       = "";
    char PlaneTailNumber[SHORTSRTING] = "";
    char PlaneDescrip[SHORTSRTING]    = "";
    char PlaneAuthor[SHORTSRTING]     = "";
    int  k;
    char FilesInFolder[5000];
    int NumberOfFiles; // modified by saar
    int TotalNumberOfFiles; // modified by saar
    char* FileIndex[250];
    int result;

    // get starting time
    clock_t time_start = clock();

    // starting the engine
    LuaIsRunning         = true;
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
    XPLMGetDatab(gPlaneDescrip, PlaneDescrip, 0, SHORTSRTING);
    lua_pushstring(FWLLua, PlaneDescrip);
    lua_setglobal(FWLLua, "PLANE_DESCRIP");
    XPLMGetDatab(gPlaneAuthor, PlaneAuthor, 0, SHORTSRTING);
    lua_pushstring(FWLLua, PlaneAuthor);
    lua_setglobal(FWLLua, "PLANE_AUTHOR");

    // run through the init script
    std::ostringstream oss_IntPathAndName;
    oss_IntPathAndName << internalsDir << "FlyWithLua.ini";
    auto IntPathAndName = oss_IntPathAndName.str();

    logMsg(logToDevCon, "FlyWithLua Info: FlyWithLua.ini full path ");
    logMsg(logToDevCon, IntPathAndName);
    if (ReadScriptFile(IntPathAndName.c_str()))
    {
        logMsg(logToDevCon, "FlyWithLua Info: Load ini file.");
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Unable to load ini file.");
        LuaIsRunning = false;
        return false;
    }

    if (XPLMGetDirectoryContents(scriptDir.c_str(), 0, FilesInFolder, sizeof(FilesInFolder), FileIndex, 250,
                                 reinterpret_cast<int *>(&TotalNumberOfFiles), reinterpret_cast<int *>(&NumberOfFiles)))
    {
        logMsg(logToDevCon, "FlyWithLua Info: Searching for Lua script files");

        // are enough files in the folder to read them out?
        if (NumberOfFiles == 0)
        {
            // nothing to load, let's start Lua without reading files
            logMsg(logToDevCon,
                   "FlyWithLua Info: The folder /Resources/plugins/FlyWithLua/Scripts/ does not exist or it is empty.");
            logMsg(logToDevCon, "FlyWithLua Info: Nothing to load at all, starting without reading files.");
            CrashReportDisplayed = false;
            if (bad_script_count > 0)
            {
                if (developer_mode == 0)
                {
                    XPLMSpeakString("found bad lua scripts that have been quarantined look in Log dot text file for more information");
                }
                bad_script_count = 0;
                speak_time = clock();
                std::ostringstream oss_speak_time;
                oss_speak_time << "FlyWithLua Info: speak_time " << speak_time;
                logMsg(logToDevCon, oss_speak_time.str());
            }
            return true;
        }

        logMsg(logToDevCon, "FlyWithLua Info: Sorting Lua script files");
        // sorting the files
        qsort(FileIndex, static_cast<size_t>(NumberOfFiles), sizeof(char*), compare_filenames);

        // reading out the files into a string step by step
        for (k = 0; k < NumberOfFiles; k++)
        {
            strncpy(FileToLoad, FileIndex[k], sizeof(FileToLoad));
            if ((FileToLoad[0] != '.') and
                ((strstr(FileToLoad, ".fwl") != nullptr and strlen(strstr(FileToLoad, ".fwl")) == 4)
                 or (strstr(FileToLoad, ".FWL") != nullptr and strlen(strstr(FileToLoad, ".FWL")) == 4)
                 #if defined(__LP64__) || defined(_WIN64)
                 or (strstr(FileToLoad, ".lua64") != nullptr and strlen(strstr(FileToLoad, ".lua64")) == 6)
                 or (strstr(FileToLoad, ".Lua64") != nullptr and strlen(strstr(FileToLoad, ".Lua64")) == 6)
                 or (strstr(FileToLoad, ".LUA64") != nullptr and strlen(strstr(FileToLoad, ".LUA64")) == 6)
                 #else
                 or (strstr(FileToLoad, ".lua32") != NULL and strlen(strstr(FileToLoad, ".lua32")) == 6)
                                            or (strstr(FileToLoad, ".Lua32") != NULL and strlen(strstr(FileToLoad, ".Lua32")) == 6)
                                            or (strstr(FileToLoad, ".LUA32") != NULL and strlen(strstr(FileToLoad, ".LUA32")) == 6)
                 #endif
                 or (strstr(FileToLoad, ".lua") != nullptr and strlen(strstr(FileToLoad, ".lua")) == 4)
                 or (strstr(FileToLoad, ".Lua") != nullptr and strlen(strstr(FileToLoad, ".Lua")) == 4)
                 or (strstr(FileToLoad, ".LUA") != nullptr and strlen(strstr(FileToLoad, ".LUA")) == 4)))
            {
                // load the script file
                std::ostringstream oss_ScrPathAndName;
                oss_ScrPathAndName << scriptDir << "/" << FileToLoad;
                auto ScrPathAndName = oss_ScrPathAndName.str();

                logMsg(logToDevCon, ("FlyWithLua Info: Start loading script file " + ScrPathAndName));
                // XPLMCheckMenuItemState(FlyWithLuaMenuId, DevModeCheckedPosition, &DevMode);
                if (ReadScriptFile(ScrPathAndName.c_str()))
                {
                    logMsg(logToDevCon,
                           ("FlyWithLua Info: Finished loading script file " + ScrPathAndName));
                } else
                {
                    logMsg(logToDevCon, ("FlyWithLua Error: Unable to load script file " + ScrPathAndName));
                    if (developer_mode == 1)
                    {
                        LuaIsRunning = false;
                        break;
                    }
                    if (developer_mode == 0)
                    {
                        // Need to move bad script to "Scripts (Quarantine)") and then run ReadAllScriptFiles().
                        std::ostringstream oss_QuaPathAndName;
                        oss_QuaPathAndName << quarantineDir << FileToLoad;
                        auto QuaPathAndName = oss_QuaPathAndName.str();
                        result = rename(ScrPathAndName.c_str(), QuaPathAndName.c_str());
                        if (result == 0)
                        {
                            logMsg(logToDevCon,
                                   ("FlyWithLua Info: 1st Moved Bad Script to " + QuaPathAndName));
                            bad_script_count = bad_script_count + 1;
                        }
                        else
                        {
                            logMsg(logToDevCon,
                                   ("FlyWithLua Info: Could not move bad script to " + QuaPathAndName));
                            LuaIsRunning = false;
                            // Added this break to prevent CTD from flooding the Log.txt file.
                            break;
                        }
                        LuaIsRunning = false;
                        return ReadAllScriptFiles();
                    }
                }

                // is Lua still running, or are there any problems?
                if (!LuaIsRunning)
                {
                    logMsg(logToDevCon,
                           ("FlyWithLua Error: The error seems to be inside of script file " + ScrPathAndName));
                    if (developer_mode == 1)
                    {
                        return false;
                    }
                    if (developer_mode == 0)
                    {
                        // Need to move bad script to "Scripts (Quarantine)") and then run ReadAllScriptFiles().
                        std::ostringstream oss_QuaPathAndName;
                        oss_QuaPathAndName << quarantineDir << FileToLoad;
                        auto QuaPathAndName = oss_QuaPathAndName.str();
                        result = rename(ScrPathAndName.c_str(), QuaPathAndName.c_str());
                        if (result == 0)
                        {
                            logMsg(logToDevCon,
                                   ("FlyWithLua Info: 2nd Moved Bad Script to " + QuaPathAndName));
                            bad_script_count = bad_script_count + 1;
                        }
                        else
                        {
                            logMsg(logToDevCon,
                                   ("FlyWithLua Info: Could not move bad script to " + QuaPathAndName));
                            LuaIsRunning = false;
                            // Added this break to prevent CTD from flooding the Log.txt file.
                            break;
                        }
                        ReadAllScriptFiles();
                        return false;
                    }
                }
            }
        }

    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Can't read out the subfolder Resources/plugins/FlyWithLua/Scripts.");
        logMsg(logToDevCon,
               "FlyWithLua Info: To fix this problem, simply create the subfolder Resources/plugins/FlyWithLua/Scripts.");
        LuaIsRunning = false;
        return false;
    }

    // get time after execution
    clock_t time_end = clock();

    // write time difference and memory usage into global variables
    if (LuaIsRunning)
    {
        lua_pushnumber(FWLLua, (double) (time_end - time_start) / CLOCKS_PER_SEC);
        lua_setglobal(FWLLua, "SCRIPTS_LOADING_TIME_SEC");
        lua_pushnumber(FWLLua, CLOCKS_PER_SEC);
        lua_setglobal(FWLLua, "CLOCKS_PER_SEC");
        if (openal_source_id_not_found == 1 || openal_buffer_id_not_found == 1)
        {
           logMsg(logToDevCon, "FlyWithLua Error: Had some issues loading the scripts will be reloading them.");
        }
        else
        {
            logMsg(logToDevCon, "FlyWithLua Info: All script files loaded successfully.");
        }
    }

    if (bad_script_count > 0)
    {
        bad_scripts_found = 1;
    }
    else if (found_bad_function_script)
    {
        bad_scripts_found = 1;
    }

    std::ostringstream oss_report_loding_time;
    oss_report_loding_time << "FlyWithLua Info: Loading time for all scripts is " << (double) (time_end - time_start) / CLOCKS_PER_SEC << " sec.";
    auto report_loding_time = oss_report_loding_time.str();
    logMsg(logToDevCon, report_loding_time.c_str());

    if ((bad_scripts_found == 1) && (first_pass == 0))
    {
        if (developer_mode == 0)
        {
            XPLMSpeakString("\n\n\n\nfound bad lua scripts that have been quarantined look in Log dot text file for more information");
        }
        bad_script_count = 0;
        bad_scripts_found = 0;
        first_pass = 1;
        send_delayed_quarantined_message();
    }
    if ((bad_scripts_found == 0) && (first_pass == 0))
    {
        bad_script_count = 0;
        bad_scripts_found = 0;
        first_pass = 1;
        send_delayed_quarantined_message();
    }

    return true; // snagar
}

bool ReadAllQuarantinedScriptFiles()
{
    char QtFileToLoad[SHORTSRTING]      = "";
    int  Qt_k;
    char QtFilesInFolder[5000];
    int NumberOfQtFiles; // modified by saar
    int TotalNumberOfQtFiles; // modified by saar
    char* QtFileIndex[250];
    int Qt_result;

    if (XPLMGetDirectoryContents(quarantineDir.c_str(), 0, QtFilesInFolder, sizeof(QtFilesInFolder), QtFileIndex, 250,
                                 reinterpret_cast<int *>(&TotalNumberOfQtFiles), reinterpret_cast<int *>(&NumberOfQtFiles)))
    {
        logMsg(logToDevCon, "FlyWithLua Info: Searching for Lua quarantined script files");

        // are enough files in the folder to read them out?
        if (NumberOfQtFiles == 1)
        {
            // nothing to load, let's start Lua without reading files
            logMsg(logToDevCon,
                   "FlyWithLua Info: The folder /Resources/plugins/FlyWithLua/Scripts (Quarantine)/ does not exist or it is empty.");

            return true;
        }

        if ((NumberOfQtFiles > 1) && (speak_second_warning == 1))
        {
            if (developer_mode == 0)
            {
                XPLMSpeakString("\n\n\n\nPlease check your quarantined scripts folder");
            }
            speak_second_warning = 0;
            return true;
        }

        logMsg(logToDevCon, "FlyWithLua Info: Sorting Lua quarantined script files");
        // sorting the files
        qsort(QtFileIndex, static_cast<size_t>(NumberOfQtFiles), sizeof(char*), compare_filenames);

        // reading out the files into a string step by step
        for (Qt_k = 0; Qt_k < NumberOfQtFiles; Qt_k++)
        {
            strncpy(QtFileToLoad, QtFileIndex[Qt_k], sizeof(QtFileToLoad));
            if ((QtFileToLoad[0] != '.') and
                ((strstr(QtFileToLoad, ".fwl") != nullptr and strlen(strstr(QtFileToLoad, ".fwl")) == 4)
                 or (strstr(QtFileToLoad, ".FWL") != nullptr and strlen(strstr(QtFileToLoad, ".FWL")) == 4)
                 #if defined(__LP64__) || defined(_WIN64)
                 or (strstr(QtFileToLoad, ".lua64") != nullptr and strlen(strstr(QtFileToLoad, ".lua64")) == 6)
                 or (strstr(QtFileToLoad, ".Lua64") != nullptr and strlen(strstr(QtFileToLoad, ".Lua64")) == 6)
                 or (strstr(QtFileToLoad, ".LUA64") != nullptr and strlen(strstr(QtFileToLoad, ".LUA64")) == 6)
                 #else
                 or (strstr(QtFileToLoad, ".lua32") != NULL and strlen(strstr(QtFileToLoad, ".lua32")) == 6)
                                            or (strstr(QtFileToLoad, ".Lua32") != NULL and strlen(strstr(QtFileToLoad, ".Lua32")) == 6)
                                            or (strstr(QtFileToLoad, ".LUA32") != NULL and strlen(strstr(QtFileToLoad, ".LUA32")) == 6)
                 #endif
                 or (strstr(QtFileToLoad, ".lua") != nullptr and strlen(strstr(QtFileToLoad, ".lua")) == 4)
                 or (strstr(QtFileToLoad, ".Lua") != nullptr and strlen(strstr(QtFileToLoad, ".Lua")) == 4)
                 or (strstr(QtFileToLoad, ".LUA") != nullptr and strlen(strstr(QtFileToLoad, ".LUA")) == 4)))
            {
                // load the quarantined script file
                std::ostringstream oss_QtPathAndName;
                oss_QtPathAndName << quarantineDir << QtFileToLoad;
                auto QtPathAndName = oss_QtPathAndName.str();

                // Need to move quarantined script from "Scripts (Quarantine)" to "Scripts"
                // After all quarantined scripts have been moved and some delay run ReadAllScriptFiles().
                std::ostringstream oss_OkPathAndName;
                oss_OkPathAndName << scriptDir << "/" << QtFileToLoad;
                auto OkPathAndName = oss_OkPathAndName.str();

                Qt_result = rename(QtPathAndName.c_str(), OkPathAndName.c_str());
                if (Qt_result == 0)
                {
                    logMsg(logToDevCon,
                           ("FlyWithLua Info: Returning Quarantine Script " + OkPathAndName));
                }
                else
                {
                    logMsg(logToDevCon,
                           ("FlyWithLua Info: Could Not Return Quarantine Script " + OkPathAndName));
                }
            }
        }
    }

    return true; // snagar
}


void send_delayed_quarantined_message()
{
    XPLMRegisterFlightLoopCallback(DelayedQuarantinedMessage_Callback, 20, nullptr);
}

float DelayedQuarantinedMessage_Callback(float /*inElapsed1*/, float /*inElapsed2*/,
                               int /*cntr*/, void * /*ref*/)
{
    speak_second_warning = 1;
    ReadAllQuarantinedScriptFiles();
    return 0;
}

void update_Lua_dataref_variables(XPLMDataRef DataRefID, int Index, float Value)
{
    if (!LuaIsRunning) return;
    if (DataRefTableLastElement < 0) return;
    for (auto i = 0; i <= DataRefTableLastElement; i++)
    {
        if ((DataRefTable[i].DataRefId == DataRefID) && (DataRefTable[i].Index == Index))
        {
            lua_pushnumber(FWLLua, Value);
            lua_setglobal(FWLLua, DataRefTable[i].LuaVariable);
        }
    }
}

void update_Lua_dataref_strings(XPLMDataRef DataRefID, int Index, char* ValueString)
{
    if (!LuaIsRunning) return;
    if (DataRefTableLastElement < 0) return;
    for (auto i = 0; i <= DataRefTableLastElement; i++)
    {
        if ((DataRefTable[i].DataRefId == DataRefID) && (DataRefTable[i].Index == Index))
        {
            lua_pushstring(FWLLua, ValueString);
            lua_setglobal(FWLLua, DataRefTable[i].LuaVariable);
        }
    }
}


PLUGIN_API int XPluginStart(
        char* outName,
        char* outSig,
        char* outDesc)
{
    // int    i;


    // Plugin Info
    strcpy(outName, PLUGIN_NAME " " PLUGIN_VERSION);
    strcpy(outSig, "CarstenLynker.FlyWithLua.NG+");
    strcpy(outDesc, PLUGIN_DESCRIPTION);

    // use posix path on Mac OSX
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

    fmodint::RegisterAccessor();

    initPluginDirectory(); // snagar

//#ifndef APL
    // init the HID access
    hid_init();
//#endif

    // DataRefs we want to use
    gXSBMetarStringXDataRef    = XPLMFindDataRef(XSB_WEATHER_METAR);
    gXSBTextMessageXDataRef    = XPLMFindDataRef(XSB_TEXT_MESSAGE);
    gXSBTextFromXDataRef       = XPLMFindDataRef(XSB_TEXT_FROM);
    gXSBTextFreqsXDataRef      = XPLMFindDataRef(XSB_TEXT_FREQS);
    gXSBTextUseXDataRef        = XPLMFindDataRef(XSB_TEXT_USE);
    gJoystickAxisAssignments   = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
    gJoystickButtonAssignments = XPLMFindDataRef("sim/joystick/joystick_button_assignments");
    gJoystickAxisReverse       = XPLMFindDataRef("sim/joystick/joystick_axis_reverse");
    gJoystickAxisValues        = XPLMFindDataRef("sim/joystick/joystick_axis_values");
    gJoystickButtonValues      = XPLMFindDataRef("sim/joystick/joystick_button_values");
    gPlaneICAO                 = XPLMFindDataRef("sim/aircraft/view/acf_ICAO");
    gPlaneTailNumber           = XPLMFindDataRef("sim/aircraft/view/acf_tailnum");
    gPlaneDescrip              = XPLMFindDataRef("sim/aircraft/view/acf_descrip");
    gPlaneAuthor               = XPLMFindDataRef("sim/aircraft/view/acf_author");

    // Lua run numbering starts at zero (no runs) when X-Plane starts
    LuaResetCount = 0;

    // init the menu item index
    FlyWithLuaMenuItem = -1;
    ATCMenuItem        = -1;
    MacroMenuItem      = -1;

#if LIN
    // init glut to prevent error on Linux
    int fwl_argc=1;
    char *fwl_argv[1] = {(char*)"FlyWithLua"};
    glutInit(&fwl_argc, fwl_argv);
#endif

    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    // clean HID device connections and free memory
    hid_exit();

    // release memory for Fmod buffers
    fmodint::deinitFmodSupport();
    // This does not work here and not sure why
    // I think after this call fmod is not getting initalized.
    // fmodint::fmod_uninitialize();

    // cleanup sound system
    if (my_context)
    {
        printf("0x%08x: deleting my context %p\n", XPLMGetMyID(), my_context);
        alcDestroyContext(my_context);
    }
    if (my_device) alcCloseDevice(my_device);
}


PLUGIN_API void XPluginDisable(void)
{
    flwnd::deinitFloatingWindowSupport();

    // run through the exit script
    if (ReadScriptFile("Resources/plugins/FlyWithLua/Internals/FlyWithLua.exit"))
    {
        logMsg(logToDevCon, "FlyWithLua Info: Exit file loaded.");
    } else
    {
        logMsg(logToDevCon, "FlyWithLua Error: Unable to load exit file.");
    }

    // close Lua
    LuaIsRunning = false;
    lua_close(FWLLua);

    // killing all commands
    if (CommandTableLastElement > -1)
    {
        for (auto i = 0; i <= CommandTableLastElement; i++)
        {
            XPLMUnregisterCommandHandler(CommandTable[i].Reference, FWLCommandCallback, 1,
                                         (void*) "FlyWithLuaCommand");
            CommandTable[i].Reference = nullptr;
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

    // release memory for Fmod buffers
    fmodint::deinitFmodSupport();
    // This does not work here and not sure why
    // I think after this call fmod is not getting initalized.
    // fmodint::fmod_uninitialize();

    // unsubscribe messages from/to XSquawkBox
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_UNSUBSCRIBE, (void*) XSB_TEXT);
    XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_UNSUBSCRIBE, (void*) XSB_WEATHER);

    // destroy the mouse event window
    XPLMDestroyWindow(FWLMouseEventWindowID);

    // remove the menus
    XPLMDestroyMenu(FlyWithLuaMenuId);
    XPLMDestroyMenu(ATCMenuId);
    XPLMDestroyMenu(MacroMenuId);
    FlyWithLuaMenuId = nullptr;
    ATCMenuId        = nullptr;
    MacroMenuId      = nullptr;

    // unregister callbacks
    XPLMUnregisterDrawCallback(FWLDrawWindowCallback, xplm_Phase_Window, 0, (void*) "FWLWindowDrawer");
    XPLMUnregisterKeySniffer(FWLKeySniffer, 0, (void*) "FWLKeySniffer");
    XPLMUnregisterCommandHandler(MyReloadScriptsCommand, MyReloadScriptsCommandHandler, 0, nullptr);
    XPLMUnregisterFlightLoopCallback(MyFastLoopCallback, nullptr);
    XPLMUnregisterFlightLoopCallback(MySlowLoopCallback, nullptr);
    XPLMUnregisterFlightLoopCallback(MyEveryFrameLoopCallback, nullptr);
	
	// Unregister and clean up the AfterFlightModel flight loop
	if (g_DoEveryFrameAfter_ID != nullptr)
	{
		XPLMDestroyFlightLoop(g_DoEveryFrameAfter_ID);
		g_DoEveryFrameAfter_ID = nullptr;
		do_every_frame_after_code.clear();
	}
	

    // write to Log.txt
    logMsg(logToDevCon, "FlyWithLua Info: FlyWithLua plugin disabled.");
}

PLUGIN_API int XPluginEnable(void)
{
    // init screen size
    XPLMGetScreenSize(&LAST_SCREEN_WIDTH, &LAST_SCREEN_HIGHT);

    // init buttons
    for (auto i = 0; i < MAXJOYSTICKBUTTONS; i++)
    {
        JoystickButtonValues[i]     = 0;
        JoystickButtonLastValues[i] = 0;
    }

    // init CommandTable
    for (auto& command : CommandTable)
    {
        command.Reference = nullptr;
        command.Name.erase();
        command.Description.erase();
        command.BeginCommand.erase();
        command.ContinueCommand.erase();
        command.EndCommand.erase();
    }

    // The commands
    MyReloadScriptsCommand = XPLMCreateCommand("FlyWithLua/debugging/reload_scripts", "Reload all Lua script files.");


    // Now tell X-Plane about our commands
    XPLMRegisterCommandHandler(MyReloadScriptsCommand,            // in Command name
                               MyReloadScriptsCommandHandler,     // in Handler
                               0,                                 // Receive input when plugin windows.
                               nullptr);                       // inRefcon.

    // Check in the main functions
    XPLMRegisterFlightLoopCallback(
            MyFastLoopCallback,        /* Callback */
            TimeBetweenCallbacks,    /* Interval */
            nullptr);                /* refcon not used. */

    XPLMRegisterFlightLoopCallback(
            MySlowLoopCallback,    /* Callback */
            TimeBetweenCallbacks,    /* Interval */
            nullptr);                /* refcon not used. */

    XPLMRegisterFlightLoopCallback(
            MyEveryFrameLoopCallback,  /* Callback */
            -1,                        /* every frame */
            nullptr);                     /* refcon not used. */

    XPLMRegisterDrawCallback(FWLDrawWindowCallback, xplm_Phase_Window, 0, (void*) "FWLWindowDrawer");
	
	Register_Do_Every_Frame_After(); // Provide a callback that runs in FlightLoop Phase After mode.

    // create the FlyWithLua menu inside the plugin menu
    if (FlyWithLuaMenuItem < 0)
        FlyWithLuaMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "FlyWithLua", (void*) "None", 1);
    FlyWithLuaMenuId       = XPLMCreateMenu("FlyWithLua", XPLMFindPluginsMenu(), FlyWithLuaMenuItem,
                                            FlyWithLuaMenuHandler,
                                            nullptr);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Reload all Lua script files", (void*) "Reload", 1);
    XPLMAppendMenuSeparator(FlyWithLuaMenuId);

    // create the macro and ATC menus inside the FlyWithLua menu
    if (MacroMenuItem < 0)
        MacroMenuItem = XPLMAppendMenuItem(FlyWithLuaMenuId, "FlyWithLua Macros", (void*) "None", 1);
    MacroMenuId       = XPLMCreateMenu("FlyWithLua Macros", FlyWithLuaMenuId, MacroMenuItem, MacroMenuHandler, nullptr);
    if (ATCMenuItem < 0) ATCMenuItem = XPLMAppendMenuItem(FlyWithLuaMenuId, "FlyWithLua ATC", (void*) "None", 1);
    ATCMenuId = XPLMCreateMenu("FlyWithLua ATC", FlyWithLuaMenuId, ATCMenuItem, MacroMenuHandler, nullptr);

    // add some diagnostic menu items
    XPLMAppendMenuSeparator(FlyWithLuaMenuId);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Write Debug file", (void*) "Debug", 1);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Stop the Lua engine", (void*) "Stop", 1);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Try to resume Lua engine (not recommended)", (void*) "Resume", 1);
    XPLMAppendMenuSeparator(FlyWithLuaMenuId);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Return all quarantined Lua scripts", (void*) "ReturnQt", 1);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Disable moving bad scripts to Quarantine", (void*) "DevMode", 1);
    XPLMCheckMenuItem(FlyWithLuaMenuId, DevModeCheckedPosition, 1);
    XPLMAppendMenuSeparator(FlyWithLuaMenuId);
    XPLMAppendMenuItem(FlyWithLuaMenuId, "Enable Verbose Logging Mode", (void*) "VerboseMode", 1);
    XPLMCheckMenuItem(FlyWithLuaMenuId, VerboseModeCheckedPosition, 1);

    // init the XSB connection
    XSBPluginId             = XPLMFindPluginBySignature("vatsim.protodev.clients.xsquawkbox");
    XSBInputUsrMsgXDataRef  = XPLMFindDataRef(XSB_INPUT_USER_MSG);
    XSBInputStringXDataRef  = XPLMFindDataRef(XSB_INPUT_STRING);
    XSBDestinationXDataRef  = XPLMFindDataRef(XSB_FP_ARRIVAL_AIRPORT);
    XSBAlternativeXDataRef  = XPLMFindDataRef(XSB_FP_ALTERNATE_AIRPORT);
    XSBStartAirportXDataRef = XPLMFindDataRef(XSB_FP_DEPARTURE_AIRPORT);
    XSBATCFreqXDataRef      = XPLMFindDataRef(XSB_ATC_FREQ);
    XSBATCCallsignXDataRef  = XPLMFindDataRef(XSB_ATC_CALLSIGN);
    if (XSBPluginId != XPLM_NO_PLUGIN_ID)
    {
        WeHaveXSB = true;
        XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SUBSCRIBE, (void*) XSB_TEXT);
        XPLMSendMessageToPlugin(XSBPluginId, XSB_CMD_SUBSCRIBE, (void*) XSB_WEATHER);
        logMsg(logToAll, PLUGIN_NAME " V" PLUGIN_VERSION " successfully connected to XSquawkBox.");
        logMsg(logToSqkBox, "Type in \">helpme\" to learn about using Lua code in XSB.");
    } else
    {
        WeHaveXSB = false;
    }

    // Initialize OpenAL sound
    init_openal_sound();

    // Starting the Lua engine
    /* use the default allocator. */
    FWLLua = luaL_newstate();
    sol::set_default_state(FWLLua);

    luaL_openlibs(FWLLua);

    // reload all script files, if the plugin is re-enabled via X-Plane's plugin manager menu
    if (LuaResetCount > 0 || XPLMGetCycleNumber() > 0)
    {
        ReadAllScriptFiles();
    }

    //register the keysniffer
    XPLMRegisterKeySniffer(FWLKeySniffer, 0, (void*) "FWLKeySniffer");

    //register the mouse capture window
    XPLMCreateWindow_t MouseWindowData;
    MouseWindowData.structSize = sizeof(MouseWindowData);
    MouseWindowData.left       = 0;
    MouseWindowData.bottom     = 0;
    XPLMGetScreenSize(&MouseWindowData.right, &MouseWindowData.top);
    MouseWindowData.visible                  = 1;
    MouseWindowData.drawWindowFunc           = FWLMouseEventWindowDraw;
    MouseWindowData.handleKeyFunc            = FWLMouseEventWindowKey;
    MouseWindowData.handleMouseClickFunc     = FWLMouseEventWindowMouse;
    MouseWindowData.handleMouseWheelFunc     = FWLMouseEventWindowMouseWheel;
    MouseWindowData.handleCursorFunc         = FWLMouseEventWindowCursor;
    MouseWindowData.handleRightClickFunc     = FWLMouseEventWindowRightMouse;
    MouseWindowData.layer                    = xplm_WindowLayerFloatingWindows;
    MouseWindowData.decorateAsFloatingWindow = xplm_WindowDecorationNone;
    MouseWindowData.refcon                   = nullptr;
    FWLMouseEventWindowID = XPLMCreateWindowEx(&MouseWindowData);

    return 1;
}

PLUGIN_API void XPluginReceiveMessage(
        XPLMPluginID inFrom,
        intptr_t inMessage,
        void* inParam)
{
    if(XPLM_PLUGIN_XPLANE == inFrom)
    {
        if(XPLM_MSG_FMOD_BANK_LOADED == inMessage && xplm_RadioBank == reinterpret_cast<uintptr_t>(inParam))
        {
            logMsg(logToDevCon, "FlyWithLua Info: XPLM_MSG_FMOD_BANK_LOADED & xplm_RadioBank  true");
            fmodint::fmod_initialization();
        }

        if(XPLM_MSG_FMOD_BANK_UNLOADING == inMessage && xplm_RadioBank == reinterpret_cast<uintptr_t>(inParam))
        {
            logMsg(logToDevCon, "FlyWithLua Info: XPLM_MSG_FMOD_BANK_UNLOADING & xplm_RadioBank  true");
            fmodint::fmod_uninitialize();
        }
    }

    char        XSBString[500];
    //char    ReadText[NORMALSTRING];
    //char    ReadText2[NORMALSTRING];
    //int     ReadInt;
    //float   ReadFloat;
    std::string StringToChange;
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
        logMsg(logToDevCon,
               "FlyWithLua: User switched to a new airport (or changed the plane). Script files have to be reloaded.");
        ReadPrefFile();
        ReadAllScriptFiles();
    }
    if (inMessage == XSB_MSG_TEXT)
    {
        char ReadXSBString[LONGSTRING];
        int  ReadXSBInteger;
        if (LuaIsRunning)
        {
            XPLMGetDatab(gXSBTextMessageXDataRef, &ReadXSBString, 0, LONGSTRING);
            lua_pushstring(FWLLua, ReadXSBString);
            lua_setglobal(FWLLua, "XSB_TEXT_MESSAGE");

            XPLMGetDatab(gXSBTextFromXDataRef, &ReadXSBString, 0, LONGSTRING);
            lua_pushstring(FWLLua, ReadXSBString);
            lua_setglobal(FWLLua, "XSB_TEXT_FROM");

            XPLMGetDatavi(gXSBTextFreqsXDataRef, &ReadXSBInteger, 0, 1);
            lua_pushnumber(FWLLua, ReadXSBInteger);
            lua_setglobal(FWLLua, "XSB_TEXT_FREQ");

            ReadXSBInteger = XPLMGetDatai(gXSBTextUseXDataRef);
            lua_pushnumber(FWLLua, ReadXSBInteger);
            lua_setglobal(FWLLua, "XSB_TEXT_USE");

            RunLuaChunk("DO_ON_NEW_XSB_TEXT_CHUNK");

            lua_getglobal(FWLLua, "XSB_TEXT_USE");
            XPLMSetDatai(gXSBTextUseXDataRef, (int) lua_tonumber(FWLLua, -1));
            lua_pop(FWLLua, 1);
        }
    }
    if (inMessage == XSB_MSG_METAR)
    {
        char ReadMetar[LONGSTRING];
        XPLMGetDatab(gXSBMetarStringXDataRef, ReadMetar, 0, LONGSTRING);
        if (LuaIsRunning)
        {
            lua_pushstring(FWLLua, ReadMetar);
            lua_setglobal(FWLLua, "XSB_METAR");
        }
        NewMetarCountdown = 0;
    }
    if (inMessage == XSB_MSG_USER_COMMAND)
    {
        XSBString[XPLMGetDatab(XSBInputStringXDataRef, XSBString, 0, sizeof(XSBString) - 1)] = 0;
        std::string StringToChange(XSBString);
        if (StringToChange == ">helpme")
        {
            logMsg(logToSqkBox,
                   "---------8<------------------------------------------------------------------------------");
            logMsg(logToSqkBox, "You can directly enter a Lua line of code, just by starting with \">\"");
            logMsg(logToSqkBox, "(a greater than char followed by the Lua code).");
            logMsg(logToSqkBox, "Example:");
            logMsg(logToSqkBox, R"(>DataRef( "Strobe", "sim/cockpit/electrical/strobe_lights_on", "writable" ))");
            logMsg(logToSqkBox, ">Strobe = 1");
            logMsg(logToSqkBox,
                   "-------------------------------------------------------------------->8-------------------");
            XPLMSetDatab(XSBInputStringXDataRef, (void*) "", 0, 0);
            return;
        }
        if (XSBString[0] == '>')
        {
            if (!StringToChange.empty())
            {
                RunLuaString(StringToChange.substr(1));
                XPLMSetDatab(XSBInputStringXDataRef, (void*) "", 0, 0);
            }
            return;
        }
    }
}


// to be processed not so often
float MySlowLoopCallback(
        float /*inElapsedSinceLastCall*/,
        float /*inElapsedTimeSinceLastFlightLoop*/,
        int /*inCounter*/,
        void* /*inRefcon*/)
{
    // get time before execution
    clock_t time_start = clock();

    if ((LongTimeCallbackCommand.length() > 1) && LuaIsRunning)
    {
        RunLuaChunk("DO_SOMETIMES_CHUNK");
    }

    // get time after execution
    clock_t time_end = clock();

    // write time difference and memory usage into global variables
    if (LuaIsRunning)
    {
        lua_pushnumber(FWLLua, (double) (time_end - time_start) / CLOCKS_PER_SEC);
        lua_setglobal(FWLLua, "DO_SOMETIMES_TIME_SEC");
    }

    return LongTimeBetweenCallbacks;
}

// Now follows the fast part of it
float MyFastLoopCallback(
        float /*inElapsedSinceLastCall*/,
        float /*inElapsedTimeSinceLastFlightLoop*/,
        int /*inCounter*/,
        void* /*inRefcon*/)
{
    // get time before execution
    clock_t time_start = clock();

    // stack overflow? pull the emergency brake!
    if ((lua_gettop(FWLLua) > 100) && LuaIsRunning)
    {
        logMsg(logToDevCon, "FlyWithLua Debug Info: Stack overflow, more than 100 elements on stack.");
        LuaIsRunning = false;
    }
    if (UserWantsANewPlane)
    {
        UserWantsANewPlane         = false;
        UserWantsToLoadASituation  = false;
        UserWantsToReplaceAircraft = false;
        UserWantsToMoveAircraft    = false;
        LuaIsRunning               = false;
        XPLMSetUsersAircraft(UserWantedFilename);
        return TimeBetweenCallbacks;
    }
    if (UserWantsToReplaceAircraft)
    {
        UserWantsANewPlane         = false;
        UserWantsToLoadASituation  = false;
        UserWantsToReplaceAircraft = false;
        UserWantsToMoveAircraft    = false;
        LuaIsRunning               = false;
        XPLMPlaceUserAtAirport(UserWantedFilename);
        return TimeBetweenCallbacks;
    }
    if (UserWantsToMoveAircraft)
    {
        UserWantsANewPlane         = false;
        UserWantsToLoadASituation  = false;
        UserWantsToReplaceAircraft = false;
        UserWantsToMoveAircraft    = false;
        LuaIsRunning               = false;
        XPLMPlaceUserAtLocation(
            UserWantedPosHdgSpd[0],
            UserWantedPosHdgSpd[1],
            UserWantedPosHdgSpd[2],
            UserWantedPosHdgSpd[3],
            UserWantedPosHdgSpd[4]
        );
        return TimeBetweenCallbacks;
    }
    if (UserWantsToLoadASituation)
    {
        UserWantsANewPlane         = false;
        UserWantsToLoadASituation  = false;
        UserWantsToReplaceAircraft = false;
        UserWantsToMoveAircraft    = false;
        LuaIsRunning               = false;
        XPLMLoadDataFile(xplm_DataFile_Situation, UserWantedFilename);
        logMsg(logToDevCon, "FlyWithLua: A situation file was loaded. Script files have to be reloaded.");
        ReadAllScriptFiles();
        return TimeBetweenCallbacks;
    }
    if ((CallbackCommand.length() > 1) && LuaIsRunning)
    {
        RunLuaChunk("DO_OFTEN_CHUNK");
    }
    if (!LuaIsRunning && !CrashReportDisplayed)
    {
        // TODO: Reconcile this code with flywithlua::panic()
        CrashReportDisplayed = true;
        int StackSize = lua_gettop(FWLLua);
        if (StackSize == 0)
        {
            logMsg(logToDevCon, "FlyWithLua Debug Info: Sorry, no debug Info on stack.");
        } else
        {
            logMsg(logToDevCon, "FlyWithLua Debug Info: The Lua stack contains the following elements:");
            for (auto i = 1; i <= StackSize; i++)
            {
                if (lua_isstring(FWLLua, i))
                {
                    logMsg(logToDevCon, lua_tostring(FWLLua, i));
                }
            }
        }
        DebugLua();
        luaL_dostring(FWLLua, "FLYWITHLUA_DEBUG()");
    }

    // get time after execution
    clock_t time_end = clock();

    // write time difference and memory usage into global variables
    if (LuaIsRunning)
    {
        lua_pushnumber(FWLLua, (double) (time_end - time_start) / CLOCKS_PER_SEC);
        lua_setglobal(FWLLua, "DO_OFTEN_TIME_SEC");
    }

    // reload all script files, if Could not find OpenAL source or buffer id
    if (openal_source_id_not_found == 1 || openal_buffer_id_not_found == 1)
    {
        if (openal_source_id_not_found == 1)
        {
            XPLMDebugString("FlyWithLua Error: OpenAL source ID not found reloading scripts\n");
            openal_source_id_not_found = 0;
        }

        if (openal_buffer_id_not_found == 1)
        {
            XPLMDebugString("FlyWithLua Error: OpenAL buffer ID not found reloading scripts\n");
            openal_buffer_id_not_found = 0;
        }

        ReadAllScriptFiles();
        return TimeBetweenCallbacks;
    }

    fmodint::fmod_data_update();
    flwnd::onFlightLoop();
    // fmodint::fmod_initialization();

    return TimeBetweenCallbacks;
}

// calculate the median of three values
float CalculateMedianOfThree(float a, float b, float c)
{
    if (((a >= b) && (a <= c)) || ((a <= b) && (a >= c)))
        return a;
    if (((b >= a) && (b <= c)) || ((b <= a) && (b >= c)))
        return b;
    return c;
}

// switch some DataRefs
void ExecuteSwitches()
{
    float FloatToSet;
    int   IntToSet;

    if (SwitchTableLastElement < 0) return;

    for (auto i = 0; i <= SwitchTableLastElement; i++)
    {
        if (SwitchTable[i].SwitchType == Switch)
        {
            if (JoystickButtonValues[SwitchTable[i].button] == 0)
            {
                if (SwitchTable[i].DataRefType == xplmType_Int)
                    XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].off_int);
                else if (SwitchTable[i].DataRefType == xplmType_Float)
                    XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].off_float);
                else if (SwitchTable[i].DataRefType == xplmType_Double)
                    XPLMSetDatad(SwitchTable[i].DataRefID, SwitchTable[i].off_double);
                else if (SwitchTable[i].DataRefType == xplmType_IntArray)
                    XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].off_int, SwitchTable[i].index, 1);
                else if (SwitchTable[i].DataRefType == xplmType_FloatArray)
                    XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].off_float, SwitchTable[i].index, 1);
            } else
            {
                if (SwitchTable[i].DataRefType == xplmType_Int)
                    XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].on_int);
                else if (SwitchTable[i].DataRefType == xplmType_Float)
                    XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].on_float);
                else if (SwitchTable[i].DataRefType == xplmType_Double)
                    XPLMSetDatad(SwitchTable[i].DataRefID, SwitchTable[i].on_double);
                else if (SwitchTable[i].DataRefType == xplmType_IntArray)
                    XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].on_int, SwitchTable[i].index, 1);
                else if (SwitchTable[i].DataRefType == xplmType_FloatArray)
                    XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].on_float, SwitchTable[i].index, 1);
            }
        }
        else if (SwitchTable[i].SwitchType == AxisMedian)
        {
            // catch a new axis value and shift all old values
            SwitchTable[i].on_float          = SwitchTable[i].off_float;
            SwitchTable[i].off_float         = SwitchTable[i].upper_limit_float;
            SwitchTable[i].upper_limit_float = SwitchTable[i].lower_limit_float;
            SwitchTable[i].lower_limit_float = SwitchTable[i].stepping_float;
            XPLMGetDatavf(gJoystickAxisValues, &SwitchTable[i].stepping_float, SwitchTable[i].index, 1);
            // calculate the median
            float median = CalculateMedianOfThree(SwitchTable[i].on_float, SwitchTable[i].upper_limit_float,
                                                  SwitchTable[i].stepping_float);
            // push it into a Lua variable
            lua_pushnumber(FWLLua, median);
            lua_setglobal(FWLLua, SwitchTable[i].DataRefName.c_str());
        }
        else if ((SwitchTable[i].SwitchType == PositiveEdge) && (JoystickButtonValues[SwitchTable[i].button] == 1) &&
            (JoystickButtonLastValues[SwitchTable[i].button] == 0))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
                XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].on_int);
            else if (SwitchTable[i].DataRefType == xplmType_Float)
                XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].on_float);
            else if (SwitchTable[i].DataRefType == xplmType_Double)
                XPLMSetDatad(SwitchTable[i].DataRefID, SwitchTable[i].on_double);
            else if (SwitchTable[i].DataRefType == xplmType_IntArray)
                XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].on_int, SwitchTable[i].index, 1);
            else if (SwitchTable[i].DataRefType == xplmType_FloatArray)
                XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].on_float, SwitchTable[i].index, 1);
        }
        else if ((SwitchTable[i].SwitchType == NegativeEdge) && (JoystickButtonValues[SwitchTable[i].button] == 0) &&
            (JoystickButtonLastValues[SwitchTable[i].button] == 1))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
                XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].on_int);
            else if (SwitchTable[i].DataRefType == xplmType_Float)
                XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].on_float);
            else if (SwitchTable[i].DataRefType == xplmType_Double)
                XPLMSetDatai(SwitchTable[i].DataRefID, static_cast<int>(SwitchTable[i].on_double));
            else if (SwitchTable[i].DataRefType == xplmType_IntArray)
                XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].on_int, SwitchTable[i].index, 1);
            else if (SwitchTable[i].DataRefType == xplmType_FloatArray)
                XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].on_float, SwitchTable[i].index, 1);
        }
        else if (((SwitchTable[i].SwitchType == PositiveFlip) && (JoystickButtonValues[SwitchTable[i].button] == 1) &&
             (JoystickButtonLastValues[SwitchTable[i].button] == 0))
            || ((SwitchTable[i].SwitchType == NegativeFlip) && (JoystickButtonValues[SwitchTable[i].button] == 0) &&
                (JoystickButtonLastValues[SwitchTable[i].button] == 1)))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
            {
                if (XPLMGetDatai(SwitchTable[i].DataRefID) == SwitchTable[i].off_int)
                    XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].on_int);
                else XPLMSetDatai(SwitchTable[i].DataRefID, SwitchTable[i].off_int);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Float)
            {
                if (XPLMGetDataf(SwitchTable[i].DataRefID) == SwitchTable[i].off_float)
                    XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].on_float);
                else XPLMSetDataf(SwitchTable[i].DataRefID, SwitchTable[i].off_float);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Double)
            {
                if (XPLMGetDatad(SwitchTable[i].DataRefID) == SwitchTable[i].off_double)
                    XPLMSetDatad(SwitchTable[i].DataRefID, SwitchTable[i].on_double);
                else XPLMSetDatad(SwitchTable[i].DataRefID, SwitchTable[i].off_double);
            }
            else if (SwitchTable[i].DataRefType == xplmType_IntArray)
            {
                int check_value_int;
                XPLMGetDatavi(SwitchTable[i].DataRefID, &check_value_int, SwitchTable[i].index, 1);
                if (check_value_int == SwitchTable[i].off_int)
                    XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].on_int, SwitchTable[i].index, 1);
                else XPLMSetDatavi(SwitchTable[i].DataRefID, &SwitchTable[i].off_int, SwitchTable[i].index, 1);
            }
            else if (SwitchTable[i].DataRefType == xplmType_FloatArray)
                XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].on_float, SwitchTable[i].index, 1);
            {
                float check_value_float;
                XPLMGetDatavf(SwitchTable[i].DataRefID, &check_value_float, SwitchTable[i].index, 1);
                if (check_value_float == SwitchTable[i].off_float)
                    XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].on_float, SwitchTable[i].index, 1);
                else XPLMSetDatavf(SwitchTable[i].DataRefID, &SwitchTable[i].off_float, SwitchTable[i].index, 1);
            }
        }
        else if ((SwitchTable[i].SwitchType == PositiveIncrement) && (JoystickButtonValues[SwitchTable[i].button] == 1) &&
            (JoystickButtonLastValues[SwitchTable[i].button] == 0))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
            {
                IntToSet = XPLMGetDatai(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_int;
                if (IntToSet > SwitchTable[i].upper_limit_int) IntToSet = SwitchTable[i].upper_limit_int;
                XPLMSetDatai(SwitchTable[i].DataRefID, IntToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Float)
            {
                FloatToSet     = XPLMGetDataf(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDataf(SwitchTable[i].DataRefID, FloatToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Double)
            {
                FloatToSet     = (float) XPLMGetDatad(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDatad(SwitchTable[i].DataRefID, (double) FloatToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_IntArray)
            {
                XPLMGetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
                IntToSet += SwitchTable[i].stepping_int;
                if (IntToSet > SwitchTable[i].upper_limit_int) IntToSet = SwitchTable[i].upper_limit_int;
                XPLMSetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
            }
            else if (SwitchTable[i].DataRefType == xplmType_FloatArray)
            {
                XPLMGetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
                FloatToSet += SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
            }
        }
        else if ((SwitchTable[i].SwitchType == NegativeIncrement) && (JoystickButtonValues[SwitchTable[i].button] == 0) &&
            (JoystickButtonLastValues[SwitchTable[i].button] == 1))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
            {
                IntToSet = XPLMGetDatai(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_int;
                if (IntToSet > SwitchTable[i].upper_limit_int) IntToSet = SwitchTable[i].upper_limit_int;
                XPLMSetDatai(SwitchTable[i].DataRefID, IntToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Float)
            {
                FloatToSet     = XPLMGetDataf(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDataf(SwitchTable[i].DataRefID, FloatToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Double)
            {
                FloatToSet     = (float) XPLMGetDatad(SwitchTable[i].DataRefID) + SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDatad(SwitchTable[i].DataRefID, (double) FloatToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_IntArray)
            {
                XPLMGetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
                IntToSet += SwitchTable[i].stepping_int;
                if (IntToSet > SwitchTable[i].upper_limit_int) IntToSet = SwitchTable[i].upper_limit_int;
                XPLMSetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
            }
            else if (SwitchTable[i].DataRefType == xplmType_FloatArray)
            {
                XPLMGetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
                FloatToSet += SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet > SwitchTable[i].upper_limit_float) FloatToSet = SwitchTable[i].upper_limit_float;
                XPLMSetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
            }
        }
        else if ((SwitchTable[i].SwitchType == PositiveDecrement) && (JoystickButtonValues[SwitchTable[i].button] == 1) &&
            (JoystickButtonLastValues[SwitchTable[i].button] == 0))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
            {
                IntToSet = XPLMGetDatai(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_int;
                if (IntToSet < SwitchTable[i].lower_limit_int) IntToSet = SwitchTable[i].lower_limit_int;
                XPLMSetDatai(SwitchTable[i].DataRefID, IntToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Float)
            {
                FloatToSet     = XPLMGetDataf(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDataf(SwitchTable[i].DataRefID, FloatToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Double)
            {
                FloatToSet     = (float) XPLMGetDatad(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDatad(SwitchTable[i].DataRefID, (double) FloatToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_IntArray)
            {
                XPLMGetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
                IntToSet -= SwitchTable[i].stepping_int;
                if (IntToSet < SwitchTable[i].lower_limit_int) IntToSet = SwitchTable[i].lower_limit_int;
                XPLMSetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
            }
            else if (SwitchTable[i].DataRefType == xplmType_FloatArray)
            {
                XPLMGetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
                FloatToSet -= SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
            }
        }
        else if ((SwitchTable[i].SwitchType == NegativeDecrement) && (JoystickButtonValues[SwitchTable[i].button] == 0) &&
            (JoystickButtonLastValues[SwitchTable[i].button] == 1))
        {
            if (SwitchTable[i].DataRefType == xplmType_Int)
            {
                IntToSet = XPLMGetDatai(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_int;
                if (IntToSet < SwitchTable[i].lower_limit_int) IntToSet = SwitchTable[i].lower_limit_int;
                XPLMSetDatai(SwitchTable[i].DataRefID, IntToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Float)
            {
                FloatToSet     = XPLMGetDataf(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDataf(SwitchTable[i].DataRefID, FloatToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_Double)
            {
                FloatToSet     = (float) XPLMGetDatad(SwitchTable[i].DataRefID) - SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDatad(SwitchTable[i].DataRefID, (double) FloatToSet);
            }
            else if (SwitchTable[i].DataRefType == xplmType_IntArray)
            {
                XPLMGetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
                IntToSet -= SwitchTable[i].stepping_int;
                if (IntToSet < SwitchTable[i].lower_limit_int) IntToSet = SwitchTable[i].lower_limit_int;
                XPLMSetDatavi(SwitchTable[i].DataRefID, &IntToSet, SwitchTable[i].index, 1);
            }
            else if (SwitchTable[i].DataRefType == xplmType_FloatArray)
            {
                XPLMGetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
                FloatToSet -= SwitchTable[i].stepping_float;
                if (SwitchTable[i].round > 0)
                    FloatToSet = static_cast<float>(fabs(FloatToSet / SwitchTable[i].round + 0.5) * SwitchTable[i].round);
                if (FloatToSet < SwitchTable[i].lower_limit_float) FloatToSet = SwitchTable[i].lower_limit_float;
                XPLMSetDatavf(SwitchTable[i].DataRefID, &FloatToSet, SwitchTable[i].index, 1);
            }
        }
    }
}

// Now follows the extreme fast part of it
float MyEveryFrameLoopCallback(
        float /*inElapsedSinceLastCall*/,
        float /*inElapsedTimeSinceLastFlightLoop*/,
        int /*inCounter*/,
        void* /*inRefcon*/)
{
    // get time before execution
    clock_t time_start = clock();

    // catching the buttons (only relevant in this callback)
    XPLMGetDatavi(gJoystickButtonValues, JoystickButtonValues, 0, MAXJOYSTICKBUTTONS);

    if (LuaIsRunning) ExecuteSwitches();

    if ((EveryFrameCallbackCommand.length() > 1) && LuaIsRunning)
    {
        // execute the Lua code
        RunLuaChunk("DO_EVERY_FRAME_CHUNK");
    }

    // do the new metar command string?
    if ((NewMetarCountdown++ == 2) && LuaIsRunning)
    {
        RunLuaChunk("DO_ON_NEW_METAR_CHUNK");
    }

    // store the joystick buttons for state analysis
    for (auto i = 0; i < MAXJOYSTICKBUTTONS; i++)
    {
        JoystickButtonLastValues[i] = JoystickButtonValues[i];
    }

    // get time after execution
    clock_t time_end = clock();

    // write time difference and memory usage into global variables
    if (LuaIsRunning)
    {
        lua_pushnumber(FWLLua, (double) (time_end - time_start) / CLOCKS_PER_SEC);
        lua_setglobal(FWLLua, "DO_EVERY_FRAME_TIME_SEC");
        lua_pushnumber(FWLLua, lua_gc(FWLLua, LUA_GCCOUNT, 0));
        lua_setglobal(FWLLua, "LUA_MEMORY_USAGE_KB");
    }

    return -1;  // -1 will set callback to every frame
}


int MyReloadScriptsCommandHandler(XPLMCommandRef /*inCommand*/,
                                  XPLMCommandPhase inPhase,
                                  void* /*inRefcon*/)
{
    if (inPhase == xplm_CommandBegin)
    {
        ReadAllScriptFiles();
        if  (developer_mode == 0)
        {
          XPLMSpeakString("All Lua Script files loaded.");
        }
    }
    return 0; // we resume the command
}

// execute the macro by setting a mission level

void MacroMenuHandler(void* /*mRef*/, void* iRef)
{
    auto MacroIndex = (std::size_t) iRef; // snagar
    if (MacroTable[MacroIndex].IsSwitch)
    {
        int ActualState;
        XPLMCheckMenuItemState(MacroMenuId, MacroTable[MacroIndex].XPLM_Index, &ActualState);
        if (ActualState == xplm_Menu_Checked)
        {
            RunLuaString(MacroTable[MacroIndex].DeactivateCommand);
            XPLMCheckMenuItem(MacroMenuId, MacroTable[MacroIndex].XPLM_Index, xplm_Menu_Unchecked);
        } else
        {
            RunLuaString(MacroTable[MacroIndex].ActivateCommand);
            XPLMCheckMenuItem(MacroMenuId, MacroTable[MacroIndex].XPLM_Index, xplm_Menu_Checked);
        }
    } else
    {
        RunLuaString(MacroTable[MacroIndex].ActivateCommand);
    }
}

// Functions called by the menu will be send to command handler.
void FlyWithLuaMenuHandler(void* /*mRef*/, void* iRef)
{
    if (!strcmp((char*) iRef, "Reload"))
    {
        logMsg(logToDevCon, "FlyWithLua: User forced a script reload.");
        first_pass = 0;
        ReadAllScriptFiles();
        return;
    }
    if (!strcmp((char*) iRef, "Debug"))
    {
        DebugLua();
        luaL_dostring(FWLLua, "FLYWITHLUA_DEBUG()");
        return;
    }
    if (!strcmp((char*) iRef, "Resume"))
    {
        DebugLua();
        lua_settop(FWLLua, 0);
        LuaIsRunning = true;
        return;
    }
    if (!strcmp((char*) iRef, "Stop"))
    {
        logMsg(logToDevCon, "FlyWithLua: Lua Engine stopped by user.");
        LuaIsRunning = false;
        return;
    }
    if (!strcmp((char*) iRef, "ReturnQt"))
    {
        ReadAllQuarantinedScriptFiles();
        return;
    }

    if (!strcmp((char*) iRef, "DevMode"))
    {
        XPLMCheckMenuItemState(FlyWithLuaMenuId, DevModeCheckedPosition, &DevMenuMode);
        if (DevMenuMode == 2)
        {
            XPLMCheckMenuItem(FlyWithLuaMenuId, DevModeCheckedPosition, 1);
            developer_mode = 0;
        }
        else
        {
            XPLMCheckMenuItem(FlyWithLuaMenuId, DevModeCheckedPosition, 2);
            developer_mode = 1;
        }
        return;
    }

    if (!strcmp((char*) iRef, "VerboseMode"))
    {
        XPLMCheckMenuItemState(FlyWithLuaMenuId, VerboseModeCheckedPosition, &VerboseMenuMode);
        if (VerboseMenuMode == 2)
        {
            XPLMCheckMenuItem(FlyWithLuaMenuId, VerboseModeCheckedPosition, 1);
            verbose_logging_mode = 0;
        }
        else
        {
            XPLMCheckMenuItem(FlyWithLuaMenuId, VerboseModeCheckedPosition, 2);
            verbose_logging_mode = 1;
        }
        return;
    }
}


/*  Snagar -- for debug */
/*  extended with logType by Tedii */
void logMsg(ELogType logType, std::string message)
{
    std::string out = message + "\n";

    //if XSquawkBox not connected / not installed fall back to DevCon
    if (!WeHaveXSB && (logType & logToSqkBox) != 0)
    {
        logType = logToDevCon;
    }

    //push message to DevCon
    if (logType == logToAll || (logType & logToDevCon) != 0)
    {
        XPLMDebugString(const_cast<char*> (out.c_str()));
    }

    //push message to Speech
    if (logType == logToAll || (logType & logToSqkBox) != 0)
    {
        XSBSpeakString(message);
    }
}

void panic(const std::string& message)
{
    LuaIsRunning = false;
    logMsg(logToDevCon, message);
}

void initPluginDirectory()
{
    char path[1024]; // hopefully path will not be longer then 1024 characters

    XPLMGetSystemPath(path);
    logMsg(logToDevCon, std::string("FlyWithLua: Your system path is ").append(path)); // debug

    char dirSep[2];
    strcpy(dirSep, XPLMGetDirectorySeparator()); // correct for each OS

    systemDir.clear();
    pluginMainDir.clear();
    scriptDir.clear();
    internalsDir.clear();
    modulesDir.clear();
    quarantineDir.clear();

    systemDir.append(path);
    pluginMainDir.append(path).append("Resources").append(dirSep).append("plugins").append(dirSep).append("FlyWithLua");
    scriptDir.append(pluginMainDir).append(dirSep).append("Scripts");
    internalsDir.append(pluginMainDir).append(dirSep).append("Internals").append(dirSep);
    modulesDir.append(pluginMainDir).append(dirSep).append("Modules").append(dirSep);
    quarantineDir.append(pluginMainDir).append(dirSep).append("Scripts (Quarantine)").append(dirSep);

    logMsg(logToDevCon, std::string("FlyWithLua: System Dir: ") + systemDir); // debug
    logMsg(logToDevCon, std::string("FlyWithLua: Plugin Dir: ") + pluginMainDir); // debug
    logMsg(logToDevCon, std::string("FlyWithLua: Plugin Scripts Dir: ") + scriptDir); // debug
    logMsg(logToDevCon, std::string("FlyWithLua: Plugin Internals Dir: ") + internalsDir); // debug
    logMsg(logToDevCon, std::string("FlyWithLua: Plugin Modules Dir: ") + modulesDir); // debug
    logMsg(logToDevCon, std::string("FlyWithLua: Plugin Scripts Quarantine Dir: ") + quarantineDir); // debug
    logMsg(logToDevCon, std::string("FlyWithLua: Plugin Version: ") + PLUGIN_VERSION); // debug
}


} // end namespace
