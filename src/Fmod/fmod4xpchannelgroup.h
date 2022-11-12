#ifndef FMOD4XPCHANNELGROUP_H
#define FMOD4XPCHANNELGROUP_H

#include "fmod4xpdefaultbus.h"

#include <string>
#include <vector>

#include "fmod.h"

using namespace std;

class FMOD4XPSystem;

class FMOD4XPChannelGroup
{
private:
    FMOD_CHANNELGROUP* fmodChannelGroup;
    FMOD4XPSystem* system;
    unsigned long long timeLastSoundEnds = 0;

    unsigned long long getTimeSoundEnds(FMOD_CHANNEL* channel, FMOD_SOUND* sound);
public:
    FMOD4XPChannelGroup(string name, FMOD4XPSystem* system, FMOD4XPDefaultBus parentDefaultBus);
    FMOD4XPChannelGroup(string name, FMOD4XPSystem* system, FMOD4XPChannelGroup* parent);

    void playSoundNow(FMOD_SOUND* sound);
    void addSoundToQueue(FMOD_SOUND* sound);

    void setVolume(float volume);
    void setPaused(bool paused);
    void setMute(bool mute);
    void stop();

    FMOD_CHANNELGROUP* getFmodChannelGroup() const;
};

#endif // FMOD4XPCHANNELGROUP_H
