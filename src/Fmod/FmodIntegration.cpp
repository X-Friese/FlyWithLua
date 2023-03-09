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

#include "XPLMSound.h"

#include "fwl_dataref.h"
#include "fwl_customdataref.h"

// fmod4xp
#include "fmod4xpchannelgroup.h"
#include "fmod4xpsystem.h"


// DataRefs --------------------------------------------------------------------
// DataRef<int> fmod_system_hi_dataref("sim/private/fmod/system_hi");
// DataRef<int> fmod_system_lo_dataref("sim/private/fmod/system_lo");
// DataRef<int> fmod_studio_hi_dataref("sim/private/fmod/studio_hi");
// DataRef<int> fmod_studio_lo_dataref("sim/private/fmod/studio_lo");


CustomDataRef<int> flywithlua_com1_channel_group_mute("FlyWithLua_Com1ChannelGroup/Mute", 1);
CustomDataRef<float> flywithlua_com1_channel_group_volume("FlyWithLua_Com1ChannelGroup/Volume", 1);
CustomDataRef<int> flywithlua_interior_channel_group_mute("FlyWithLua_InteriorChannelGroup/Mute", 1);
CustomDataRef<float> flywithlua_interior_channel_group_volume("FlyWithLua_InteriorChannelGroup/Volume", 1);
CustomDataRef<int> flywithlua_ui_channel_group_mute("FlyWithLua_UIChannelGroup/Mute", 1);
CustomDataRef<float> flywithlua_ui_channel_group_volume("FlyWithLua_UIChannelGroup/Volume", 1);
CustomDataRef<int> flywithlua_master_channel_group_mute("FlyWithLua_MasterChannelGroup/Mute", 1);
CustomDataRef<float> flywithlua_master_channel_group_volume("FlyWithLua_MasterChannelGroup/Volume", 1);


// FMOD Core --------------------------------------------------------------------
FMOD_SYSTEM *fmod_system_pointer = nullptr;
FMOD_SYSTEM* fmod_system_sdk = nullptr;

FMOD_SOUND *fmod_sound_pointer[400];
FMOD_CHANNELGROUP *flywithlua_com1_channel_group = nullptr;
FMOD_CHANNELGROUP *flywithlua_interior_channel_group = nullptr;
FMOD_CHANNELGROUP *flywithlua_ui_channel_group = nullptr;
FMOD_CHANNELGROUP *flywithlua_master_channel_group = nullptr;

FMOD_CHANNELGROUP *cg_sdk_audio_radio_com1 = nullptr;
FMOD_CHANNELGROUP *cg_sdk_audio_interior = nullptr;
FMOD_CHANNELGROUP *cg_sdk_audio_ui = nullptr;
FMOD_CHANNELGROUP *cg_sdk_master = nullptr;

// FMOD Studio ------------------------------------------------------------------
FMOD_STUDIO_SYSTEM *fmod_studio_system_pointer = nullptr;
FMOD_STUDIO_SYSTEM *fmod_studio_sdk = nullptr;

void FMODErrorHandler(const string &file, int line, FMOD_RESULT result)
{
    if(result != FMOD_OK)
    {
        string errorMsg = "Error in " + file + ", line " + to_string(line) + ": " + FMOD_ErrorString(result);
        throw runtime_error(errorMsg.c_str());
    }
}

int fmod_index_num = 0;
bool init = true;

namespace fmodint {

struct FmodSoundsStructure
{
    std::string filename;
    int         index{};
};

static std::vector<FmodSoundsStructure> FmodSounds;

int fmod_sound_index = 0;

bool mute_com1_channel_group;
float com1_channel_group_volume;
bool mute_interior_channel_group;
float interior_channel_group_volume;
bool mute_ui_channel_group;
float ui_channel_group_volume;
bool mute_master_channel_group;
float master_channel_group_volume;


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
    FmodSounds.back().index    = fmod_sound_index;

    // fill the debug table with information
    // This is how OpenAL works but not working for Fmod
    // FmodSoundsStructure* fmod_sound = &FmodSounds.back();

    // fmod_sound->filename = FmodFileNameToLoad;
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

        FMOD_RESULT fwl_result;

        // Get the channel DSP clock, which serves as a reference
        fwl_result = FMOD_Channel_GetDSPClock(fmod_playing_channel, 0, &fmod_start_delay);
        fmod_error_handler(__FILE__, __LINE__-1, fwl_result);

        // Grab the length of the playing sound, and its frequency, so we can caluate where to place the new sound on the time line.
        fwl_result = FMOD_Channel_GetCurrentSound(fmod_playing_channel, &fmod_playing_sound);
        fmod_error_handler(__FILE__, __LINE__-1, fwl_result);
        fwl_result = FMOD_Sound_GetLength(fmod_playing_sound, &fmod_sound_length, FMOD_TIMEUNIT_PCM);
        fmod_error_handler(__FILE__, __LINE__-1, fwl_result);
        fwl_result = FMOD_Channel_GetFrequency(fmod_playing_channel, &fmod_sound_frequency);
        fmod_error_handler(__FILE__, __LINE__-1, fwl_result);

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


static int LuaPlaySoundOnCom1Bus(lua_State* L)
{
    string fmod_sound_path;

    if (!lua_isnumber(L, 1))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnCom1Bus() Missing Play Fmod Sound index number. You will have to give an integer..");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    fmod_index_num = static_cast<int>(lua_tointeger(L, 1));
    if ((fmod_index_num < 0) || (fmod_index_num >= int(FmodSounds.size())))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnCom1Bus() Play Fmod Sound index number out of range.");
    }

    // We need to do this to get the index's lined up and have not found a better way
    fmod_index_num = fmod_index_num + 1;

    if (!FmodSounds.empty())
    {
        for (const FmodSoundsStructure & fmod_sound : FmodSounds)
        {
            if (fmod_sound.index == fmod_index_num)
            {
                fmod_sound_path = fmod_sound.filename;

                if (flywithlua::verbose_logging_mode == 1) {
                    char buf2[200];
                    sprintf(buf2, "FlyWithLua Info: In LuaPlayFmodSoundOnCom1Bus() fmod_index_num  =  %d    fwl_sound3Path  =  %s  \n", fmod_index_num, fmod_sound_path.c_str());
                    XPLMDebugString(buf2);
                }
            }
        }
    } else
    {
        XPLMDebugString("FlyWithLua Error: No Fmod sounds created by Lua.\n");
    }

    try
    {
        if(flywithlua_com1_channel_group != nullptr)
        {
            FMOD_RESULT fwl_result;
            // We are using fwl_soundFMod[Fmod_IndexNo] so we know which sound file to play
            // Still working on if the is the best approach

            fwl_result = FMOD_System_CreateSound(fmod_system_sdk, fmod_sound_path.c_str(), FMOD_DEFAULT, nullptr, &fmod_sound_pointer[fmod_index_num]);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);

            // The channel on which the sound will be played
            // We are passing nullptr because this feature is not being used.
            FMOD_CHANNEL* flywithlua_channel_pointer = nullptr;
            // Actually play the sound on the FlyWithLua Channel Group
            fwl_result = FMOD_System_PlaySound(fmod_system_sdk, fmod_sound_pointer[fmod_index_num], flywithlua_com1_channel_group, false, &flywithlua_channel_pointer);
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


static int LuaStopSoundOnCom1Bus(lua_State* L)
{
    try
    {
        FMOD_RESULT fwl_result;

        if(flywithlua_com1_channel_group != nullptr)
        {
            // Stop the sound for the FlyWithLua radio channel group
            fwl_result = FMOD_ChannelGroup_Stop(flywithlua_com1_channel_group);
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


void setCom1Volume(float com1_channel_group_volume)
{
    // Set the volume for the FlyWithLua channel group
    FMOD_RESULT fwl_result = FMOD_ChannelGroup_SetVolume(flywithlua_com1_channel_group, com1_channel_group_volume);
    if(FMOD_OK != fwl_result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetVolume: " + FMOD_ErrorString(fwl_result);
        throw runtime_error(errorMsg.c_str());
    }
}


void setCom1Mute(bool mute_com1_channel_group)
{
    // Mute the sound for the FlyWithLua channel group
    FMOD_RESULT fwl_result = FMOD_ChannelGroup_SetMute(flywithlua_com1_channel_group, mute_com1_channel_group);
    if(FMOD_OK != fwl_result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetMute: " + FMOD_ErrorString(fwl_result);
        throw runtime_error(errorMsg.c_str());
    }
}


static int LuaPlaySoundOnInteriorBus(lua_State* L)
{
    string fmod_sound_path;

    if (!lua_isnumber(L, 1))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnInteriorBus() Missing Play Fmod Sound index number. You will have to give an integer..");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    fmod_index_num = static_cast<int>(lua_tointeger(L, 1));
    if ((fmod_index_num < 0) || (fmod_index_num >= int(FmodSounds.size())))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnInteriorBus() Play Fmod Sound index number out of range.");
    }

    // We need to do this to get the index's lined up and have not found a better way
    fmod_index_num = fmod_index_num + 1;

    if (!FmodSounds.empty())
    {
        for (const FmodSoundsStructure & fmod_sound : FmodSounds)
        {
            if (fmod_sound.index == fmod_index_num)
            {
                fmod_sound_path = fmod_sound.filename;

                if (flywithlua::verbose_logging_mode == 1) {
                    char buf2[200];
                    sprintf(buf2, "FlyWithLua Info: In LuaPlayFmodSoundOnInteriorBus() fmod_index_num  =  %d    fwl_sound3Path  =  %s  \n", fmod_index_num, fmod_sound_path.c_str());
                    XPLMDebugString(buf2);
                }
            }
        }
    } else
    {
        XPLMDebugString("FlyWithLua Error: No Fmod sounds created by Lua.\n");
    }

    try
    {
        if(flywithlua_interior_channel_group != nullptr)
        {
            FMOD_RESULT fwl_result;
            // We are using fwl_soundFMod[Fmod_IndexNo] so we know which sound file to play
            // Still working on if the is the best approach

            fwl_result = FMOD_System_CreateSound(fmod_system_sdk, fmod_sound_path.c_str(), FMOD_DEFAULT, nullptr, &fmod_sound_pointer[fmod_index_num]);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);

            // The channel on which the sound will be played
            // We are passing nullptr because this feature is not being used.
            FMOD_CHANNEL* flywithlua_channel_pointer = nullptr;
            // Actually play the sound on the FlyWithLua Channel Group
            fwl_result = FMOD_System_PlaySound(fmod_system_sdk, fmod_sound_pointer[fmod_index_num], flywithlua_interior_channel_group, false, &flywithlua_channel_pointer);
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


static int LuaStopSoundOnInteriorBus(lua_State* L)
{
    try
    {
        FMOD_RESULT fwl_result;

        if(flywithlua_interior_channel_group != nullptr)
        {
            // Stop the sound for the FlyWithLua interior channel group
            fwl_result = FMOD_ChannelGroup_Stop(flywithlua_interior_channel_group);
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
    FMOD_RESULT fwl_result = FMOD_ChannelGroup_SetVolume(flywithlua_interior_channel_group, interior_channel_group_volume);
    if(FMOD_OK != fwl_result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetVolume: " + FMOD_ErrorString(fwl_result);
        throw runtime_error(errorMsg.c_str());
    }
}


void setInteriorMute(bool mute_interior_channel_group)
{
    // Mute the sound for the FlyWithLua channel group
    FMOD_RESULT fwl_result = FMOD_ChannelGroup_SetMute(flywithlua_interior_channel_group, mute_interior_channel_group);
    if(FMOD_OK != fwl_result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetMute: " + FMOD_ErrorString(fwl_result);
        throw runtime_error(errorMsg.c_str());
    }
}



static int LuaPlaySoundOnUiBus(lua_State* L)
{
    string fmod_sound_path;

    if (!lua_isnumber(L, 1))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnUiBus() Missing Play Fmod Sound index number. You will have to give an integer..");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    fmod_index_num = static_cast<int>(lua_tointeger(L, 1));
    if ((fmod_index_num < 0) || (fmod_index_num >= int(FmodSounds.size())))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnUiBus() Play Fmod Sound index number out of range.");
    }

    // We need to do this to get the index's lined up and have not found a better way
    fmod_index_num = fmod_index_num + 1;

    if (!FmodSounds.empty())
    {
        for (const FmodSoundsStructure & fmod_sound : FmodSounds)
        {
            if (fmod_sound.index == fmod_index_num)
            {
                fmod_sound_path = fmod_sound.filename;

                if (flywithlua::verbose_logging_mode == 1) {
                    char buf2[200];
                    sprintf(buf2, "FlyWithLua Info: In LuaPlayFmodSoundOnUiBus() fmod_index_num  =  %d    fwl_sound3Path  =  %s  \n", fmod_index_num, fmod_sound_path.c_str());
                    XPLMDebugString(buf2);
                }
            }
        }
    } else
    {
        XPLMDebugString("FlyWithLua Error: No Fmod sounds created by Lua.\n");
    }

    try
    {
        if(flywithlua_ui_channel_group != nullptr)
        {
            FMOD_RESULT fwl_result;
            // We are using fwl_soundFMod[Fmod_IndexNo] so we know which sound file to play
            // Still working on if the is the best approach

            fwl_result = FMOD_System_CreateSound(fmod_system_sdk, fmod_sound_path.c_str(), FMOD_DEFAULT, nullptr, &fmod_sound_pointer[fmod_index_num]);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);

            // The channel on which the sound will be played
            // We are passing nullptr because this feature is not being used.
            FMOD_CHANNEL* flywithlua_channel_pointer = nullptr;
            // Actually play the sound on the FlyWithLua Channel Group
            fwl_result = FMOD_System_PlaySound(fmod_system_sdk, fmod_sound_pointer[fmod_index_num], flywithlua_ui_channel_group, false, &flywithlua_channel_pointer);
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


static int LuaStopSoundOnUiBus(lua_State* L)
{
    try
    {
        FMOD_RESULT fwl_result;

        if(flywithlua_ui_channel_group != nullptr)
        {
            // Stop the sound for the FlyWithLua radio channel group
            fwl_result = FMOD_ChannelGroup_Stop(flywithlua_ui_channel_group);
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


void setUiVolume(float ui_channel_group_volume)
{
    // Set the volume for the FlyWithLua channel group
    FMOD_RESULT fwl_result = FMOD_ChannelGroup_SetVolume(flywithlua_ui_channel_group, ui_channel_group_volume);
    if(FMOD_OK != fwl_result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetVolume: " + FMOD_ErrorString(fwl_result);
        throw runtime_error(errorMsg.c_str());
    }
}


void setUiMute(bool mute_ui_channel_group)
{
    // Mute the sound for the FlyWithLua channel group
    FMOD_RESULT fwl_result = FMOD_ChannelGroup_SetMute(flywithlua_ui_channel_group, mute_ui_channel_group);
    if(FMOD_OK != fwl_result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetMute: " + FMOD_ErrorString(fwl_result);
        throw runtime_error(errorMsg.c_str());
    }
}



static int LuaPlaySoundOnMasterBus(lua_State* L)
{
    string fmod_sound_path;

    if (!lua_isnumber(L, 1))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnMasterBus() Missing Play Fmod Sound index number. You will have to give an integer..");
        flywithlua::LuaIsRunning = false;
        return 0;
    }

    fmod_index_num = static_cast<int>(lua_tointeger(L, 1));
    if ((fmod_index_num < 0) || (fmod_index_num >= int(FmodSounds.size())))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: LuaPlayFmodSoundOnMasterBus() Play Fmod Sound index number out of range.");
    }

    // We need to do this to get the index's lined up and have not found a better way
    fmod_index_num = fmod_index_num + 1;

    if (!FmodSounds.empty())
    {
        for (const FmodSoundsStructure & fmod_sound : FmodSounds)
        {
            if (fmod_sound.index == fmod_index_num)
            {
                fmod_sound_path = fmod_sound.filename;

                if (flywithlua::verbose_logging_mode == 1) {
                    char buf2[200];
                    sprintf(buf2, "FlyWithLua Info: In LuaPlayFmodSoundOnMasterBus() fmod_index_num  =  %d    fwl_sound3Path  =  %s  \n", fmod_index_num, fmod_sound_path.c_str());
                    XPLMDebugString(buf2);
                }
            }
        }
    } else
    {
        XPLMDebugString("FlyWithLua Error: No Fmod sounds created by Lua.\n");
    }

    try
    {
        if(flywithlua_master_channel_group != nullptr)
        {
            FMOD_RESULT fwl_result;
            // We are using fwl_soundFMod[Fmod_IndexNo] so we know which sound file to play
            // Still working on if the is the best approach

            fwl_result = FMOD_System_CreateSound(fmod_system_sdk, fmod_sound_path.c_str(), FMOD_DEFAULT, nullptr, &fmod_sound_pointer[fmod_index_num]);
            fmod_error_handler(__FILE__, __LINE__-1, fwl_result);

            // The channel on which the sound will be played
            // We are passing nullptr because this feature is not being used.
            FMOD_CHANNEL* flywithlua_channel_pointer = nullptr;
            // Actually play the sound on the FlyWithLua Channel Group
            fwl_result = FMOD_System_PlaySound(fmod_system_sdk, fmod_sound_pointer[fmod_index_num], flywithlua_master_channel_group, false, &flywithlua_channel_pointer);
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


static int LuaStopSoundOnMasterBus(lua_State* L)
{
    try
    {
        FMOD_RESULT fwl_result;

        if(flywithlua_master_channel_group != nullptr)
        {
            // Stop the sound for the FlyWithLua radio channel group
            fwl_result = FMOD_ChannelGroup_Stop(flywithlua_master_channel_group);
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
    FMOD_RESULT fwl_result = FMOD_ChannelGroup_SetVolume(flywithlua_master_channel_group, master_channel_group_volume);
    if(FMOD_OK != fwl_result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetVolume: " + FMOD_ErrorString(fwl_result);
        throw runtime_error(errorMsg.c_str());
    }
}


void setMasterMute(bool mute_master_channel_group)
{
    // Mute the sound for the FlyWithLua channel group
    FMOD_RESULT fwl_result = FMOD_ChannelGroup_SetMute(flywithlua_master_channel_group, mute_master_channel_group);
    if(FMOD_OK != fwl_result)
    {
        string errorMsg = "Error in " + string(__FUNCTION__) + ", calling FMOD_ChannelGroup_SetMute: " + FMOD_ErrorString(fwl_result);
        throw runtime_error(errorMsg.c_str());
    }
}


void RegisterFmodFunctionsToLua(lua_State* L)
{
    lua_register(L, "load_fmod_sound", LuaLoadFmodSound);

    lua_register(L, "play_sound_on_com1_bus", LuaPlaySoundOnCom1Bus);
    lua_register(L, "stop_sound_on_com1_bus", LuaStopSoundOnCom1Bus);
    lua_register(L, "play_sound_on_interior_bus", LuaPlaySoundOnInteriorBus);
    lua_register(L, "stop_sound_on_interior_bus", LuaStopSoundOnInteriorBus);
    lua_register(L, "play_sound_on_ui_bus", LuaPlaySoundOnUiBus);
    lua_register(L, "stop_sound_on_ui_bus", LuaStopSoundOnUiBus);
    lua_register(L, "play_sound_on_master_bus", LuaPlaySoundOnMasterBus);
    lua_register(L, "stop_sound_on_master_bus", LuaStopSoundOnMasterBus);
}


void RegisterAccessor()
{
    flywithlua_com1_channel_group_mute.registerAccessor();
    flywithlua_com1_channel_group_volume.registerAccessor();
    flywithlua_interior_channel_group_mute.registerAccessor();
    flywithlua_interior_channel_group_volume.registerAccessor();
    flywithlua_ui_channel_group_mute.registerAccessor();
    flywithlua_ui_channel_group_volume.registerAccessor();
    flywithlua_master_channel_group_mute.registerAccessor();
    flywithlua_master_channel_group_volume.registerAccessor();
}


int fmod_initialization()
{
    // Use the X-Plane 12 SDK sound API to get a pointer to the FMOD studio system
    fmod_studio_sdk = XPLMGetFMODStudio();
    if(nullptr == fmod_studio_sdk)
    {
        XPLMDebugString("FlyWithLua Info: No pointer from SDK for Studio system\n");
        return 0;
    }

    try {
        FMOD_RESULT fwl_result;

        // Get a pointer to the FMOD core system
        fwl_result = FMOD_Studio_System_GetCoreSystem(XPLMGetFMODStudio(), &fmod_system_sdk);
        FMODErrorHandler(__FILE__, __LINE__-1, fwl_result);


        // Create a custom flywithlua com1 channel
        fwl_result = FMOD_System_CreateChannelGroup(fmod_system_sdk, "FlyWithLua_Com1_Channel", &flywithlua_com1_channel_group);
        FMODErrorHandler(__FILE__, __LINE__-1, fwl_result);

        cg_sdk_audio_radio_com1 = XPLMGetFMODChannelGroup(xplm_AudioRadioCom1);
        fwl_result = FMOD_ChannelGroup_AddGroup(cg_sdk_audio_radio_com1, flywithlua_com1_channel_group, true, nullptr);
        FMODErrorHandler(__FILE__, __LINE__-1, fwl_result);


        // Create a custom flywithlua interior channel
        fwl_result = FMOD_System_CreateChannelGroup(fmod_system_sdk, "FlyWithLua_Interior_Channel", &flywithlua_interior_channel_group);
        FMODErrorHandler(__FILE__, __LINE__-1, fwl_result);

        cg_sdk_audio_interior = XPLMGetFMODChannelGroup(xplm_AudioInterior);
        fwl_result = FMOD_ChannelGroup_AddGroup(cg_sdk_audio_interior, flywithlua_interior_channel_group, true, nullptr);
        FMODErrorHandler(__FILE__, __LINE__-1, fwl_result);


        // Create a custom flywithlua ui channel
        fwl_result = FMOD_System_CreateChannelGroup(fmod_system_sdk, "FlyWithLua_Ui_Channel", &flywithlua_ui_channel_group);
        FMODErrorHandler(__FILE__, __LINE__-1, fwl_result);

        cg_sdk_audio_ui = XPLMGetFMODChannelGroup(xplm_AudioUI);
        fwl_result = FMOD_ChannelGroup_AddGroup(cg_sdk_audio_ui, flywithlua_ui_channel_group, true, nullptr);
        FMODErrorHandler(__FILE__, __LINE__-1, fwl_result);


        // Create a custom flywithlua master channel
        fwl_result = FMOD_System_CreateChannelGroup(fmod_system_sdk, "FlyWithLua_Master_Channel", &flywithlua_master_channel_group);
        FMODErrorHandler(__FILE__, __LINE__-1, fwl_result);

        cg_sdk_master = XPLMGetFMODChannelGroup(xplm_Master);
        fwl_result = FMOD_ChannelGroup_AddGroup(cg_sdk_master, flywithlua_master_channel_group, true, nullptr);
        FMODErrorHandler(__FILE__, __LINE__-1, fwl_result);
    }
    catch (exception const& e)
    {
        string fmod_error_msg = "FlyWithLua Error: " + string(e.what()) + "\n";
        XPLMDebugString(fmod_error_msg.c_str());
        return 0; // The plugin is failing, so do not execute any further loop
    }

    return 0;
}

int fmod_uninitialize()
{
    char buf4[200];
    sprintf(buf4, "FlyWithLua Info: fmod_uninitialize() FmodSounds.size =  %d\n", int(FmodSounds.size()));
    XPLMDebugString(buf4);

    if (int(FmodSounds.size()) > 0) {
        sprintf(buf4, "FlyWithLua Info: fmod_uninitialize() if (int(FmodSounds.size()) > 0) FmodSounds.size =  %d\n", int(FmodSounds.size()));
        XPLMDebugString(buf4);
        for (int i = 1; i <= int(FmodSounds.size()); ++i) {
            FMOD_Sound_Release(fmod_sound_pointer[i]);
        }
    }

    fmod_studio_sdk = nullptr;
    fmod_system_sdk = nullptr;
    flywithlua_com1_channel_group = nullptr;
    flywithlua_interior_channel_group = nullptr;
    flywithlua_ui_channel_group = nullptr;
    flywithlua_master_channel_group = nullptr;

    cg_sdk_audio_radio_com1 = nullptr;
    cg_sdk_audio_interior = nullptr;
    cg_sdk_audio_ui = nullptr;
    cg_sdk_master = nullptr;

    FmodSounds.clear();
    fmod_sound_index = 0;

    sprintf(buf4, "FlyWithLua Info: fmod_uninitialize()  Should be 0 now FmodSounds.size =  %d\n", int(FmodSounds.size()));
    XPLMDebugString(buf4);

    return 0;
}


int fmod_data_update()
{
    if(init)
    {
        com1_channel_group_volume = 1.0;
        mute_com1_channel_group = false;
        interior_channel_group_volume = 1.0;
        mute_interior_channel_group = false;
        ui_channel_group_volume = 1.0;
        mute_ui_channel_group = false;
        master_channel_group_volume = 1.0;
        mute_master_channel_group = false;
    }

    if(flywithlua_com1_channel_group != nullptr)
    {
        mute_com1_channel_group = flywithlua_com1_channel_group_mute.get();
        com1_channel_group_volume = flywithlua_com1_channel_group_volume.get();
        setCom1Mute(mute_com1_channel_group);
        setCom1Volume(com1_channel_group_volume);
    }

    if(flywithlua_interior_channel_group != nullptr)
    {
        mute_interior_channel_group = flywithlua_interior_channel_group_mute.get();
        interior_channel_group_volume = flywithlua_interior_channel_group_volume.get();
        setInteriorMute(mute_interior_channel_group);
        setInteriorVolume(interior_channel_group_volume);
    }

    if(flywithlua_ui_channel_group != nullptr)
    {
        mute_ui_channel_group = flywithlua_ui_channel_group_mute.get();
        ui_channel_group_volume = flywithlua_ui_channel_group_volume.get();
        setUiMute(mute_ui_channel_group);
        setUiVolume(ui_channel_group_volume);
    }


    if(flywithlua_master_channel_group != nullptr)
    {
        mute_master_channel_group = flywithlua_master_channel_group_mute.get();
        master_channel_group_volume = flywithlua_master_channel_group_volume.get();
        setMasterMute(mute_master_channel_group);
        setMasterVolume(master_channel_group_volume);
    }

    return 0;
}


void deinitFmodSupport() {
    // Trying to understand why I need two different
    // functions to de or un initalize fmod.
    char buf3[200];
    sprintf(buf3, "FlyWithLua Info:  deinitFmodSupport() FmodSounds.size =  %d\n", int(FmodSounds.size()));
    XPLMDebugString(buf3);
    if (int(FmodSounds.size()) > 0) {
        sprintf(buf3, "FlyWithLua Info: deinitFmodSupport() if (int(FmodSounds.size()) > 0) FmodSounds.size =  %d\n", int(FmodSounds.size()));
        XPLMDebugString(buf3);
        for (int i = 1; i <= int(FmodSounds.size()); ++i) {
            FMOD_Sound_Release(fmod_sound_pointer[i]);
        }
    }

    FmodSounds.clear();
    fmod_sound_index = 0;

    sprintf(buf3, "FlyWithLua Info: deinitFmodSupport()  Should be 0 now FmodSounds.size =  %d\n", int(FmodSounds.size()));
        XPLMDebugString(buf3);
}

} // end namespace fmodint
