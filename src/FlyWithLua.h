#ifndef _FLYWITHLUA_H_
#define _FLYWITHLUA_H_

#include <string>
#include <lua.hpp>

//Teddii: Enum fuer "logMsg"
enum ELogType
{
    logToAll    = 0,
    logToDevCon = 1,
    logToSqkBox = 2
};

namespace flywithlua {

void logMsg (ELogType logType, std::string message ); //Teddii: added parameter logType //void logMsg ( std::string message );
void CopyDataRefsToLua( void );
void CopyDataRefsToXPlane( void );

/**
 * Log the error message and stop FlyWithLua.
 *
 * This is a very rudimentary error handling function at this time, as it doesn't do anything to interrupt Lua execution
 * if it is called from within Lua code. Hopefully this will improve in future. It is recommended to use this function
 * instead of simply setting LuaIsRunning to false because it would make transition to the new error handling simpler.
 *
 * @param message Message to report.
 */
void panic(const std::string& message);

extern bool LuaIsRunning;                       // Are we working with Lua?
extern bool WeAreNotInDrawingState;
extern lua_State   *FWLLua;

extern std::string scriptDir;
extern std::string quarantineDir;
extern bool ReadAllScriptFiles();
extern int found_bad_function_script;
extern void DebugLua();
}

#endif
