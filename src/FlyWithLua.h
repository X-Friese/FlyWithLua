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

extern bool LuaIsRunning;                       // Are we working with Lua?
extern bool WeAreNotInDrawingState;
extern lua_State   *FWLLua;
}

#endif
