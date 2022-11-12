/*
 *   Fmod insperation from Camille Bachmann
 *   https://bitbucket.org/Squirrel_FS/fmodplugin/src/master/
 *   https://bitbucket.org/Squirrel_FS/fmod4xp/src/master/
 *
 *   Fmod integration for FlyWithLua
 *   Copyright (C) 2022 William Good
 *   Released as public domain code.
 *
 */
 
#include <iostream>
#include <vector>
#include <string>
#include <string.h>

#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include "XPLMProcessing.h"

#include "FmodIntegration.h"
#include "../FlyWithLua.h"

// include Fmod
#include "fmod.h"
#include "fmod_studio.h"
#include "fmod_errors.h"

#include "fwl_dataref.h"
#include "fwl_customdataref.h"

// fmod4xp
#include "fmod4xpchannelgroup.h"
#include "fmod4xpsystem.h"


// DataRefs --------------------------------------------------------------------
DataRef<int> fmod_system_hi_dataref("sim/private/fmod/system_hi");
DataRef<int> fmod_system_lo_dataref("sim/private/fmod/system_lo");
DataRef<int> fmod_studio_hi_dataref("sim/private/fmod/studio_hi");
DataRef<int> fmod_studio_lo_dataref("sim/private/fmod/studio_lo");


CustomDataRef<int> flywithlua_master_channel_group_mute("FlyWithLua_MasterChannelGroup/Mute", 1);
CustomDataRef<float> flywithlua_master_channel_group_volume("FlyWithLua_MasterChannelGroup/Volume", 1);
CustomDataRef<int> flywithlua_radios_channel_group_mute("FlyWithLua_RadiosChannelGroup/Mute", 1);
CustomDataRef<float> flywithlua_radios_channel_group_volume("FlyWithLua_RadiosChannelGroup/Volume", 1);
CustomDataRef<int> flywithlua_interior_channel_group_mute("FlyWithLua_InteriorChannelGroup/Mute", 1);
CustomDataRef<float> flywithlua_interior_channel_group_volume("FlyWithLua_InteriorChannelGroup/Volume", 1);


// CustomDataRef<int> flywithlua_copilot_channel_group_mute("FlyWithLua_CopilotChannelGroup/Mute", 1);
// CustomDataRef<float> flywithlua_copilot_channel_group_volume("FlyWithLua_CopilotChannelGroup/Volume", 1);
// CustomDataRef<int> flywithlua_ui_channel_group_mute("FlyWithLua_UiChannelGroup/Mute", 1);
// CustomDataRef<float> flywithlua_ui_channel_group_volume("FlyWithLua_UiChannelGroup/Volume", 1);


// FMOD Core --------------------------------------------------------------------
FMOD_SYSTEM *fmod_system_pointer = nullptr;
FMOD_SOUND *fmod_sound_pointer[400];
FMOD_CHANNELGROUP *flywithlua_master_channel_group = nullptr;
FMOD_CHANNELGROUP *flywithlua_radios_channel_group = nullptr;
FMOD_CHANNELGROUP *flywithlua_interior_channel_group = nullptr;


// FMOD_CHANNELGROUP *flywithlua_exterior_processed_channel_group = nullptr;
// FMOD_CHANNELGROUP *flywithlua_exterior_unprocessed_channel_group = nullptr;
// FMOD_CHANNELGROUP *flywithlua_exterior_processed_aircraft_channel_group = nullptr;
// FMOD_CHANNELGROUP *flywithlua_exterior_processed_environment_channel_group = nullptr;
// FMOD_CHANNELGROUP *flywithlua_copilot_channel_group = nullptr;
// FMOD_CHANNELGROUP *flywithlua_ui_channel_group = nullptr;

// FMOD Studio ------------------------------------------------------------------
FMOD_STUDIO_SYSTEM *fmod_studio_system_pointer = nullptr;

int Fmod_IndexNo = 0;
bool init = true;

namespace fmodint {

struct FmodSoundsStructure
{
    std::string filename;
    float       frequency{};
    float       pitch{};
    float       volume{};
    int         index{};
};

static std::vector<FmodSoundsStructure> FmodSounds;

int fmod_sound_index = 0;

bool mute_master_channel_group;
float master_channel_group_volume;
bool mute_radios_channel_group;
float radios_channel_group_volume;
bool mute_interior_channel_group;
float interior_channel_group_volume;


// bool mute_copilot_channel_group;
// float copilot_channel_group_volume;
// bool mute_ui_channel_group;
// float ui_channel_group_volume;


void fmod_error_handler(const string &file, int fwl_line, FMOD_RESULT fwl_result)
{
    if(fwl_result != FMOD_OK)
    {
        string fmod_error_msg = "Error in " + file + ", line " + to_string(fwl_line) + ": " + FMOD_ErrorString(fwl_result);
        throw runtime_error(fmod_error_msg.c_str());
    }
}


static int LuaLoadFmodSound(lua_State* L)
{
    if (!lua_isstring(L, 1))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaLoadFmodFile() Missing filename of Fmod file to load. You will have to give a string.");
        flywithlua::LuaIsRunning = false;
        return 0;
    }
    char fmod_filename_to_load[250];
    strncpy(fmod_filename_to_load, lua_tostring(L, 1), sizeof(fmod_filename_to_load));
    fmod_sound_index = fmod_sound_index + 1;

    if (flywithlua::verbose_logging_mode == 1) {
        char buf[200];
        sprintf(buf, "FlyWithLua Info: LuaLoadFmodFile() fmod_sound_index = %d     fmod_filename_to_load = %s\n", fmod_sound_index, fmod_filename_to_load);
        XPLMDebugString(buf);
    }

    FmodSounds.emplace_back(); // Make space to store information about the sound.
    FmodSounds.back().filename = fmod_filename_to_load;
    FmodSounds.back().frequency    = 1.0f;
    FmodSounds.back().pitch    = 1.0f;
    FmodSounds.back().volume     = 1.0f;
    FmodSounds.back().index    = fmod_sound_index;

    // fill the debug table with information
    // This is how OpenAL works but not working for Fmod
    // FmodSoundsStructure* fmod_sound = &FmodSounds.back();

    // fmod_sound->filename = FmodFileNameToLoad;
    // fmod_sound->pitch = 1.0f;
    // fmod_sound->volume = 1.0f;
    // fmod_sound->loop = false;
    // fmod_sound->index = fmod_sound_index;

    if (flywithlua::verbose_logging_mode == 1)
    {
        flywithlua::logMsg(logToDevCon, std::string("FlyWithLua: Loaded fmod sound file \"").append(fmod_filename_to_load).append("\"."));
    }
//    CHECK_ERR();

    // give back to index number


    lua_pushnumber(flywithlua::FWLLua, FmodSounds.size() - 1);
    return 1;
}


unsigned long long fmod_get_delay_to_current_sound_end(int fmod_output_rate, FMOD_CHANNEL *fmod_playing_channel)
{
    // From the example granular_synth.cpp
    if(fmod_playing_channel != nullptr)
    {
        unsigned long long fmod_start_delay = 0;
        unsigned int fmod_sound_length = 0;
        float fmod_sound_frequency = 0.f;
        FMOD_SOUND *fmod_playing_sound = nullptr;

        FMOD_RESULT result;

        // Get the channel DSP clock, which serves as a reference
        result = FMOD_Channel_GetDSPClock(fmod_playing_channel, 0, &fmod_start_delay);
        fmod_error_handler(__FILE__, __LINE__-1, result);

        // Grab the length of the playing sound, and its frequency, so we can caluate where to place the new sound on the time line.
        result = FMOD_Channel_GetCurrentSound(fmod_playing_channel, &fmod_playing_sound);
        fmod_error_handler(__FILE__, __LINE__-1, result);
        result = FMOD_Sound_GetLength(fmod_playing_sound, &fmod_sound_length, FMOD_TIMEUNIT_PCM);
        fmod_error_handler(__FILE__, __LINE__-1, result);
        result = FMOD_Channel_GetFrequency(fmod_playing_channel, &fmod_sound_frequency);
        fmod_error_handler(__FILE__, __LINE__-1, result);

        /*
            Now calculate the length of the sound in 'output samples'.
            Ie if a 44khz sound is 22050 samples long, and the output rate is 48khz, then we want to delay by 24000 output samples.
        */
        fmod_sound_length *= fmod_output_rate;
        fmod_sound_length /= fmod_sound_frequency;

        fmod_start_delay += fmod_sound_length; // Add output rate adjusted sound length, to the clock value of the sound that is currently playing

        return fmod_start_delay;
    }
    return 0;
}


static int LuaPlayFmodSoundOnMasterBus(lua_State* L)
{
    string fmod_sound_path;

    if (!lua_isnumber(L, 1))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnMasterBus() Missing Play Fmod Sound index number. You will have to give an integer..");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    Fmod_IndexNo = static_cast<int>(lua_tointeger(L, 1));
    if ((Fmod_IndexNo < 0) || (Fmod_IndexNo >= int(FmodSounds.size())))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnMasterBus() Play Fmod Sound index number out of range.");
    }

    // We need to do this to get the index's lined up and have not found a better way
    Fmod_IndexNo = Fmod_IndexNo + 1;

    if (!FmodSounds.empty())
    {
        for (const FmodSoundsStructure & fmod_sound : FmodSounds)
        {
            if (fmod_sound.index == Fmod_IndexNo)
            {
                fmod_sound_path = fmod_sound.filename;

                if (flywithlua::verbose_logging_mode == 1) {
                    char buf2[200];
                    sprintf(buf2, "FlyWithLua Info: In LuaPlayFmodSoundOnMasterBus() Fmod_IndexNo  =  %d    fwl_sound3Path  =  %s  \n", Fmod_IndexNo, fmod_sound_path.c_str());
                    XPLMDebugString(buf2);
                }
            }
        }
    } else
    {
        XPLMDebugString("FlyWithLua Error: No Fmod sounds created by Lua.\n");
    }

    /* This is purely an experiment!
     * Don't copy directly this code, since it is not safe: most pointers are not checked and error codes are ignored!
     *
     * Fmod insperation from Camille Bachmann
     * https://bitbucket.org/Squirrel_FS/fmodplugin/src/master/
     */

    try
    {
        if(flywithlua_master_channel_group != nullptr)
        {
            FMOD_RESULT fwl_result;
            // We are using fwl_soundFMod[Fmod_IndexNo] so we know which sound file to play
            // Still working on if the is the best approach

            fwl_result = FMOD_System_CreateSound(fmod_system_pointer, fmod_sound_path.c_str(), FMOD_DEFAULT, nullptr, &fmod_sound_pointer[Fmod_IndexNo]);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);

            // The channel on which the sound will be played
            // We are passing nullptr because this feature is not being used.
            FMOD_CHANNEL* flywithlua_channel_pointer = nullptr;
            // Actually play the sound on the FlyWithLua Channel Group
            fwl_result = FMOD_System_PlaySound(fmod_system_pointer, fmod_sound_pointer[Fmod_IndexNo], flywithlua_master_channel_group, false, &flywithlua_channel_pointer);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);
        }
    }
    catch (exception const& e)
    {
        string fmod_error_msg = "FlyWithLua Info: " + string(e.what()) + "\n";
        XPLMDebugString(fmod_error_msg.c_str());
        return 0.f; // The plugin is failing, so do not execute any further loop
    }

    return 0;
}


static int LuaStopFmodSoundOnMasterBus(lua_State* L)
{
    try
    {
        FMOD_RESULT result;

        if(flywithlua_master_channel_group != nullptr)
        {
            // Stop the sound for the FlyWithLua radio channel group
            result = FMOD_ChannelGroup_Stop(flywithlua_master_channel_group);
        }
    }
    catch (exception const& e)
    {
        string fmod_error_msg = "FlyWithLua Info: " + string(e.what()) + "\n";
        XPLMDebugString(fmod_error_msg.c_str());
        return 0; // The plugin is failing, so do not execute any further loop
    }

    return 0;
}


void setMasterVolume(float master_channel_group_volume)
{
    // Set the volume for the FlyWithLua channel group
    FMOD_RESULT result = FMOD_ChannelGroup_SetVolume(flywithlua_master_channel_group, master_channel_group_volume);
    if(FMOD_OK != result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetVolume: " + FMOD_ErrorString(result);
        throw runtime_error(errorMsg.c_str());
    }
}


void setMasterMute(bool mute_master_channel_group)
{
    // Mute the sound for the FlyWithLua channel group
    FMOD_RESULT result = FMOD_ChannelGroup_SetMute(flywithlua_master_channel_group, mute_master_channel_group);
    if(FMOD_OK != result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetMute: " + FMOD_ErrorString(result);
        throw runtime_error(errorMsg.c_str());
    }
}


static int LuaPlayFmodSoundOnRadiosBus(lua_State* L)
{
    string fmod_sound_path;

    if (!lua_isnumber(L, 1))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnRadiosBus() Missing Play Fmod Sound index number. You will have to give an integer..");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    Fmod_IndexNo = static_cast<int>(lua_tointeger(L, 1));
    if ((Fmod_IndexNo < 0) || (Fmod_IndexNo >= int(FmodSounds.size())))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnRadiosBus() Play Fmod Sound index number out of range.");
    }

    // We need to do this to get the index's lined up and have not found a better way
    Fmod_IndexNo = Fmod_IndexNo + 1;

    if (!FmodSounds.empty())
    {
        for (const FmodSoundsStructure & fmod_sound : FmodSounds)
        {
            if (fmod_sound.index == Fmod_IndexNo)
            {
                fmod_sound_path = fmod_sound.filename;

                if (flywithlua::verbose_logging_mode == 1) {
                    char buf2[200];
                    sprintf(buf2, "FlyWithLua Info: In LuaPlayFmodSoundOnRadiosBus() Fmod_IndexNo  =  %d    fwl_sound3Path  =  %s  \n", Fmod_IndexNo, fmod_sound_path.c_str());
                    XPLMDebugString(buf2);
                }
            }
        }
    } else
    {
        XPLMDebugString("FlyWithLua Error: No Fmod sounds created by Lua.\n");
    }

    /* This is purely an experiment!
     * Don't copy directly this code, since it is not safe: most pointers are not checked and error codes are ignored!
     *
     * Fmod insperation from Camille Bachmann
     * https://bitbucket.org/Squirrel_FS/fmodplugin/src/master/
     */

    try
    {
        if(flywithlua_radios_channel_group != nullptr)
        {
            FMOD_RESULT fwl_result;
            // We are using fwl_soundFMod[Fmod_IndexNo] so we know which sound file to play
            // Still working on if the is the best approach

            fwl_result = FMOD_System_CreateSound(fmod_system_pointer, fmod_sound_path.c_str(), FMOD_DEFAULT, nullptr, &fmod_sound_pointer[Fmod_IndexNo]);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);

            // The channel on which the sound will be played
            // We are passing nullptr because this feature is not being used.
            FMOD_CHANNEL* flywithlua_channel_pointer = nullptr;
            // Actually play the sound on the FlyWithLua Channel Group
            fwl_result = FMOD_System_PlaySound(fmod_system_pointer, fmod_sound_pointer[Fmod_IndexNo], flywithlua_radios_channel_group, false, &flywithlua_channel_pointer);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);
        }
    }
    catch (exception const& e)
    {
        string fmod_error_msg = "FlyWithLua Info: " + string(e.what()) + "\n";
        XPLMDebugString(fmod_error_msg.c_str());
        return 0.f; // The plugin is failing, so do not execute any further loop
    }

    return 0;
}


static int LuaStopFmodSoundOnRadiosBus(lua_State* L)
{
    try
    {
        FMOD_RESULT result;

        if(flywithlua_radios_channel_group != nullptr)
        {
            // Stop the sound for the FlyWithLua radio channel group
            result = FMOD_ChannelGroup_Stop(flywithlua_radios_channel_group);
        }
    }
    catch (exception const& e)
    {
        string fmod_error_msg = "FlyWithLua Info: " + string(e.what()) + "\n";
        XPLMDebugString(fmod_error_msg.c_str());
        return 0; // The plugin is failing, so do not execute any further loop
    }

    return 0;
}


void setRadioVolume(float radios_channel_group_volume)
{
    // Set the volume for the FlyWithLua channel group
    FMOD_RESULT result = FMOD_ChannelGroup_SetVolume(flywithlua_radios_channel_group, radios_channel_group_volume);
    if(FMOD_OK != result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetVolume: " + FMOD_ErrorString(result);
        throw runtime_error(errorMsg.c_str());
    }
}


void setRadiosMute(bool mute_radios_channel_group)
{
    // Mute the sound for the FlyWithLua channel group
    FMOD_RESULT result = FMOD_ChannelGroup_SetMute(flywithlua_radios_channel_group, mute_radios_channel_group);
    if(FMOD_OK != result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetMute: " + FMOD_ErrorString(result);
        throw runtime_error(errorMsg.c_str());
    }
}


static int LuaPlayFmodSoundOnInteriorBus(lua_State* L)
{
    string fmod_sound_path;

    if (!lua_isnumber(L, 1))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnInteriorBus() Missing Play Fmod Sound index number. You will have to give an integer..");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    Fmod_IndexNo = static_cast<int>(lua_tointeger(L, 1));
    if ((Fmod_IndexNo < 0) || (Fmod_IndexNo >= int(FmodSounds.size())))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnInteriorBus() Play Fmod Sound index number out of range.");
    }

    // We need to do this to get the index's lined up and have not found a better way
    Fmod_IndexNo = Fmod_IndexNo + 1;

    if (!FmodSounds.empty())
    {
        for (const FmodSoundsStructure & fmod_sound : FmodSounds)
        {
            if (fmod_sound.index == Fmod_IndexNo)
            {
                fmod_sound_path = fmod_sound.filename;

                if (flywithlua::verbose_logging_mode == 1) {
                    char buf2[200];
                    sprintf(buf2, "FlyWithLua Info: In LuaPlayFmodSoundOnInteriorBus() Fmod_IndexNo  =  %d    fwl_sound3Path  =  %s  \n", Fmod_IndexNo, fmod_sound_path.c_str());
                    XPLMDebugString(buf2);
                }
            }
        }
    } else
    {
        XPLMDebugString("FlyWithLua Error: No Fmod sounds created by Lua.\n");
    }

    /* This is purely an experiment!
     * Don't copy directly this code, since it is not safe: most pointers are not checked and error codes are ignored!
     *
     * Fmod insperation from Camille Bachmann
     * https://bitbucket.org/Squirrel_FS/fmodplugin/src/master/
     */

    try
    {
        if(flywithlua_interior_channel_group != nullptr)
        {
            FMOD_RESULT fwl_result;
            // We are using fwl_soundFMod[Fmod_IndexNo] so we know which sound file to play
            // Still working on if the is the best approach

            fwl_result = FMOD_System_CreateSound(fmod_system_pointer, fmod_sound_path.c_str(), FMOD_DEFAULT, nullptr, &fmod_sound_pointer[Fmod_IndexNo]);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);

            // The channel on which the sound will be played
            // We are passing nullptr because this feature is not being used.
            FMOD_CHANNEL* flywithlua_channel_pointer = nullptr;
            // Actually play the sound on the FlyWithLua Channel Group
            fwl_result = FMOD_System_PlaySound(fmod_system_pointer, fmod_sound_pointer[Fmod_IndexNo], flywithlua_interior_channel_group, false, &flywithlua_channel_pointer);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);
        }
    }
    catch (exception const& e)
    {
        string fmod_error_msg = "FlyWithLua Info: " + string(e.what()) + "\n";
        XPLMDebugString(fmod_error_msg.c_str());
        return 0.f; // The plugin is failing, so do not execute any further loop
    }

    return 0;
}


static int LuaStopFmodSoundOnInteriorBus(lua_State* L)
{
    try
    {
        FMOD_RESULT result;

        if(flywithlua_interior_channel_group != nullptr)
        {
            // Stop the sound for the FlyWithLua interior channel group
            result = FMOD_ChannelGroup_Stop(flywithlua_interior_channel_group);
        }
    }
    catch (exception const& e)
    {
        string fmod_error_msg = "FlyWithLua Info: " + string(e.what()) + "\n";
        XPLMDebugString(fmod_error_msg.c_str());
        return 0; // The plugin is failing, so do not execute any further loop
    }

    return 0;
}


void setInteriorVolume(float interior_channel_group_volume)
{
    // Set the volume for the FlyWithLua channel group
    FMOD_RESULT result = FMOD_ChannelGroup_SetVolume(flywithlua_interior_channel_group, interior_channel_group_volume);
    if(FMOD_OK != result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetVolume: " + FMOD_ErrorString(result);
        throw runtime_error(errorMsg.c_str());
    }
}


void setInteriorMute(bool mute_interior_channel_group)
{
    // Mute the sound for the FlyWithLua channel group
    FMOD_RESULT result = FMOD_ChannelGroup_SetMute(flywithlua_interior_channel_group, mute_interior_channel_group);
    if(FMOD_OK != result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetMute: " + FMOD_ErrorString(result);
        throw runtime_error(errorMsg.c_str());
    }
}


void RegisterFmodFunctionsToLua(lua_State* L)
{
    lua_register(L, "load_fmod_sound", LuaLoadFmodSound);
    lua_register(L, "play_sound_on_master_bus", LuaPlayFmodSoundOnMasterBus);
    lua_register(L, "stop_sound_on_master_bus", LuaStopFmodSoundOnMasterBus);
    lua_register(L, "play_sound_on_radios_bus", LuaPlayFmodSoundOnRadiosBus);
    lua_register(L, "stop_sound_on_radios_bus", LuaStopFmodSoundOnRadiosBus);
    lua_register(L, "play_sound_on_interior_bus", LuaPlayFmodSoundOnInteriorBus);
    lua_register(L, "stop_sound_on_interior_bus", LuaStopFmodSoundOnInteriorBus);
}


void RegisterAccessor()
{
    flywithlua_master_channel_group_mute.registerAccessor();
    flywithlua_master_channel_group_volume.registerAccessor();
    flywithlua_radios_channel_group_mute.registerAccessor();
    flywithlua_radios_channel_group_volume.registerAccessor();
    flywithlua_interior_channel_group_mute.registerAccessor();
    flywithlua_interior_channel_group_volume.registerAccessor();
    // flywithlua_copilot_channel_group_mute.registerAccessor();
    // flywithlua_copilot_channel_group_volume.registerAccessor();
    // flywithlua_ui_channel_group_mute.registerAccessor();
    // flywithlua_ui_channel_group_volume.registerAccessor();
}


int fmod_initialization()
{
    /* This is purely an experiment!
     * Don't copy directly this code, since it is not safe: most pointers are not checked and error codes are ignored!
     */

    try
    {
        if(init)
        {

            // Pointer to the FMOD system provided by XP
            uint64_t coreHiPart1 = static_cast<uint32_t>(fmod_system_hi_dataref.get());
            uint64_t corePointer1 = (coreHiPart1 << 32 | static_cast<uint32_t>(fmod_system_lo_dataref.get()));
            string debugMsg = "FlyWithLua Info: Core pointer (from dataref):" + to_string(corePointer1) + "\n";
            XPLMDebugString(debugMsg.c_str());
            fmod_system_pointer = reinterpret_cast<FMOD_SYSTEM*>(corePointer1);
            if(nullptr == fmod_system_pointer)
            {
                XPLMDebugString("FlyWithLua Info: No pointer for Core system\n");
            }

            uint64_t studioHiPart1 = static_cast<uint32_t>(fmod_studio_hi_dataref.get());
            uint64_t studioPointer1 = (studioHiPart1 << 32 | static_cast<uint32_t>(fmod_studio_lo_dataref.get()));
            debugMsg = "FlyWithLua Info: Studio pointer (from dataref):" + to_string(studioPointer1) + "\n";
            XPLMDebugString(debugMsg.c_str());
            fmod_studio_system_pointer = reinterpret_cast<FMOD_STUDIO_SYSTEM*>(studioPointer1);
            if(nullptr == fmod_studio_system_pointer)
            {
                XPLMDebugString("FlyWithLua Info: No pointer for Studio system\n");
            }
        }
        if(fmod_system_pointer != nullptr && fmod_studio_system_pointer != nullptr) // Later the studio pointer could be used alone (since the core pointer can be retrieved from the studio)
        {
            if(init)
            {
                init = false;

                FMOD_RESULT result;

                // Testing the core system pointer
                unsigned int fmod_version1;
                result = FMOD_System_GetVersion(fmod_system_pointer, &fmod_version1);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                if(result == FMOD_OK)
                {
                    XPLMDebugString("FlyWithLua Info: Core system pointer seems to be OK, it has a version number\n");
                }

                // Accessing the core system via Studio
                FMOD_SYSTEM* fmod_system_from_studio1 = nullptr;
                result = FMOD_Studio_System_GetCoreSystem(fmod_studio_system_pointer, &fmod_system_from_studio1);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                if(result == FMOD_OK)
                {
                    XPLMDebugString("FlyWithLua Info: Studio system pointer seems to be OK, it provides a core system\n");
                    string debugMsg = "FlyWithLua Info: Core pointer (from Studio):" + to_string(reinterpret_cast<uintptr_t>(fmod_system_from_studio1)) + "\n";
                    XPLMDebugString(debugMsg.c_str());
                }

                // Do we have access to the same core system?
                bool fmod_accessing_same_system1 = fmod_system_from_studio1 == fmod_system_pointer;
                if(!fmod_accessing_same_system1)
                {
                    XPLMDebugString("FlyWithLua Info: Pointers to the Core and Studio systems are not coherent\n");
                }


                // Master Channel Group
                // Accessing a studio bus, using the GUID provided in X-Plane doc
                // https://developer.x-plane.com/article/using-fmod-with-x-plane/
                // FMOD GUID described here: http://www.bom.fmod.com/docs/2.02/api/core-api-common.html#fmod_guid
                // Master bus GUID: 7f7c05a4-e05a–4d44–94fe-d4691147a9b7
                FMOD_STUDIO_BUS* fmod_master_bus = nullptr;
                FMOD_GUID fmod_master_bus_GUID;
                // First three fields of the GUID provided in X-Plane documentation
                fmod_master_bus_GUID.Data1 = 0x7f7c05a4;
                fmod_master_bus_GUID.Data2 = 0xe05a;
                fmod_master_bus_GUID.Data3 = 0x4d44;
                // Merged last two fields of the GUID provided in X-Plane documentation
                char str[] = "94fed4691147a9b7";
                unsigned i = 0;
                unsigned char c;
                while(i < 8 && 1 == sscanf(&str[2*i], "%02hhx", &c))
                {
                    fmod_master_bus_GUID.Data4[i] = c;
                    i ++;
                }

                result = FMOD_Studio_System_GetBusByID(fmod_studio_system_pointer, &fmod_master_bus_GUID, &fmod_master_bus);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Bridge with the Core API: get the channel group
                FMOD_CHANNELGROUP* fmod_masterchannels_group = nullptr;
                result = FMOD_Studio_Bus_GetChannelGroup(fmod_master_bus, &fmod_masterchannels_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Creating our own channel, trying not to mess with the others
                // Create new FlyWithLua Master channel group
                result = FMOD_System_CreateChannelGroup(fmod_system_pointer, "FlyWithLua_Master_Channel", &flywithlua_master_channel_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                // Add the FlyWithLua channel gropup to the existing radios channel group
                result = FMOD_ChannelGroup_AddGroup(fmod_masterchannels_group, flywithlua_master_channel_group, true, nullptr);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                if(flywithlua_master_channel_group != nullptr)
                {
                    XPLMDebugString("FlyWithLua Info: fmod_masterchannels_group has been created so OK to use\n");
                }


                // Radios Channel Group
                // Accessing a studio bus, using the GUID provided in X-Plane doc
                // https://developer.x-plane.com/article/using-fmod-with-x-plane/
                // FMOD GUID described here: http://www.bom.fmod.com/docs/2.02/api/core-api-common.html#fmod_guid
                // Radios bus GUID: d8526059-b27c–4eae–9b14-a1ee1cb3dfa1
                FMOD_STUDIO_BUS* fmod_radios_bus = nullptr;
                FMOD_GUID fmod_radios_bus_GUID;
                // First three fields of the GUID provided in X-Plane documentation
                fmod_radios_bus_GUID.Data1 = 0xd8526059;
                fmod_radios_bus_GUID.Data2 = 0xb27c;
                fmod_radios_bus_GUID.Data3 = 0x4eae;
                // Merged last two fields of the GUID provided in X-Plane documentation
                char str_radios[] = "9b14a1ee1cb3dfa1";
                unsigned i_radios = 0;
                unsigned char c_radios;
                while(i_radios < 8 && 1 == sscanf(&str_radios[2*i_radios], "%02hhx", &c_radios))
                {
                    fmod_radios_bus_GUID.Data4[i_radios] = c_radios;
                    i_radios ++;
                }

                result = FMOD_Studio_System_GetBusByID(fmod_studio_system_pointer, &fmod_radios_bus_GUID, &fmod_radios_bus);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Bridge with the Core API: get the channel group
                FMOD_CHANNELGROUP* fmod_radioschannels_group = nullptr;
                result = FMOD_Studio_Bus_GetChannelGroup(fmod_radios_bus, &fmod_radioschannels_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Creating our own channel, trying not to mess with the others
                // Create new FlyWithLua Radios channel group
                result = FMOD_System_CreateChannelGroup(fmod_system_pointer, "FlyWithLua_Radio_Channel", &flywithlua_radios_channel_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                // Add the FlyWithLua channel gropup to the existing radios channel group
                result = FMOD_ChannelGroup_AddGroup(fmod_radioschannels_group, flywithlua_radios_channel_group, true, nullptr);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                if(flywithlua_radios_channel_group != nullptr)
                {
                    XPLMDebugString("FlyWithLua Info: fmod_radioschannels_group has been created so OK to use\n");
                }


                // Interior Channel Group
                // Since sometimes this fails so we want to intilize this group last.
                // Bug report has been filed as XPD-13315.
                // Accessing a studio bus, using the GUID provided in X-Plane doc
                // https://developer.x-plane.com/article/using-fmod-with-x-plane/
                // FMOD GUID described here: http://www.bom.fmod.com/docs/2.02/api/core-api-common.html#fmod_guid
                // Interior bus GUID: 751f5066–4651–4afb-a410–4cb9db0894c9
                FMOD_STUDIO_BUS* fmod_interior_bus = nullptr;
                FMOD_GUID fmod_interior_bus_GUID;
                // First three fields of the GUID provided in X-Plane documentation
                fmod_interior_bus_GUID.Data1 = 0x751f5066;
                fmod_interior_bus_GUID.Data2 = 0x4651;
                fmod_interior_bus_GUID.Data3 = 0x4afb;
                // Merged last two fields of the GUID provided in X-Plane documentation
                char str_interior[] = "a4104cb9db0894c9";
                unsigned i_interior = 0;
                unsigned char c_interior;
                while(i_interior < 8 && 1 == sscanf(&str_interior[2*i_interior], "%02hhx", &c_interior))
                {
                    fmod_interior_bus_GUID.Data4[i_interior] = c_interior;
                    i_interior ++;
                }

                result = FMOD_Studio_System_GetBusByID(fmod_studio_system_pointer, &fmod_interior_bus_GUID, &fmod_interior_bus);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Bridge with the Core API: get the channel group
                FMOD_CHANNELGROUP* fmod_interiorchannels_group = nullptr;
                result = FMOD_Studio_Bus_GetChannelGroup(fmod_interior_bus, &fmod_interiorchannels_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Creating our own channel, trying not to mess with the others
                // Create new FlyWithLua Interior channel group
                result = FMOD_System_CreateChannelGroup(fmod_system_pointer, "FlyWithLua_Interior_Channel", &flywithlua_interior_channel_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                // Add the FlyWithLua channel gropup to the existing interior channel group
                result = FMOD_ChannelGroup_AddGroup(fmod_interiorchannels_group, flywithlua_interior_channel_group, true, nullptr);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                if(flywithlua_interior_channel_group != nullptr)
                {
                    XPLMDebugString("FlyWithLua Info: fmod_interiorchannels_group has been created so OK to use\n");
                }


                /*
                // Exterior Processed Channel Group
                // Accessing a studio bus, using the GUID provided in X-Plane doc
                // https://developer.x-plane.com/article/using-fmod-with-x-plane/
                // FMOD GUID described here: http://www.bom.fmod.com/docs/2.02/api/core-api-common.html#fmod_guid
                // Exterior Processed bus GUID: a5a85a81-afd5–4425–9dc2–6e50f7776e6a
                FMOD_STUDIO_BUS* fmod_exterior_processed_bus = nullptr;
                FMOD_GUID fmod_exterior_processed_bus_GUID;
                // First three fields of the GUID provided in X-Plane documentation
                fmod_exterior_processed_bus_GUID.Data1 = 0xa5a85a81;
                fmod_exterior_processed_bus_GUID.Data2 = 0xafd5;
                fmod_exterior_processed_bus_GUID.Data3 = 0x4425;
                // Merged last two fields of the GUID provided in X-Plane documentation
                char str_exterior_processed[] = "9dc26e50f7776e6a";
                unsigned i_exterior_processed = 0;
                unsigned char c_exterior_processed;
                while(i_exterior_processed < 8 && 1 == sscanf(&str_exterior_processed[2*i_exterior_processed], "%02hhx", &c_exterior_processed))
                {
                    fmod_exterior_processed_bus_GUID.Data4[i_exterior_processed] = c_exterior_processed;
                    i_exterior_processed ++;
                }

                result = FMOD_Studio_System_GetBusByID(fmod_studio_system_pointer, &fmod_exterior_processed_bus_GUID, &fmod_exterior_processed_bus);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Bridge with the Core API: get the channel group
                FMOD_CHANNELGROUP* fmod_exterior_processedchannels_group = nullptr;
                result = FMOD_Studio_Bus_GetChannelGroup(fmod_exterior_processed_bus, &fmod_exterior_processedchannels_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Creating our own channel, trying not to mess with the others
                // Create new FlyWithLua Exterior Processed channel group
                result = FMOD_System_CreateChannelGroup(fmod_system_pointer, "FlyWithLua_Exterior_Processed_Channel", &flywithlua_exterior_processed_channel_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                // Add the FlyWithLua channel gropup to the existing interior channel group
                result = FMOD_ChannelGroup_AddGroup(fmod_exterior_processedchannels_group, flywithlua_exterior_processed_channel_group, true, nullptr);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                if(flywithlua_exterior_processed_channel_group != nullptr)
                {
                    XPLMDebugString("FlyWithLua Info: fmod_exterior_processedchannels_group has been created so OK to use\n");
                }
                */


                /*
                // Exterior Unprocessed Channel Group
                // Accessing a studio bus, using the GUID provided in X-Plane doc
                // https://developer.x-plane.com/article/using-fmod-with-x-plane/
                // FMOD GUID described here: http://www.bom.fmod.com/docs/2.02/api/core-api-common.html#fmod_guid
                // Exterior Unprocessed bus GUID: 8dbbc9d7-a954–4ed0-b1f1-f8111b03cbd1
                FMOD_STUDIO_BUS* fmod_exterior_unprocessed_bus = nullptr;
                FMOD_GUID fmod_exterior_unprocessed_bus_GUID;
                // First three fields of the GUID provided in X-Plane documentation
                fmod_exterior_unprocessed_bus_GUID.Data1 = 0x8dbbc9d7;
                fmod_exterior_unprocessed_bus_GUID.Data2 = 0xa954;
                fmod_exterior_unprocessed_bus_GUID.Data3 = 0x4ed0;
                // Merged last two fields of the GUID provided in X-Plane documentation
                char str_exterior_unprocessed[] = "b1f1f8111b03cbd1";
                unsigned i_exterior_unprocessed = 0;
                unsigned char c_exterior_unprocessed;
                while(i_exterior_unprocessed < 8 && 1 == sscanf(&str_exterior_unprocessed[2*i_exterior_unprocessed], "%02hhx", &c_exterior_unprocessed))
                {
                    fmod_exterior_unprocessed_bus_GUID.Data4[i_exterior_unprocessed] = c_exterior_unprocessed;
                    i_exterior_unprocessed ++;
                }

                result = FMOD_Studio_System_GetBusByID(fmod_studio_system_pointer, &fmod_exterior_unprocessed_bus_GUID, &fmod_exterior_unprocessed_bus);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Bridge with the Core API: get the channel group
                FMOD_CHANNELGROUP* fmod_exterior_unprocessedchannels_group = nullptr;
                result = FMOD_Studio_Bus_GetChannelGroup(fmod_exterior_unprocessed_bus, &fmod_exterior_unprocessedchannels_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Creating our own channel, trying not to mess with the others
                // Create new FlyWithLua Exterior Unprocessed channel group
                result = FMOD_System_CreateChannelGroup(fmod_system_pointer, "FlyWithLua_Exterior_Unprocessed_Channel", &flywithlua_exterior_unprocessed_channel_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                // Add the FlyWithLua channel gropup to the existing interior channel group
                result = FMOD_ChannelGroup_AddGroup(fmod_exterior_unprocessedchannels_group, flywithlua_exterior_unprocessed_channel_group, true, nullptr);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                if(flywithlua_exterior_unprocessed_channel_group != nullptr)
                {
                    XPLMDebugString("FlyWithLua Info: fmod_exterior_unprocessedchannels_group has been created so OK to use\n");
                }
                */


                /*
                // Exterior Processed Aircraft Channel Group
                // Accessing a studio bus, using the GUID provided in X-Plane doc
                // https://developer.x-plane.com/article/using-fmod-with-x-plane/
                // FMOD GUID described here: http://www.bom.fmod.com/docs/2.02/api/core-api-common.html#fmod_guid
                // Exterior Processed Aircraft bus GUID: 06db5351–0b2f–4648-b97a-a33b40a00e11
                FMOD_STUDIO_BUS* fmod_exterior_processed_aircraft_bus = nullptr;
                FMOD_GUID fmod_exterior_processed_aircraft_bus_GUID;
                // First three fields of the GUID provided in X-Plane documentation
                fmod_exterior_processed_aircraft_bus_GUID.Data1 = 0x06db5351;
                fmod_exterior_processed_aircraft_bus_GUID.Data2 = 0x0b2f;
                fmod_exterior_processed_aircraft_bus_GUID.Data3 = 0x4648;
                // Merged last two fields of the GUID provided in X-Plane documentation
                char str_exterior_processed_aircraft[] = "b97aa33b40a00e11";
                unsigned i_exterior_processed_aircraft = 0;
                unsigned char c_exterior_processed_aircraft;
                while(i_exterior_processed_aircraft < 8 && 1 == sscanf(&str_exterior_processed_aircraft[2*i_exterior_processed_aircraft], "%02hhx", &c_exterior_processed_aircraft))
                {
                    fmod_exterior_processed_aircraft_bus_GUID.Data4[i_exterior_processed_aircraft] = c_exterior_processed_aircraft;
                    i_exterior_processed_aircraft ++;
                }

                result = FMOD_Studio_System_GetBusByID(fmod_studio_system_pointer, &fmod_exterior_processed_aircraft_bus_GUID, &fmod_exterior_processed_aircraft_bus);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Bridge with the Core API: get the channel group
                FMOD_CHANNELGROUP* fmod_exterior_processed_aircraftchannels_group = nullptr;
                result = FMOD_Studio_Bus_GetChannelGroup(fmod_exterior_processed_aircraft_bus, &fmod_exterior_processed_aircraftchannels_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Creating our own channel, trying not to mess with the others
                // Create new FlyWithLua Exterior Processed Aircraft channel group
                result = FMOD_System_CreateChannelGroup(fmod_system_pointer, "FlyWithLua_Exterior_Processed_Aircraft_Channel", &flywithlua_exterior_processed_aircraft_channel_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                // Add the FlyWithLua channel gropup to the existing exterior processed aircraft channel group
                result = FMOD_ChannelGroup_AddGroup(fmod_exterior_processed_aircraftchannels_group, flywithlua_exterior_processed_aircraft_channel_group, true, nullptr);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                if(flywithlua_exterior_processed_aircraft_channel_group != nullptr)
                {
                    XPLMDebugString("FlyWithLua Info: fmod_exterior_processed_aircraftchannels_group has been created so OK to use\n");
                }
                */


                /*
                // Exterior Processed Environment Channel Group
                // Accessing a studio bus, using the GUID provided in X-Plane doc
                // https://developer.x-plane.com/article/using-fmod-with-x-plane/
                // FMOD GUID described here: http://www.bom.fmod.com/docs/2.02/api/core-api-common.html#fmod_guid
                // Exterior Processed Environment bus GUID: b6c156f1–9861–405c-a622–92b2ab8010a7
                FMOD_STUDIO_BUS* fmod_exterior_processed_environment_bus = nullptr;
                FMOD_GUID fmod_exterior_processed_environment_bus_GUID;
                // First three fields of the GUID provided in X-Plane documentation
                fmod_exterior_processed_environment_bus_GUID.Data1 = 0xb6c156f1;
                fmod_exterior_processed_environment_bus_GUID.Data2 = 0x9861;
                fmod_exterior_processed_environment_bus_GUID.Data3 = 0x405c;
                // Merged last two fields of the GUID provided in X-Plane documentation
                char str_exterior_processed_environment[] = "a62292b2ab8010a7";
                unsigned i_exterior_processed_environment = 0;
                unsigned char c_exterior_processed_environment;
                while(i_exterior_processed_environment < 8 && 1 == sscanf(&str_exterior_processed_environment[2*i_exterior_processed_environment], "%02hhx", &c_exterior_processed_environment))
                {
                    fmod_exterior_processed_environment_bus_GUID.Data4[i_exterior_processed_environment] = c_exterior_processed_environment;
                    i_exterior_processed_environment ++;
                }

                result = FMOD_Studio_System_GetBusByID(fmod_studio_system_pointer, &fmod_exterior_processed_environment_bus_GUID, &fmod_exterior_processed_environment_bus);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Bridge with the Core API: get the channel group
                FMOD_CHANNELGROUP* fmod_exterior_processed_environmentchannels_group = nullptr;
                result = FMOD_Studio_Bus_GetChannelGroup(fmod_exterior_processed_environment_bus, &fmod_exterior_processed_environmentchannels_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Creating our own channel, trying not to mess with the others
                // Create new FlyWithLua Exterior Processed Environment channel group
                result = FMOD_System_CreateChannelGroup(fmod_system_pointer, "FlyWithLua_Exterior_Processed_Enviroment_Channel", &flywithlua_exterior_processed_environment_channel_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                // Add the FlyWithLua channel gropup to the existing exterior processed enviroment channel group
                result = FMOD_ChannelGroup_AddGroup(fmod_exterior_processed_environmentchannels_group, flywithlua_exterior_processed_environment_channel_group, true, nullptr);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                if(flywithlua_exterior_processed_environment_channel_group != nullptr)
                {
                    XPLMDebugString("FlyWithLua Info: fmod_exterior_processed_environmentchannels_group has been created so OK to use\n");
                }
                */


                /*
                // Copilot Channel Group
                // Can't get this to work and is giving the following error.
                // FmodIntegration.cpp, line 819: The specified resource is not loaded, so it can't be unloaded.
                // Bug report has been filed as XPD-12880
                // Accessing a studio bus, using the GUID provided in X-Plane doc
                // https://developer.x-plane.com/article/using-fmod-with-x-plane/
                // FMOD GUID described here: http://www.bom.fmod.com/docs/2.02/api/core-api-common.html#fmod_guid
                // Copilot bus GUID: e3492bec-f854–4fbb-b2bc–23586269032e
                FMOD_STUDIO_BUS* fmod_copilot_bus = nullptr;
                FMOD_GUID fmod_copilot_bus_GUID;
                // First three fields of the GUID provided in X-Plane documentation
                fmod_copilot_bus_GUID.Data1 = 0xe3492bec;
                fmod_copilot_bus_GUID.Data2 = 0xf854;
                fmod_copilot_bus_GUID.Data3 = 0x4fbb;
                // Merged last two fields of the GUID provided in X-Plane documentation
                char str_copilot[] = "b2bc23586269032e";
                unsigned i_copilot = 0;
                unsigned char c_copilot;
                while(i_copilot < 8 && 1 == sscanf(&str_copilot[2*i_copilot], "%02hhx", &c_copilot))
                {
                    fmod_copilot_bus_GUID.Data4[i_copilot] = c_copilot;
                    i_copilot ++;
                }

                result = FMOD_Studio_System_GetBusByID(fmod_studio_system_pointer, &fmod_copilot_bus_GUID, &fmod_copilot_bus);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Bridge with the Core API: get the channel group
                FMOD_CHANNELGROUP* fmod_copilotchannels_group = nullptr;
                result = FMOD_Studio_Bus_GetChannelGroup(fmod_copilot_bus, &fmod_copilotchannels_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Creating our own channel, trying not to mess with the others
                // Create new FlyWithLua Copilot channel group
                result = FMOD_System_CreateChannelGroup(fmod_system_pointer, "FlyWithLua_Copilot_Channel", &flywithlua_copilot_channel_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                // Add the FlyWithLua channel gropup to the existing copilot channel group
                result = FMOD_ChannelGroup_AddGroup(fmod_copilotchannels_group, flywithlua_copilot_channel_group, true, nullptr);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                if(flywithlua_copilot_channel_group != nullptr)
                {
                    XPLMDebugString("FlyWithLua Info: fmod_copilotchannels_group has been created so OK to use\n");
                }
                */


                /*
                // Ui Channel Group
                // Can't get this to work and is giving the following error.
                // FmodIntegration.cpp, line 819: The specified resource is not loaded, so it can't be unloaded.
                // Bug report has been filed as XPD-12880
                // Accessing a studio bus, using the GUID provided in X-Plane doc
                // https://developer.x-plane.com/article/using-fmod-with-x-plane/
                // FMOD GUID described here: http://www.bom.fmod.com/docs/2.02/api/core-api-common.html#fmod_guid
                // Ui bus GUID: 77575f3b-b9cd–4bff–9056–637cff744a14
                FMOD_STUDIO_BUS* fmod_ui_bus = nullptr;
                FMOD_GUID fmod_ui_bus_GUID;
                // First three fields of the GUID provided in X-Plane documentation
                fmod_ui_bus_GUID.Data1 = 0x77575f3b;
                fmod_ui_bus_GUID.Data2 = 0xb9cd;
                fmod_ui_bus_GUID.Data3 = 0x4bff;
                // Merged last two fields of the GUID provided in X-Plane documentation
                char str_ui[] = "9056637cff744a14";
                unsigned i_ui = 0;
                unsigned char c_ui;
                while(i_ui < 8 && 1 == sscanf(&str_ui[2*i_ui], "%02hhx", &c_ui))
                {
                    fmod_ui_bus_GUID.Data4[i_ui] = c_ui;
                    i_ui ++;
                }

                result = FMOD_Studio_System_GetBusByID(fmod_studio_system_pointer, &fmod_ui_bus_GUID, &fmod_ui_bus);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Bridge with the Core API: get the channel group
                FMOD_CHANNELGROUP* fmod_uichannels_group = nullptr;
                result = FMOD_Studio_Bus_GetChannelGroup(fmod_ui_bus, &fmod_uichannels_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                // Creating our own channel, trying not to mess with the others
                // Create new FlyWithLua Ui channel group
                result = FMOD_System_CreateChannelGroup(fmod_system_pointer, "FlyWithLua_Ui_Channel", &flywithlua_ui_channel_group);
                fmod_error_handler(__FILE__, __LINE__-1, result);
                // Add the FlyWithLua channel gropup to the existing ui channel group
                result = FMOD_ChannelGroup_AddGroup(fmod_uichannels_group, flywithlua_ui_channel_group, true, nullptr);
                fmod_error_handler(__FILE__, __LINE__-1, result);

                if(flywithlua_ui_channel_group != nullptr)
                {
                    XPLMDebugString("FlyWithLua Info: fmod_uichannels_group has been created so OK to use\n");
                }
                */

            }
            else
            {
                // We now play the sound on demand so this is not needed
            }
        }
    }
    catch (exception const& e)
    {
        string fmod_error_msg = "FlyWithLua Error: " + string(e.what()) + "\n";
        XPLMDebugString(fmod_error_msg.c_str());
        return 0; // The plugin is failing, so do not execute any further loop
    }

    return 0;
}


int fmod_data_update()
{
    if(init)
    {
        master_channel_group_volume = 1.0;
        mute_master_channel_group = false;
        radios_channel_group_volume = 1.0;
        mute_radios_channel_group = false;
        interior_channel_group_volume = 1.0;
        mute_interior_channel_group = false;
    }

    if(flywithlua_master_channel_group != nullptr)
    {
        mute_master_channel_group = flywithlua_master_channel_group_mute.get();
        master_channel_group_volume = flywithlua_master_channel_group_volume.get();
        setMasterMute(mute_master_channel_group);
        setMasterVolume(master_channel_group_volume);
    }

    if(flywithlua_radios_channel_group != nullptr)
    {
        mute_radios_channel_group = flywithlua_radios_channel_group_mute.get();
        radios_channel_group_volume = flywithlua_radios_channel_group_volume.get();
        setRadiosMute(mute_radios_channel_group);
        setRadioVolume(radios_channel_group_volume);
    }

    if(flywithlua_interior_channel_group != nullptr)
    {
        mute_interior_channel_group = flywithlua_interior_channel_group_mute.get();
        interior_channel_group_volume = flywithlua_interior_channel_group_volume.get();
        setInteriorMute(mute_interior_channel_group);
        setInteriorVolume(interior_channel_group_volume);
    }

    return 0;
}


void deinitFmodSupport() {
    FmodSounds.clear();
    fmod_sound_index = 0;
}

} // end namespace fmodint
