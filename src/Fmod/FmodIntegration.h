/*
 *   Fmod integration for FlyWithLua
 *   Copyright (C) 2022 William Good
 *   Released as public domain code.
 *
 */
#ifndef FMOD_FMODINTEGRATION_H_
#define FMOD_FMODINTEGRATION_H_

#include "lua.hpp"



#include "fwl_dataref.h"
#include "fwl_customdataref.h"

// fmod4xp
#include "fmod4xpchannelgroup.h"
#include "fmod4xpsystem.h"


extern lua_State* lState;

namespace fmodint {

int fmod_initialization();
int fmod_uninitialize();
int fmod_data_update();
void deinitFmodSupport();
void RegisterFmodFunctionsToLua(lua_State* L);
void RegisterAccessor();

} // end namespace fmodint


#endif // FMOD_FMODINTEGRATION_H_
 
